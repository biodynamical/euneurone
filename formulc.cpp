/* FORMULCPP.C 3.0 as of 20/07/1999 */
/* definitive  version */
/* A fast interpreter of mathematical functions */

/* Original Copyright (c) 1995 by Harald Helfgott, released restrictions 30/6/97*/
/* Copyright  by Tjeerd olde Scheper
Modified 1996 - 30/6/1997 - 20/7/1999
tvolde-scheper@brookes.ac.uk
Modified for Qt 18/7/2003
Added:  Support for multiple equations
	improved parser
	added support for sizeof(unsigned int) variables in multiple equations
	(approx. 4294967295 variables)
	added some standard C math functions to list
	CPP version
	MS-Windows compatible
	Qt Compatible
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <qapplication.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qcolor.h>

#ifdef USE_GSL
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_bessel.h>
#else
#include "random250.h"
#endif

#include "dataspinner.h"
#include "formulc.h"

using namespace NS_Formu;
using namespace NS_DataSpinner;

//#define FORMU_DEBUG

#define ROUNDOFF 1.69E-10
#define INTERPOL 20

#define MULTIPLE_EQUATIONS

//Windows doesn't have a log2
#ifdef _WINDOWS
static double log2(double);
#endif

//random generator
#ifdef USE_GSL
extern gsl_rng *ranGenerator;
extern const gsl_rng_type *ranType;
#endif

static double pi(void);
double Random(double, double);
double Ran();
double Gauss(double);
double Gaussn(double,double);
double Pulse(double cur, double low, double high);
double Pattern(double p, double len, double b);
double Threshold(double var, double thres);
double LowThres(double var, double thres, double res);
double HighThres(double var, double thres, double res);
double Equal(double d1, double d2);
double And(double d1, double d2);
double Or(double d1, double d2);
double Xor(double d1, double d2);
double Not(double d);
double If(double d);
double Ifelse(double d1, double d2, double d3);
double Step(double d1, double d2, double d3);
double Stair(double d1, double d2, double d3, double d4);
double Sign(double d);
double Besselj(double d1, double d2);
double Besseli(double d1, double d2);
double Bessely(double d1, double d2);
double Besselk(double d1, double d2);

static const char *i_error; /*pointer to the character in source[]
			that causes an error */

const char *validChars="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-=*/^()[]~%,.' \t";

#define EVER        ;;
#define CHARDOUBLE 'D'
#define CHARVAR 'V'
#define CHAREXT 'E'
#define CHARFUNC 'F'
#define CHARDEL 'T'
#define CHARIDEL 'I'
#define CHARNEG 'M'
#define CHARSOCK 'S'

static char *my_strtok(char *s);

static const char *errmes;
int error, cn_err;
static void fset_error(const char *);
int npstep, npval, npcount;

static Variable AGlobVar;

// Changed ToS 2008: GCC 4.x doesn't like const char * for static string data: changed to string array char name[10]
typedef struct {
  char name[10];
  Func f;    /* pointer to function*/
  int n_pars; /* number of parameters */
  int varying; /* Does the result of the function vary
		  even when the parameters stay the same?
		  varying=1 for e.g. random-number generators. */
} formu_item;

#define IDINVALID 0xFFFFFFFF

#define TABLESIZE 51
#define STD_LIB_NUM 46
#define DELAY_IDX 43
#define IDELAY_IDX 44
#define SOCKET_IDX 45
formu_item ftable[TABLESIZE]=
{
  {{"sin"}, sin,1,0},
  {{"cos"}, cos,1,0},
  {{"tan"}, tan,1,0},
  {{"asin"}, asin,1,0},
  {{"acos"}, acos,1,0},
  {{"atan"}, atan,1,0},
  {{"atan2"},(Func) atan2,2,0},
  {{"sinh"}, sinh, 1, 0},
  {{"cosh"}, cosh, 1, 0},
  {{"tanh"}, tanh, 1, 0},
  {{"exp"}, exp,1,0},
  {{"ln"},  log,1,0},
  {{"log"}, log10,1,0},
  {{"log2"}, log2,1,0},
  {{"abs"},  fabs,1,0},
  {{"sqrt"},  sqrt,1,0},
  {{"pow"},  (Func)pow,2,0},  
  {{"mod"}, (Func)fmod,2,0},
  {{"hypot"}, (Func)hypot, 2, 0},
  {{"pi"}, (Func)pi,0,0},
  {{"rnd"}, (Func)Ran, 0, 1}, /*returns a random number from 0 to 1 */
  {{"gauss"}, (Func)Gauss, 1, 1},
  {{"gaussn"}, (Func)Gaussn, 2, 1},  
  {{"random"}, (Func)Random, 2, 1},
  {{"pulse"}, (Func)Pulse,3,0},
  {{"thres"}, (Func)Threshold,2,0},
  {{"lthres"}, (Func)LowThres,3,0},
  {{"hthres"}, (Func)HighThres,3,0},
  {{"equal"}, (Func)Equal,2,0},
  {{"AND"}, (Func)And, 2, 0},
  {{"OR"}, (Func)Or, 2, 0},
  {{"XOR"}, (Func)Xor, 2, 0},
  {{"NOT"}, Not, 1, 0},
  {{"pattern"}, (Func)Pattern, 3, 1},
  {{"if"}, (Func)If, 1, 0},
  {{"ifelse"}, (Func)Ifelse, 3, 0},
  {{"sign"}, (Func)Sign, 1, 0},
  {{"step"}, (Func)Step, 3, 0},
  {{"stair"}, (Func)Stair, 4, 0},
  {{"besselj"}, (Func)Besselj, 2, 0},
  {{"besseli"}, (Func)Besseli, 2, 0},
  {{"bessely"}, (Func)Bessely, 2, 0},
  {{"besselk"}, (Func)Besselk, 2, 0},
  {{"delay"}, (Func)0x1,2,0},
  {{"idelay"}, (Func)0x2,2,0},
   {{"socket"}, (Func)0x3,2,0},
  {{"\0"},NULL,0,0}
};

static bool bExtMode=false;

/* a reliable method for comparing two char arrays,
   a substring is different from the complete string: e.g.
   'str' is different from 'strcmp' */

int StrnCmp(const char *t, const char *s,size_t n)
{
 if (n<strlen(t))
  while (isalnum(t[n])) n++;
 if (n<strlen(s))
  while (isalnum(s[n])) n++;
// #ifdef FORMU_DEBUG
//  qDebug("DEBUG: StrnCmp, s %s, t %s, n %d.",s,t,n);
// #endif
 return strncmp(t,s,n);
}

/* returns idx if a var has already been defined, used for multiple equation support */
int formuClass::isdefined(const char *begin, int len)
{
 unsigned int i;
 Variable *pAVar;

 for (i=0;i<aVars.size();i++)
	{
	 pAVar=&aVars[i];
	 if (StrnCmp(pAVar->Name.c_str(),begin,len)==0)
		{
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: IsDefined, found name %s, begin %s, len %d, i %d.",pAVar->Name.c_str(),begin,len,i);
 #endif
		 return i;
		}
 //#ifdef FORMU_DEBUG
//  qDebug("DEBUG: IsDefined, name %s, begin %s, len %d, i %d.",pAVar->Name.c_str(),begin,len,i);
// #endif
	}
 fset_error("undefined variable");
 return -1;
}

int formuClass::isextdefined(const char *begin, int len)
{
 unsigned int i;
 ExtVariable *pExtVar;

 for (i=0;i<aExtVars.size();i++)
 	{
 	 pExtVar=&aExtVars[i];
 	 if (StrnCmp(pExtVar->Name.c_str(),begin,len)==0)
 	 	{
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: IsExtDefined, found name %s, begin %s, len %d, i %d.",pExtVar->Name.c_str(),begin,len,i);
 #endif
 	 	 return i;
		}
 //#ifdef FORMU_DEBUG
//  qDebug("DEBUG: IsExtDefined, name %s, begin %s, len %d, i %d.",pExtVar->Name.c_str(),begin,len,i);
// #endif
 	}
 fset_error("undefined external object");
 return -1;
}

int formuClass::isinextdefined(ExtVariable &ev, std::string &n)
{
 unsigned int i;
 Variable *pVar;

 for (i=0;i<ev.aVars.size();i++)
	{
 	 pVar=&ev.aVars[i];
 	 if (StrnCmp(pVar->Name.c_str(),n.c_str(),n.length())==0)
 	 	{
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: IsInExtDefined, found name %s, i %d.",pVar->Name.c_str(),i);
 #endif
 	 	 return i;
 	 	}
 //#ifdef FORMU_DEBUG
//  qDebug("DEBUG: IsInExtDefined, name %s, i %d.",pVar->Name.c_str(),i);
// #endif
	}
 fset_error("undefined external reference object");
 return -1;
}

