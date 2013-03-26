//---------------------------------------------------------------------------
#ifndef integratorH
#define integratorH
//---------------------------------------------------------------------------
#include <vector>
#include <qthread.h>
#ifndef SPINNER
#include <qmutex.h>
#endif
#include "equation.h"

namespace NS_Integrator {

typedef std::deque<double> dblDeque;
typedef std::vector<double> dblVector;
typedef std::vector<dblDeque> dblVectorOfDeque;

//---------------------------------------------------------------------------
class Integrator : public QThread
{
private:
        double t;
        double tau;
        double tbegin;
	double tstart;
        double tend;
        double h;
	double horg;
        double hnext;
        double hdid;
        double htry;
	double lyapdist;
	double lyapdim;
        float eps;
        int nLag;
        unsigned int N;
        unsigned int Nlyap;
        unsigned int nError;
        unsigned int uStep;
	unsigned int uMaxEst;
	unsigned int uNumSys;
	unsigned int uLyapSize;
        int nIntegrat;
	int nLyapMod;
        bool bError;
        bool bReset;
	bool bStopped;
	bool bStlError;
	bool bLyapunov;
	bool bLyapRestart;
#ifdef SPINNER
        NS_DataSpinner::TDataSpinner *spinner;
#else
	bool bLocked;
        QMutex *pMutex;
#endif
        NS_Equation::Equation *equationObject;
        std::string sException;
	//local data store
	dblVector dvValues;
	dblVector dvOldValues;
        dblVector dvDerived;
        dblVector dvNewValues;
        dblVector dvStepValues;
	dblVector dvError;
        dblVector dvLyapVals;
	dblVector dvLyapNewVals;
	dblVector dvLyapAvg;
	dblVector dvLyapSum;
#ifndef SPINNER
	dblVectorOfDeque vddLyap;
#endif
        dblVector yscal;
protected:
        void  Euler();
	void  Gear1();
	void  rk2();
	void  rk2imp();
        void  rk4();
        void  rk4imp();
        void  rk45();
        void  rkck(dblVector &,dblVector &,double);
        void  rkqs();
	void  rkpd();
        void  OdeIntFixed();
        void  OdeIntVar();
	void  doLyapStep();
        void  DoReset();
        void  Done();
#ifndef SPINNER
	bool  lockMutex();
	void  unlockMutex();
#endif
        void  CalcMaxEst();
	
public:
        void  Init();
        void  Reset();
        void  Start();
        void  Stop();
        void  SetLag(double f);
        void  Seth(double h);
        void  SetBegin(double b);
        void  SetEnd(double e);
	double GetBegin() { return tbegin; }
	double GetEnd() { return tend; }
	double GetStep() { return h; }
        int  GetIntegrator();
	bool  SetIntegrator(int);
        void  SetStart(double);
        void  SetStop(double);
        void  SetStep(double);
        void  ReloadValues();
	void SetLyapunov(bool b) { bLyapunov=b; }
	bool hasLyapunov() { return bLyapunov; }
	void SetLyapDistance(double d) { lyapdist=d; }
	double GetLyapDistance() { return lyapdist; }
	double GetLyapDim() { return lyapdim; }
	double getLyapAvg(unsigned int idx);
	void SetLyapSteps(unsigned int n) { nLyapMod=n; }
	unsigned int GetLyapSteps() { return nLyapMod; }
	unsigned int GetNumSystems() { return uNumSys; }
	void SetNumSystems(unsigned int n) { uNumSys=n; }
#ifndef SPINNER
	unsigned int getLyapSize();
	dblDeque *getLyapData(unsigned int idx);
#endif
	const char *GetErrorStr();
	bool IsOk() { return !bError;}
	bool IsStlError() { return bStlError;}
        double  Gett() { return t; }
        unsigned int  GetSteps() { return uStep; }
        int  GetLag() { return nLag; }
#ifdef SPINNER
        void registerObjects(NS_Equation::Equation *equationObject, NS_DataSpinner::TDataSpinner *spinObject);
#else
	void registerObjects(NS_Equation::Equation *Equation, QMutex *p);
#endif
	int GetNumIntegrators();
	unsigned int GetNumSteps() { return uStep; }
	unsigned int GetMaxEstimate() { return uMaxEst; }
	const char *GetIntegratorName(int idx);
	const char *GetIntegratorName();
	Integrator();
	 ~Integrator();
	virtual void run();
};
//---------------------------------------------------------------------------
}; //end of NS_Integrator
#endif
