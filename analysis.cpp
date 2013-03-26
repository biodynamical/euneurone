// analysis: analysis.cpp
#include <qmutex.h>
#include <qapplication.h>
#include <qsettings.h>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <ext/functional>
#include <gsl/gsl_statistics_double.h>
#include  <gsl/gsl_machine.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_version.h>
#include <gsl/gsl_complex_math.h>

#include "controlobject.h"
#include "analysis.h"

using namespace NS_Control;
using namespace NS_Analysis;
using namespace std;

extern Control *pControl;
#define IDINVALID 0xFFFFFFFF

#define INT_LOCK_TRY 600
#define INT_LOCK_MS 50
#define PAUSE 50

#define BOX 1024
#define NMAX 256
#define MACC		4	//# of interpolation points per 1/4 cycle of highest frequency

const char *achAnaFailIntLock="Failed to lock data memory in Analysis thread";
const char *achAnaFailLock="Failed to lock analysis memory in Analysis thread";
const char *achNoSettings="No settings found for this type of analysis";

extern QString qsApplication;
const char *achAnalysis="Analysis";
const char *achRecur="Recurrence plot";
const char *achMaxLyap="Maximal Lyapunov Exponent";
const char *achPeriod="Periodogram (Lomb)";
const char *achPower="Powerspectrum";
const char *achPoincare="Poincare section";
const char *achInterSI="Interspike estimate";
const char *achSpike="Spike estimate";
const char *achType="Type";
const char* achDelay="Delay";
const char* achDim="Embedding dimension";
const char* achMinLen="MinLength";
const char* achSteps="Steps";
const char* achWin="Window";
const char* achNeigh="Size neighbourhood";
const char* achCut="Cut";
const char* achDir="Direction";
const char* achThres="Threshold";
const char* achUse="Use";

//static int nFactorial[11]={0,1,1,2,6,24,120,710,5040,40320,362880};


/**
 * Generates Bartlett type window
 * @param x : current index in windowq
 * @param y : length of window
 * @return Bartlett value in window (start with 0)
 * @reference : Intel Integrated Performance Reference p 5-100.
 */
struct bartlett : public unary_function<double, double>
{
  unsigned int i,m,n;
  bartlett(unsigned int len) : i(0), n(len) { m=(len-1)/2; }
  void reset(unsigned int len) { n=len; i=0; m=(len-1)/2;}
  result_type operator()(result_type x)
  	{
	 double d;
	 if (i<=m)
	 	d=(2.0*(double)i/(double)(n-1))*x;
	 else
	 	d=(2.0-(2.0*(double)i/(double)(n-1)))*x;
	 i++;
	 return d;
	}
};

/**
 * Generates Blackman type window
 * @param x : current index in windowq
 * @param y : length of window
 * @return Blackman value in window (start with 0)
 * @reference : Intel Integrated Performance Reference p 5-103.
 */
struct blackman : public unary_function<double, double>
{
  unsigned int i,n;
  static const double alpha=-0.16;
  
  blackman(unsigned int len) : i(0), n(len) { }
  void reset(unsigned int len) { n=len; i=0;}
  result_type operator()(double x)
  	{
	 double d;
	 d=(((alpha+1)/2)-(0.5*cos((2.0*M_PI*(double)i/(double)(n-1))))-(alpha/2.0*cos((4.0*M_PI*(double)i/(double)(n-1)))))*x;
	 i++;
	 return d;
	}
};

/**
 * Generates Hamming type window
 * @param x : current index in windowq
 * @param y : length of window
 * @return Hamming value in window (start with 0)
 * @reference : Intel Integrated Performance Reference p 5-105.
 */
struct hamming : public unary_function<double, double>
{
  unsigned int i,n;
  static const double alpha=0.46, beta=0.54;
  
  hamming(unsigned int len) : i(0), n(len) { }
  void reset(unsigned int len) { n=len; i=0;}
  result_type operator()(double x)
  	{
	 double d;
	 d=(beta-(alpha*cos((2.0*M_PI*(double)i/(double)(n-1)))))*x;
	 i++;
	 return d;
	}
};

/**
 * Generates Hann type window
 * @param x : current index in windowq
 * @param y : length of window
 * @return Hann value in window (start with 0)
 * @reference : Intel Integrated Performance Reference p 5-107.
 */
struct hann : public unary_function<double, double>
{
  unsigned int i,n;
  static const double alpha=0.5;
  
  hann(unsigned int len) : i(0), n(len) { }
  void reset(unsigned int len) { n=len; i=0;}
  result_type operator()(double x)
  	{
	 double d;
	 d=(alpha-(alpha*cos((2.0*M_PI*(double)i/(double)(n-1)))))*x;
 	 i++;
	 return d;
	}
};
//====================================================
/**
 * Analysis
 * Main constructor Analysis object
 * @param pi : pointer to integrator mutex
 * @param pa : pointer to analysis mutex
 */
Analysis::Analysis(QMutex *pi,QMutex *pa) : pixThreadOn(QPixmap::fromMimeSource("threadon.png")), pixThreadOff(QPixmap::fromMimeSource("threadoff.png"))
{
 pIntMutex=pi;
 pAnaMutex=pa;
 pLbl=NULL;
 bIntLocked=false;
 bAnaLocked=false;
 bStopped=false;
 bError=false;
  //turn gsl error handling off
  gsl_set_error_handler_off();
}

/**
 * ~Analysis
 * Destructor
 currently unused
 * @return 
 */
Analysis::~Analysis()
{
}

/**
 * RegisterData
 * Allows analysis to make local copy of the variables array
 * @param v : reference to variables array
 */
void  Analysis::RegisterData(NS_Control::TVarDataVector &v)
{
 //local copy of the variables array
 vvdVars=v;
}

/**
 * setLabel
 * Set the pointer to the label object fo on/off pixmap
 * @param pl 
 */
void Analysis::setLabel(QLabel *pl)
{
 pLbl=pl;
}

/**
 * GetErrorStr
 * returns const pointer to error message
 * @return const pointer
 */
const char *Analysis::GetErrorStr()
{
 if (bError)
  return sException.c_str();
 return NULL;
}

/**
 * Reset
 * Remove all calculated results
 */
void Analysis::Reset()
{
 unsigned int i;
 
 bError=false;
 for (i=0;i<avList.size();i++)
	{ 
 	 avList[i].bUpdate=false;
	 avList[i].ddData.clear();
	 avList[i].ddTime.clear();
	 avList[i].ddXResult.clear();
	 avList[i].ddYResult.clear();
	 avList[i].ddResult.clear();
	}
}

/**
 * Start
 * Start thread
 */
void Analysis::Start()
{
 if (!running())
 {
   bStopped=false;
   if (pLbl)
   	    pLbl->setPixmap(pixThreadOn);
   start();
 }
}

/**
 * Stop
 * Stop thread
 */
void Analysis::Stop()
{
 if (running())
 {
  //check if locked
   if(bIntLocked)
   {
    pIntMutex->unlock();
    bIntLocked=false;
   }
   if(bAnaLocked)
   {
    pAnaMutex->unlock();
    bAnaLocked=false;
   }
  bStopped=true;
   if (pLbl)
   	    pLbl->setPixmap(pixThreadOff);
 }
}