/* clears the variable array */
void formuClass::reset_vars()
{
 src.erase(src.begin(),src.end());
 aVars.erase(aVars.begin(),aVars.end());
 aExtVars.erase(aExtVars.begin(),aExtVars.end());
 aDelays.erase(aDelays.begin(),aDelays.end());
 aConsts.erase(aConsts.begin(),aConsts.end());
 code.erase(code.begin(),code.end());
 ctable.erase(ctable.begin(),ctable.end());
 results.erase(results.begin(),results.end());
 param.erase(param.begin(),param.end());
 var.erase(var.begin(),var.end());
 aSocks.erase(aSocks.begin(),aSocks.end());
}

/* output all variables */
void formuClass::print(QTextEdit *st)
{
 unsigned int i,j,c;
 //char buf[81];
 QString qs,qs2;

 st->append("<font color=#0010FF>Source:</font>");
 for (i=0;i<src.size();i++)
     {
      switch(GetVariableType(i))
      {
       default:
       case etUnknown: c='?'; break;
       case etParAsVar: c='!'; break;
       case etParam: c=' '; break;
       case etVar: c=' '; break;
       case etImm: c=' '; break;
       case etDeriv: c='\''; break;
       case etMap: c='~'; break;
      }
      qs.sprintf("%s%c=%s",GetVariableName(i),c,src[i].c_str());
      st->append(qs);
     }
 qs.setLength(0);
 st->append("<font color=#0010FF>Code:</font>");
 for (i=0;i<code.size();i++)
     {
      for (j=0;j<code[i].size();j++)
          {
           c=code[i][j];
	   if (c==0) break;
	   if ((c==CHARVAR)||(c==CHARDOUBLE)||(c==CHAREXT)||(c==CHARFUNC)||(c==CHARDEL)||(c==CHARIDEL)||(c==CHARSOCK))
	   {
	    if (c==CHAREXT)
	     {
	      qs2.sprintf("%c(%u,%u)",code[i][j],code[i][j+1],code[i][j+2]);
	      j+=2;
	     }
	    else
             {
	      qs2.sprintf("%c(%u)",code[i][j],code[i][j+1]);
              j++;
             }
           }
           else
            qs2.sprintf("%c",code[i][j]);
           qs.append(qs2);
          }
      st->append(qs);
      qs.setLength(0);
     }
 qs.setLength(0);
 qs2.sprintf("<font color=#0010FF>Variables (%u):</font>",(unsigned int)aVars.size());
 st->append(qs2);
 for (i=0;i<aVars.size();i++)
     {
      qs.sprintf("%d %s (%d)=%g (%g)",aVars[i].id, 
      				aVars[i].Name.c_str(),aVars[i].eqType,aVars[i].value,aVars[i].initval);
      st->append(qs);
     }
 qs.setLength(0);
 if (aExtVars.size()>0)
 {
	 qs2.sprintf("<font color=#0010FF>External Variables (%u):</font>",(unsigned int)aExtVars.size());
  st->append(qs2);
  for (i=0;i<aExtVars.size();i++)
     {
      qs.sprintf("%d %s (Op=0x%x, ExtId=0x%x)",aExtVars[i].id, 
      				aExtVars[i].Name.c_str(),aExtVars[i].nOp,aExtVars[i].extid);
      st->append(qs);
     }
  qs.setLength(0);
  qs.sprintf("<font color=#0010FF>Variables in external references:</font>");
  st->append(qs);
  for (j=0;j<aExtVars.size();j++)
     {
      for (i=0;i<aExtVars[j].aVars.size();i++)
      {
       qs.sprintf("%d %s (ExtId=0x%x)",
       			aExtVars[j].aVars[i].id,aExtVars[j].aVars[i].Name.c_str(),aExtVars[j].iVars[i]);
	st->append(qs);
      }
     }
 }
 qs.setLength(0);
 qs2.sprintf("<font color=#0010FF>Constants (%u):</font>",(unsigned int)aConsts.size());
 st->append(qs2);
 for (i=0;i<aConsts.size();i++)
     {
      qs.sprintf("%d %lf",i,aConsts[i]);
      st->append(qs);
     }
}

/*********************************************************/
/* The following routines manipulate the table of functions */

int read_table(int i, char *name, int *n_pars, int *varying)
/* returns 1 if succesful */
/* returns 0 otherwise */
{
 if(!ftable[i].f) {
  fset_error("index out of bounds");
  return 0;
 }
 else {
  strncpy(name,ftable[i].name,10);
  *n_pars = ftable[i].n_pars;
  *varying = ftable[i].varying;
  fset_error(NULL);
  return 1;
 }
}

int where_table(char *name,int len)
/* If the function exists, where_table() returns the index of its name
    in the table. Otherwise, it returns -1. */
{
 formu_item *table_p;

 #ifdef FORMU_DEBUG
  qDebug("DEBUG: where is %s.",name);
 #endif
 for(table_p=ftable; table_p->f != NULL && StrnCmp(name,table_p->name,len); table_p++)
 {
 }
 if(table_p->f == NULL) /*The end of the table has been reached,
		 but name[] is not there. */
  {
    return -1;
  }
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: where, p %s.",table_p->name);
 #endif
 return table_p - ftable;
}

int fdel(char *name)
/* If the function exists, it is deleted and a non-negative value
    is returned. */
/* Otherwise, -1 is returned. */
/* Original library functions may not be deleted. */
{
 int place;
 formu_item *scan;

 if((place=where_table(name,strlen(name))) == -1)
  return -1; /* there is an error message already */
 if(place<STD_LIB_NUM) {
  fset_error("original functions may not be deleted");
  return -1;
 }
 delete ftable[place].name;
 for(scan = &ftable[place]; scan->f!=NULL; scan++) {
  #ifdef FORMU_DEBUG
   qDebug("DEBUG: [fdel] scan->name=%s",scan->name);
  #endif
  strncpy(scan->name , (scan+1)->name, 10);
  scan->f     =  (scan+1) -> f;
  scan->n_pars = (scan+1) -> n_pars;
 }
 fset_error(NULL);
 return scan-ftable;
} /*end of fdel */

int fnew(char *name, Func f, int n_pars, int varying)
/* 0 is rendered if there is an error */
/* 1 is rendered otherwise */
{
 formu_item *where;

 if(n_pars<0 || n_pars>MAXPAR) {
  fset_error("invalid number of parameters");
  return 0;
 }
 for(where=ftable; where->f != NULL && strcmp(name,where->name); where++);
 if(where->f != NULL) {
  where->f=f;
  where->varying = varying;
  where->n_pars = n_pars;   /*old function is superseded */
  fset_error(NULL);
  return 1;
 } else if((where-ftable) >= TABLESIZE-1) {
  fset_error("function table full");
  return 0;
 }
// Changed ToS 2008: GCC 4.x doesn't like const char * for static string data: changed to string array char name[10]
//  else {
//   where->name = (char *) calloc(strlen(name)+1,sizeof(char));
//   if(where->name==NULL) {
//     fset_error("no memory");
//     return 0;
//   }
  strncpy(where->name,name,10);
  where->f=f;
  where->varying = varying;
  where->n_pars = n_pars;
  fset_error(NULL);
  return 1;
// }
}  /* end of fnew */

/***********************************************************/
/* Error functions                                         */

void fset_error(const char *s)
/* fset_error(NULL) and fset_error("") erase
   any previous error message */
{
 if (s == NULL || *s == '\0') errmes = NULL; /* an empty error message means
						   that there is no error */
 else errmes = s;
}

const char *formuClass::fget_error(void)
{
 return errmes;
}

