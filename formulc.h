/* FORMULC.H     as of 20/07/1999 (v3.0 definitive)*/
/* Original Copyright (c) 1995 by Harald Helfgott, released restrictions 30/6/97*/
/* Copyright  by Tjeerd olde Scheper */
/* This program is provided "as is", without any explicit or */
/* implicit warranty. */

#ifndef FORMULC_H
#define FORMULC_H

#include <memory>
#include <vector>
#include <string>
#include <deque>
#include <functional>
#include <stack>
#include <utility>
#include <queue>

namespace NS_Formu {

#define MAXPAR 5	    /* maximum number of parameters, arbitrary number, currently 5 */
/*Structure defines a variable, 'id' is internal format, 'Name' is for reference */
typedef std::deque<double> DblDeque;
typedef std::vector<double> DblVector;
typedef std::vector<DblDeque> dblVectorOfDeque;
typedef std::stack<double> DblStack;
typedef std::vector<unsigned int> IntVector;
typedef std::vector<IntVector> VectorOfVInt;
typedef std::vector<std::string> StrVector;

typedef enum {etUnknown, etParAsVar, etParam, etVar, etImm, etDeriv, etMap} TEqTypes;

typedef struct {
std::string Name;
unsigned int id;
TEqTypes eqType;
double value;
double initval;
} Variable;
typedef std::vector<Variable> VarVector;

typedef struct {
std::string Name;
unsigned int id;
unsigned int nOp;
unsigned int extid;
VarVector aVars;
IntVector iVars;
} ExtVariable;

typedef std::vector<ExtVariable> ExtVarVector;

typedef struct {
unsigned int id;
unsigned int varid;
double value;
double evol;
double length;
} DelayedVar;
typedef std::vector<DelayedVar> DelVector;

typedef struct {
unsigned int id;
unsigned int sid;
unsigned int num;
double value;
} SocketVar;
typedef std::vector<SocketVar> SockVector;

class formuClass {
private:
       VectorOfVInt code;
       StrVector src;
       DblVector ctable;
       DblVector results;
       IntVector param;
       IntVector var;
       DblStack dblstack;
#ifdef SPINNER
       NS_DataSpinner::TDataSpinner *spinner;
#else
       const dblVectorOfDeque *vData;
#endif
protected:
       int isdefined(const char *,int);
       int isextdefined(const char *,int);
       int isinextdefined(ExtVariable &, std::string &);
       size_t max_size(const char *source);
       unsigned int *comp_time(unsigned int *function, unsigned int *fend, unsigned int npars);
       unsigned int *i_trans(unsigned int *function, char *begin, char *end);
       double value(unsigned int *function,unsigned int len);
       double findDelay(unsigned int idx);
       double interpolateDelay(unsigned int idx,double type);
       double socketData(unsigned int idx);
public:
       formuClass();
       ~formuClass();
       DblVector aConsts;
       VarVector aVars;
       ExtVarVector aExtVars;
       DelVector aDelays;
       SockVector aSocks;
       void AddSource(char *);
       void AddVariable(char *,TEqTypes);
       bool translate(char **,int);
       int Convert();
       const char *fget_error(void);
       double NumEval(char *p);
       int get_var_idx(const char *var);
       unsigned int GetVarIdx(const char *);
       unsigned int get_num_var();
       void reset_vars();
       const char *get_var_name(unsigned int n);
       TEqTypes GetVariableType(const char *);
       TEqTypes GetVariableType(unsigned int n);
       TEqTypes get_var_type(unsigned int n);
       unsigned int get_var_count();
       const char *GetVariableName(unsigned int n);
       bool SetVariableName(unsigned int n,const char *p);
       const char *GetParameterName(unsigned int n);
       double GetVariableVal(unsigned int n);
       double GetVariableIniVal(unsigned int n);
       double GetVariableVal(QString &);
       double GetParameterVal(unsigned int n);
       double GetConstantVal(unsigned int n);
       void reset();
       void make_var(const char *var, double value);
       void make_ini(const char *var, double value);
       void make_idxvar(unsigned int idx, double value);
       void MakeVarVal(unsigned int idx, double value);
       void MakeVarIniVal(unsigned int idx, double value);
       void MakeParamVal(unsigned int idx, double value);
       void MakeParamVal(const char *var, double value);
       bool IsImmediate(unsigned int idx);
       bool IsDifferential(unsigned int idx);
       bool IsMap(unsigned int idx);
       const DblVector *evalue();
       void make_empty();
       void print(QTextEdit *);
       bool IsEmpty();
       const char *GetEquation(unsigned int);
       const char *GetErrorEquation();
       int GetErrorChar();
       unsigned int GetNumberVars() { return var.size(); }
       unsigned int GetNumberPars() { return param.size(); }
       unsigned int GetNumberConsts() { return aConsts.size(); }
#ifdef SPINNER
       void registerSpinner(NS_DataSpinner::TDataSpinner *p) {Q_ASSERT(p); spinner=p;};
#else
       void RegisterData(const dblVectorOfDeque *v) {vData=v;};
#endif
};
}; // namespace

typedef double (*Func)(double);
typedef double (*Func0)(void);
typedef double (*Func2)(double,double);
typedef double (*Func3)(double,double,double);
typedef double (*Func4)(double,double,double,double);
typedef double (*Func5)(double,double,double,double,double);

int fnew(char *name, Func f, int n_of_pars, int varying);
int read_table(int i, char *name, int *n_of_pars, int *varying);
int where_table(char *name,int);
int fdel(char *name);
int StrnCmp(const char *t, const char *s,size_t n);

#endif //FORMULC_H
