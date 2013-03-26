//---------------------------------------------------------------------------
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdexcept>
#include <algorithm>
#include <qapplication.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qbuttongroup.h>
#include <qlistview.h>

#ifdef SPINNER
#include "dataspinner.h"
#endif
#include "controlobject.h"
#include "integrator.h"

using namespace NS_Integrator;
using namespace NS_Equation;
using namespace std;
#ifdef SPINNER
using namespace NS_DataSpinner;
#endif

extern NS_Control::Control *pControl;

//try for 30s
#define INT_LOCK_TRY 600
#define INT_LOCK_MS 50
//---------------------------------------------------------------------------
//Integration defines
//---------------------------------------------------------------------------
#define EVER		;;
#define LITTLE		1.0e-10			//fix for roundoff
#define SAFETY 0.9
#define PGROW -0.2
#define PSHRNK -0.25
#define ERRCON 1.89e-4
//the value ERRCON equals (5/SAFETY) raised to the power (1/PGROW)
//#define EPS		1.0e-4			//approx. square root of machine precision
#define TINY		1.0e-30
#define SIGN(a,b)	((b)>=0.0 ? fabs(a) : -fabs(a))
#define LYAPUNOV_DIST 1.0e-10
#ifndef M_SQRT3
#define M_SQRT3    1.73205080756887729352744634151      /* sqrt(3) */
#endif
//---------------------------------------------------------------------------
// Error handling...
//---------------------------------------------------------------------------
#define NOERROR 0
#define CALCOFV 1
#define STEPUND 2
#define STEPLTL 3
#define STEPMNY 4
#define NOMATRIX 5
#define SINGULARMTRX 6
#define STEPNSIG 7
#define ERRMAXTRY 8
#define NOMEM  9
#define NOSORTSTACK 10
#define HEAPWRONGM 11
#define UNKNOWN 12

#define MAXERRORS 13
const char *IntErrors[MAXERRORS]=
	{
	 "No error",
	 "Calculation overflow",
	 "Stepsize underflow in Variable Step RK",
    	 "Stepsize too small in Variable Step RK",
    	 "Too many steps in Variable Step RK",
    	 "Wrong size matrix in LU decomposition",
    	 "Singular matrix in LU decomposition",
    	 "Stepsize not significant in Stiff",
    	 "Exceeded # of tries in Stiff",
    	 "Out of memory for plot",
    	 "Out of stack space in sort",
    	 "Search criteria for HeapSelect too large",
    	 "Unknown error"
	};

#define NUMINTEGRATORS 10

#define MAP		0
#define EULER	1
#define GEAR1	2
#define RK2IMP	3
#define RK2		4
#define RK4IMP	5
#define RK4		6
#define RK45		7
#define RK8		8
#define RKVAR	9

const char *IntegratorNames[NUMINTEGRATORS]=
	{
         "Map",
	 "Euler",
	 "Gear1",
	 "Implicit-RK2",
	 "Runge-Kutta2",
	 "Implicit-RK4",
	 "Runge-Kutta",
	 "Fehlberg-RK",
	 "Prince-Dormand-RK",
	 "Cash-Karp-RK"
	};
	
#ifndef SPINNER
const char *FailLock="Failed to lock memory in Integrator thread";
#endif
//-----------------------------------------------------------------------------------------------------------------------------------
struct stdfabs : public unary_function<double, double>
{
  double operator()(double x) { return fabs(x); }
};