#ifdef SPINNER
// New delay function using spinbuffer: note that the pointer to spinner must already be set using registerSpinner
// The use of direct functions here may be suspect. It may be necessary to write a spinbuffer version for for
// individual values....
double formuClass::findDelay(unsigned int idx)
{
 double de,dd;
 unsigned int n,pos;

 try
 {
 if (spinner->evolutionSize()<1)
	return DBL_MIN;          //smallest possible value for double>0
 //find in time array the element we are looking for:
 dd=DBL_MIN;
 n=aDelays[idx].varid+1;         //+1 to skip evolution
 de=spinner->lastEvolution();
 de-=aDelays[idx].length;
 de+=ROUNDOFF;
 if (de==aDelays[idx].evol)
 {
  dd=aDelays[idx].value;
 }
 else
 {
  aDelays[idx].evol=de;
  if (de>spinner->firstEvolution())
  {
   pos=spinner->evolutionPos(de);
   if (pos>0)
   	dd=spinner->value(n,pos);
  }
  aDelays[idx].value=dd;
 }
 }
 catch (std::exception &stdex)
 {
#ifdef _DEBUG
	qDebug("[formuClass::findDelay] exception in delay: %s",stdex.what());
#endif
  return DBL_MIN;
 }
 return dd;
}
#else
// Old delay function using local data store:
double formuClass::findDelay(unsigned int idx)
{
 double de,dd;
 unsigned int n,pos,v;

 try
 {
 if ((*vData)[0].size()<1)
	return DBL_MIN;          //smallest possible value for double>0
 //find in time array the element we are looking for:
 dd=DBL_MIN;
 n=aDelays[idx].varid+1;         //+1 to skip evolution
 de=(*vData)[0].back();
 de-=aDelays[idx].length;
 de+=ROUNDOFF;
 if (de==aDelays[idx].evol)
 {
  dd=aDelays[idx].value;
 }
 else
 {
  aDelays[idx].evol=de;
  if (de>(*vData)[0].front())
  {
   pos=std::lower_bound((*vData)[0].begin(),(*vData)[0].end(),de)-(*vData)[0].begin();
   if (pos>0)
   {
    v=(*vData)[n].size();   //to protect against GPE
    if (pos<v)
     dd=(*vData)[n][pos];
   }
  }
  aDelays[idx].value=dd;
 }
 }
 catch (std::exception &stdex)
 {
#ifdef _DEBUG
	qDebug("[formuClass::FindDelay] exception in delay: %s",stdex.what());
#endif
  return DBL_MIN;
 }
 return dd;
}
#endif

double formuClass::interpolateDelay(unsigned int idx,double type)
{
 double de,dd;
 unsigned int n,t,nb,ne,pos;
 
 if ((*vData)[0].size()<INTERPOL)
	return DBL_MIN;          //smallest possible value for double>0
 t=5;//(unsigned int)type;
 dd=DBL_MIN;
 //find in time array the element we are looking for:
 n=aDelays[idx].varid+1;         //+1 to skip evolution
 de=(*vData)[0].back();
 de-=aDelays[idx].length;
 de+=ROUNDOFF;
 if (de==aDelays[idx].evol)
 	dd=aDelays[idx].value;
 else
 {
  aDelays[idx].evol=de;
  pos=std::upper_bound((*vData)[0].begin(),(*vData)[0].end(),de)-(*vData)[0].begin();
#ifdef _DEBUG
 qDebug("[Interpolation (%d): pos=%d,d=%f,x=%f,l=%f",idx,pos,de,(*vData)[0].back(),aDelays[idx].length);
#endif
 //check if we are before time
  if (pos==0)
 	return dd;
 //now we have the time point around which we create an interpolation function
 // de=(*vData)[0].back();
 // de-=aDelays[idx].length;
  if (pos>INTERPOL)
  {
   nb=pos-INTERPOL;
   ne=pos+INTERPOL;
  }
  else
  {
   nb=0;
   ne=2*INTERPOL;
  }
  if (ne>=(*vData)[0].size())
  	ne=(*vData)[0].size();
  pos=ne-nb;
#ifdef _DEBUG
 qDebug("Interpolation: ne=%d,nb=%d,pos=%d",ne,nb,pos);
#endif
 //create data series from which to interpolate
 //double px[pos];
 //double py[pos];
  double *px=new double[pos];
  double *py=new double[pos]; 
  for (unsigned int i=0;i<pos;i++)
 	{
	 px[i]=(*vData)[0][i+nb];
	 py[i]=(*vData)[n][i+nb];
#ifdef _DEBUG
 //qDebug("Interpolation: i=%d,x=%f,y=%f",i,px[i],py[i]);
#endif
	}
  //create interpolation function
/*  gsl_interp_accel *acc=gsl_interp_accel_alloc();
  gsl_spline *spline;
  switch(t)
  {
   default:
   case 0: spline=gsl_spline_alloc(gsl_interp_cspline,pos); break;
   case 1: spline=gsl_spline_alloc(gsl_interp_linear,pos); break;
   case 2: spline=gsl_spline_alloc(gsl_interp_polynomial,pos); break;
   case 3: spline=gsl_spline_alloc(gsl_interp_cspline_periodic,pos); break;
   case 4: spline=gsl_spline_alloc(gsl_interp_akima,pos); break;
   case 5: spline=gsl_spline_alloc(gsl_interp_akima_periodic,pos); break;
  }
  gsl_spline_init(spline,px,py,pos);
  dd=gsl_spline_eval(spline,de,acc);
  gsl_spline_free(spline);
  gsl_interp_accel_free(acc);
  delete[] py;
  delete[] px;*/
 gsl_interp_accel *acc=gsl_interp_accel_alloc();
 gsl_interp *spline;
 switch(t)
 {
  default:
  case 0: spline=gsl_interp_alloc(gsl_interp_cspline,pos); break;
  case 1: spline=gsl_interp_alloc(gsl_interp_linear,pos); break;
  case 2: spline=gsl_interp_alloc(gsl_interp_polynomial,pos); break;
  case 3: spline=gsl_interp_alloc(gsl_interp_cspline_periodic,pos); break;
  case 4: spline=gsl_interp_alloc(gsl_interp_akima,pos); break;
  case 5: spline=gsl_interp_alloc(gsl_interp_akima_periodic,pos); break;
 }
 gsl_interp_init(spline,px,py,pos);
 //if(gsl_interp_eval_e(spline,px,py,de,acc,&dd)!=0)
 if(gsl_interp_eval_e(spline,px,py,de,acc,&dd)!=0)
   dd=aDelays[idx].value;
 gsl_interp_free(spline);
 gsl_interp_accel_free(acc);
 delete[] py;
 delete[] px;
 }
 if (gsl_isnan(dd))
  	dd=aDelays[idx].value;
 else
 	aDelays[idx].value=dd;
#ifdef _DEBUG
 qDebug("Interpolation (%d): de=%f,dd=%f]\n",idx,de,dd);
#endif
 return dd;
}

double formuClass::socketData(unsigned int idx)
{
 double dd;
// unsigned int sidx;
 
 try
 {
   //sidx=dynamic_cast<unsigned int>(streamIdx);
   dd=aSocks[idx].value;
// #ifdef _DEBUG
// 	qDebug("[formuClass::socketData] socket idx %d data: %f",idx,dd);
// #endif
 }
 catch (std::exception &stdex)
 {
#ifdef _DEBUG
	qDebug("[formuClass::socketData] exception in socket data: %s",stdex.what());
#endif
  return DBL_MIN;
 }
 return dd;
}

/**********************************************************/
/* Evaluating functions                                   */

int formuClass::get_var_idx(const char *v)
{
 #ifdef FORMU_DEBUG
  qDebug("\tGetvaridx %s.",v);
 #endif
 return isdefined((char *)v,strlen(v));
}

unsigned int formuClass::get_num_var()
{
 return aVars.size();
}

const char *formuClass::get_var_name(unsigned int n)
{
 Variable *pAVar;
 if(n<aVars.size())
 	{
 	 pAVar=&aVars[n];
 	 fset_error(pAVar->Name.c_str());	/*this way you can get the name always*/
 	 return pAVar->Name.c_str();
 	}
 else
 	return NULL;
}

unsigned int formuClass::get_var_count()
{
 return aVars.size();
}

TEqTypes formuClass::get_var_type(unsigned int n)
{
 if(n<aVars.size())
         return aVars[n].eqType;
 return etUnknown;
}

TEqTypes formuClass::GetVariableType(unsigned int n)
{
 Variable *pAVar;
 unsigned int idx;
 if(n<var.size())
 	{
         idx=var[n];
         if (idx<aVars.size())
         {
          pAVar=&aVars[idx];
          return pAVar->eqType;
         }
 	}
 return etUnknown;
}

TEqTypes formuClass::GetVariableType(const char *p)
{
 Variable *pAVar;
 int idx=get_var_idx(p);
 if(idx>=0)
 	{
         pAVar=&aVars[idx];
         return pAVar->eqType;
 	}
 return etUnknown;
}

const char *formuClass::GetVariableName(unsigned int n)
{
 Variable *pAVar;
 unsigned int idx;
 if(n<var.size())
 	{
         idx=var[n];
         if (idx<aVars.size())
         {
 	      pAVar=&aVars[idx];
          fset_error(pAVar->Name.c_str());	/*this way you can get the name always*/
          return pAVar->Name.c_str();
         }
 	}
 return NULL;
}

