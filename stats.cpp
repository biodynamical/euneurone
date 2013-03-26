// statistics: stats.cpp
#include <qmutex.h>
#include <stdexcept>
#include <numeric>
#include <gsl/gsl_statistics_double.h>
#include "controlobject.h"
#include "stats.h"

using namespace NS_Control;
using namespace NS_Stats;
using namespace std;

#define INT_LOCK_TRY 600
#define INT_LOCK_MS 50
#define PAUSE 20

const char *achStatFailIntLock="Failed to lock data memory in Statistics thread";
const char *achStatFailStatLock="Failed to lock GUI memory in Statistics thread";
//====================================================
Statistics::Statistics(QMutex *pi,QMutex *ps, QLabel *pl) : statQ(QPixmap::fromMimeSource("statq.png")), statP(QPixmap::fromMimeSource("statp.png")), statOk(QPixmap::fromMimeSource("statok.png")), statOn(QPixmap::fromMimeSource("threadon.png")), statOff(QPixmap::fromMimeSource("threadoff.png"))
{
 pIntMutex=pi;
 pStatMutex=ps;
 pLbl=pl;
 bMean=true;
 bVariance=true;
 bIntLocked=false;
 bStatLocked=false;
 bStopped=false;
 bError=false;
}

Statistics::~Statistics()
{
}

void  Statistics::RegisterData(NS_Control::TVarDataVector &vd, NS_Control::TVarDataVector &ve)
{
 //local copy of the variables array
 vvdVars=vd;
 vvdErrors=ve;
 initColumns(true);
}

void Statistics::setLabel(QLabel *pl)
{
 pLbl=pl;
}
//---------------------------------------------------------------------------
const char *Statistics::GetErrorStr()
{
 if (bError)
  return sException.c_str();
 return NULL;
}

void Statistics::initColumns(bool bzero)
{
 unsigned int i,j;
 
 try
 {
  if (!LockStatMutex())
   	throw runtime_error(achStatFailStatLock);
  qsNum.setNum(0.0);
  for (j=COLUMN_MEAN;j<=COLUMN_VARIANCE;j++)
  {
   for (i=0;i<vvdVars.size();i++)
   {
    if (vvdVars[i].nVarId==IDINVALID)
  	continue;
    vvdVars[i].pItem->setPixmap(j,statQ);
    if (bzero)
   	vvdVars[i].pItem->setText(j,qsNum);
   }
   for (i=0;i<vvdErrors.size();i++)
   {
    if (vvdErrors[i].nVarId==IDINVALID)
  	continue;
    vvdErrors[i].pItem->setPixmap(j,statQ);
    if (bzero)
   	vvdErrors[i].pItem->setText(j,qsNum);
   }
  }
  pStatMutex->unlock();
  bStatLocked=false;
 }
 catch (exception &stdex)
 {
   bError=true;
   sException=stdex.what();
 }
}

void Statistics::setVarColumnData(int col,unsigned int idx,double d)
{
 if (idx>=vvdVars.size())
 	return;
 
 try
 {
  if (!LockStatMutex())
   	throw runtime_error(achStatFailStatLock);
  qsNum.setNum(d);
  vvdVars[idx].pItem->setPixmap(col,statOk);
  vvdVars[idx].pItem->setText(col,qsNum);
  pStatMutex->unlock();
  bStatLocked=false;
 }
 catch (exception &stdex)
 {
   bError=true;
   sException=stdex.what();
 }
}

void Statistics::flagVarColumn(int col,unsigned int idx)
{
 if (idx>=vvdVars.size())
 	return;
 
 try
 {
  if (!LockStatMutex())
   	throw runtime_error(achStatFailStatLock);
  vvdVars[idx].pItem->setPixmap(col,statP);
  pStatMutex->unlock();
  bStatLocked=false;
 }
 catch (exception &stdex)
 {
   bError=true;
   sException=stdex.what();
 }
}

void Statistics::setErrorColumnData(int col,unsigned int idx,double d)
{
 if (idx>=vvdErrors.size())
 	return;
 
 try
 {
  if (!LockStatMutex())
   	throw runtime_error(achStatFailStatLock);
  qsNum.setNum(d);
  vvdErrors[idx].pItem->setPixmap(col,statOk);
  vvdErrors[idx].pItem->setText(col,qsNum);
  pStatMutex->unlock();
  bStatLocked=false;
 }
 catch (exception &stdex)
 {
   bError=true;
   sException=stdex.what();
 }
}

void Statistics::flagErrorColumn(int col,unsigned int idx)
{
 if (idx>=vvdVars.size())
 	return;
 
 try
 {
  if (!LockStatMutex())
   	throw runtime_error(achStatFailStatLock);
  vvdErrors[idx].pItem->setPixmap(col,statP);
  pStatMutex->unlock();
  bStatLocked=false;
 }
 catch (exception &stdex)
 {
   bError=true;
   sException=stdex.what();
 }
}

void Statistics::Reset()
{
 bError=false;
 initColumns(true);
}

void Statistics::Start()
{
 if (!running())
 {
   bStopped=false;
   if (pLbl)
   	    pLbl->setPixmap( statOn);
   start();
 }
}

void Statistics::Stop()
{
 if (running())
 {
  //check if locked
   if(bIntLocked)
   {
    pIntMutex->unlock();
    bIntLocked=false;
   }
   if(bStatLocked)
   {
    pStatMutex->unlock();
    bStatLocked=false;
   }
  bStopped=true;
   if (pLbl)
   	    pLbl->setPixmap(statOff);
 }
}