struct stdlog : public unary_function<double, double>
{
  double operator()(double x) { return log(x); }
};
//-----------------------------------------------------------------------------------------------------------------------------------
Integrator::Integrator() : QThread()
{
#ifdef SPINNER
 spinner=NULL;
#else
 pMutex=NULL;
#endif
 equationObject=NULL;
 nIntegrat=RK4;
 eps=2.0e-8;
 nError=NOERROR;
 bError=false;
 bReset=false;
 bStlError=false;
 bLyapunov=false;
 bLyapRestart=true;
 bLocked=false;
 N=0;
 Nlyap=0;
 tbegin=0.0;
 tstart=0.0;
 tend=10000.0;
 h=0.01;
 horg=h;
 nLyapMod=20;
 lyapdist=LYAPUNOV_DIST;
 lyapdim=0.0;
 uNumSys=1;
 CalcMaxEst();
}
//---------------------------------------------------------------------------
Integrator::~Integrator()
{
 if (yscal.size()>0)
 	 yscal.clear();
}
//---------------------------------------------------------------------------
void  Integrator::run()
{
 bStopped=false;
 switch (nIntegrat)
         {
          case RKVAR:
               {
              	OdeIntVar();
                break;
               }
	  case RK8:
	  case RK45:
          case RK4:
	  case RK4IMP:
	  case RK2:
	  case RK2IMP:
	  case GEAR1:
          case EULER:
          case MAP:
          default:
               {
              	OdeIntFixed();
                break;
               }
         }
  Done();
 #ifdef _DEBUG
  qDebug("[Integrator::run] Used %d, done.",nIntegrat);
 #endif
}
//---------------------------------------------------------------------------
#ifdef SPINNER
void Integrator::registerObjects(NS_Equation::Equation *Equation, NS_DataSpinner::TDataSpinner *spinObject)
#else
void Integrator::registerObjects(NS_Equation::Equation *Equation, QMutex *p)
#endif
{
#ifdef SPINNER
 if (spinObject)
   {
    spinner=spinObject;
   }
#else
 if (p)
 	pMutex=p;
#endif
 if (Equation)
    {
     equationObject=Equation;
     N=equationObject->GetN();
     Init();
    }
}
//---------------------------------------------------------------------------
int Integrator::GetNumIntegrators()
{
 return NUMINTEGRATORS;
}
//---------------------------------------------------------------------------
const char *Integrator::GetIntegratorName(int idx)
{
 if ((idx>=0)&&(idx<NUMINTEGRATORS))
 	return IntegratorNames[idx];
 return NULL;
}
//---------------------------------------------------------------------------
const char *Integrator::GetIntegratorName()
{
 if (nIntegrat<NUMINTEGRATORS)
 	return IntegratorNames[nIntegrat];
 return NULL;
}
//---------------------------------------------------------------------------
void  Integrator::CalcMaxEst()
{
 if (h>0.0)
 {
  switch(nIntegrat)
  {
   default:
   	{
	 uMaxEst=(unsigned int)(ceil((tend-tstart)/h));
	 break;
	}
   case RKVAR:
   	{//in this case uMaxEst is ussually a worst case scenario, so make it proportional to t
	 if ((t!=0.0)&&(uStep>0))
	 	uMaxEst=(unsigned int)(ceil(((tend-tstart)/t)*uStep));
	 else
	 	uMaxEst=(unsigned int)(ceil((tend-tstart)/h));
	 break;
	}
  }
 }
}
//---------------------------------------------------------------------------
void  Integrator::Start()
{
 if (!running())
 {
   bStopped=false;
   start();
 }
}
//---------------------------------------------------------------------------
void  Integrator::Stop()
{
 if (running())
 {
   tstart=t; //save current step
   bStopped=true;
 }
}
//---------------------------------------------------------------------------
void  Integrator::Init()
{
 unsigned int i;
 double d;
 
 t=tbegin;
 tstart=tbegin;
 uStep=0;
 uLyapSize=0;
 bReset=false;
 dvValues.clear();
 dvNewValues.clear();
 if (equationObject!=NULL)
 {
  N=equationObject->GetN();
  for (i=0;i<N;i++)
    {
     d=equationObject->GetVarValue(i);
     dvValues.push_back(d);
     dvNewValues.push_back(d);
    }
  dvOldValues.assign(N,0.0);
  dvStepValues.assign(N,0.0);
  dvDerived.assign(N,0.0);
  dvError.assign(N,0.0);
  yscal.assign(N,0.0);
  Nlyap=equationObject->GetNumDiffMap();
  dvLyapVals.assign(Nlyap,0.0);
  dvLyapNewVals.assign(Nlyap,0.0);
  dvLyapSum.assign(Nlyap,0.0);
  dvLyapAvg.assign(Nlyap,0.0);
#ifndef SPINNER
  vddLyap.clear();
  for (i=0;i<Nlyap+1;i++)
  	vddLyap.push_back(dblDeque());
#endif
  equationObject->DoRegister();
 }
}
//---------------------------------------------------------------------------
const char *Integrator::GetErrorStr()
{
 if (bStlError)
  return sException.c_str();
 if (nError<MAXERRORS)
   return IntErrors[nError];
 return NULL;
}
//---------------------------------------------------------------------------
void  Integrator::DoReset()
{
 if (!lockMutex())
 	return;
 bError=false;
 bStlError=false;
 bLyapRestart=true;
 nError=NOERROR;
 
 uStep=0;
 uLyapSize=0;
 t=tbegin;
 tstart=tbegin;
 CalcMaxEst();

 equationObject->Reset();
 pControl->resetSockets();
 ReloadValues();
 dvNewValues.assign(N,0.0);
 dvDerived.assign(N,0.0);
 dvOldValues.assign(N,0.0);
 dvStepValues.assign(N,0.0);
 dvLyapSum.assign(Nlyap,0.0);
 dvLyapAvg.assign(Nlyap,0.0);
 dvError.assign(N,0.0);
 yscal.assign(N,0.0);
#ifndef SPINNER
 for (unsigned int i=0;i<vddLyap.size();i++)
    vddLyap[i].erase(vddLyap[i].begin(),vddLyap[i].end());
#endif
 unlockMutex();
 bReset=false;
}
//---------------------------------------------------------------------------
void  Integrator::Reset()
{
 bReset=true;
 if (!running())
    DoReset();
}
//---------------------------------------------------------------------------
void  Integrator::ReloadValues()
{
 double d;
 
 for (unsigned int i=0;i<N;i++)  
 {
  d=equationObject->GetVarValue(i);
  dvValues[i]=d;
 }
 fill(dvLyapVals.begin(),dvLyapVals.end(),0.0);
 fill(dvLyapNewVals.begin(),dvLyapNewVals.end(),0.0);
}
//---------------------------------------------------------------------------
bool Integrator::lockMutex()
{
 if (!pMutex)
 	return false;
 if (bLocked)
	return true; 
 
  bLocked=pMutex->tryLock();
 if (!bLocked)
 {//could not lock at this point,try for INT_LOCK_TRY times for INT_LOCK_MS (=n*m s) to lock it
   int i=0;
   while((i<INT_LOCK_TRY)&&(!bLocked))
   {
    //wait for 50ms
    msleep(INT_LOCK_MS);
    bLocked=pMutex->tryLock();
    i++;
   }
  }
#ifdef _DEBUG
//  qDebug("[Integrator::lockMutex] locked=%d", bLocked);
#endif
 return bLocked; 
}
//---------------------------------------------------------------------------
void Integrator::unlockMutex()
{
 if ((!pMutex)||(!bLocked))
	return;
 pMutex->unlock();
 bLocked=false;
#ifdef _DEBUG
//  qDebug("[Integrator::unlockMutex] locked=%d", bLocked);
#endif
}
//---------------------------------------------------------------------------
// Integration engines
//---------------------------------------------------------------------------
void  Integrator::doLyapStep()
{
 unsigned int i,j;

   //firstly, backup the real step data
 copy(dvValues.begin(),dvValues.end(),dvOldValues.begin());
 copy(dvNewValues.begin(),dvNewValues.end(),dvStepValues.begin());
 
 if (bLyapRestart)
 { //if we are at the beginning of a lyap range
  //create lyapunov distance
  j=0;
  for (i=0;i<dvValues.size();i++)
  {
   if (equationObject->IsDiffMap(i))
   {
   	dvValues[i]+=SIGN(lyapdist,dvValues[i]);
	 //keep the difference with the initial real step (d0)
   	dvLyapVals[j]=dvValues[i]-dvOldValues[i];
	j++;
   }
  }
  bLyapRestart=false;
 }
 else
 { //in the middle of a range
  j=0;
  for (i=0;i<dvValues.size();i++)
  {
   if (equationObject->IsDiffMap(i))
   {
    dvValues[i]=dvLyapNewVals[j];
    j++;
   }
  }
 }
 //step
 equationObject->EvaluateAll(dvValues, dvDerived, dvNewValues);
 switch (nIntegrat)
         {
          case MAP:
               {//for a map the newvalues replace the old values
	        for (unsigned int i=0; i<N; i++)
		{
	         if (!equationObject->IsImmediate(i))
	           dvNewValues[i]=dvDerived[i];
		}
                break;
               }
          case RK8:
               {
                rkpd();
                break;
               }
          case RK45:
               {
                rk45();
                break;
               }
          case RK4:
               {
                rk4();
                break;
               }
          case RK4IMP:
               {
                rk4imp();
                break;
               }
	  case RK2:
	       {
	        rk2();
		break;
	       }
	  case RK2IMP:
	       {
	        rk2imp();
		break;
	       }
          case GEAR1:
               {
              	Gear1();
                break;
	       }
          case EULER:
               {
              	Euler();
                break;
	       }
          case RKVAR:
               {
         	rkqs();
                break;
               }
          default:
              {//apparently we are using a different integrator, return
                return;
              }
         }
 if ((uStep%nLyapMod)==0)
	{ //have done enough steps, calc avg
 	 //calculate log(|d1/d0|)
	 //	d1=v_lyap-v_org
	j=0;
  	for (i=0;i<dvValues.size();i++)
  	{
   	 if (equationObject->IsDiffMap(i))
   	 {
    	  dvLyapNewVals[j]=dvNewValues[i]-dvStepValues[i];
    	  j++;
   	 }
  	}
#ifdef SPINNER
  	 uLyapSize=spinner->getLyapSize();
#else
	uLyapSize=vddLyap[0].size();
#endif
  	//	divide d1 by d0 
 	transform(dvLyapNewVals.begin(),dvLyapNewVals.end(),dvLyapVals.begin(),dvLyapNewVals.begin(),divides<double>());
 	//	make absolute values
 	transform(dvLyapNewVals.begin(),dvLyapNewVals.end(),dvLyapNewVals.begin(),stdfabs());
 	//	do logarithm
 	transform(dvLyapNewVals.begin(),dvLyapNewVals.end(),dvLyapNewVals.begin(),stdlog());
 	// 	add to previous values
 	transform(dvLyapSum.begin(),dvLyapSum.end(),dvLyapNewVals.begin(),dvLyapSum.begin(),plus<double>());
	//calc running average
 	transform(dvLyapSum.begin(),dvLyapSum.end(),dvLyapAvg.begin(),bind2nd(divides<double>(), uLyapSize));
	//	calc lyapunov dimension estimate
	//	first copy into array to sort, dvLyapVals is not needed anymore at this point
	copy(dvLyapAvg.begin(),dvLyapAvg.end(),dvLyapVals.begin());
	//	sort them in descending order
	sort(dvLyapVals.begin(),dvLyapVals.end());
	reverse(dvLyapVals.begin(),dvLyapVals.end());
	//	find the zero crossing
	double d1,d2,dsum;
	d1=dvLyapVals[0];
	d2=0.0;
	j=1;
	for (i=1;i<dvLyapVals.size();i++)
		{
		 d2=dvLyapVals[i];
		 if (d2>0.0)
		 	{
		 	 d1=d2;
			 j++;
			 continue;
			}
		 if (fabs(d1)<fabs(d2))
		 	{ // d1 is nearest to zero
			 break;
			}
		 else
		 	{ // d2 is nearest to zero
			 d1=d2;
			 j++;
			 continue;
			}
		}
	// calc estimate:
	dsum=0.0;
	for (i=0;i<j;i++)
		dsum+=fabs(dvLyapVals[i]);
	if (uNumSys>1)
		dsum=dsum/(double)uNumSys;
//	lyapdim=j+(dsum/fabs(d2));
	lyapdim=(j-1)+(dsum/fabs(d2));
/*#ifdef _DEBUG
	qDebug("[ Integrator::doLyapStep] j=%d, d1=%f, d2=%f, dsum=%f, luapdim=%f",j,d1,d2,dsum,lyapdim);
	for (i=0;i<dvLyapVals.size();i++)
		qDebug("[ Integrator::doLyapStep] lambda %d=%f",i+1,dvLyapVals[i]);
#endif*/
	 //store values
#ifdef SPINNER
	spinner->storeLyap(static_cast<TProducer>(this),t,dvLyapNewVals);
#else
	if (!lockMutex())
		throw runtime_error(FailLock);
	vddLyap[0].push_back(t);
	for (i=0;i<dvLyapNewVals.size();i++)
		vddLyap[i+1].push_back(dvLyapNewVals[i]);
	unlockMutex();
#endif
	bLyapRestart=true;
	}
 else
 	{//keep the values
	  j=0;
  	 for (i=0;i<dvNewValues.size();i++)
  	 {
   	  if (equationObject->IsDiffMap(i))
   	  {
    	   dvLyapNewVals[j]=dvNewValues[i];
    	   j++;
   	  }
  	 }
	}
 //lastly, restore the values for the next step
 copy(dvStepValues.begin(),dvStepValues.end(),dvValues.begin());
}