/**
 * clearFlags
 * clear the update flag of all the analysis objects, indicating that it needs to be recalculated
 */
void Analysis::clearFlags()
{
 unsigned int i;
 
 for (i=0;i<avList.size();i++)
 	 avList[i].bUpdate=false;
}

/**
 * clearFlag
 * clear the update flag of an analysis object, indicating that it needs to be recalculated
 * @param n : index of object
 */
void Analysis::clearFlag(unsigned int n)
{
 unsigned int i;
 
 for (i=0;i<avList.size();i++)
 	{
 	 if (n==avList[i].nGraph)
	 	avList[i].bUpdate=false;
	}
}


/**
 * setFlag
 * set the update flag of an analysis object, indicating that it is complete
 * @param n : index of object
 */
void Analysis::setFlag(unsigned int n)
{
 unsigned int i;
 
 for (i=0;i<avList.size();i++)
 	{
 	 if (n==avList[i].nGraph)
	 	avList[i].bUpdate=true;
	}
}

/**
 * loadSettings
 * load settings from settings file pointed to by ps.
 * @param ps : pointer to qsettings object
 */
void Analysis::loadSettings(QSettings *ps)
{
  QStringList slKeys, slVals;
  double d;
  unsigned int i,j;
  AnaSetT set;
  QString qs;
  
  svList.clear();
  ps->beginGroup(qsApplication);
  slKeys=ps->subkeyList( achAnalysis );
  ps->beginGroup(achAnalysis);
  for (i=0;i<slKeys.size();i++)
  {
   set.qsTitle=slKeys[i];
   slVals=ps->entryList(slKeys[i]);
  ps->beginGroup(slKeys[i]);
   for (j=0;j<slVals.size();j++)
   	{
	 if (slVals[j].find('!')==0)
	 	{
	 	 qs=ps->readEntry(slVals[j]);
		 set.qsDescr=qs;
		 continue;
		}
	 if (slVals[j].find('$')==0)
	 	{
	 	 qs=ps->readEntry(slVals[j]);
		 set.slDescr.append(qs);
		 continue;
		}
	 else
	 	{
	 	 d=ps->readDoubleEntry(slVals[j]);
		 if (slVals[j]==achType)
		 	set.type=(anaOpType)d;
		 else
		 	{
			 set.slNames.push_back(slVals[j]);
		 	 set.ddVals.push_back(d);
			}
		}
	}
   svList.push_back(set);
   set.type=aotNone;
   set.ddVals.clear();
   set.slNames.clear();
   set.slDescr.clear();
   set.qsTitle="";
   set.qsDescr="";
  ps->endGroup();
  }
  ps->endGroup();
  ps->endGroup();
#ifdef _DEBUG
 dumpSettings();
#endif
}

#ifdef _DEBUG
/**
 * dumpSettings
 * output settings to debugger
 */
void Analysis::dumpSettings()
{
  unsigned int i,j;
  
  qDebug("[Analysis::dumpSettings]");
  for (i=0;i<svList.size();i++)
  {
   qDebug("i=%d, type=%d, name=%s.",i,svList[i].type,svList[i].qsTitle.ascii());
   qDebug("Description=%s.",svList[i].qsDescr.ascii());
   for (j=0;j<svList[i].slNames.size();j++)
	 qDebug("\tNames (%d)=%s.",j,svList[i].slNames[j].ascii());
   for (j=0;j<svList[i].slDescr.size();j++)
	 qDebug("\tDescription (%d)=%s.",j,svList[i].slDescr[j].ascii());
   for (j=0;j<svList[i].ddVals.size();j++)
	 qDebug("Values (%d)=%f.",j,svList[i].ddVals[j]);
  }
}
#endif

/**
 * get
 * return pointer to analysis object of type t
 * @param t : analysis type to return
 * @return pointer to object
 */
AnaSetT *Analysis::get(anaOpType t)
{
 unsigned int i;
 
  for (i=0;i<svList.size();i++)
  {
   if (svList[i].type==t)
   	return &svList[i];
  }
  return NULL;
}

/**
 * get1
 * return pointer to analysis object of type t1
 * @param t : analysis type to return
 * @return pointer to object
 */
AnaSetT *Analysis::get1(anaOpType t, unsigned int n)
{
 if (n<avList.size())
 	return &avList[n].set1;
  return NULL;
}

/**
 * get2
 * return pointer to analysis object of type t2
 * @param t : analysis type to return
 * @return pointer to object
 */
AnaSetT *Analysis::get2(anaOpType t, unsigned int n)
{
 if (n<avList.size())
 	return &avList[n].set2;
  return NULL;
}

/**
 * getValue
 * return stored settings value associated with analysis type t and string s
 * @param n : index of analysis object
 * @param s : string settings
 * @return value or GSL_DBL_MIN if not found
 */
double Analysis::getValue(unsigned int n, const char *s, bool b)
{
 unsigned int j;
 AnaSetT *pSet;
 
 if (n<avList.size())
 {
  if (!b)
  	pSet=&avList[n].set1;
  else
  	pSet=&avList[n].set2;
  for (j=0;j<pSet->slNames.size();j++)
	 	{
		 if (pSet->slNames[j]==s)
   	 	 	return pSet->ddVals[j];
		}
  }
  return GSL_DBL_MIN; 
}

/**
 * getValue
 * return stored settings value associated with analysis set s and string qs
 * @param set : local settings
 * @param s : string settings
 * @return value or GSL_DBL_MIN if not found
 */
double Analysis::getValue(AnaSetT &set, const char *s)
{
 unsigned int j;
  
  for (j=0;j<set.slNames.size();j++)
	 	{
		 if (set.slNames[j]==s)
   	 	 	return set.ddVals[j];
		}
  return GSL_DBL_MIN; 
}

/**
 * getValue
 * return stored settings value associated with analysis type t and string s
 * @param t : analysis type
 * @param s : string settings
 * @return value or GSL_DBL_MIN if not found
 */
double Analysis::getValue(anaOpType t, const char *s)
{
 unsigned int i,j;
 
  for (i=0;i<svList.size();i++)
  {
   if (svList[i].type==t)
   	{
	 for (j=0;j<svList[i].slNames.size();j++)
	 	{
		 if (svList[i].slNames[j]==s)
   	 	 	return svList[i].ddVals[j];
		}
	}
  }
  return GSL_DBL_MIN; 
}

/**
 * run
 * thread main loop
 * iterate through the analysis objects and start calculations if needed
 */
