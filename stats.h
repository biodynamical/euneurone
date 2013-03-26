//---------------------------------------------------------------------------
#ifndef stats_H
#define stats_H
//---------------------------------------------------------------------------
#include <vector>
#include <qthread.h>
#include <qmutex.h>
#include <qlabel.h>
#include <qpixmap.h>
#include "equation.h"

namespace NS_Stats {
#define STAT_LOCK_TRY 600
#define STAT_LOCK_MS 50
//---------------------------------------------------------------------------
class Statistics : public QThread
{
private:
	bool bIntLocked;
	bool bStatLocked;
	bool bStopped;
	bool bError;
	bool bMean;
	bool bVariance;
        QMutex *pIntMutex;
        QMutex *pStatMutex;
	QLabel *pLbl;
        std::string sException;
        QString qsNum;
        QPixmap statQ, statP, statOk, statOn, statOff;

protected:
        NS_Equation::Equation *EquationObject;
	bool LockIntMutex();
	bool LockStatMutex();
	void setVarColumnData(int col,unsigned int idx,double d);
	void flagVarColumn(int col,unsigned int idx);
	void setErrorColumnData(int col,unsigned int idx,double d);
	void flagErrorColumn(int col,unsigned int idx);
	double calcMean(unsigned int idx);
	double calcStddev(unsigned int idx);
        void initColumns(bool);
	void calcVars();
	void calcErrors();
	
public:
	//NS_Formu::DblVector dvMeanValues;
	NS_Formu::DblDeque ddData;	
	NS_Control::TVarDataVector vvdVars;
	NS_Control::TVarDataVector vvdErrors;
        void  Reset();
        void  Start();
        void  Stop();
	const char *GetErrorStr();
	bool IsOk() { return !bError;}
        void  RegisterData(NS_Control::TVarDataVector &vd,NS_Control::TVarDataVector &ve);
	Statistics(QMutex *pIntMutex,QMutex *pStatMutex, QLabel *pLabel = 0);
	 ~Statistics();
	virtual void run();
	//getters and setters
	bool mean() {return bMean;}
	void setMean(bool b) { bMean=b;}
	bool stddev() {return bVariance;}
	void setStddev(bool b) { bVariance=b;}
	bool error() { return bError; }
	void setError(bool b) { bError=b; }
	void setLabel(QLabel *p);
};
//---------------------------------------------------------------------------
}; //end of NS_Stats
#endif