void  Integrator::OdeIntFixed()
{
 try
 {
 // Correction 25/2/2005 ToS
  // Added initial state to list of values
  // The point added is at evolution t+h (the next point!)
  // This should fix the problem that when t=0, T'=1 becomes T=0.1 when h=0.1 instead of T=0.1 only when evolution=0.1
// equationObject->DoneStep(t,dvValues,dvError);
#ifdef SPINNER
        spinner->storeData(static_cast<TProducer>(this),t,dvValues,dvError);
#else
	 if (!lockMutex())
		throw runtime_error(FailLock);
        equationObject->DoneStep(tstart,dvValues,dvError);
	pControl->updateSockets(tstart);
	unlockMutex();
#endif
  for (t=tstart;t<tend;t+=h)
 	{
         if (bReset)
         {
          DoReset();
          continue;
         }
         equationObject->EvaluateAll(dvValues, dvDerived, dvNewValues);
         switch (nIntegrat)
         {
          case MAP:
               {//for a map the newvalues replace the old values
	        for (unsigned int i=0; i<N; i++)
		{
	         if (!equationObject->IsImmediate(i))
	           dvNewValues[i]=dvDerived[i];
		}
                break;
               }
          case RK8:
               {
                rkpd();
                break;
               }
          case RK45:
               {
                rk45();
                break;
               }
          case RK4:
               {
                rk4();
                break;
               }
          case RK4IMP:
               {
                rk4imp();
                break;
               }
	  case RK2:
	       {
	        rk2();
		break;
	       }
	  case RK2IMP:
	       {
	        rk2imp();
		break;
	       }	       
          case GEAR1:
               {
              	Gear1();
                break;
	       }
          case EULER:
               {
              	Euler();
                break;
	       }
          default:
              	{//apparently we are using a different integrator, return
                 return;
                }
         }	 //some error occured
	 if (bError)
	  return;
	 //try to lock mutex, and store values
#ifdef SPINNER
        spinner->storeData(static_cast<TProducer>(this),t+h,dvNewValues,dvError);
#else
	 if (!lockMutex())
		throw runtime_error(FailLock);
        equationObject->DoneStep(t+h,dvNewValues,dvError);
	pControl->updateSockets(t+h);
	unlockMutex();
#endif
	//values contains the previous step values
	//newvalues the new values
	if (bLyapunov)
		doLyapStep();
	else
	{//update values for next step
          copy(dvNewValues.begin(),dvNewValues.end(),dvValues.begin());
	}
        uStep++;
	//stopped
        if (bStopped)
       	  return;
    }
  uMaxEst=uStep;
 }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

void  Integrator::OdeIntVar()
{
 uint64_t nstp,mxstp,i;

 try
 {
  h=SIGN(h,tend-tbegin);
  if (h==0.0)
 	h=0.001;
  mxstp=(unsigned int)ceil(fabs((tend-tbegin)/h)+1);
  // Correction 25/2/2005 ToS
  // Changed initial start time to t=tbegin+h
  // Added initial state to list of values
  // This should fix the problem that when t=0, T'=1 becomes T=0.1 when h=0.1 instead of T=0.1 only when evolution=0.1
  // This does not work properly under varstep integrators as it can not predict the correct step size
//  equationObject->DoneStep(t,dvValues,dvError);
#ifdef SPINNER
        spinner->storeData(static_cast<TProducer>(this),t,dvValues,dvError);
#else
	 if (!lockMutex())
		throw runtime_error(FailLock);
        equationObject->DoneStep(t,dvValues,dvError);
	pControl->updateSockets(t);
	unlockMutex();
#endif
  for (nstp=1;nstp<=mxstp;nstp++)
   {
     if (bReset)
      {
       DoReset();
       h=SIGN(h,tend-tbegin);
       if (h==0.0)
	    h=0.001;
       continue;
      }
     equationObject->EvaluateAll(dvValues, dvDerived, dvNewValues);
     //scaling used to monitor accuracy
     for (i=0;i<N;i++)
         	yscal[i]=fabs(dvValues[i])+fabs(dvDerived[i]*h)+TINY;
     //if stepsize can overshoot, decrease
     if ((t+h-tend)*(t+h-tbegin)>0.0)
         	h=tend-t;
     switch (nIntegrat)
         {
          case RKVAR:
            {
         	 rkqs();
             break;
            }
          default:
          	{//apparently we are using a different integrator, return
             return;
            }
         }
#ifdef SPINNER
        spinner->storeData(static_cast<TProducer>(this),t+hnext,dvNewValues,dvError);
#else
	 //try to lock mutex
	 if (!lockMutex())
		throw runtime_error(FailLock);
        equationObject->DoneStep(t+hnext,dvNewValues,dvError);
	pControl->updateSockets(t+hnext);
	unlockMutex();
#endif
	if (bLyapunov)
		doLyapStep();
	else
	{//update values for next step
          copy(dvNewValues.begin(),dvNewValues.end(),dvValues.begin());
	}	
	uStep++;
     //are we done?
     if (((t-tend)*(tend-tbegin)>=0.0)||(bStopped))
      	{
         bError=false;
	 uMaxEst=uStep;
         return;
        }
     //stepsize too small
     if (fabs(hnext)<=TINY)
        {
         nError=STEPLTL;
         throw runtime_error(IntErrors[nError]);
        }
     if (bError)
        return;
     h=hnext;
     CalcMaxEst(); //estimate the total number of steps
    }
  //too many steps
  nError=STEPMNY;
  throw runtime_error(IntErrors[nError]);
 }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//---------------------------------------------------------------------------
// Integration routines
//---------------------------------------------------------------------------

// Euler type of integration
void  Integrator::Euler()
{
 unsigned int i;
 
 try 
    {
      for (i=0; i<N; i++)
      {
        if (!equationObject->IsImmediate(i))
		dvNewValues[i]+=(h*dvDerived[i]);
       }
      for (i=0; i<N; i++)
    	 dvError[i] = h * h * dvDerived[i];
     }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

// Gear 1 type of integration
void  Integrator::Gear1()
{
 const unsigned int num_steps = 3;
 unsigned int i,j;
 
 try 
    {
      dblVector yt(N);
      dblVector dyt(N);
    
      for (i=0; i<N; i++)
         yt[i]=dvValues[i];
      for (j=0;j<num_steps;j++)
      {
        equationObject->Evaluate(yt,dyt);
        for (i=0; i<N; i++)
           yt[i]=dvValues[i]+(h*dyt[i]);
      }
      for (i=0; i<N; i++)
      {
         if (!equationObject->IsImmediate(i))
		dvNewValues[i]=yt[i];       
    	 dvError[i] = h * h * dyt[i];
       }
     }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//  Second/third order Runge-Kutta 
//  rk2()  : 2nd Runge-Kutta
// GNU Scientific Library 1.4 : file rk2.c
void Integrator::rk2()
{
  unsigned int i;
  
  try
    {
      dblVector ak1(N);
      dblVector ak2(N);
      dblVector ak3(N);
      dblVector ytemp(N);

      // ak1 step
     for (i=0;i<N;i++)
 		ak1[i]=dvDerived[i];

     for (i=0; i<N; i++)
        ytemp[i] = dvValues[i] + 0.5 * h * ak1[i];

    // ak2 step
     equationObject->Evaluate(ytemp, ak2);
     
     for (i=0; i<N; i++)
       ytemp[i] = dvValues[i] + h * (-ak1[i] + 2.0 * ak2[i]);

    // ak3 step
     equationObject->Evaluate(ytemp, ak3);
  
    // final sum and error estimate
     for (i=0; i<N; i++)
     {
      const double ksum3 = (ak1[i] + 4.0 * ak2[i] + ak3[i]) / 6.0;
      if (!equationObject->IsImmediate(i))
		dvNewValues[i] = dvValues[i] + h * ksum3;
      dvError[i] = h * (ak2[i] - ksum3);
     }
   }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//  Second/third order Runge-Kutta Implicit
//  rk2()  : 2nd Runge-Kutta
// GNU Scientific Library 1.4 : file rk2imp.c
void Integrator::rk2imp()
{
  const unsigned int num_steps = 3;
  unsigned int i,j;
  
  try
  {
    dblVector ak(N);
    dblVector ytemp(N);
      
    // ak step
    for (i=0;i<N;i++)
 		ak[i]=dvDerived[i];
    // iterative solution
   for (j= 0; j< num_steps; j++)
    {
       for (i=0;i<N;i++)
          ytemp[i] = dvValues[i] + 0.5 * h * ak[i];
      equationObject->Evaluate(ytemp, ak);
    }

     // assign result
     for (i=0; i<N; i++)
     {
        if (!equationObject->IsImmediate(i))
		dvNewValues[i] = dvValues[i] + h * ak[i];
       dvError[i] = h * h * ak[i];
     }
   }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//Fourth order Runge-Kutta Integration
// Press et al. pp 713-714
void  Integrator::rk4()
{
 unsigned int i;
 double hh,h6;

 try {
 	dblVector dym(N);
	dblVector dyt(N);
	dblVector yt(N);

	hh=h*0.5;
	h6=h/6.0;
 //Step one
	for (i=0;i<N;i++)
 		yt[i]=dvValues[i]+hh*dvDerived[i];
    equationObject->Evaluate(yt, dyt);
 //step two
	for (i=0;i<N;i++)
 		yt[i]=dvValues[i]+hh*dyt[i];
    equationObject->Evaluate(yt, dym);
 //Step three
	for (i=0;i<N;i++)
 	{
         yt[i]=dvValues[i]+h*dym[i];
         dym[i]+=dyt[i];
        }
    equationObject->Evaluate(yt, dyt);
 //Step four
	for (i=0;i<N;i++)
        {
         if (!equationObject->IsImmediate(i))
		    dvNewValues[i]=dvValues[i]+h6*(dvDerived[i]+dyt[i]+2.0*dym[i]);
        }
 }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//  Fourth/Fifth order Runge-Kutta Implicit
//  rk4imp()  : 4nd Runge-Kutta
// GNU Scientific Library 1.4 : file rk4imp.c
void Integrator::rk4imp()
{
  const unsigned int num_steps = 3;
  const double ir3 = 1.0 / M_SQRT3;
  unsigned int i,j;

  try
    {
      dblVector ak1(N);
      dblVector ak2(N);
      dblVector ytemp1(N);
      dblVector ytemp2(N);
      
      // ak1 step
     for (i=0;i<N;i++)
     	{
 	  ak1[i]=dvDerived[i];
	  ak2[i]=dvDerived[i];
	}

  // iterative solution
  for (j=0; j<num_steps; j++)
    {
     for (i=0;i<N;i++)
        {
          ytemp1[i] =dvValues[i] + h * (0.25 * ak1[i] + 0.5 * (0.5 - ir3) * ak2[i]);
          ytemp2[i] =dvValues[i] + h * (0.25 * ak2[i] + 0.5 * (0.5 + ir3) * ak1[i]);
        }
     equationObject->Evaluate(ytemp1, ak1);
     equationObject->Evaluate(ytemp2, ak2);
    }

  // assignment 
  for (i=0;i<N;i++)
    {
      const double d_i = 0.5 * (ak1[i] + ak2[i]);
      if (!equationObject->IsImmediate(i))
      	dvNewValues[i] = dvValues[i] + h * d_i;
      dvError[i] = h * h * d_i;    /* FIXME: is this an overestimate ? */
    }
   }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

/* Runge-Kutta-Fehlberg constants */
static const double FE_ah[] = { 1.0/4.0, 3.0/8.0, 12.0/13.0, 1.0, 1.0/2.0 };
static const double FE_b3[] = { 3.0/32.0, 9.0/32.0 };
static const double FE_b4[] = { 1932.0/2197.0, -7200.0/2197.0, 7296.0/2197.0};
static const double FE_b5[] = { 8341.0/4104.0, -32832.0/4104.0, 29440.0/4104.0, -845.0/4104.0};
static const double FE_b6[] = { -6080.0/20520.0, 41040.0/20520.0, -28352.0/20520.0, 9295.0/20520.0, -5643.0/20520.0};
static const double FE_c1 = 902880.0/7618050.0;
static const double FE_c3 = 3953664.0/7618050.0;
static const double FE_c4 = 3855735.0/7618050.0;
static const double FE_c5 = -1371249.0/7618050.0;
static const double FE_c6 = 277020.0/7618050.0;


static const double FE_ec[] = { 0.0,
  1.0 / 360.0,
  0.0,
  -128.0 / 4275.0,
  -2197.0 / 75240.0,
  1.0 / 50.0,
  2.0 / 55.0
};

// Runge-Kutta-Fehlberg 4(5)
void Integrator::rk45()
{
  unsigned int i;

  dblVector ytemp(N);
  dblVector ak1(N);  
  dblVector ak2(N);  
  dblVector ak3(N);  
  dblVector ak4(N);  
  dblVector ak5(N);  
  dblVector ak6(N);  

  /* k1 step */
  equationObject->Evaluate(dvValues, ak1);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] +  FE_ah[0] * h * ak1[i];
  
  /* k2 step */
  equationObject->Evaluate(ytemp, ak2);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] +  h * (FE_b3[0] * ak1[i] + FE_b3[1] * ak2[i]);
    
  /* k3 step */
  equationObject->Evaluate(ytemp, ak3);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (FE_b4[0] * ak1[i] + FE_b4[1] * ak2[i] + FE_b4[2] * ak3[i]);

  /* k4 step */  
  equationObject->Evaluate(ytemp, ak4);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (FE_b5[0] * ak1[i] + FE_b5[1] * ak2[i] + FE_b5[2] * ak3[i] + FE_b5[3] * ak4[i]);

  /* k5 step */
  equationObject->Evaluate(ytemp, ak5);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (FE_b6[0] * ak1[i] + FE_b6[1] * ak2[i] + FE_b6[2] * ak3[i] + FE_b6[3] * ak4[i] + FE_b6[4] * ak5[i]);

  /* k6 step and final sum */
  equationObject->Evaluate(ytemp, ak6);
  for (i=0;i<N;i++)
    {
      const double d_i = FE_c1 * ak1[i] + FE_c3 * ak3[i] + FE_c4 * ak4[i] + FE_c5 * ak5[i] + FE_c6 * ak6[i];
      if (!equationObject->IsImmediate(i))
		dvNewValues[i] = dvValues[i] + h * d_i;
    }

  /* difference between 4th and 5th order */
  for (i=0;i<N;i++)
    dvError[i] = h * (FE_ec[1] * ak1[i] + FE_ec[3] * ak3[i] + FE_ec[4] * ak4[i] + FE_ec[5] * ak5[i] + FE_ec[6] * ak6[i]);
}

// Prince-Dormand constants
static const double PD_Abar[] = {
  14005451.0 / 335480064.0,
  0.0,
  0.0,
  0.0,
  0.0,
  -59238493.0 / 1068277825.0,
  181606767.0 / 758867731.0,
  561292985.0 / 797845732.0,
  -1041891430.0 / 1371343529.0,
  760417239.0 / 1151165299.0,
  118820643.0 / 751138087.0,
  -528747749.0 / 2220607170.0,
  1.0 / 4.0
};

static const double PD_A[] = {
  13451932.0 / 455176623.0,
  0.0,
  0.0,
  0.0,
  0.0,
  -808719846.0 / 976000145.0,
  1757004468.0 / 5645159321.0,
  656045339.0 / 265891186.0,
  -3867574721.0 / 1518517206.0,
  465885868.0 / 322736535.0,
  53011238.0 / 667516719.0,
  2.0 / 45.0
};

static const double PD_ah[] = {
  1.0 / 18.0,
  1.0 / 12.0,
  1.0 / 8.0,
  5.0 / 16.0,
  3.0 / 8.0,
  59.0 / 400.0,
  93.0 / 200.0,
  5490023248.0 / 9719169821.0,
  13.0 / 20.0,
  1201146811.0 / 1299019798.0
};

static const double PD_b21 = 1.0 / 18.0;
static const double PD_b3[] = { 1.0 / 48.0, 1.0 / 16.0 };
static const double PD_b4[] = { 1.0 / 32.0, 0.0, 3.0 / 32.0 };
static const double PD_b5[] = { 5.0 / 16.0, 0.0, -75.0 / 64.0, 75.0 / 64.0 };
static const double PD_b6[] = { 3.0 / 80.0, 0.0, 0.0, 3.0 / 16.0, 3.0 / 20.0 };
static const double PD_b7[] = {
  29443841.0 / 614563906.0,
  0.0,
  0.0,
  77736538.0 / 692538347.0,
  -28693883.0 / 1125000000.0,
  23124283.0 / 1800000000.0
};
static const double PD_b8[] = {
  16016141.0 / 946692911.0,
  0.0,
  0.0,
  61564180.0 / 158732637.0,
  22789713.0 / 633445777.0,
  545815736.0 / 2771057229.0,
  -180193667.0 / 1043307555.0
};
static const double PD_b9[] = {
  39632708.0 / 573591083.0,
  0.0,
  0.0,
  -433636366.0 / 683701615.0,
  -421739975.0 / 2616292301.0,
  100302831.0 / 723423059.0,
  790204164.0 / 839813087.0,
  800635310.0 / 3783071287.0
};
static const double PD_b10[] = {
  246121993.0 / 1340847787.0,
  0.0,
  0.0,
  -37695042795.0 / 15268766246.0,
  -309121744.0 / 1061227803.0,
  -12992083.0 / 490766935.0,
  6005943493.0 / 2108947869.0,
  393006217.0 / 1396673457.0,
  123872331.0 / 1001029789.0
};
static const double PD_b11[] = {
  -1028468189.0 / 846180014.0,
  0.0,
  0.0,
  8478235783.0 / 508512852.0,
  1311729495.0 / 1432422823.0,
  -10304129995.0 / 1701304382.0,
  -48777925059.0 / 3047939560.0,
  15336726248.0 / 1032824649.0,
  -45442868181.0 / 3398467696.0,
  3065993473.0 / 597172653.0
};
static const double PD_b12[] = {
  185892177.0 / 718116043.0,
  0.0,
  0.0,
  -3185094517.0 / 667107341.0,
  -477755414.0 / 1098053517.0,
  -703635378.0 / 230739211.0,
  5731566787.0 / 1027545527.0,
  5232866602.0 / 850066563.0,
  -4093664535.0 / 808688257.0,
  3962137247.0 / 1805957418.0,
  65686358.0 / 487910083.0
};
static const double PD_b13[] = {
  403863854.0 / 491063109.0,
  0.0,
  0.0,
  -5068492393.0 / 434740067.0,
  -411421997.0 / 543043805.0,
  652783627.0 / 914296604.0,
  11173962825.0 / 925320556.0,
  -13158990841.0 / 6184727034.0,
  3936647629.0 / 1978049680.0,
  -160528059.0 / 685178525.0,
  248638103.0 / 1413531060.0,
  0.0
};

//  Eigth order Runge-Kutta Prince-Dormand
//  rkpd()  :  Runge-Kutta
// GNU Scientific Library 1.4 : file rk8pd.c
void  Integrator::rkpd()
{
  unsigned int i;
  //double *const ytmp = state->ytmp;
  
  dblVector ytemp(N);
  dblVector ak1(N);
  dblVector ak2(N);
  dblVector ak3(N);
  dblVector ak4(N);
  dblVector ak5(N);
  dblVector ak6(N);
  dblVector ak7(N);
  dblVector ak8(N);
  dblVector ak9(N);
  dblVector ak10(N);
  dblVector ak11(N);
  dblVector ak12(N);
  dblVector ak13(N);

  /* ak1 step */
  equationObject->Evaluate(dvValues, ak1);
  for (i=0;i<N;i++)
 	ytemp[i]=dvValues[i]+PD_b21*h*ak1[i];
	
  /* ak2 step */
  equationObject->Evaluate(ytemp, ak2);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b3[0] * ak1[i] + PD_b3[1] * ak2[i]);

  /* k3 step */
  equationObject->Evaluate(ytemp, ak3);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b4[0] * ak1[i] + PD_b4[2] * ak3[i]);

  /* k4 step */
  equationObject->Evaluate(ytemp, ak4);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b5[0] * ak1[i] + PD_b5[2] * ak3[i] + PD_b5[3] * ak4[i]);
    
  /* k5 step */    
  equationObject->Evaluate(ytemp, ak5);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b6[0] * ak1[i] + PD_b6[3] * ak4[i] + PD_b6[4] * ak5[i]);
    
  /* k6 step */
  equationObject->Evaluate(ytemp, ak6);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b7[0] * ak1[i] + PD_b7[3] * ak4[i] + PD_b7[4] * ak5[i] + PD_b7[5] * ak6[i]);

  /* k7 step */
  equationObject->Evaluate(ytemp, ak7);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b8[0] * ak1[i] + PD_b8[3] * ak4[i] + PD_b8[4] * ak5[i] + PD_b8[5] * ak6[i] + PD_b8[6] * ak7[i]);

  /* k8 step */
  equationObject->Evaluate(ytemp, ak8);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] +  h * (PD_b9[0] * ak1[i] + PD_b9[3] * ak4[i] + PD_b9[4] * ak5[i] + PD_b9[5] * ak6[i] + PD_b9[6] * ak7[i] + PD_b9[7] * ak8[i]);
    
  /* k9 step */
  equationObject->Evaluate(ytemp, ak9);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b10[0] * ak1[i] + PD_b10[3] * ak4[i] + PD_b10[4] * ak5[i] + PD_b10[5] * ak6[i] + PD_b10[6] * ak7[i] + PD_b10[7] * ak8[i] + PD_b10[8] * ak9[i]);

  /* k10 step */
  equationObject->Evaluate(ytemp, ak10);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b11[0] * ak1[i] + PD_b11[3] * ak4[i] + PD_b11[4] * ak5[i] + PD_b11[5] * ak6[i] +
    						     PD_b11[6] * ak7[i] + PD_b11[7] * ak8[i] + PD_b11[8] * ak9[i] + PD_b11[9] * ak10[i]);

  /* k11 step */
  equationObject->Evaluate(ytemp, ak11);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b12[0] * ak1[i] + PD_b12[3] * ak4[i] + PD_b12[4] * ak5[i] + PD_b12[5] * ak6[i] + PD_b12[6] * ak7[i] +
    						     PD_b12[7] * ak8[i] + PD_b12[8] * ak9[i] + PD_b12[9] * ak10[i] + PD_b12[10] * ak11[i]);

  /* k12 step */
  equationObject->Evaluate(ytemp, ak12);
  for (i=0;i<N;i++)
    ytemp[i] = dvValues[i] + h * (PD_b13[0] * ak1[i] + PD_b13[3] * ak4[i] + PD_b13[4] * ak5[i] + PD_b13[5] * ak6[i] + PD_b13[6] * ak7[i] + PD_b13[7] * ak8[i] +
                  				     PD_b13[8] * ak9[i] + PD_b13[9] * ak10[i] + PD_b13[10] * ak11[i] + PD_b13[11] * ak12[i]);

  /* k13 step */
  equationObject->Evaluate(ytemp, ak13);

  /* final sum and error estimate */
  for (i=0;i<N;i++)
    {
      const double ksum8 =
        PD_Abar[0] * ak1[i] + PD_Abar[5] * ak6[i] + PD_Abar[6] * ak7[i] +
        PD_Abar[7] * ak8[i] + PD_Abar[8] * ak9[i] + PD_Abar[9] * ak10[i] +
        PD_Abar[10] * ak11[i] + PD_Abar[11] * ak12[i] + PD_Abar[12] * ak13[i];
      const double ksum7 =
        PD_A[0] * ak1[i] + PD_A[5] * ak6[i] + PD_A[6] * ak7[i] + PD_A[7] * ak8[i] +
        PD_A[8] * ak9[i] + PD_A[9] * ak10[i] + PD_A[10] * ak11[i] + PD_A[11] * ak12[i];
        if (!equationObject->IsImmediate(i))
		dvNewValues[i] = dvValues[i] + h * ksum8;
       dvError[i] = h * (ksum7 - ksum8);
    }
}