void Analysis::run()
{
 unsigned int i;
// unsigned int i,idx1, idx2;

 #ifdef _DEBUG
  qDebug("[Analysis::run] begin.");
 #endif
 try
 {
  //this part is the main bit
  for (i=0;i<avList.size();i++)
  {
/*   idx1=avList[i].nVar1;
   if (idx1>=vvdVars.size())
  	continue;
  if (vvdVars[idx1].nVarId==IDINVALID)
  	continue;
    //check if we need second data set
   idx2=avList[i].nVar2;
   if (idx2!=IDINVALID)
   	if (idx2>=vvdVars.size())
  		continue;
    //Check if it needs to be recalculated
     if (avList[i].bUpdate)
     	continue;
    //first allow the integrator and main thread some peace
    msleep(PAUSE);
    //next try to lock the equation data and make a local copy
    if (!LockIntMutex())
   		throw runtime_error(achAnaFailIntLock);
   avList[i].ddData=*(vvdVars[idx1].pData);
   //need time and data two are mutually exclusive
   if (avList[i].bNeedTime)
   	avList[i].ddTime=*(vvdVars[0].pData);
   if (idx2!=IDINVALID)
   	avList[i].ddTime=*(vvdVars[idx2].pData);
    UnlockIntMutex();
    if (!LockAnaMutex())
   		throw runtime_error(achAnaFailLock);
 #ifdef _DEBUG
  qDebug("[Analysis::run] started %d, type=%d.",i,avList[i].type);
 #endif
   //perform operation
   switch(avList[i].type)
    {
     case aotRecur: doRecur(avList[i]); break;
     case aotMaxLyap: maxlyap(avList[i]); break;
     case aotPeriod: doPeriod(avList[i]); break;
     case aotPower: doPower(avList[i]); break;
     case aotPoincare1d: poincare1d(avList[i]); break;
     case aotPoincare2d: poincare2d(avList[i]); break;
     case aotSpike: spike(avList[i]); break;
     case aotIsi: isi(avList[i]); break;
     default: break;
    }
 #ifdef _DEBUG
  qDebug("[Analysis::run] done %d, type=%d.",i,avList[i].type);
 #endif
    UnlockAnaMutex();
    //post an event that something is done
    AnalysisCompleteEvent* ace = new AnalysisCompleteEvent(i);
    QApplication::postEvent( pControl , ace );  // Qt will delete it when done
    //a bit of a breather
    msleep(PAUSE);
   //free up memory
    avList[i].ddData.clear();
    if (avList[i].bNeedTime)
    	avList[i].ddTime.clear();*/
 #ifdef _DEBUG
  qDebug("[Analysis::doanalysis] started %d, type=%d.",i,avList[i].type);
 #endif
    if (!doAnalysis(avList[i]))
    	continue;
   //post an event that something is done
   AnalysisCompleteEvent* ace = new AnalysisCompleteEvent(i);
   QApplication::postEvent( pControl , ace );  // Qt will delete it when done
 #ifdef _DEBUG
  qDebug("[Analysis::doanalysis] done %d, type=%d.",i,avList[i].type);
 #endif
    //a bit of a breather
    msleep(PAUSE);
    //stop if asked
    if (bStopped)
	break;
  }
   if (pLbl)
   	    pLbl->setPixmap(pixThreadOff);
 }
 catch (exception &stdex)
     {
      bError=true;
      sException=stdex.what();
 #ifdef _DEBUG
  qDebug("[Analysis::run] ERROR (error=%d, sE=%s).",bError,sException.c_str());
 #endif
  }
  UnlockIntMutex();
  UnlockAnaMutex();
   //post an event that thread is done
  AnalysisCompleteEvent* ace = new AnalysisCompleteEvent(IDINVALID);
  QApplication::postEvent( pControl , ace );  // Qt will delete it when done
 #ifdef _DEBUG
  qDebug("[Analysis::run] done (error=%d, sE=%s).",bError,sException.c_str());
 #endif
}

bool Analysis::doAnalysis(AnaTypeT &t)
{
 unsigned int idx1, idx2;
 
 try
 {
  idx1=t.nVar1;
  if (idx1>=vvdVars.size())
  	return false;
  if (vvdVars[idx1].nVarId==IDINVALID)
  	return false;
  //check if we need second data set
 idx2=t.nVar2;
 if (idx2!=IDINVALID)
   	if (idx2>=vvdVars.size())
  		  return false;
  //Check if it needs to be recalculated
  if (t.bUpdate)
     	  	return false;
  //first allow the integrator and main thread some peace
  msleep(PAUSE);
  //next try to lock the equation data and make a local copy
  if (!LockIntMutex())
   		throw runtime_error(achAnaFailIntLock);
  t.ddData=*(vvdVars[idx1].pData);
   //need time and data two are mutually exclusive
  if (t.bNeedTime)
   	t.ddTime=*(vvdVars[0].pData);
   if (idx2!=IDINVALID)
   	t.ddTime=*(vvdVars[idx2].pData);
   UnlockIntMutex();
   if (!LockAnaMutex())
   		throw runtime_error(achAnaFailLock);
   //perform operation
   switch(t.type)
    {
     case aotRecur: doRecur(t); break;
     case aotMaxLyap: maxlyap(t); break;
     case aotPeriod: doPeriod(t); break;
     case aotPower: doPower(t); break;
     case aotPoincare1d: poincare1d(t); break;
     case aotPoincare2d: poincare2d(t); break;
     case aotSpike: spike(t); break;
     case aotIsi: isi(t); break;
     default: break;
    }
    UnlockAnaMutex();
    //a bit of a breather
    msleep(PAUSE);
   //free up memory
    t.ddData.clear();
    if (t.bNeedTime)
    	t.ddTime.clear();
 }
 catch (exception &stdex)
     {
      bError=true;
      sException=stdex.what();
 #ifdef _DEBUG
  qDebug("[Analysis::doanalysis] ERROR (error=%d, sE=%s).",bError,sException.c_str());
 #endif
  }
 return true;
}

/**
 * LockIntMutex
 * Lock the integrator mutex
 * @return success/failure to lock
 */
bool Analysis::LockIntMutex()
{
 if (bIntLocked)
	return true; 
 if (pIntMutex->tryLock())
 	bIntLocked=true;
 else
 {//could not lock at this point,try for INT_LOCK_TRY times for INT_LOCK_MS (=n*m s) to lock it
   int i=0;
   while(i<INT_LOCK_TRY)
   {
    //msleep for 50ms
    msleep(INT_LOCK_MS);
    if (pIntMutex->tryLock())
    {
     bIntLocked=true;
     break;
    }
    i++;
   }
  }
 return bIntLocked; 
}

/**
 * UnlockIntMutex
 * Unlock the integrator mutex
 */
void Analysis::UnlockIntMutex()
{
 if (bIntLocked)
 	pIntMutex->unlock();
 bIntLocked=false;
}

/**
 * LockAnaMutex
 * Lock the analysis mutex
 * @return success/failure to lock
 */
bool Analysis::LockAnaMutex()
{
 if (bAnaLocked)
	return true; 
 if (pAnaMutex->tryLock())
 	bAnaLocked=true;
 else
 {//could not lock at this point,try for ANA_LOCK_TRY times for ANA_LOCK_MS (=n*m s) to lock it
   int i=0;
   while(i<ANA_LOCK_TRY)
   {
    //msleep
    msleep(ANA_LOCK_MS);
    if (pAnaMutex->tryLock())
    {
     bAnaLocked=true;
     break;
    }
    i++;
   }
  }
 return bAnaLocked; 
}

/**
 * UnlockAnaMutex
 * Unlock the analysis mutex
 */
void Analysis::UnlockAnaMutex()
{
 if (bAnaLocked)
 	pAnaMutex->unlock();
 bAnaLocked=false;
}