bool formuClass::SetVariableName(unsigned int n,const char *p)
{
 Variable *pAVar;
 unsigned int idx;
 bool b=false;
 
 if (isdefined(p,strlen(p))>=0)
  return b;
 if(n<var.size())
 	{
         idx=var[n];
         if (idx<aVars.size())
         {
 	  pAVar=&aVars[idx];
	  pAVar->Name=p;
	  b=true;
         }
 	}
 return b;
}

const char *formuClass::GetParameterName(unsigned int n)
{
 Variable *pAVar;
 unsigned int idx;
 if(n<param.size())
 	{
         idx=param[n];
         if (idx<aVars.size())
         {
 	  pAVar=&aVars[idx];
          fset_error(pAVar->Name.c_str());	/*this way you can get the name always*/
 	  return pAVar->Name.c_str();
         }
 	}
 return NULL;
}

double formuClass::GetVariableVal(unsigned int n)
{
 unsigned int idx;
 if(n<var.size())
 	{
         idx=var[n];
         if (idx<aVars.size())
          return aVars[idx].value;
 	}
 return 0.0;
}

double formuClass::GetVariableIniVal(unsigned int n)
{
 unsigned int idx;
 if(n<var.size())
 	{
         idx=var[n];
         if (idx<aVars.size())
          return aVars[idx].initval;
 	}
 return 0.0;
}
double formuClass::GetVariableVal(QString &qs)
{
 int idx;

 idx=get_var_idx((const char *)qs);
 if (idx>=0)
	 return aVars[idx].value;
 return 0.0;
}

double formuClass::GetParameterVal(unsigned int n)
{
 unsigned int idx;
 if(n<param.size())
 	{
         idx=param[n];
         if (idx<aVars.size())
          return aVars[idx].value;
 	}
 return 0.0;
}

double formuClass::GetConstantVal(unsigned int n)
{
 if(n<aConsts.size())
          return aConsts[n];
 return 0.0;
}

void formuClass::reset()
{
 unsigned int i, idx;
 for(i=0;i<var.size();i++)
 	{
         idx=var[i];
         if (idx<aVars.size())
	 	{
#ifdef _DEBUG
	qDebug("[formuClass:reset]vars i=%d, idx=%d, value=%g, initval=%g",i,idx,aVars[idx].value,aVars[idx].initval);
#endif
  		 aVars[idx].value=aVars[idx].initval;
		}
	}
 npcount=0;
 npstep=-1;
 while(dblstack.size()>0) dblstack.pop();
}

void formuClass::make_var(const char *var, double value)
{
  int idx=isdefined(var,strlen(var));
  if (idx>=0)
      aVars[idx].value=value;
}

void formuClass::make_ini(const char *var, double value)
{
  int idx=isdefined(var,strlen(var));
  if (idx>=0)
      aVars[idx].initval=value;
}

void formuClass::make_idxvar(unsigned int idx, double value)
{
 if (idx<aVars.size())
      aVars[idx].value=value;
}

void formuClass::MakeVarVal(unsigned int n, double value)
{
 unsigned int idx;
 if(n<var.size())
  {
     idx=var[n];
     if (idx<aVars.size())
 	  aVars[idx].value=value;
  }
}

unsigned int formuClass::GetVarIdx(const char *p)
{
 unsigned int idx,i;
 for (i=0;i<var.size();i++)
 	{
         idx=var[i];
         if (aVars[idx].Name==p)
 	  return i;
 	}
 return IDINVALID;
}

void formuClass::MakeVarIniVal(unsigned int n, double value)
{
 unsigned int idx;
 if(n<var.size())
  {
     idx=var[n];
     if (idx<aVars.size())
 	  aVars[idx].initval=value;
  }
}

void formuClass::MakeParamVal(unsigned int n, double value)
{
 unsigned int idx;
 if(n<param.size())
  {
     idx=param[n];
     if (idx<aVars.size())
 	  aVars[idx].value=value;
  }
}

void formuClass::MakeParamVal(const char *var, double value)
{
  int idx=isdefined(var,strlen(var));
  if (idx>=0)
      aVars[idx].value=value;
}

bool formuClass::IsImmediate(unsigned int n)
{
 unsigned int idx;
 if(n<var.size())
  {
     idx=var[n];
     if (idx<aVars.size())
 	  return (aVars[idx].eqType==etImm);
  }
 return false;
}

bool formuClass::IsDifferential(unsigned int n)
{
 unsigned int idx;
 if(n<var.size())
  {
     idx=var[n];
     if (idx<aVars.size())
 	  return (aVars[idx].eqType==etDeriv);
  }
 return false;
}

bool formuClass::IsMap(unsigned int n)
{
 unsigned int idx;
 if(n<var.size())
  {
     idx=var[n];
     if (idx<aVars.size())
 	  return (aVars[idx].eqType==etMap);
  }
 return false;
}

const DblVector *formuClass::evalue()
{
 //first load the constants
 ctable=aConsts;
 for (unsigned int i=0;i<code.size();i++)
     {
      // Some Brilliant hacker at gcc thought it was a good idea to return the 
      // type of begin() as a class __normal_iterator, so you can iterate through
      // the vector independent of type. (isn't that what they are for?; also do not mix types!)
      // Anyway, the UNDOCUMENTED function base() gives you the actual pointer
      // that you wanted in the first place......
      results[i]=value(code[i].begin().base(),code[i].size());
      //change values of immediates
      if (IsImmediate(i))
        MakeVarVal(i,results[i]);
     }
 return &results;
}

double formuClass::value(unsigned int *function, unsigned int len)
{
 double y,result;
 unsigned int index;
 unsigned int stksize=dblstack.size();   //get the current stack size, at the end of
                                //this function this must be the same.

 if(len<1)
 {
   fset_error("empty coded function");
   return 0.0; // non-existent function; result of an unsuccesful call to translate
 }
 for(index=0;index<len;index++)
  {
   switch(function[index])
   {
    case '\0': break;
    case CHARDOUBLE:
             { //push the next value on the stack:
              index++;
              dblstack.push(ctable[function[index]]);
              break;
             }
    case CHARVAR:
             {
              index++;
              dblstack.push(aVars[function[index]].value);
              break;
             }
   case CHAREXT:
             {
              index++;
              result=aExtVars[function[index]].aVars[function[index+1]].value;
	      dblstack.push(result);
              index++;
	      break;
	     }
   case CHARDEL:
	     {
	      index++;
	      aDelays[function[index]].length=dblstack.top();
	      dblstack.pop();
	      //ignore variable value:
	      dblstack.pop();
	      result=findDelay(function[index]);
	      dblstack.push(result);
	      break;
	     }
   case CHARIDEL:
	     {
	      index++;
	      //x=type of interpolation
	    //  x=dblstack.top();
	     // dblstack.pop();
	      aDelays[function[index]].length=dblstack.top();
	      dblstack.pop();
	      //ignore variable value:
	      dblstack.pop();
	      result=interpolateDelay(function[index],0);
	      dblstack.push(result);
	      break;
	     }
   case CHARSOCK:
	    {
	      index++;
	      //y is number of stream, not needed
	      //y=dblstack.top();
	      dblstack.pop();
	      //also pop off stream id, not needed
	      dblstack.pop();
	      result=socketData(function[index]);
	      dblstack.push(result);
	      break;
	    }
    case CHARNEG:
            {
              //negate value on stack, and push new value
	         result = -dblstack.top();
		 dblstack.pop();
	         dblstack.push(result);
             break;
            }
    case '+':
            {
	         y = dblstack.top();
		 dblstack.pop();
	         result = y + dblstack.top();
		 dblstack.pop();
	         dblstack.push(result);
             break;
            }
    case '-':
            {
	         y = dblstack.top();
		 dblstack.pop();
	         result= dblstack.top() - y;
		 dblstack.pop();
	         dblstack.push(result);
	         break;
            }
    case '*':
	    {
	         y = dblstack.top();
		 dblstack.pop();
	         result= dblstack.top() * y;
		 dblstack.pop();
	         dblstack.push(result);
	         break;
            }
    case '/':
            {
	         y = dblstack.top();
		 dblstack.pop();
	         result= dblstack.top() / y;
		 dblstack.pop();
	         dblstack.push(result);
	         break;
            }
    case '^':
            {
	         y = dblstack.top();
		 dblstack.pop();
	         result= pow(dblstack.top(),y);
		 dblstack.pop();
	         dblstack.push(result);
	         break;
            }
    case CHARFUNC:
            {
             index++;
	     double x,z,p,q;
    	     switch(ftable[function[index]].n_pars)
              {
	           case 0:
                      {
                       dblstack.push(((Func0)ftable[function[index]].f)());
		       break;
                      }
	           case 1:
                      {
                       x = dblstack.top();
		       dblstack.pop();
		       dblstack.push(ftable[function[index]].f(x));
		       break;
                      }
	           case 2:
                      {
                       y = dblstack.top();
		       dblstack.pop();
                       x = dblstack.top();
		       dblstack.pop();
        	       dblstack.push(((Func2)ftable[function[index]].f)(x,y));
		       break;
                      }
	           case 3:
                      {
                       z = dblstack.top();
		       dblstack.pop();
		       y = dblstack.top();
		       dblstack.pop();
		       x = dblstack.top();
		       dblstack.pop();
		       dblstack.push(((Func3)ftable[function[index]].f)(x,y,z));
		       break;
                      }
	           case 4:
                      {
                       p = dblstack.top();
		       dblstack.pop();
                       z = dblstack.top();
		       dblstack.pop();
		       y = dblstack.top();
		       dblstack.pop();
		       x = dblstack.top();
		       dblstack.pop();
		       dblstack.push(((Func4)ftable[function[index]].f)(x,y,z,p));
		       break;
                      }
	           case 5:
                      {
                       q = dblstack.top();
		       dblstack.pop();
                       p = dblstack.top();
		       dblstack.pop();
                       z = dblstack.top();
		       dblstack.pop();
		       y = dblstack.top();
		       dblstack.pop();
		       x = dblstack.top();
		       dblstack.pop();
		       dblstack.push(((Func5)ftable[function[index]].f)(x,y,z,p,q));
		       break;
                      }
	           default:
                      {
                       fset_error("I2: too many parameters");
		       return 0.0;
                      }
	         }
	         break;
            }
    default:
           {
            fset_error("I1: unrecognised operator");
	    return 0.0;
           }
   }
  }
 //check if we're out of stack, or not
 result=dblstack.size();
 if(result!=stksize+1)
 {
  fset_error("I3: corrupted buffer");
 }
 
 result=dblstack.top();
 dblstack.pop();
 return result;
} /* end of value */