//Fifth-order Runge-Kutta step with monitoring of local truncation error to
//ensure accuracy and adjust stepsize.
// Press et al. pp 719
void  Integrator::rkqs()
{
 unsigned int i;
 double errmax, hl, htemp;

 try
 {
  dblVector yerr(N);
  dblVector ytemp(N);

  hl=h;

  for(EVER)
  {
   rkck(ytemp,yerr,hl);
   errmax=0.0;
   for (i=0;i<N;i++)
  	errmax=max(errmax,fabs(yerr[i]/yscal[i]));
   errmax/=eps;
   
   if (errmax<=1.0)	//step succesful, compute next step
  	break;
   htemp=SAFETY*hl*pow(errmax,PSHRNK);
   //truncate error too large, reduce stepsize
   hl=(hl>=0.0 ? max(htemp,0.1*hl) : min(htemp, 0.1*hl));
   //but no more than a factor 10
   
   if (hl<=TINY)
         {//stepsize underflow
          nError=STEPUND;
          throw runtime_error(IntErrors[nError]);
         }
  }
  if (errmax>ERRCON)
  	hnext=SAFETY*hl*pow(errmax,PGROW);
  else
  	hnext=5.0*hl;
  t+=(hdid=hl);
  for (i=0;i<N;i++)
 	{
         if (!equationObject->IsImmediate(i))
 	 	dvNewValues[i]=ytemp[i];
	}
 }
 catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
 }