AnaTypeT *Analysis::get(unsigned int n)
{
 if (n<avList.size())
 	return &avList[n];
 return NULL;
}

void Analysis::remove(unsigned int n)
{
 if (n<avList.size())
 {
  avList[n].ddData.clear();
  avList[n].ddTime.clear();
  avList[n].ddXResult.clear();
  avList[n].ddYResult.clear();
  avList[n].ddResult.clear();
  AnalysisVector::iterator it(&avList[n]);
  avList.erase(it);
#ifdef _DEBUG
  qDebug("[Analysis::remove]  removed %d, size=%u.",n,(unsigned int)avList.size());
 #endif
 }
}

unsigned int Analysis::addAnalysis(anaOpType type,unsigned int n,unsigned int g)
{
 try
 {
  if (type<=aotNone)
 	return IDINVALID;

  AnaSetT *pset; 
  AnaTypeT t;
  t.type=type;
  t.bUpdate=false;
  t.nVar1=n;
  t.nVar2=IDINVALID;
  t.nGraph=g;
  switch(type)
  {
   case aotPeriod:
   case aotPower:
   case aotPoincare1d: 
   case aotSpike:
   case aotIsi: t.bNeedTime=true; break;
   default: t.bNeedTime=false; break;
  }
  pset=get(type);
  if (!pset)
	throw runtime_error(achNoSettings);
  t.set1=*pset;
  avList.push_back(t);
#ifdef _DEBUG
  qDebug("[Analysis::addAnalysis] type=%d  var=%d,ngraph=%d (size=%u).",type,n,g,(unsigned int)avList.size());
 #endif
  return ((unsigned int)avList.size()-1);
 }
 catch (exception &stdex)
     {
      bError=true;
      sException=stdex.what();
 #ifdef _DEBUG
  qDebug("[Analysis::addanalysis] ERROR (error=%d, sE=%s).",bError,sException.c_str());
 #endif
  }
 return IDINVALID;
}

unsigned int Analysis::addAnalysis2d(anaOpType type,unsigned int n1, unsigned int n2,unsigned int g)
{
 if (type<=aotNone)
 	return IDINVALID;
 AnaTypeT t;
 t.type=type;
 t.bUpdate=false;
 t.nVar1=n1;
 t.nVar2=n2;
 t.nGraph=g;
 t.bNeedTime=false;
 t.set1=*get(type);
 t.set2=*get(type);
 avList.push_back(t);
#ifdef _DEBUG
  qDebug("[Analysis::addAnalysis2d] type=%d  var1=%d, var2=%d, ngraph=%d (size=%u).",type,n1,n2,g,(unsigned int)avList.size());
 #endif
 return ((unsigned int)avList.size()-1);
}

bool Analysis::hasAnalysis(anaOpType t, unsigned int n)
{
 unsigned int i;
 
  for (i=0;i<avList.size();i++)
  {
   if (avList[i].type==t)
   	if (avList[i].nVar1==n)
   		return true;
  }
  return false;
}

void Analysis::purgeAnalysis(anaOpType t)
{
 unsigned int i;
 
 bError=false;
 for (i=0;i<avList.size();i++)
  {
   if (avList[i].type==t)
   {
    avList[i].ddData.clear();
    avList[i].ddTime.clear();
    avList[i].ddXResult.clear();
    avList[i].ddYResult.clear();
    avList[i].ddResult.clear();
    AnalysisVector::iterator it(&avList[i]);
    avList.erase(it);
    i=0;
   }
  }
}

//=== Recurrence Plot =================
void Analysis::doRecur(AnaTypeT &t)
{
 double min,max;
 unsigned int dim=1,delay=1,step=0,n;
 double eps=1.e-3;
 unsigned int i,x;
 double epsinv;
 int ibox=BOX-1;

 n=t.ddData.size();
 if ((t.bUpdate)||(n==0))
 	return;
 // Get settings values
 dim=(unsigned int)getValue(t.set1,achDim);
 delay=(unsigned int)getValue(t.set1,achDelay);
 step=(unsigned int)getValue(t.set1,achNeigh);
 
 rescaleData(t.ddData,&min,&max);
#ifdef _DEBUG
  qDebug("[Analysis::dorecur]  dim=%d, delay=%d (size=%d).",dim,delay,n);
 #endif
 //make box
 epsinv=1./eps;
 sIntDeque ivBox(BOX,-1);
 sIntDeque ivList(n,0);
 
 for (i=(dim-1)*delay;i<n;i++)
 {
  x=(int)(t.ddData[i]*epsinv)&ibox;
  ivList[i]=ivBox[x];
  ivBox[x]=i;
 }
#ifdef _DEBUG
  qDebug("[Analysis::dorecur] before neig.");
 #endif
 findNeighbours(t.nGraph,t.ddData,t.ddXResult, t.ddYResult,ivBox,ivList,dim,delay, step,eps,&(t.bUpdate));
#ifdef _DEBUG
  qDebug("[Analysis::dorecur] after neig.");
 #endif
 t.bUpdate=true;
 //post an event of end of progress set to 100%
 AnalysisProgressEvent* ape = new AnalysisProgressEvent(ANA_EVENT, t.nGraph,100,100);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done
}

void Analysis::rescaleData(DblDeque &d,double *min,double *interval)
{
  unsigned int i;
  
  *min=*std::min_element(d.begin(),d.end());
  *interval=*max_element(d.begin(),d.end());
  *interval -= *min;

  for (i=0;i<d.size();i++)
    d[i]=(d[i]- *min)/ *interval;
#ifdef _DEBUG
  qDebug("[Analysis::rescale] min=%f, interval=%f.",*min,*interval);
 #endif
}

void Analysis::findNeighbours(unsigned int nGraph, DblDeque &d, DblDeque &vx, DblDeque &vy, 
					sIntDeque &box,sIntDeque &list, unsigned int dim, unsigned int delay,unsigned int step, double eps, bool *bdone)
{
  unsigned int i,i1,j,j1;
  int ibox=BOX-1,element;
  int n,length;
  double dx,epsinv;
  epsinv=1./eps;

  length=d.size();
  if (step==0)
  	step=std::max(length/1000,1);
  for (n=(dim-1)*delay;n<length;n++)
	{
	 if ((bStopped)||(*bdone))
	 	return;
	 if ((n%step)==0)
	 {
	  AnalysisProgressEvent* ape = new AnalysisProgressEvent(ANA_EVENT,nGraph,length,n);
    	  QApplication::postEvent( pControl , ape );  // Qt will delete it when done
	 }
         i=(int)(d[n]*epsinv)&ibox;
         for (i1=i-1;i1<=i+1;i1++)
         {
          element=box[i1&ibox];
          while (element >= n)
          {
           for (j=0;j<dim;j++)
           {
	    j1=j*delay;
	    dx=fabs(d[n-j1]-d[element-j1]);
	    if (dx > eps)
	      break;
	   }
	  if (j == dim)
          {
           if (n!=element)
           {
            vx.push_back(n);
            vy.push_back(element);
	   }
	  }
	  element=list[element];
	  }
        }
      }
}