/**********************************************************/
/* Manipulation of data of type formuClass                */

formuClass::formuClass()
{
 fset_error(NULL);
 npcount=0;
 npstep=-1;
}

formuClass::~formuClass()
{
 make_empty();
}

void formuClass::make_empty()
{
 fset_error(NULL);
 code.erase(code.begin(),code.end());
 src.erase(src.begin(),src.end());
 ctable.erase(ctable.begin(),ctable.end());
 results.erase(results.begin(),results.end());
 aVars.erase(aVars.begin(),aVars.end());
 aExtVars.erase(aExtVars.begin(),aExtVars.end());
 aConsts.erase(aConsts.begin(),aConsts.end());
 var.erase(var.begin(),var.end());
 param.erase(param.begin(),param.end());
}

bool formuClass::IsEmpty()
{
 fset_error(NULL);
 if (code.size()>0)
 	return true;
 return false;
}

const char *formuClass::GetEquation(unsigned int n)
{
 if (n<src.size())
  return src[n].c_str();
 return NULL;
}

const char *formuClass::GetErrorEquation()
{
 if (error==0)
	return NULL;
 unsigned int err=error-1;
 if (err<src.size())
  return src[err].c_str();
 return NULL;
}

int formuClass::GetErrorChar()
{
 return cn_err;
}
/*********************************************************/
/* Interpreting functions                                */

static int isoper(char c)
{
 return ((c == '+') || (c == '-') || (c == '*') || (c == '/')
		    || (c == '^'));
}

static int is_code_oper(int c)
{
 return ((c == '+') || (c == '-') || (c == '*') || (c == '/')
		    || (c == '^') || (c == CHARNEG));
}
static int isin_real(char c)
/* + and - are not included */
{
 return (isdigit(c) || c=='.' || c=='E');
}

size_t formuClass::max_size(const char *source)
/* gives an upper estimate of the size required for
   the coded form of source (including the final '\0') */
/* Take care when modifying: the upper estimate
   returned by max_size must not also accomodate
   *proper* output, but also *improper* output
   which takes place before the translator detects an error. */
{
 int numbers=0;
 int functions=0;
 int operators=0;
 int variables=0;

 const size_t var_size=3*sizeof(int);
 const size_t num_size=sizeof(int)+sizeof(double);
 const size_t op_size=sizeof(int);
 const size_t end_size=sizeof('\0');

 const char *scan;

 for(scan=source; *scan; scan++)
  if(isalpha(*scan) && (*scan != 'E'))
  {
    if(isalpha(*(scan+1))) ; /* it is a function name,
				it will be counted later on */
    else
     if(*(scan+1) == '(')  functions++;
     else variables++;
  }

 if(isoper(*source)) operators++;
 if(*source != '\0')
  for(scan = source+1; *scan; scan++)
   if(isoper(*scan) && *(scan-1) != 'E') operators++;

 /* counting numbers.. */
 scan=source;
 while(*scan)
  if(isin_real(*scan) || ((*scan == '+' || *scan == '-') &&
			   scan>source && *(scan-1)=='E'))
   {numbers++;
    scan++;
    while(isin_real(*scan) || ((*scan == '+' || *scan == '-') &&
				scan>source && *(scan-1)=='E'))
     scan++;
   }
  else scan++;
 return(numbers*num_size + operators*op_size + functions*num_size
			 + variables*var_size + end_size);
 /*Do you wonder why "function" is multiplied with "num_size"
   and not with func_size? This function calculates an upper-bound
   (i.e. pessimistic) estimate. It supposes that all functions are
   converted into doubles by comp_time. For example, pi() actually
   becomes a double. */
}

/***********************************************************/
/* Interface for interpreting functions                     */
void formuClass::AddSource(char *p)
{
 if (p==NULL)
    return;
 src.push_back(p);
  #ifdef FORMU_DEBUG
  qDebug("DEBUG: addsource ,  [%s]",p);
 #endif   
}

void formuClass::AddVariable(char *p, TEqTypes t)
{
 int idx;

 if (p==NULL)
    return;
 idx=isdefined(p,strlen(p));
 if (idx==-1)
 {
  Variable AVar;
  AVar.Name=p;
  AVar.id=aVars.size();
  AVar.value=0.0;
  AVar.initval=0.0;
  AVar.eqType=t;
  aVars.push_back(AVar);
 }
 else
 {//update it to the new type
  switch (aVars[idx].eqType)
  {
    case etUnknown:  aVars[idx].eqType=t; break;
    case etParam:
    	{
	 switch (t)
	 {
    	   case etVar:
	   case etDeriv:
	   case etMap:
	   case etImm: aVars[idx].eqType=etParAsVar; break;
	   default: break;
	 }
 	}
   default: break;
  }
 }
}

bool formuClass::translate(char **p,int l)
{
 std::string s;
 int i;
 char *ps;

 if (p==NULL)
    return false;
 for (i=0;i<l;i++)
     {
      ps=p[i];
      if (ps==NULL)
	 break;
      s=ps;
      src.push_back(s);
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: translate , ps %s, s %s (%d).",ps,s.c_str(),l);
 #endif   
     }
 if (src.size()>0)
    {
     return (Convert()==0);
    }
 return false;
}

int formuClass::Convert()
{
 unsigned int *result, *function;
 unsigned int i;
 size_t size_estim; /* upper bound for the size of the
					coded function */
 i_error=NULL;

 bExtMode=false;
 try
 {
  ctable.clear();
  for (i=0;i<src.size();i++)
  {
   size_estim=max_size(src[i].c_str()); /* upper estimate of the size
				 of the coded function,
				 which doesn't exist yet */
   function=new unsigned int[size_estim];
   for(unsigned int j=0; j<size_estim; j++)
   	function[j]=0;
   fset_error(NULL);
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: convert , begin %s, end %s (%d).",src[i].begin().base(),src[i].end().base(),(int)distance(src[i].begin(),src[i].end()));
 #endif   
   /* THIS IS THE CORE STATEMENT */
   result=i_trans(function,src[i].begin().base(),src[i].end().base());

   if(!result || fget_error())
   {
    if(i_error)
    {
     error = i+1;
     cn_err=i_error-src[i].begin().base();
    }
    else error = -1; /* internal error or out of memory */
    delete[] function;
    return error;
   }
   /* OK */
   error = -1;
   IntVector iv;
   iv.assign(function,result);
   iv.push_back(0);
   code.push_back(iv);
   results.push_back(0.0);
   delete[] function;
  }
  //solve the constants problem
  aConsts=ctable; 
  //solve the type problem:
  for (i=0;i<aVars.size();i++)
       {
	if ((aVars[i].eqType==etParam)||(aVars[i].eqType==etParAsVar))
	   param.push_back(aVars[i].id);
	else
	   var.push_back(aVars[i].id);
       }
  //solve the delay problem
  for (i=0; i<aDelays.size();i++)
       {
	for (unsigned int j=0;j<var.size();j++)
	 {
	  if (aDelays[i].varid==var[j])
	  {
	   aDelays[i].varid=j;
	   break;
	  }
	 }
       }
 } //end of try
 catch (std::exception &)
 {// oh,dear...
  return error;
 }
 error=0;
 fset_error(NULL);
 return 0;
}  /* end of translate */