//Cash-Karp Runge-Kutta step routine
//static variables for Cash Karp RK, see Press page 720-721:
static const double CK_a2=0.2,
	CK_a3=0.3,
	CK_a4=0.6,
	CK_a5=1.0,
	CK_a6=0.875,
	CK_b21=0.2,
	CK_b31=3.0/40.0,
	CK_b32=9.0/40.0,
	CK_b41=0.3,
	CK_b42=-0.9,
	CK_b43=1.2,
	CK_b51=-11.0/54.0,
	CK_b52=2.5,
	CK_b53=-70.0/27.0,
	CK_b54=35.0/27.0,
	CK_b61=1631.0/55296.0,
	CK_b62=175.0/512.0,
	CK_b63=575.0/13824.0,
	CK_b64=44275.0/110592.0,
	CK_b65=253.0/4096.0,
	CK_c1=37.0/378.0,
	CK_c3=250.0/621.0,
	CK_c4=125.0/594.0,
	CK_c6=512.0/1771.0,
	CK_dc5=-277.0/14336.0,
	CK_dc1=CK_c1-2825.0/27648.0,
	CK_dc3=CK_c3-18575.0/48384.0,
	CK_dc4=CK_c4-13525.0/55296.0,
	CK_dc6=CK_c6-0.25;