//====== Maximal Lyapunov Exponent ====
void Analysis::maxlyap(AnaTypeT &t)
{
  bool alldone;
  unsigned int i,n, maxlength,count;
  double min,max;
  unsigned int dim=2,delay=1,steps=50, mindist=0;
  double eps=1.e-3,epsinv;

  n=t.ddData.size();
  if ((t.bUpdate)||(n==0))
 	return;
//  dim=(unsigned int)getValue(t.set,achDim);
//  delay=(unsigned int)getValue(t.set,achDelay);
//  steps=(unsigned int)getValue(t.set,achSteps);
//  mindist= (unsigned int)getValue(t.set,achWin);
 //step=(unsigned int)getValue(t.set,achNeigh);


  UIntVector ivBox(NMAX*NMAX,-1);
  UIntVector ivFound(steps+1,0);
  UIntVector ivList(n,0);
  std::vector<bool> bDone(n,false);

 rescaleData(t.ddData,&min,&max);
#ifdef _DEBUG
  qDebug("[Analysis::maxlyap]  (size=%d).",n);
 #endif
 
  eps /= max;

  t.ddXResult.assign(steps+1,0.0);
  t.ddYResult.assign(steps+1,0.0);

  maxlength=(n-delay*(dim-1)-steps-1-mindist);
  alldone=false;
  count=0;
  while(!alldone)
  {
   epsinv=1.0/eps;
   put_in_boxes(ivBox,ivList,t.ddYResult,dim,delay,steps,epsinv);
   alldone=true;
   for (i=0;i<=maxlength;i++)
   {
    if (!bDone[i])
	  bDone[i]=make_iterate(i,t.ddData,ivBox,ivList,ivFound,t.ddYResult,dim,delay,steps,mindist,eps);
    alldone &= bDone[i];
   }
   eps*=1.1;
   if (bStopped)
   	return;
    count++;
    //if ((count%)==0)
	 {
	  AnalysisProgressEvent* ape = new AnalysisProgressEvent(ANA_EVENT,t.nGraph,count,n);
    	  QApplication::postEvent( pControl , ape );  // Qt will delete it when done
	 }
  }
  for (i=0;i<=steps;i++)
  {
    if (ivFound[i])
    {
       t.ddXResult[i]=i;
       t.ddYResult[i]=(t.ddYResult[i]/ivFound[i]/2.0);
     }
   }
 //post an event of end of progress set to 100%
 AnalysisProgressEvent* ape = new AnalysisProgressEvent(ANA_EVENT,t.nGraph,100,100);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done   
}

void Analysis::put_in_boxes(UIntVector &box,UIntVector &list, DblDeque &series,unsigned int dim, unsigned int delay, unsigned int steps,double epsinv)
{
  int x,y;
  unsigned int n,i,del;
  unsigned int cNMax=NMAX-1;

  std::fill(box.begin(),box.end(),-1);

  del=delay*(dim-1);
  n=series.size();
  for (i=0;i<(n-delay-steps);i++)
  {
   x=(int)(series[i]*epsinv) & cNMax;
   y=(int)(series[i+del]*epsinv) & cNMax;
   list[i]=box[x+(y*NMAX)];
   box[x+(y*NMAX)]=i;
  }
}

bool Analysis::make_iterate(int idx, DblDeque &series, UIntVector &box, UIntVector &list, UIntVector &found,DblDeque &lyap,
						unsigned int dim, unsigned int delay, unsigned int steps,unsigned int mindist, double eps)
{
  bool bOk=false;
  unsigned int x,y,i,j,i1,k,dist,del1=dim*delay;
  int element,minelement= -1;
  double dx,mindx=1.0;
  unsigned int cNMax=NMAX-1;
  double epsinv=1.0/eps;

 #ifdef _DEBUG
  qDebug("[Analysis::make_iterate]  begin, i=%d, dim=%d,delay=%d, steps=%d, min=%d, eps=%f.",idx,dim,delay,steps,mindist,eps);
 #endif
  x=(int)(series[idx]*epsinv)& cNMax;
  y=(int)(series[idx+delay*(dim-1)]*epsinv)& cNMax;
  for (i=x-1;i<=x+1;i++)
  {
    i1=i & cNMax;
    for (j=y-1;j<=y+1;j++)
    {
     element=box[i1+((j*NMAX)& cNMax)];
     while (element != -1)
     {
      if (element>=idx)
      	dist=0;
      else
      	dist=(unsigned int)abs(idx-element);
      if (dist > mindist)
      {
	    dx=0.0;
	    for (k=0;k<del1;k+=delay)
            {
	     dx+=(series[idx+k]-series[element+k])*(series[idx+k]-series[element+k]);
	     if (dx > eps)
	     	break;
	    }
	    if (k==del1)
            {
	      if (dx < mindx)
              {
	        bOk=true;
	        if (dx > 0.0)
                {
		    mindx=dx;
		    minelement=element;
	        }
	       }
	     }//end if (k==del1)
	   }
	   element=list[element];
     }//end while
    }//end for j
  }//end for i
  if (minelement != -1)
  {
   idx--;
   minelement--;
   for (i=0;i<=steps;i++)
   {
    idx++;
    minelement++;
    dx=0.0;
    for (j=0;j<del1;j+=delay)
    	dx+=(series[idx+j]-series[minelement+j])*(series[idx+j]-series[minelement+j]);
    if (dx > 0.0)
    {
      found[i]++;
      lyap[i] += log(dx);
    }
   }//end for i
  }//end if
  return bOk;
}

// ===== Various useful functions =====
double Analysis::sqr(double d)
{
 if (d==0.0)
 	return 0.0;
 return (d*d);
}

void Analysis::davevar(DblDeque &data, double *ave, double *var)
{
 unsigned int j,n;
 double s,ep;

 n=data.size();
 for (*ave=0.0,j=1;j<=n;j++)
 	*ave+=data[j];
 *ave/=n;
 *var=ep=0.0;
 for (j=1;j<=n;j++)
 	{
 	 s=data[j]-(*ave);
 	 ep+=s;
	 *var+=s*s;
 	}
 *var=(*var-ep*ep/n)/(n-1);	//two pass formula pp 613
}

unsigned int Analysis::getIntPow(unsigned int val)
{
 unsigned int l=1;
 while (l<=val) l<<=1;
 if (l==UINT_MAX)
  return 0;
 return l;
}

bool Analysis::isIntPow(unsigned int val)
{
 unsigned int l=1;
 unsigned int count=0;

 while (l<=val)
	{
    	 if (l&val)
        	count++;
    	  l<<=1;
   	}
 if (count==1)
 	return true;
 return false;
}