double formuClass::NumEval(char *p)
{
 char *end;
 unsigned int *function;
 double d;

 unsigned int size_estim=max_size(p); /* upper estimate of the size
				 of the coded function,
				 which doesn't exist yet */
 ctable.clear();
 function=new unsigned int[size_estim];
 //memset(function,0,(size_estim*sizeof(unsigned int)));
 for(unsigned int j=0; j<size_estim; j++)
   	function[j]=0;
 fset_error(NULL);
 end=strchr(p,0);
 i_trans(function,p,end);
 d=ctable.back();
 delete[] function;
 ctable.clear();
 return d;
}

unsigned int *formuClass::comp_time(unsigned int *function, unsigned int *fend, unsigned int npars)
  /* calculates at "compile time" */
  /* Postconditions: If the coded expression in *function..*(fend-1)
      can be calculated, its value is stored in *function..*(fend-1) */
  /* comp_time returns a pointer to the first character after the
     end of the coded function; if this function cannot be evaluated
     at compile time, comp_time returns fend, of course.  */
  /* Only memory positions from *function to *comp_time are touched. */
{
  unsigned int *scan;
  int temp;
  double tempd;
  unsigned int i;

  #ifdef FORMU_DEBUG
   qDebug("Entering comp_time");
  #endif
  scan=function;
  for(i=0; i<npars; i++) {
   if(*scan++ != CHARDOUBLE) return fend;
   scan++;
  }

  if(!( ( (scan == fend - 2)&&
         ((*(fend-2) == CHARFUNC) && ftable[*(fend-1)].varying == 0)) ||
	 ( (scan == fend - 1) &&
          is_code_oper(*(fend-1)) ) )
    )
    {
    /* compile-time evaluation is done only if
       1) everything but the ending function consists of doubles
       AND
       2) the function does not vary when its parameters remain the same
          (i.e. random-number generators are not evaluated at compile time)
	  */
  #ifdef FORMU_DEBUG
   qDebug("Exiting comp_time succesfully:nothing to do");
  #endif
   return fend;
  }
  temp = *fend;
  *fend = '\0';

  tempd = value(function,fend-function);
  *fend = temp;
  //the following caused errors in compiled code:
  //the new algorithm of only storing single copies
  //of doubles may mean that some values are here
  //incorrectly deleted, replaced with new code
  //Old code:
  //add double to list and remove npars values
//   *function++ = CHARDOUBLE;
//   temp = ctable.size()-npars;
//   *function++ = temp;
//   ctable.erase(ctable.end()-npars,ctable.end());
//   ctable.push_back(tempd);
// New code:
  DblVector::iterator it=find(ctable.begin(),ctable.end(),tempd);
  if (it!=ctable.end())
  {
    *function++ = CHARDOUBLE;
    *function++ = distance(ctable.begin(),it);
  }
  else
  {
    *function++ = CHARDOUBLE;
    *function++ = ctable.size();
    ctable.push_back(tempd);
  }
  #ifdef FORMU_DEBUG
   qDebug("Exiting comp_time succesfully");
  #endif
  return function;
} /* end of comp_time */

static char *my_strtok(char *s)
/* a version of strtok that respects parentheses */
/* token delimiter = comma */
{
 int pars;
 static char *token=NULL;
 char *next_token;

 if(s!=NULL) token=s;
 else if(token!=NULL) s=token;
 else return NULL;

 for(pars=0; *s != '\0' && (*s != ',' || pars!=0); s++) {
   if(*s == '(') ++pars;
   if(*s == ')') --pars;
 }
 if(*s=='\0') {
  next_token=NULL;
  s=token;

  token=next_token;
  #ifdef FORMU_DEBUG
   qDebug("The token is: %s",s);
  #endif
  return s;
 } else {
  *s = '\0';
  next_token=s+1;
  s=token;

  token=next_token;
  #ifdef FORMU_DEBUG
   qDebug("The token is: %s",s);
  #endif
  return s;
 }
} /* end of my_strtok */


/************************************************************/
/* Here begins the core of interpretation                   */

#define TWO_OP {                                 \
    tempu=i_trans(function,begin,scan);	\
    if (tempu) /* Added ToS, don't do next i_trans if an error occurred */\
     temp3=i_trans(tempu,scan+1,end);	\
    if(tempu && temp3 ) {       \
    *temp3++ = *scan; /* copies operator */                 \
     temp3 = comp_time(function,temp3,2); /*tries to simplify expression*/ \
   if(fget_error()) return NULL; /* internal error in comp_time */  \
   else return temp3; /* expression has been translated */ \
  } else return NULL; /* something is wrong with the operands */ \
 }

#define ERROR_MEM {    \
   fset_error("no memory"); \
   i_error=NULL;  \
   return NULL;   \
  }
