//---------------------------------------------------------------------------
#ifndef ANALYSIS_H
#define ANALYSIS_H
//---------------------------------------------------------------------------
#include <qthread.h>
#include <qmutex.h>
#include <qevent.h>
#include <qsettings.h>
#include <qlabel.h>
#include <vector>

#include "equation.h"

namespace NS_Analysis {
#define ANA_LOCK_TRY 600
#define ANA_LOCK_MS 50

#define ANA_EVENT QEvent::User+11
#define ANA_ISIEVENT QEvent::User+12

typedef std::vector<unsigned int> UIntVector;
typedef std::deque<double> DblDeque;
typedef std::deque<unsigned int> IntDeque;
typedef std::deque<int> sIntDeque;
typedef enum { aotNone, aotRecur, aotMaxLyap, aotPeriod, aotPower, aotPoincare1d, aotPoincare2d, aotSpike, aotIsi } anaOpType;
typedef enum { swNone, swHamming, swHann, swBartlett, swBlackman} specWinType;
typedef enum { pdBoth, pdUp, pdDown} poinDirection;

typedef struct
{
 anaOpType type;
 DblDeque ddVals;
 QStringList slNames;
 QStringList slDescr;
 QString qsTitle;
 QString qsDescr;
} AnaSetT;

typedef struct AnaTypeT
{
 bool bUpdate;
 bool bNeedTime;
 unsigned int nVar1;
 unsigned int nVar2;
 unsigned int nGraph;
 anaOpType type;
 DblDeque ddData;
 DblDeque ddTime;
 DblDeque ddXResult;
 DblDeque ddYResult;
 DblDeque ddResult;
 AnaSetT set1;
 AnaSetT set2;
} AnaType;

typedef std::vector<AnaTypeT> AnalysisVector;
typedef std::vector<AnaSetT> SettingsVector;
//---------------------------------------------------------------------------
class AnalysisCompleteEvent : public QCustomEvent
{
public:
        AnalysisCompleteEvent( unsigned int id )
            : QCustomEvent( QEvent::User+10 ) { nId=id; }
        unsigned int getIdx() const { return nId; }
private:
        unsigned int nId;
};
//---------------------------------------------------------------------------
class AnalysisProgressEvent : public QCustomEvent
{
public:
        AnalysisProgressEvent(int eventId, unsigned int id, unsigned int max, unsigned int cur )
            : QCustomEvent( eventId ) { nId=id; nMax=max, nCur=cur; }
        unsigned int getIdx() const { return nId; }
        unsigned int getMax() const { return nMax; }
        unsigned int getCur() const { return nCur; }
private:
        unsigned int nId;
        unsigned int nMax;
        unsigned int nCur;
};
//---------------------------------------------------------------------------
class Analysis : public QThread
{
private:
	bool bIntLocked;
	bool bAnaLocked;
	bool bStopped;
	bool bError;
        QMutex *pIntMutex;
        QMutex *pAnaMutex;
	QLabel *pLbl;
	QPixmap pixThreadOn, pixThreadOff;
        std::string sException;
	AnalysisVector avList;
	SettingsVector svList;
protected:
        NS_Equation::Equation *EquationObject;
	bool LockIntMutex();
	bool LockAnaMutex();
	void UnlockIntMutex();
	void UnlockAnaMutex();
	void doRecur(AnaTypeT &t);
	void maxlyap(AnaTypeT &t);
	void doPeriod(AnaTypeT &t);
	void doPower(AnaTypeT &t);
	void spike(AnaTypeT &t);
	void isi(AnaTypeT &t);
	void rescaleData(DblDeque &d,double *min,double *interval);
	double deltaAverage(DblDeque &data);
	void findNeighbours(unsigned int nGraph, DblDeque &d, DblDeque &vx, DblDeque &vy,
				 sIntDeque &box,sIntDeque &list,unsigned  int dim,unsigned  int delay,unsigned int step, double eps,bool *bdone);
	void put_in_boxes(UIntVector &box,UIntVector &list, DblDeque &series,unsigned int dim, unsigned int delay, unsigned int step,double epsinv);
	bool make_iterate(int idx, DblDeque &series, UIntVector &box, UIntVector &list, UIntVector &found,DblDeque &lyap,unsigned int dim, unsigned int delay, unsigned int steps,unsigned int mindist, double eps);
	void davevar(DblDeque &data, double *ave, double *var);
	unsigned int getIntPow(unsigned int val);
	bool isIntPow(unsigned int val);
	double sqr(double);
	void dspread(double y, NS_Formu::DblVector &yy, double x, int m);
	unsigned int dfastper(unsigned int nGraph, DblDeque &x, DblDeque &y, double ofac, double hifac, unsigned int nwk, NS_Formu::DblVector &wk1, NS_Formu::DblVector &wk2,double *per, double *freq, double *prob);
	void powerSpectrum(NS_Formu::DblVector &data, unsigned int m, specWinType type);
	void poincare1d(AnaTypeT &t);
	void poincare2d(AnaTypeT &t);
	//void interSpikeIntervals(NS_Formu::DblVector &data);
public:
	NS_Control::TVarDataVector vvdVars;
	bool doAnalysis(AnaTypeT &t);
        void  Reset();
        void  Start();
        void  Stop();
	const char *GetErrorStr();
	bool IsOk() { return !bError;}
	void clearFlags();
	void clearFlag(unsigned int n);
	void setFlag(unsigned int n);
	void remove(unsigned int n);
	unsigned int addAnalysis(anaOpType t,unsigned int n,unsigned int g);
	unsigned int addAnalysis2d(anaOpType t,unsigned int n1, unsigned int n2,unsigned int g);
	bool hasAnalysis(anaOpType t, unsigned int n);
	void purgeAnalysis(anaOpType t);
	void loadSettings(QSettings *);
	void windowBartlett(DblDeque &d);
	void windowBlackman(DblDeque &d);
	void windowHamming(DblDeque &d);
	void windowHann(DblDeque &d);
#ifdef _DEBUG
	void dumpSettings();
#endif
        void  RegisterData(NS_Control::TVarDataVector &v);
	Analysis(QMutex *pIntMutex,QMutex *pAnaMutex);
	 ~Analysis();
	virtual void run();
	//getters and setters
	AnaTypeT *get(unsigned int n);
	AnaSetT *get(anaOpType);
	AnaSetT *get1(anaOpType, unsigned int n);
	AnaSetT *get2(anaOpType, unsigned int n);
	double getValue(anaOpType t, const char *s);
	double getValue(AnaSetT &set, const char *s);
	void setValue(AnaSetT &set, const char *s,double);
	double getValue(unsigned int, const char *s,bool b=false);
	void setLabel(QLabel *pl);
};
//---------------------------------------------------------------------------
}; //end of NS_Analysis
#endif