void  Integrator::rkck(dblVector &yout,dblVector &yerr,double hl)
{
 unsigned int i;
 
 try
 {
  dblVector ak2(N);
  dblVector ak3(N);
  dblVector ak4(N);
  dblVector ak5(N);
  dblVector ak6(N);
  dblVector ytemp(N);

  //first step
  for (i=0;i<N;i++)
 	ytemp[i]=dvValues[i]+CK_b21*hl*dvDerived[i];
  equationObject->Evaluate(ytemp, ak2);
  //second step
  for (i=0;i<N;i++)
 	ytemp[i]=dvValues[i]+hl*(CK_b31*dvDerived[i]+CK_b32*ak2[i]);
  equationObject->Evaluate(ytemp, ak3);
  //third step
  for (i=0;i<N;i++)
 	ytemp[i]=dvValues[i]+hl*(CK_b41*dvDerived[i]+CK_b42*ak2[i]+CK_b43*ak3[i]);
  equationObject->Evaluate(ytemp, ak4);
  //fourth step
  for (i=0;i<N;i++)
  	ytemp[i]=dvValues[i]+hl*(CK_b51*dvDerived[i]+CK_b52*ak2[i]+CK_b53*ak3[i]+CK_b54*ak4[i]);
  equationObject->Evaluate(ytemp, ak5);
  //fifth step
  for (i=0;i<N;i++)
  	ytemp[i]=dvValues[i]+hl*(CK_b61*dvDerived[i]+CK_b62*ak2[i]+CK_b63*ak3[i]+CK_b64*ak4[i]+CK_b65*ak5[i]);
  equationObject->Evaluate(ytemp, ak6);
  //sixth step
  //accumulate increments with proper weights
  for (i=0;i<N;i++)
  	yout[i]=dvValues[i]+hl*(CK_c1*dvDerived[i]+CK_c3*ak3[i]+CK_c4*ak4[i]+CK_c6*ak6[i]);
  //estimate error as difference between fourth and fifth order methods
  for (i=0;i<N;i++)
  	yerr[i]=hl*(CK_dc1*dvDerived[i]+CK_dc3*ak3[i]+CK_dc4*ak4[i]+CK_dc5*ak5[i]+CK_dc6*ak6[i]);
  for (i=0;i<N;i++)
  	dvError[i]=yerr[i];
 }
catch (exception &stdex)
     {
      bError=true;
      bStlError=true;
      sException=stdex.what();
     }
}

