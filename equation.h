// equation.h

#ifndef EQUATION_H
#define EQUATION_H

#include <queue>
#include <deque>
#include <vector>

#include "formulc.h"

#define NUMHEADER 6
#define NUM_OPTIONS 11
#define NUM_OBOPTIONS 6
#define IDCHARCOMMENT '%'
#define IDCHARHEADLEFT '<'
#define IDCHARHEADRIGHT '>'
#define IDCHAREQUATE '='
#define IDCHARRANGE ':'

namespace NS_Equation {

enum OptionsType {opIntegrator=0, opLength, opStepSize, opOutput, opGraph, op3D, opLog, opLyap, opStats, opSocket, opClient, opNone };

struct structOptionT
{
 OptionsType type;
 std::string s;
 double d;
};

typedef std::queue<double>  doubleQueue;

//Socket data object
typedef struct
{
  int nId;
  int nIdx;
  double d;
  double tstamp;
}  TSockDataPoint;
typedef std::deque<TSockDataPoint>  dataDeque;
typedef dataDeque::iterator iterDataDeque;

//Functor for time comparison
 struct smallerTime : public std::binary_function<TSockDataPoint&, double, bool>
 {
   bool operator()(TSockDataPoint &p, double t)
	{ //return true if time is smaller than tstamp
	  return p.tstamp <= t;
	}
 };

typedef std::vector<structOptionT> OptionsVector;
typedef std::vector<bool> BoolVector;

class Equation
{
        NS_Formu::formuClass pEquation;
     	QString FileName;
        double t;
     	unsigned int N;
     	int line, charnum;
     	int error;
    	unsigned int nDelayFrom;
    	unsigned int nDelayTo;
    	double dDelayLen;
    	int nWarn;
    	std::string sModelName;
    	std::string sInfo;
        QTextEdit *wText;
    	bool bEqBefore;
        bool bSaveData;
        bool bBinary;
        bool bStoreData;
        bool bDelay;
        bool bFixed;
        int ExtractParam(char *p,bool bini);
    	bool GetDeclare(char *);
    	bool BuildEqList();
    	bool GetOption(char *);
	void Error(int);
	bool isValidString(char *p);
    	void FatalError(const char *,const char *);
        void WriteData(NS_Formu::DblVector &dv);
	bool ParseEquation(QStringList &list);
	bool checkVariable(char *,NS_Formu::TEqTypes);
        FILE *fp;
	OptionsVector ovOptions;
	BoolVector bvDoneHeader;
#ifndef SPINNER
	bool bKeepError;
#endif
 public:
 	Equation();
 	Equation(int,QTextEdit *);
 	~Equation();
#ifndef SPINNER
        NS_Formu::dblVectorOfDeque vdvData;
        NS_Formu::dblVectorOfDeque vdvDataError;
	NS_Formu::DblDeque dvDataError;
#endif
 	bool ReadEquation(QStringList &list);
 	void Print();
	double Solve(double);	//solve eq. for a value
	double Derived(double);	//calc derivative for a value
	void SetFunction(QTextEdit *);
	void Flush();
#ifndef SPINNER
	bool getKeepError() {return bKeepError;};
	void setKeepError(bool);
#endif
	void newEquation(QTextEdit *);
	const char *GetEquation(unsigned int);
	const char *GetName();
	const char *GetInfo();
	const char *GetVarName(unsigned int);
	NS_Formu::TEqTypes GetVariableType(unsigned int n);
	bool SetVarName(unsigned int,const char *);
	const char * GetParmName(unsigned int);
	double GetVarValue(unsigned int);
	double GetVarIniValue(unsigned int);
	double GetParmValue(unsigned int);
	double GetConstValue(unsigned int);
	void SetParmValue(unsigned int,double);
	void SetParmValue(QString &,double);
	void SetVarValue(unsigned int,double);
	void SetVarIniValue(unsigned int,double);
	void SetVarValue(QString &,double);
	void SetVarIniValue(QString &,double);
	unsigned int GetN() { return N;}
   	unsigned int GetNPar() {return pEquation.GetNumberPars();};
   	unsigned int GetNConsts() {return pEquation.GetNumberConsts();};
        unsigned int GetVarIdx(QString &qs) { return pEquation.GetVarIdx((const char *)qs); }
	int GetNumWarnings() { return nWarn; }
	void EvaluateAll(NS_Formu::DblVector &, NS_Formu::DblVector &, NS_Formu::DblVector &);
	void Evaluate(NS_Formu::DblVector &, NS_Formu::DblVector &);
	int NumberOfImmediates();
	void Reset();
	structOptionT *GetOption(unsigned int n);
	unsigned int GetNumOptions() { return ovOptions.size();}
	bool IsImmediate(unsigned int);
	bool IsDiffMap(unsigned int idx);
	unsigned int GetNumDiffMap();
        void StripWhiteSpace(char *t,char *p);
        bool StripComment(char *p);
        bool IsHeader(const char *p);
        bool CheckHeader(const char *p,int *n);
        bool IsNumericalTerm(const char *p);
	bool NumericalTermValue(char *p, double *);
        void DoneStep(double,NS_Formu::DblVector &,NS_Formu::DblVector &);
	void DoRegister();
	void setSocketData(TSockDataPoint &dp);
};

}; //end of namespace

#endif