unsigned int *formuClass::i_trans(unsigned int *function, char *begin, char *end)
 /* the source is *begin .. *(end-1) */
 /* returns NULL if a normal error or an internal error occured;
    otherwise, returns a pointer
    to the first character after the end of function[] */
 /* i_trans() does not write a '\0' at the end of function[], */
 /* but it MAY touch its end (i.e. *i_trans) without changing it.*/
{
 int pars;     /* parentheses */
 char *scan;
 unsigned int *tempu, *temp3=NULL;
 char *temps=0;
 char tempch;
 double tempd;
 char *endf;     /* points to the opening
		    parenthesis of a function (e.g. of sin(x) ) */
 int n_function;
 int space;
 int i;
 int isdef;

 char *paramstr[MAXPAR];
 char *par_buf;

 if(begin>=end) {
  fset_error("missing operand");
  i_error = begin;
  return NULL;
 }

 /* test for invalid symbols */
 for(pars=0, scan=begin; scan<end && pars>=0; scan++) {
  if (!strchr(validChars,*scan))
  	{
	 fset_error("syntax error");
	 i_error=scan;
	 return NULL;
	}
  pars++;
 } 
 /* test paired parentheses */
 for(pars=0, scan=begin; scan<end && pars>=0; scan++) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
 }
 if(pars<0 || pars>0) {
  fset_error("unmatched parentheses");
  i_error = scan-1;
  return NULL;
 }

 /* test paired square parentheses */
 for(pars=0, scan=begin; scan<end && pars>=0; scan++) {
  if(*scan == '[') pars++;
  else if(*scan == ']') pars--;
 }
 if(pars<0 || pars>0) {
  fset_error("unmatched square parentheses");
  i_error = scan-1;
  return NULL;
 }

 /* plus and binary minus */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '+' || ((*scan == '-') && scan!=begin))
					  /* recognizes unary
					     minuses */
	     && (scan==begin || *(scan-1)!='E') )
	  /* be wary of misunderstanding exponential notation */
   break;
 }

 if(scan >= begin) TWO_OP

 /* multiply and divide */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '*' || *scan == '/' ))
   break;
 }

 if(scan >= begin) TWO_OP

 /* unary minus */
 if(*begin == '-') {
   tempu=i_trans(function,begin+1,end);
   if(tempu) {
     *tempu++ = CHARNEG;
     /*
      27/10/2006
      Found a problem with this bit of code, the comp_time code
      simplifies if only doubles or charneg exist in the function,
      e.g. -2*3
     */
     tempu=comp_time(function,tempu,1); /*tries to simplify expression*/
     if(fget_error()) return NULL; /* internal error in comp_time */
     else return tempu;
   } else return NULL;
 }

 /* power */
 for(pars=0, scan=end-1; scan>=begin; scan--) {
  if(*scan == '(') pars++;
  else if(*scan == ')') pars--;
  else if(!pars && (*scan == '^'))
   break;
 }

 if(scan >= begin) TWO_OP

 /* erase white space */
 while(isspace(*begin))
  begin++;
 while(isspace(*(end-1)))
  end--;

 /* parentheses around the expression */
 if(*begin == '(' && *(end-1) == ')')
  return i_trans(function,begin+1,end-1);

 /* number */
 tempch = *end;
 *end = '\0';
 tempd=strtod(begin,(char**) &tempu);
 *end = tempch;
 if((char*) tempu == end)
 {
  // Modified 23/2/2005
  // Changed from always adding constant explicitly
  // to finding constant in list and replacing reference with
  // already existing constant, thereby reducing duplication.
 /* Old code:
    *function++ = CHARDOUBLE;
  *function++ = ctable.size();
  ctable.push_back(tempd);
 */
  // New code
  DblVector::iterator it=find(ctable.begin(),ctable.end(),tempd);
  if (it!=ctable.end())
  {
    *function++ = CHARDOUBLE;
    *function++ = distance(ctable.begin(),it);
  }
  else
  {
    *function++ = CHARDOUBLE;
    *function++ = ctable.size();
    ctable.push_back(tempd);
  } 
  return function;
 }

 /*function*/
 if(!isalpha(*begin) && *begin != '_')
			/* underscores are allowed */
 {
  fset_error("syntax error");
  i_error=begin;
  return NULL;
 }

 for(endf = begin+1; endf<end && (isalnum(*endf) || *endf=='_');endf++);

 /* variable */
 #ifdef FORMU_DEBUG
  qDebug("DEBUG: where_table, begin %s, end %d.",begin,(int)(endf-begin));
 #endif
 if((n_function=where_table(begin,(endf-begin))) == -1) {
/*  lets say this is a variable */
     fset_error(NULL);
     /* check if it s an external reference: */
     if(*endf == '[' || *(end-1) == ']') {
         //check if we're not already in extmode:
         if (bExtMode)
	    {
             fset_error("no nested external references allowed");
             i_error=begin;
	     return NULL;
            }
         // if it is make it one:
         bExtMode=true;
         tempch = *(end-1);
         *(end-1) = '\0';
         par_buf = new char[(strlen(endf+1)+1)];
         if(!par_buf)
                     ERROR_MEM;
         strcpy(par_buf, endf+1);
         *(end-1) = tempch;
         // see if there is at least one parameter
         if( (temps=my_strtok(par_buf)) == NULL )
          {
           // too few parameters
	   delete[] par_buf;
	   i_error=end-2;
	   fset_error("too few parameters in external reference");
	   return NULL;
	  }
	 if((temps=my_strtok(NULL))!=NULL)
	  {
	   // too many parameters
	   delete[] par_buf;
	   i_error=(temps-par_buf)+(endf+1);
	    // points to the first character of the first superfluous parameter
	   fset_error("too many parameters in external reference");
	   return NULL;
	  }
	 tempu=function;
	 tempu=i_trans(tempu, par_buf, par_buf+strlen(par_buf));
	 if(!tempu)
	     {
              i_error=(i_error-par_buf)+(endf+1);
              // moves i_error to the permanent copy of the parameter
              delete[] par_buf;
              return NULL; // error in one of the parameters
             }
         // OK
         int n,extdef;
         bExtMode=false;
         delete[] par_buf;
         isdef=isextdefined(begin,(endf-begin));
         if (isdef==-1)
     	 {
          fset_error(NULL);
          ExtVariable ExtVar;
          ExtVar.Name.assign(begin,(endf-begin));
	      ExtVar.id=aExtVars.size();
          ExtVar.nOp=0;
          ExtVar.extid=IDINVALID;
          aExtVars.push_back(ExtVar);
          //now get the previous object and store it in extvar:
          tempu-=2;
          if (*tempu==CHARVAR)
		{
                 tempu++;
                 n=*tempu--;
		}
          else
                {
	         fset_error("invalid external reference");
                 return NULL;
                }
          *tempu++ = CHAREXT;
          *tempu++ = aExtVars.size()-1;
          Variable AVar;
          if (n!=-1)
           AVar=aVars[n];
          else
           AVar=AGlobVar;
          AVar.id=0;
          aExtVars[aExtVars.size()-1].aVars.push_back(AVar);
          aExtVars[aExtVars.size()-1].iVars.push_back(IDINVALID);
	      *tempu++ = 0;
	      tempu = comp_time(function,tempu,1);
	      if(fget_error()) return NULL; // internal error in comp_time
	       else return tempu;
	   }
	 //now get the previous object and store it in extvar:
	 tempu-=2;
	 if (*tempu==CHARVAR)
		{
		 tempu++;
		 n=*tempu--;
		}
	     else
		{
		 fset_error("invalid external reference");
		 return NULL;
		}
	*tempu++ = CHAREXT;
	*tempu++ = aExtVars[isdef].id;
        if (n!=-1)
            extdef=isinextdefined(aExtVars[isdef],aVars[n].Name);
	else
            extdef=isinextdefined(aExtVars[isdef],AGlobVar.Name);
        if (extdef==-1)
        {
         fset_error(NULL);
         Variable AVar;
          if (n!=-1)
           AVar=aVars[n];
	  else
           AVar=AGlobVar;
         AVar.id=aExtVars[isdef].aVars.size();
	 aExtVars[isdef].aVars.push_back(AVar);
         aExtVars[isdef].iVars.push_back(IDINVALID);
         *tempu++ = AVar.id;
        }
        else
	 *tempu++ = extdef;
        tempu = comp_time(function,tempu,1);
	if(fget_error()) return NULL; // internal error in comp_time
        else return tempu;
       }
     isdef=isdefined(begin,(endf-begin));
     if (isdef==-1)
	{
	 /*new variable, so reset error status */
	 fset_error(NULL);
	 if (!bExtMode)
	 {
	  Variable AVar;
	  AVar.Name.assign(begin,(endf-begin));
	  AVar.id=aVars.size();
	  AVar.value=0.0;
	  AVar.initval=0.0;
	  AVar.eqType=etUnknown;
	  *function++ = CHARVAR;
	  *function++ = aVars.size();
	  aVars.push_back(AVar);
	 }
	 else
	 {
	  AGlobVar.Name.assign(begin,(endf-begin));
	  AGlobVar.id=0;
	  AGlobVar.value=0.0;
	  AGlobVar.initval=0.0;
	  AGlobVar.eqType=etUnknown;
	  *function++ = CHARVAR;
	  *function++ = IDINVALID;
	 }
	 return function;
	}
     /*increase function, pAVars still points to the variable */
     *function++ = CHARVAR;
     *function++ = aVars[isdef].id;
     return function;
 }

 if(*endf != '(' || *(end-1) != ')') {

  fset_error("improper function syntax");
  i_error=endf;
  return NULL;
 }
 if(ftable[n_function].n_pars==0) {
  /*function without parameters (e.g. pi() ) */
   space=1;
   for(scan=endf+1; scan<(end-1); scan++)
    if(!isspace(*scan)) space=0;
   if(space) {
    *function++ = CHARFUNC;
    *function++ = n_function;
    function = comp_time(function-2,function,0);
    if(fget_error()) return NULL; /* internal error in comp_time */
    else return function;
   } else {
    i_error=endf+1;
    fset_error("too many parameters in function");
    return NULL;
   }
 } else {    /*function with parameters*/
    tempch = *(end-1);
    *(end-1) = '\0';
    par_buf = new char[strlen(endf+1)+1];
    if(!par_buf)
     ERROR_MEM;
    strcpy(par_buf, endf+1);
    *(end-1) = tempch;
    /* look at the first parameter */
    for(i=0; i<ftable[n_function].n_pars; i++) {
     if( ( temps=my_strtok((i==0) ? par_buf : NULL) ) == NULL )
      break; /* too few parameters */
     paramstr[i]=temps;
    }
    if(temps==NULL) {
     /* too few parameters */
     delete[] par_buf;
     i_error=end-2;
     fset_error("too few parameters in function");
     return NULL;
    }
    if((temps=my_strtok(NULL))!=NULL) {
     /* too many parameters */
     delete[] par_buf;
     i_error=(temps-par_buf)+(endf+1); /* points to the first character
					  of the first superfluous
					  parameter */
     fset_error("too many parameters in function");
     return NULL;
    }

    tempu=function;
    for(i=0; i<ftable[n_function].n_pars; i++)
    {
     tempu=i_trans(tempu, paramstr[i], paramstr[i]+strlen(paramstr[i]));
     if(!tempu)
     {
      i_error=(i_error-par_buf)+(endf+1); /* moves i_error to
					   the permanent copy of the
					   parameter */
      delete[] par_buf;
      return NULL; /* error in one of the parameters */
     }
    }
    /* OK */
    delete[] par_buf;
    //first check if it is a delay function
    switch(n_function)
    {
     case IDELAY_IDX:
     {
       if (*function!=CHARVAR)
	{
	 i_error=endf+1;
	 fset_error("invalid delay variable");
	 return NULL;
	}
      DelayedVar DVar;
      DVar.varid=*(function+1);
      DVar.id=aDelays.size();
      DVar.value=DBL_MIN;
      DVar.evol=DBL_MIN;
      DVar.length=0.0;
      *tempu++ = CHARIDEL;
      *tempu++ = aDelays.size();
      aDelays.push_back(DVar);
      break;
     }
     case DELAY_IDX:
     {//because each delay line may have different length, delays are NOT
      //checked and are always newly created
      // function points to first argument: a variable (V(n))
      if (*function!=CHARVAR)
	{
	 i_error=endf+1;
	 fset_error("invalid delay variable");
	 return NULL;
	}
      DelayedVar DVar;
      DVar.varid=*(function+1);
      DVar.id=aDelays.size();
      DVar.value=DBL_MIN;
      DVar.evol=DBL_MIN;
      DVar.length=0.0;
      *tempu++ = CHARDEL;
      *tempu++ = aDelays.size();
      aDelays.push_back(DVar);
      break;
     }
     case SOCKET_IDX:
     {
      if (*function!=CHARDOUBLE)
	{
	 i_error=endf+1;
	 fset_error("invalid socket number");
	 return NULL;
	}
      SocketVar SVar;
      SVar.sid=ctable[*(function+1)];
      if (*(function+2)!=CHARDOUBLE)
	{
	 i_error=endf+2;
	 fset_error("invalid number of socket streams");
	 return NULL;
	}
      SVar.num=ctable[*(function+3)];
      SVar.id=aSocks.size();
      SVar.value=DBL_MIN;
      *tempu++ = CHARSOCK;
      *tempu++ = aSocks.size();
      aSocks.push_back(SVar);
  #ifdef FORMU_DEBUG
   qDebug("DEBUG [i_trans]: socket id %u, sid %u, num %u",SVar.id,SVar.sid,SVar.num);
  #endif
      break;
     }
     default:
     {
      *tempu++ = CHARFUNC;
      *tempu++ = n_function;
      break;
     }
    }
    tempu = comp_time(function,tempu,ftable[n_function].n_pars);
    if(fget_error()) return NULL; /* internal error in comp_time */
    else return tempu;
 }
}