void Statistics::run()
{
 #ifdef _DEBUG
  qDebug("[Statistics::run] begin.");
 #endif
 try
 {
  //First calc the stats of each variable
  calcVars();
  calcErrors();
 }
 catch (exception &stdex)
     {
      bError=true;
      sException=stdex.what();
     }
 if (pLbl)
   	    pLbl->setPixmap( statOff);
 #ifdef _DEBUG
 if (bError)
  qDebug("[Statistics::run] done (error=%s).",sException.c_str());
 else
  qDebug("[Statistics::run] done.");
 #endif
}

void Statistics::calcErrors()
{
  unsigned int i,n;
  double d;
  
  n=vvdErrors.size();
  for (i=0;i<n;i++)
  {
   //stop if asked
   if (bStopped)
	break;
   if (vvdErrors[i].nVarId==IDINVALID)
  	continue;
   //first allow the integrator and main thread some peace
   msleep(PAUSE);
   //next try to lock the equation data and make a local copy
  if (!LockIntMutex())
   		throw runtime_error(achStatFailIntLock);
   ddData=*(vvdErrors[i].pData);
   pIntMutex->unlock();
   bIntLocked=false;
   //a bit of a breather
   msleep(PAUSE);
   //do the various calculations on this data set
   // If the nVarId!=0 then data has been stored
   // this means we need to calc the mean etc.
   //otherwise just display the running average
   if (vvdErrors[i].nVarId)
   {
    if (bMean)
   	{
         flagErrorColumn(COLUMN_MEAN,i);
   	 d=calcMean(i);
	 setErrorColumnData(COLUMN_MEAN,i,d);
	}
    //a bit of a breather
    msleep(PAUSE);
    if (bVariance)
   	{
	 flagErrorColumn(COLUMN_VARIANCE,i);
   	 d=calcStddev(i);
	 setErrorColumnData(COLUMN_VARIANCE,i,d);
	}
   }
  else
  { //already have mean values
   d=ddData[i]; //size of errorarray is same as the size of display data array
   setErrorColumnData(COLUMN_MEAN,i,d);
  }   
  //free up memory
   ddData.clear();
/* #ifdef _DEBUG
   qDebug("[Statistics::calcErrors()] done i=%d of %d (%f).",i,n,d);
  #endif*/
  //check if locked, shouldn't be
  if(bIntLocked)
   {
    pIntMutex->unlock();
    bIntLocked=false;
   }
  if(bStatLocked)
   {
    pStatMutex->unlock();
    bStatLocked=false;
   }
   //a bit of a breather
   msleep(PAUSE);
  }
}

void Statistics::calcVars()
{
  unsigned int i,n;
  double d;
  
  n=vvdVars.size();
  for (i=0;i<n;i++)
  {
   //stop if asked
   if (bStopped)
	break;
   if (vvdVars[i].nVarId==IDINVALID)
  	continue;
   //first allow the integrator and main thread some peace
  // msleep(PAUSE);
   //next try to lock the equation data and make a local copy
   if (!LockIntMutex())
   		throw runtime_error(achStatFailIntLock);
   ddData=*(vvdVars[i].pData);
   pIntMutex->unlock();
   bIntLocked=false;
   //a bit of a breather
   msleep(PAUSE);
    //do the various calculations on this data set
    if (bMean)
   	{
         flagVarColumn(COLUMN_MEAN,i);
   	 d=calcMean(i);
	 setVarColumnData(COLUMN_MEAN,i,d);
	}
    //a bit of a breather
    msleep(PAUSE);
    if (bVariance)
   	{
	 flagVarColumn(COLUMN_VARIANCE,i);
   	 d=calcStddev(i);
	 setVarColumnData(COLUMN_VARIANCE,i,d);
	}
  //free up memory
   ddData.clear();
// #ifdef _DEBUG
//   qDebug("[Statistics::run] done i=%d of %d.",i,n);
//  #endif
  //check if locked, shouldn't be
  if(bIntLocked)
   {
    pIntMutex->unlock();
    bIntLocked=false;
   }
  if(bStatLocked)
   {
    pStatMutex->unlock();
    bStatLocked=false;
   }
   //a bit of a breather
   msleep(PAUSE);
  }
}

bool Statistics::LockIntMutex()
{
 if (bIntLocked)
	return true; 
 bIntLocked=pIntMutex->tryLock();
 if (!bIntLocked)
 {//could not lock at this point,try for INT_LOCK_TRY times for INT_LOCK_MS (=n*m s) to lock it
   int i=0;
   while((i<INT_LOCK_TRY)&&(!bIntLocked))
   {
    //wait for 50ms
    msleep(INT_LOCK_MS);
    bIntLocked=pIntMutex->tryLock();
    i++;
   }
  }
 return bIntLocked; 
}

bool Statistics::LockStatMutex()
{
 if (bStatLocked)
	return true; 
 if (pStatMutex->tryLock())
 	bStatLocked=true;
 else
 {//could not lock at this point,try for STAT_LOCK_TRY times for STAT_LOCK_MS (=n*m s) to lock it
   int i=0;
   while(i<STAT_LOCK_TRY)
   {
    //wait
    msleep(STAT_LOCK_MS);
    if (pStatMutex->tryLock())
    {
     bStatLocked=true;
     break;
    }
    i++;
   }
  }
 return bStatLocked; 
}

double Statistics::calcMean(unsigned int idx)
{
 double d,dsum;
 //note that if ddData.size()==0 the division should return INF/NaN
 d=0.0; 
 dsum=accumulate(ddData.begin(),ddData.end(),0.0);
 d=dsum/ddData.size();
 return d;
}

double Statistics::calcStddev(unsigned int idx)
{
 double *pData;
 double d;
 
 //copy data
 pData=new double[ddData.size()];
 for (unsigned int i=0;i<ddData.size();i++)
 	pData[i]=ddData[i];
 d=gsl_stats_sd(pData,1,ddData.size());
 delete[] pData;
 return d;
}