// ======== Periodogram ===============
void Analysis::dspread(double y, NS_Formu::DblVector &yy, double x, int m)
{
 int ihi,ilo,ix,j,nden,n;
 double fac;

 if (m>10)
// 	throw runtime_error(QString("reported by %1 %2 in\n%3\nat line %4 :\n%5").arg("Analysis").arg(gsl_version).arg(__PRETTY_FUNCTION__).arg(__LINE__-2).arg( gsl_strerror(status)).ascii());
 	throw runtime_error(QString().sprintf("[Analysis::dspread]: factorial table too small. m=%d, MACC=%d\n",m,MACC));
 n=yy.size();
 ix=(int)x;
 if (x==(double)ix)
 	yy[ix]+=y;
 else
 	{
 	 ilo=GSL_MAX((long)(x-0.5*m+1.0),1);
 	 ilo=GSL_MIN(ilo,n-m+1);
 	 ihi=ilo+m-1;
// 	 nden=nFactorial[m];
 	 nden=static_cast<int>(gsl_sf_fact(m));
 	 fac=x-ilo;
 	 for (j=ilo+1;j<=ihi;j++)
 	 	fac*=(x-j);
 	 yy[ihi]+=y*fac/(nden*(x-ihi));
 	 for (j=ihi-1;j>=ilo;j--)
 	 	{
 	 	 nden=(nden/(j+1-ilo))*(j-ihi);
 	 	 yy[j]+=y*fac/(nden*(x-j));
 	 	}
 	}
}

//fast period alg. press, pp582
unsigned int Analysis::dfastper(unsigned int nGraph, DblDeque &x, DblDeque &y, double ofac, double hifac, unsigned int nwk, NS_Formu::DblVector &wk1, NS_Formu::DblVector &wk2, double *per, double *freq, double *prob)
/*double *x, double *y, unsigned int n, double ofac, double hifac,
			 unsigned int nwk, unsigned int *nout, double **ewk1, double **ewk2,
			 double *freq, double *prob)*/
{
 unsigned int j,k,ndim,nfreq,nfreqt,jmax=0,n, nout;
 int status;
 double ave,ck,ckk,cterm,cwt,den,df,effm,expy,fac,fndim,hc2wt;
 double hs2wt,hypo,pmax,sterm,swt,var,xdif,xmax,xmin;
 AnalysisProgressEvent* ape;
 
#ifdef _DEBUG
 qDebug("[Analysis::dfastper] started");
#endif

 n=x.size();
 nout=(unsigned int)(0.5*ofac*hifac*n);
 nfreqt=(unsigned int)(ofac*hifac*n*MACC);
 nfreq=64;
 if (nfreq<nfreqt)
 	while(nfreq<nfreqt) nfreq<<=1;
 ndim=nfreq<<1;
 if (ndim>nwk)
 	{
	 wk1.assign(ndim+nwk,0.0);
	 wk2.assign(ndim+nwk,0.0);
#ifdef _DEBUG
 qDebug("[Analysis::dfastper]  Workspace too small.\nWant %d, got %d.\n",ndim,nwk);
#endif
	}
 else
 	{
	 wk1.assign(nwk,0.0);
	 wk2.assign(nwk,0.0);
#ifdef DEBUG
 qDebug("[Analysis::dfastper]: made workspace.\nWant %d, got %d.\n",ndim,nwk);
#endif
 	}
 davevar(y,&ave,&var); //mean,variance and range of y
 //find min max and diff of x
 xmin=*min_element(x.begin(),x.end());
 xmax=*max_element(x.begin(),x.end());
 xdif=xmax-xmin;

 fac=ndim/(xdif*ofac);
 fndim=ndim;
 for (j=1;j<=n;j++)
 	{				//extirpolate the data into the workspaces
 	 ck=(x[j]-xmin)*fac;
	 ck=fmod(ck,fndim);
 	 ckk=2.0*(ck++);
 	 ckk=fmod(ckk,fndim);
 	 ++ckk;
 	 dspread(y[j]-ave,wk1,ck,MACC);
 	 dspread(1.0,wk2,ckk,MACC);
 	}
  // done first part
  ape = new AnalysisProgressEvent(ANA_EVENT,nGraph,33,100);
  QApplication::postEvent( pControl , ape );  // Qt will delete it when done
	
 //do fft
 status=gsl_fft_real_radix2_transform(wk1.begin().base(),1,nwk);
 if (status!=GSL_SUCCESS)
 	throw runtime_error(QString("reported by %1 %2 in\n%3\nat line %4 :\n%5").arg("GSL").arg(gsl_version).arg(__PRETTY_FUNCTION__).arg(__LINE__-2).arg( gsl_strerror(status)).ascii());
 status=gsl_fft_real_radix2_transform(wk2.begin().base(),1,nwk);
 if (status!=GSL_SUCCESS)
 	throw runtime_error(QString("reported by %1 %2 in\n%3\nat line %4 :\n%5").arg("GSL").arg(gsl_version).arg(__PRETTY_FUNCTION__).arg(__LINE__-2).arg( gsl_strerror(status)).ascii());
  //done FFT
  ape = new AnalysisProgressEvent(ANA_EVENT,nGraph,66,100);
  QApplication::postEvent( pControl , ape );  // Qt will delete it when done

 // Distribute freq
 df=1.0/(xdif*ofac);
 pmax=-1.0;
 for (k=3,j=1;j<=(nout);j++,k+=2)
 	{				//compute the Lomb value for each frequency
 	 hypo=sqrt(wk2[k]*wk2[k]+wk2[k+1]*wk2[k+1]);
 	 hc2wt=0.5*wk2[k]/hypo;
 	 hs2wt=0.5*wk2[k+1]/hypo;
 	 cwt=sqrt(0.5+hc2wt);
 	 swt=GSL_SIGN(hs2wt)*sqrt(0.5-hc2wt);
 	 den=0.5*n+hc2wt*wk2[k]+hs2wt*wk2[k+1];
 	 cterm=sqr(cwt*wk1[k]+swt*wk1[k+1])/den;
 	 sterm=sqr(cwt*wk1[k+1]-swt*wk1[k])/(n-den);
 	 wk1[j]=j*df;
 	 wk2[j]=(cterm+sterm)/(2.0*var);
 	 if (wk2[j]>pmax) pmax=wk2[(jmax=j)];
 	}
 expy=exp(-pmax);			//estimate significance of largest peak value
 effm=2.0*nout/ofac;
 *prob=effm*expy;
 if (*prob>0.01)
 	*prob=1.0-pow(1.0-expy,effm);
/*#ifdef DEBUG
 fprintf(filep,"DEBUG [dfastper]: jmax=%lu val=%f, period=%f, prob=%2.10f.\n",jmax,wk1[jmax],1/wk1[jmax],*prob);
#endif*/
 *freq=wk1[jmax];
/*#ifdef DEBUG
 fprintf(filep,"DEBUG [dfastper]: end.");
#endif*/
 if (*freq!=0.0)
 	*per=(1/(*freq));
 else
 	*per=DBL_MAX;
 return nout;
}