/* Here is the definition of some functions in the FORMULCPP standard
   library */
//---------------------------------------------------------------------------
// Math functions
//---------------------------------------------------------------------------
double Equal(double d1, double d2)
{
 //returns 1 if d1==d2 +/- ROUNDOFF
 if (((d1-ROUNDOFF)<d2)&&((d1+ROUNDOFF)>d2))
  return 1;
 return 0;
}

double And(double d1, double d2)
{
 //returns 1 if d1 AND d2 ie d1!=0 AND d2!=0
 if ((d1)&&(d2))
  return 1;
 return 0;
}

double Or(double d1, double d2)
{
 //returns 1 if d1 OR d2 ie d1!=0 or d2!=0
 if ((d1)||(d2))
  return 1;
 return 0;
}

double Xor(double d1, double d2)
{
 //returns 1 if d1 XOR d2 ie d1!=0 xor d1!=0
 if ((d1!=0.0)&&(d2!=0.0))
  return 0;
 if ((d1==0.0)&&(d2==0.0))
  return 0;
 return 1;
}

double Not(double d)
{
 if (d)
  return 0;
 return 1;
}

double Ran()
{
#ifdef USE_GSL
 return gsl_rng_uniform(ranGenerator);
#else
 return rnd();
#endif
}

double Random(double low, double high)
{
 double d;

#ifdef USE_GSL
 if (low==high)
  return (gsl_rng_uniform(ranGenerator)*high);
 d=high-low;
 return ((gsl_rng_uniform(ranGenerator)*d)+low);
#else
 if (low==high)
  return (rnd()*high);
 d=high-low;
 return ((rnd()*d)+low);
#endif
}

double Gauss(double s)
{
#ifdef USE_GSL
 return gsl_ran_gaussian(ranGenerator,s);
#else
return DBL_MIN;
#endif
}

double Gaussn(double m,double s)
{
#ifdef USE_GSL
 return gsl_ran_gaussian(ranGenerator,s)+m;
#else
return DBL_MIN;
#endif
}

double Step(double cur, double len, double stepsize)
{
 // increment/decrement when cur has passed len steps,
 // can not use the equal, because with relative large steps
 // the modulus is always much larger than roundoff
 // use stepsize to check for bounds
 double d=fmod(cur,len);
 if (((d-stepsize)<0.0)&&((d+stepsize)>0.0))
 {
//	 qDebug("[Formulc::Step]: within bounds!\n");
 	return 1.0;
 }
 return 0.0;
}

double Stair(double cur, double len, double old, double stepsize)
{
 double d=fmod(cur,len);
 if (((d-stepsize)<0.0)&&((d+stepsize)>0.0))
 {
//	 qDebug("[Formulc::Stair]: within bounds!\n");
 	return old+1.0;
 }
 return old;
}

double Pulse(double cur, double low, double high)
{
 // low indicates time of low signal, high time of high signal, off is offset
 // into time, begin with low signal (unless offset==low)

 double pos,per,phase;

 per=(low+high);
 if (per<=0.0)
 	return 0;
 phase=cur/per;
 //pos=fmod(phase,per);
 pos=phase-(int)phase;
 phase=low/per;
// #ifdef _DEBUG
//  qDebug("[Pulse] cur=%f, low=%f, high=%f, per=%f, phase=%f, pos=%f",cur,low,high,per,phase,pos);
// #endif
 if (pos>=phase)
 	return 1;
 else
 	return 0;
}

double Threshold(double var, double thres)
{
// var is variable to evaluate, thres the threshold, cut of value if above threshold
 if (var<=thres)
  	return var;
 else
 	return thres;
}

double LowThres(double var, double thres, double res)
{
// var is variable to evaluate, thres the threshold and
// res the value returned if below thres
 if (var<=thres)
  	return res;
 else
 	return 0;
}

double HighThres(double var, double thres, double res)
{
// var is variable to evaluate, thres the threshold and
// res the value returned if above thres
 if (var>=thres)
 	return res;
 else
	return 0;
}

double Pattern(double p, double len, double b)
{//returns 1 when bit in p is set otherwise 0
 unsigned long l1,l2;

 if (p<=0.0) return 0.0;

 if (npcount>=log2(p))
 {
  if (b)
   npcount=0;
  else
   return 0.0;
 }
 if (npstep>=0)
 {
  npstep++;
  if (npstep<=len)
   return npval;
 }
 npstep=0;
 l1=(unsigned long)ceil(pow(2.0,npcount));
 l2=(unsigned long)ceil(p);
 npcount++;
 if (l1&l2)
  npval=1;
 else
  npval=0;
 return npval;
}

double If(double d)
{// returns 1 if d != 0
 if (d!=0.0)
 	return 1.0;
 return 0.0;
}

double Ifelse(double d1, double d2, double d3)
{// returns d2 if d1 != 0 else d3
 if (d1!=0.0)
 	return d2;
 return d3;
}

double Sign(double d)
{
  if (d==0.0)
      return 0.0;
  if (d>0.0)
      return 1.0;
  else
      return -1.0;
}

//use gsl to calculate nu order bessel function value for x
double Besselj(double nu,double x)
{
//note that only integer values of nu are supported:
#ifdef USE_GSL
return gsl_sf_bessel_Jn(static_cast<int>(nu),x);
#else
//if no gsl available fail:
return DBL_MIN;
#endif
}

double Besselk(double nu,double x)
{
//note that only integer values of nu are supported:
#ifdef USE_GSL
return gsl_sf_bessel_Kn(static_cast<int>(nu),x);
#else
//if no gsl available fail:
return DBL_MIN;
#endif
}

double Besseli(double nu,double x)
{
//note that only integer values of nu are supported:
#ifdef USE_GSL
return gsl_sf_bessel_In(static_cast<int>(nu),x);
#else
//if no gsl available fail:
return DBL_MIN;
#endif
}

double Bessely(double nu,double x)
{
//note that only integer values of nu are supported:
#ifdef USE_GSL
return gsl_sf_bessel_Yn(static_cast<int>(nu),x);
#else
//if no gsl available fail:
return DBL_MIN;
#endif
}

static double pi(void)
{
#ifndef M_PI
 return 3.14159265358979323846264;
#else
 return M_PI;
#endif
}

#ifdef _WINDOWS
static double log2(double l)
{
 return (log10(l)/log10(2.0));
}
#endif
