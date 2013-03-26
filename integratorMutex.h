//---------------------------------------------------------------------------
#ifndef integratorH
#define integratorH
//---------------------------------------------------------------------------
#include <vector>
#include <qthread.h>
#include <qmutex.h>
#include "equation.h"

namespace NS_Integrator {

typedef std::deque<double> DblDeque;
typedef std::vector<double> DblVector;
typedef std::vector<DblDeque> dblVectorOfDeque;

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
        int nIntegrat;
	int nLyapMod;
        bool bError;
        bool bReset;
	bool bLocked;
	bool bStopped;
	bool bStlError;
	bool bLyapunov;
	bool bLyapRestart;
        QMutex *pMutex;
        std::string sException;

protected:
        NS_Equation::Equation *EquationObject;
        void  Euler();
	void  Gear1();
	void  rk2();
	void  rk2imp();
        void  rk4();
        void  rk4imp();
        void  rk45();
        void  rkck(NS_Formu::DblVector &,NS_Formu::DblVector &,double);
        void  rkqs();
	void  rkpd();
        void  OdeIntFixed();
        void  OdeIntVar();
	void  doLyapStep();
        void  DoReset();
        void  Done();
	bool  LockMutex();
	void  UnlockMutex();
        void  CalcMaxEst();
	
public:
	DblVector dvValues;
	DblVector dvOldValues;
        DblVector dvDerived;
        DblVector dvNewValues;
        DblVector dvStepValues;
	DblVector dvError;
        DblVector dvLyapVals;
	DblVector dvLyapNewVals;
	DblVector dvLyapAvg;
	DblVector dvLyapSum;
	dblVectorOfDeque vddLyap;
        DblVector yscal;
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
	void SetLyapSteps(unsigned int n) { nLyapMod=n; }
	unsigned int GetLyapSteps() { return nLyapMod; }
	unsigned int GetNumSystems() { return uNumSys; }
	void SetNumSystems(unsigned int n) { uNumSys=n; }
	const char *GetErrorStr();
	bool IsOk() { return !bError;}
	bool IsStlError() { return bStlError;}
        const double  Gett() { return t; }
        const unsigned int  GetSteps() { return uStep; }
        const int  GetLag() { return nLag; }
        void  RegisterEquation(NS_Equation::Equation *EquationObject);
	int GetNumIntegrators();
	unsigned int GetNumSteps() { return uStep; }
	unsigned int GetMaxEstimate() { return uMaxEst; }
	const char *GetIntegratorName(int idx);
	const char *GetIntegratorName();
	Integrator(QMutex *p);
	 ~Integrator();
	virtual void run();
};
//---------------------------------------------------------------------------
}; //end of NS_Integrator
#endif