//---------------------------------------------------------------------------
void  Integrator::SetStart(double d)
{
 tbegin=d;
 CalcMaxEst();
}
//---------------------------------------------------------------------------
void  Integrator::SetStop(double d)
{
 tend=d;
 CalcMaxEst();
}
//---------------------------------------------------------------------------
void  Integrator::SetStep(double d)
{
 if (d>0)
 {
  h=d;
  horg=d;
 }
 CalcMaxEst();
}
//---------------------------------------------------------------------------
int  Integrator::GetIntegrator()
{
 return nIntegrat;
}
//---------------------------------------------------------------------------
bool  Integrator::SetIntegrator(int n)
{
 if ((n<0)||(n>=NUMINTEGRATORS))
    {
     pControl->Error("Integrator not implemented");
     return false;
    }
 nIntegrat=n;
 h=horg;
 CalcMaxEst();
 return true;
}
//---------------------------------------------------------------------------
void  Integrator::Done()
{
 unlockMutex();
 pControl->Done();
}
//---------------------------------------------------------------------------
#ifndef SPINNER
unsigned int Integrator::getLyapSize()
{
 return vddLyap.size();
}
//---------------------------------------------------------------------------
dblDeque *Integrator::getLyapData(unsigned int idx)
{
 if (idx<vddLyap.size())
 	return &vddLyap[idx];
 else
 	return NULL;
}
#endif
//---------------------------------------------------------------------------
double Integrator::getLyapAvg(unsigned int idx)
{
 if (idx<dvLyapAvg.size())
 	return dvLyapAvg[idx];
 else
 	return DBL_MIN;
}
//---------------------------------------------------------------------------
void  Integrator::Seth(double newh)
{
 if ((newh-LITTLE)>0.0)
            h=newh;
}
//---------------------------------------------------------------------------
void  Integrator::SetBegin(double newb)
{
 if ((newb-LITTLE)>0.0)
         tbegin=newb;
}
//---------------------------------------------------------------------------
void  Integrator::SetEnd(double newe)
{
 if ((newe-LITTLE)>0.0)
            tend=newe;
}
//---------------------------------------------------------------------------
void  Integrator::SetLag(double f)
{
 nLag=0;
 if ((nIntegrat<RKVAR)&&(h!=0.0))
    {
      nLag=(int)ceil(f/h);
    }
}