void Analysis::doPeriod(AnaType &t)
{
 unsigned int nn, n;
 double per, prob, freq;

 n=t.ddTime.size();
 if (n==0)
 	return;

  nn=getIntPow(n);
  t.ddXResult.clear();
  t.ddYResult.clear();
  t.ddResult.clear();
 
 //the Press algortims begin with matrix at index =1, not 0!
 //post an event of end of progress set to 100%
 AnalysisProgressEvent* ape = new AnalysisProgressEvent(ANA_EVENT,t.nGraph,0,100);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done   
 
 NS_Formu::DblVector wk1, wk2;
 nn=dfastper(t.nGraph,t.ddTime,t.ddData,4.0,1.0,nn,wk1,wk2,&per,&freq,&prob);
/* delete[] pfy;
 delete[] pfx;*/
 // copy results into output arrays
 //t.ddXResult.assign(nn,0.0);
 //t.ddYResult.assign(nn,0.0);
 for (unsigned int i=1;i<nn;i++)
   {
     if (wk2[i]>0.0)
     	{
	  t.ddXResult.push_back(wk1[i]);
	  t.ddYResult.push_back(wk2[i]);
	}
   }
 //post an event of end of progress set to 100%
 ape = new AnalysisProgressEvent(ANA_EVENT,t.nGraph,100,100);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done   
 //store the period, frequency and probability
 t.ddResult.push_back(per);
 t.ddResult.push_back(freq);
 t.ddResult.push_back(prob);
//  if (per==DBL_MAX)
// 	return 0;
//  else
// 	return per;
// 
}

// ==== Window filter functions =======
void Analysis::windowBartlett(DblDeque &d)
{
 transform(d.begin(),d.end(),d.begin(),bartlett(d.size()));
#ifdef _DEBUG
 for (unsigned int i=0;i<d.size();i++)
	qDebug("[Analysis::windowBartlett] [%d]=%f",i,d[i]);
#endif
}

void Analysis::windowBlackman(DblDeque &d)
{
 transform(d.begin(),d.end(),d.begin(),blackman(d.size()));
}

void Analysis::windowHamming(DblDeque &d)
{
 transform(d.begin(),d.end(),d.begin(),hamming(d.size()));
}

void Analysis::windowHann(DblDeque &d)
{
 transform(d.begin(),d.end(),d.begin(),hann(d.size()));
}

// ==== Powerspectrum =================
void Analysis::doPower(AnaType &t)
{
 unsigned int n, nn, m, i;
 double d, dmean, dl;

 n=t.ddTime.size();
 if (n==0)
 	return;
  
  nn=getIntPow(n);
  t.ddXResult.clear();
  t.ddYResult.clear();
  t.ddResult.clear();
 
  m=2*(unsigned int)sqrt((double)nn);  //optimal selection of window size according to Chatfield
 // find average sample
 dmean=fabs(deltaAverage(t.ddTime));
 //copy into vector
 NS_Formu::DblVector dv(nn,0.0);
 for (i=0;i<t.ddData.size();i++)
 	dv[i]=t.ddData[i];
 powerSpectrum(dv, m, swBartlett);
  for (i=0;i<nn;i+=2)
       	{//plot normalised frequency
         d=(2.0*(double)i)/((double)nn*dmean*M_PI);
         if (dv[i]>0)
                dl=log10(dv[i]);
         else
                dl=LOG_MIN;
         if (d>0.0)
	 {
	   t.ddXResult.push_back(d);
	   t.ddYResult.push_back(dl);
	 }
        }
}

double Analysis::deltaAverage(DblDeque &data)
{
 unsigned int i,n;
 double fm, fsum;

 n=data.size();
 if (n==0)
 	return 0.0;
  fsum=0.0;
  for (i=0;i<(n-1);i++)
  	fsum+=(data[i+1]-data[i]);
  fm=fsum/(n-1);
  return fm;
}

void Analysis::powerSpectrum(NS_Formu::DblVector &data, unsigned int m, specWinType type)
{
 int status;
 unsigned int nn,k,i,l,off;

 if (m==0)
 	return;
 gsl_complex gc;
//first step: ensure data is power of two
nn=data.size();
 if (!isIntPow(nn))
	throw runtime_error(QString().sprintf("[Analysis::powerSpectrum]: called with data size not power of 2 (%u)",nn));
//second step: calculate the number of segments k of size m
// m is window size, k is number of windows
 k=nn/m;
 if (nn%k)
 	k++;
//third step: get the window function by copying it into the p buffer
if (type!=swNone)
 {
  DblDeque pwin(m,1.0); // unit vector of size m
  switch (type)
   {
    default: break;
    case swBartlett:	windowBartlett(pwin); break;
    case swBlackman: windowBlackman(pwin); break;
    case swHamming: windowHamming(pwin); break;
    case swHann: windowHann(pwin); break;
   }
  //fourth A step: apply window to data
  for (i=0;i<k;i++)
  	{
    	off=i*m;
	l=(nn-off);
    	if (l<m) //truncate window
		 pwin.resize(l);
	transform(pwin.begin(),pwin.end(),&data[off],&data[off],multiplies<double>());
/*#ifdef DEBUG
fprintf(filep,"DEBUG [dPowerSpectrum], applying window %d: i=%d, off=%d, m=%d.\n",SpecWin,i,off,m);
#endif*/
   }
 }
//Fifth step: FFT the windowed data
 status=gsl_fft_real_radix2_transform(data.begin().base(),1,nn);
 if (status!=GSL_SUCCESS)
 	throw runtime_error(QString("reported by %1 %2 in\n%3\nat line %4 :\n%5").arg("GSL").arg(gsl_version).arg(__PRETTY_FUNCTION__).arg(__LINE__-2).arg( gsl_strerror(status)).ascii());

// nspdRealFftNip(fp, FreqSp, off, NSP_Forw);
// nspzbConjExtend1(FreqSp, nn/2);
 for (i=0;i<nn;i+=2)
 {
  gc.dat[0]=data[i];
  gc.dat[1]=data[i+1];
  gc=gsl_complex_conjugate(gc);
 //sixth step: calculate power
 // nspzbPowerSpectr(FreqSp,p,nn);
  gc=gsl_complex_pow_real(gc,2.0);
  data[i]=gc.dat[0];
  data[i+1]=gc.dat[1];
 }
}

// ====== General Poincare section ====
void Analysis::poincare1d(AnaTypeT &t)
{
 double cut,a,b,time,delta,xcut;
 unsigned int i,n;
 poinDirection dir;
 
 n=t.ddData.size();
 if ((t.bUpdate)||(n==0))
 	return;
 cut=getValue(t.set1,achCut);
 dir=(poinDirection)getValue(t.set1,achDir);
 time=0.0;
 for(i=0;i<(n-1);i++)
 {
  a=t.ddData[i];
  b=t.ddData[i+1];
  switch(dir)
  {
   case pdUp:
   {
    if ((a<cut)&&(b>=cut))
  	{
  	 delta=(a-cut)/(a-b);
  	 time=t.ddTime[i];
	 xcut=a+delta*(b-a);
 	 t.ddXResult.push_back(time);
	 t.ddYResult.push_back(xcut);
  	}
    break;
   }
   case pdDown:
   {
    if ((a>cut)&&(b<=cut))
  	{
  	 delta=(a-cut)/(a-b);
  	 time=t.ddTime[i];
	 xcut=a+delta*(b-a);
 	 t.ddXResult.push_back(time);
	 t.ddYResult.push_back(xcut);
  	}
    break;
   }
//   case pdBoth:
   default:
   {
    if (((a<cut)&&(b>=cut))||((a>cut)&&(b<=cut)))
  	{
  	 delta=(a-cut)/(a-b);
  	 time=t.ddTime[i];
	 xcut=a+delta*(b-a);
 	 t.ddXResult.push_back(time);
	 t.ddYResult.push_back(xcut);
  	}
    break;
   }
  }
 }
 t.bUpdate=true;
#ifdef _DEBUG
  qDebug("[Analysis::poincare1d]  cut=%f, dir=%d (time=%f, size=%d).",cut,(int)dir,time,(unsigned int)t.ddXResult.size());
 #endif
}

void Analysis::poincare2d(AnaTypeT &t)
{
 double cut1,cut2,a,b,time,delta,xcut;
 unsigned int i,n;
 poinDirection dir1,dir2;
 
 n=t.ddData.size();
 if ((t.bUpdate)||(n==0))
 	return;
 cut1=(unsigned int)getValue(t.set1,achCut);
 dir1=(poinDirection)getValue(t.set1,achDir);
 cut2=(unsigned int)getValue(t.set2,achCut);
 dir2=(poinDirection)getValue(t.set2,achDir);
 
//  for(i=0;i<(n-1);i++)
//  {
//   a=t.ddData[i];
//   b=t.ddData[i+1];
//   switch(dir)
//   {
//    case pdUp:
//    {
//     if ((a<cut)&&(b>=cut))
//   	{
//   	 delta=(a-cut)/(a-b);
//   	 time=t.ddTime[i];
// 	 xcut=a+delta*(b-a);
//  	 t.ddXResult.push_back(time);
// 	 t.ddYResult.push_back(xcut);
//   	}
//     break;
//    }
//    case pdDown:
//    {
//     if ((a>cut)&&(b<=cut))
//   	{
//   	 delta=(a-cut)/(a-b);
//   	 time=t.ddTime[i];
// 	 xcut=a+delta*(b-a);
//  	 t.ddXResult.push_back(time);
// 	 t.ddYResult.push_back(xcut);
//   	}
//     break;
//    }
//    case pdBoth:
//    default:
//    {
//     if (((a<cut)&&(b>=cut))||((a>cut)&&(b<=cut)))
//   	{
//   	 delta=(a-cut)/(a-b);
//   	 time=t.ddTime[i];
// 	 xcut=a+delta*(b-a);
//  	 t.ddXResult.push_back(time);
// 	 t.ddYResult.push_back(xcut);
//   	}
//     break;
//    }
//   }
//  }
#ifdef _DEBUG
  qDebug("[Analysis::poincare2d]  (time=%f).",time);
 #endif
}

// ====== Sike detection ====
void Analysis::spike(AnaTypeT &t)
{
 double a,b,c,d,e,time,cut,xcut;
 unsigned int i,n;
 bool bUse;
 
 n=t.ddData.size();
 if ((t.bUpdate)||(n<=2))
 	return;
 cut=getValue(t.set1,achThres);
 bUse=(bool)getValue(t.set1,achUse);
 time=0.0;
 //Step 1: find maxima
 for(i=1;i<(n-1);i++)
 {
  a=t.ddData[i-1];
  b=t.ddData[i];
  c=t.ddData[i+1];
    if ((b>=a) && (b>c))
  	{
	 d=(c-a)/2.0;
	 e=(c-2.0*b+a)/2.0;
	 xcut=b-d*d/4.0/e;
  	 time=t.ddTime[i];
 	 t.ddXResult.push_back(time);
	 t.ddYResult.push_back(xcut);
  	}
  }
#ifdef _DEBUG
  qDebug("[Analysis::spike]  cut=%f, use=%d (found size=%d).",cut,(int)bUse,(unsigned int)t.ddYResult.size());
 #endif
 //Step 2: optional threshold
 if (bUse)
 	{
 	 //for some reason you can not get a normal iterator for a deque as you would for a vector based on the index of an element
 	 //Presumably, due to the segmented data structure, but it is a pain.
 	 //Solution :
 	 DblDeque::iterator ity,itx;
 	 while((ity=find_if(t.ddYResult.begin(),t.ddYResult.end(),bind2nd(less<double>(),cut)))!=t.ddYResult.end())
 	 	{
 	 	 itx=t.ddXResult.begin();
 	 	 advance(itx,distance(t.ddYResult.begin(),ity));
 	 	 t.ddYResult.erase(ity);
 	 	 t.ddXResult.erase(itx);
 	 	}
 	}
 t.bUpdate=true;
#ifdef _DEBUG
  qDebug("[Analysis::spike]  cut=%f, use=%d (time=%f,new size=%d).",cut,(int)bUse,time,(unsigned int)t.ddYResult.size());
 #endif
}
// ====== ISI ====
void Analysis::isi(AnaTypeT &t)
{
 double a,b,c,d,e,time,cut,xcut;
 unsigned int i,n;
 AnalysisProgressEvent* ape;
 
 n=t.ddData.size();
 if ((t.bUpdate)||(n<=2))
 	return;
 cut=getValue(t.set1,achThres);
 time=0.0;
 //Step 1: find maxima
 for(i=1;i<(n-1);i++)
 {
  a=t.ddData[i-1];
  b=t.ddData[i];
  c=t.ddData[i+1];
    if ((b>=a) && (b>c))
  	{
	 d=(c-a)/2.0;
	 e=(c-2.0*b+a)/2.0;
	 xcut=b-d*d/4.0/e;
  	 time=t.ddTime[i];
 	 t.ddXResult.push_back(time);
	 t.ddYResult.push_back(xcut);
  	}
  }
ape = new AnalysisProgressEvent(ANA_ISIEVENT, 0,100,33);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done
#ifdef _DEBUG
  qDebug("[Analysis::isi]  cut=%f, (found size=%d).",cut,(unsigned int)t.ddYResult.size());
 #endif
 //Step 2: threshold
 //for some reason you can not get a normal iterator for a deque as you would for a vector based on the index of an element
 //Presumably, due to the segmented data structure, but it is a pain.
 //Solution :
 DblDeque::iterator ity,itx;
 while((ity=find_if(t.ddYResult.begin(),t.ddYResult.end(),bind2nd(less<double>(),cut)))!=t.ddYResult.end())
 	 	{
 	 	 itx=t.ddXResult.begin();
 	 	 advance(itx,distance(t.ddYResult.begin(),ity));
 	 	 t.ddYResult.erase(ity);
 	 	 t.ddXResult.erase(itx);
 	 	}
 ape = new AnalysisProgressEvent(ANA_ISIEVENT, 0,100,66);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done
 //Step 3: calc the difference
 t.ddResult.assign(t.ddXResult.size(),0.0);
 adjacent_difference(t.ddXResult.begin(),t.ddXResult.end(),t.ddResult.begin());
 ape = new AnalysisProgressEvent(ANA_ISIEVENT, 0,100,100);
 QApplication::postEvent( pControl , ape );  // Qt will delete it when done
 t.bUpdate=true;
#ifdef _DEBUG
  qDebug("[Analysis::isi]  cut=%f, (time=%f,new size=%d)\n\tdif size=%d\tvals size=%d.",cut,time,(unsigned int)t.ddXResult.size(),(unsigned int)t.ddResult.size(),(unsigned int)t.ddYResult.size());
 #endif
}
