// equation.cpp
// 30-6-1997
//
// Author: Tjeerd olde Scheper
//
// Last revision: 16-12-1998
// History:	-First setup for HP-UX
//              -Ported to MsWindows 11-12-1997
//		-Ported to Qt 18/7/2003
// Bugs: UNDEFINED
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <float.h>
#include <errno.h> 
#include <algorithm>
#include <stdexcept>
#include <qapplication.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qcolor.h> 

#include "formulc.h"
#include "equation.h"

using namespace NS_Equation;
using namespace std;

#undef isspace
#undef iscntrl
#undef isalnum

#define MAXCOLORS 17
#define MODCOLORS 18

#define IDCHARDERIVE '\''
#define IDCHARMAP '~'
#define DATABEGIN '['
#define DATAEND ']'

#define IDERR_NOMODE 0
#define IDERR_DOPPEL 1
#define IDERR_WRONGNAME 2
#define IDERR_VARDEF 3
#define IDERR_IMMED 4
#define IDERR_TOOMANY 5
#define IDERR_NOTUSED 6
#define IDERR_WRONGOPT 7
#define IDERR_OPTVAL 8
#define IDERR_WRONGHEAD 9
#define IDERR_INIUNKNOWN 10
#define IDERR_ERRINI 11
#define IDERR_UNDEFVAR 12
#define IDERR_USEPARASVAR 13
#define IDERR_ALREADYDEF 14
#define IDERR_INIUNKVAR 15
#define IDERR_INIPARVAR 16
#define IDERR_NOEQS 17
#define IDERR_ISEQFILE 18
#define IDERR_OLDOPT 19
#define IDERR_SYNTAX 20
#define IDERR_UNRECOG 21

#define MAXBUFFER 10240
static char DataBuffer[MAXBUFFER], ReadBuffer[MAXBUFFER];

#define HEAD_NAM 1
#define HEAD_DESC 2
#define HEAD_PAR 3
#define HEAD_EQ 4
#define HEAD_INI 5
#define HEAD_OPT 6

const char *Headers[NUMHEADER]=
	{
	 "Name",
	 "Description",
	 "Parameters",
	 "Equation",
	 "Initial",
	 "Options"
	};

const char *Options[NUM_OPTIONS]=
	{
	 "Integrator",
	 "Duration",
	 "Step",
	 "Output",
	 "Graph",
	 "Graph3D",
         "Logfile",
	 "Lyapunov",
	 "Statistics",
	 "Socket",
	 "Client"
	};

const char *ObsoleteOptions[NUM_OBOPTIONS]=
{
	 "Delay",
	 "X-axis",
	 "Y-axis",
	 "X-range",
	 "GraphType",
	 "Y-range"
};

#define NUMERRORS 22
const char *Errors[NUMERRORS]=
	{
	 "Definition found outside header",
	 "Duplicate header",
	 "Invalid name",
	 "Unknown variable definition",
	 "Immediate follows equation(s)",
	 "Too many variables/parameters",
	 "Parameter never used",
	 "Unknown option",
	 "Invalid option value",
	 "Unknown header",
	 "Initialisation of unknown variable",
	 "Error in initialization, set to zero",
         "Undefined variable",
         "Cannot use previously declared parameter as variable",
         "Variable already declared",
         "Attempt to initialize undefined variable",
         "Attempt to initialize parameter as variable",
	 "No equations found",
	 "Is this an EQ file?",
	 "Obsolete option",
	 "Syntax error",
	 "Unknown error"
	};

extern const char *validChars;

/*===========================================================================*\
| *     *     *     *     *     *     *     *     *     *     *     *     *   |
| SECTION : Equation class definition                                         |
|                                                                             |
| REFERENCE :                                                                 |
|                                                                             |
|                                                                             |
\*===========================================================================*/
Equation::Equation()
{
 N=0;
 wText=NULL;
 nWarn=0;
 bSaveData=false;
 bStoreData=true;
 bDelay=false;
#ifndef SPINNER
 bKeepError=false;
#endif
 fp=NULL;
}

Equation::Equation(int n, QTextEdit *memo)
{
 wText=memo;
 N=n;
 nWarn=0;
 bSaveData=false;
 bStoreData=true;
 bDelay=false;
#ifndef SPINNER
 bKeepError=false;
#endif
 fp=NULL;
}

Equation::~Equation()
{//nothing to do
}

void Equation::Flush()
{
 nWarn=0;
 vdvData.erase(vdvData.begin(),vdvData.end());
 vdvDataError.erase(vdvDataError.begin(),vdvDataError.end());
 dvDataError.erase(dvDataError.begin(),dvDataError.end());
 pEquation.reset_vars();
 sInfo.clear();
 sModelName.clear();
 ovOptions.clear();
 bvDoneHeader.clear();
 N=0;
}

bool Equation::StripComment(char *p)
{
 char *pc=strchr(p,IDCHARCOMMENT);
 if (pc!=NULL)
 {
  *pc=0;
  return true;
 }
 return false;
}

void Equation::newEquation(QTextEdit *te)
{
 int i;
 QString qs;
 
 Q_ASSERT(te!=NULL);
 
 for (i=0;i<NUMHEADER;i++)
 {
  qs.sprintf("%c%s%c\n\n",IDCHARHEADLEFT,Headers[i],IDCHARHEADRIGHT);
  te->append(qs);
 }
 Flush();

 error=0;
 line=0;
 charnum=-1;
 N=0;
 nWarn=0;
 bEqBefore=false;
 sInfo.clear();
 sModelName.clear();
 ovOptions.clear();
 bvDoneHeader.assign(NUMHEADER,false);
}

bool Equation::IsHeader(const char *p)
{
 if (strchr(p,IDCHARHEADLEFT)&&strchr(p,IDCHARHEADRIGHT))
 	return true;
 return false;
}

bool Equation::CheckHeader(const char *p, int *mode)
{
 int i;
 bool fnd=false;
 
 for (i=0;i<NUMHEADER;i++)
  	 	{
  	 	 if (StrnCmp(p,Headers[i],strlen(Headers[i]))==0)
  	 	 	{
  	 	 	 *mode=i+1;
  	 	 	 fnd=true;
  	 	 	 break;
  	 	 	}
  	 	}
 return fnd;
}
		
bool Equation::IsNumericalTerm(const char *p)
{ //checks if string p is a numerical valid term, just digits and operators
 int i,j,len;

 len=strlen(p);
 for (i=0;i<len;i++)
 	{
 	 if (!strchr("0123456789-+*//^().",p[i]))
	  {
	   //check if it is a function
	   for (j=i;j<len;j++)
	   	{
	   	 if (!isalnum(p[j])||(j>=len))
		 	break;
	   	}
	   //is it a valid function?
#ifdef _DEBUG
qDebug("checking for function %s at %s (%d) with %s (%d) len=%d, diff=%d",p,&p[i],i,&p[j],j,len,j);
#endif
	   if (where_table(const_cast<char *>(&p[i]),j-i)<0)
	   	return false;
	   i=j;
	  }
 	}
 return true;
}

bool Equation::NumericalTermValue(char *p, double *pd)
{
 bool b=false;
 double d;
 
 Q_ASSERT(p!=NULL);
 Q_ASSERT(pd!=NULL);
  //check if it is a num term in the first place;
 b=IsNumericalTerm(p);
 if (!b)
 	return b;
 //try to get the value of it
 d=pEquation.NumEval(p);
 if(pEquation.fget_error())
 	return false;
 *pd=d;
 return true;
}

void Equation::StripWhiteSpace(char *t,char *p)
{
 int i,j,len;
 bool b=false;

 len=strlen(p);
 for (i=0,j=0;i<len;i++)
 	{
         if (p[i]=='\"')
            b=!b;
         if (!b)
         {
     	  if ((!isspace(p[i]))&&(!iscntrl(p[i])))
	        t[j++]=p[i];
         }
         else
	    t[j++]=p[i];
 	}
 t[j]=0;
}

bool Equation::isValidString(char *p)
{
 unsigned int i;
  for(i=0; i<strlen(p);i++)
  {
   if (!strchr(validChars,p[i]))
   	return false;
  }
  return true;
}

bool Equation::GetOption(char *p)
{
 //extract assignment, return in p the equation part
 char *opval=strchr(p,IDCHAREQUATE);
 double d;
 OptionsType i;
 int j;
 bool bFound;

 if (opval)
 	{
 	 //get option name
	 int l=opval-p;
	 if (l<1)
		{
	   	 Error(IDERR_WRONGOPT);
	 	 return false;
	 	}
	 //check if it is an obsolete option
  	 for (j=0;j<NUM_OBOPTIONS;j++)
  	 	{
 	 	 if (StrnCmp(DataBuffer,ObsoleteOptions[j],strlen(ObsoleteOptions[j]))==0)
		 	{
		 	 Error(IDERR_OLDOPT);
			 return true;
			}
		}
	 bFound=true;
	 //check for option
	 structOptionT op;
  	 for (i=opIntegrator;i<opNone;i=(OptionsType)((int)i+1))
  	 	{
  	 	 if (StrnCmp(DataBuffer,Options[i],strlen(Options[i]))==0)
  	 	 	{
#ifdef _DEBUG
qDebug("DEBUG: option, found option %s ",DataBuffer);
#endif
			 opval++;
			 switch(i)
			 {
			  case opIntegrator:
			  case op3D:
			  case opLog:
			  case opLyap:
			  case opStats:
			  case  opGraph:
			  case opClient:
				 	{
					 if (isalpha(opval[0]))
				 	 {
					  op.type=i;
					  op.s=opval;
					  op.d=0.0;
					  ovOptions.push_back(op);
					 }
					 else
			  	 	 	Error(IDERR_OPTVAL);
					 break;
				 	}
			 case opLength:
			 case opStepSize:
			 case opOutput:
			 case opSocket:
				 	{
					 d=atof(opval);
					 op.type=i;
					 op.s.clear();
					 op.d=d;
					 ovOptions.push_back(op);
					 break;
					}
			default: bFound=false; break;
			}
  	 	      }
  	 	}
  	 if (!bFound)
  	 	Error(IDERR_WRONGOPT);
 	}
 else
 	return false;
 return true;
}

structOptionT *Equation::GetOption(unsigned int n)
{
 if (n<ovOptions.size())
	 return &ovOptions[n];
 return NULL;
}

bool Equation::checkVariable(char *p, NS_Formu::TEqTypes t)
{
  bool b=false;
 //check if variable p is valid
 NS_Formu::TEqTypes type=pEquation.GetVariableType(p);
 //check for incompatible combinations:
 switch(type)
 {
  default:
  case NS_Formu::etUnknown:
  	{
  	 b=true;
  	 break;
   	}
  case NS_Formu::etParAsVar: break;
  case NS_Formu::etParam:
  	{
  	 switch(t)
  	 {
  	  case NS_Formu::etParAsVar:
  	  case NS_Formu::etParam:
  	  default:
  	  	{
  	  	 b=true;
  	  	 break;
  	  	}
  	   case NS_Formu::etVar:
  	   case NS_Formu::etImm:
  	   case NS_Formu::etDeriv:
  	   case NS_Formu::etMap:
  	   	{
  	   	 b=false;
  	   	 Error(IDERR_USEPARASVAR);
  	   	 break;
  	   	}
  	 }
  	 break;
  	}
       case NS_Formu::etImm:
       case NS_Formu::etDeriv:
       case NS_Formu::etMap:
       case NS_Formu::etVar:
  	{
   	 b=false;
   	 Error(IDERR_ALREADYDEF);
  	 break;
  	}
 }
 return b;
}

int Equation::ExtractParam(char *p, bool bparam)
{
 double d;
 char *eq=strchr(p,IDCHAREQUATE);

 //check if the param starts with a digit:nono
 if (isdigit(*p))
	return IDERR_WRONGNAME;
 if (eq)
 	{
 	 //set a 0 at the equal sign
 	 *eq=0;
 	 eq++;
 	}
#ifdef _DEBUG
qDebug("DEBUG: ExtractParam, found %s from %s.",p,eq);
#endif
 if (bparam)
  pEquation.AddVariable(p,NS_Formu::etParam);
 else
  pEquation.AddVariable(p,NS_Formu::etVar);

 if ((!eq)||!IsNumericalTerm(eq))
 	 return IDERR_ERRINI;
 d=pEquation.NumEval(eq);
 if(pEquation.fget_error())
 	 return IDERR_ERRINI;
 pEquation.make_var(p,d);
 pEquation.make_ini(p,d);
#ifdef _DEBUG
qDebug("DEBUG: ExtractParam, set %s to %f.",p,d);
#endif
 return 0;
}

bool Equation::GetDeclare(char *p)
{
 //extract assignment, return in p the equation part
 char *eq=strchr(p,IDCHAREQUATE);
 char *b;
 int len,dlen;
 
 if (eq)
 	{
 	 //get variable name
	 dlen=strlen(p);
	 len=eq-p;
	 if (len<1)
		{
	   	 Error(IDERR_VARDEF);
	 	 return false;
	 	}
	 b=new char[len+1];
	 memset(b,0,len+1);
	 strncpy(b,p,len);
#ifdef _DEBUG
qDebug("DEBUG: declare,before, p [%s], eq [%s],b [%s], len=%d. ",p,eq,b,len);
#endif
	 //check for valid definition
	 switch(b[len-1])
         {
           case IDCHARDERIVE:
	 	{
	 	 //it's a derivative
		 b[len-1]=0;
		 bEqBefore=true;
		 if (checkVariable(b,NS_Formu::etDeriv))
                 	pEquation.AddVariable(b,NS_Formu::etDeriv);
#ifdef _DEBUG
qDebug("DEBUG: variable found: %s ",b);
#endif
                 break;
	 	}
	   case IDCHARMAP:
	 	{
	 	 //it's a map
		 b[len-1]=0;
		 bEqBefore=true;
		 if (checkVariable(b,NS_Formu::etMap))
                 	pEquation.AddVariable(b,NS_Formu::etMap);
#ifdef _DEBUG
qDebug("DEBUG: map found: %s",b);
#endif
                 break;
	 	}
	   default:
	 	{
	 	 //it's an immediate
#ifdef _DEBUG
qDebug("DEBUG: Immediate found %s.",b);
#endif
	 	 if (bEqBefore)
			{
		   	 Error(IDERR_IMMED);
		   	 delete b;
	 	 	 return false;
			}
		 if (checkVariable(b,NS_Formu::etImm))
                 	pEquation.AddVariable(b,NS_Formu::etImm);
                 break;
	 	}
         }
	 delete[] b;
    // TVOS 13/3/2012: changed to indirect copy to prevent copy overrun which could happen for immediates:
	 dlen=dlen-len;
	 b=new char[dlen];
	 memset(b,0,dlen);
 	 eq++;
 	 strncpy(b,eq,dlen);
 	 strncpy(p,b,dlen);
#ifdef _DEBUG
qDebug("DEBUG: declare,after p [%s], eq [%s], b [%s], len=%d, dlen=%d. ",p,eq,b,len,dlen);
#endif
	 delete[] b;
 	}
 else
 	return false;
 return true;
}

bool Equation::BuildEqList()
{
 int error;
 unsigned int i;
 bool bOk=false;
#ifdef _DEBUG
qDebug("DEBUG: [BuildeqList] begin");
#endif
 error=pEquation.Convert();
 if(error)
   {
#ifdef _DEBUG
qDebug("Error at parameter #%d of the function [%s] is : %s.",error,DataBuffer,pEquation.fget_error());
#endif
    charnum=pEquation.GetErrorChar();
    FatalError(pEquation.GetErrorEquation(),pEquation.fget_error());
    Flush();
    return bOk;
   }
  N=pEquation.GetNumberVars();
#ifndef SPINNER
 //Create data store
 for (i=0;i<=N;i++)
 	vdvData.push_back(NS_Formu::DblDeque());
 //Create store for estimated integration error
 if (bKeepError)
 {
  for (i=0;i<N;i++)
 	vdvDataError.push_back(NS_Formu::DblDeque());
}
else
 	dvDataError.assign(N,0.0);
#endif
 line=-1;
 if (N==0)
 {
    FatalError(Errors[IDERR_ISEQFILE],Errors[IDERR_NOEQS]);
    return bOk;
 }
 bOk=true;
 //check for definition errors
 for (i=0;i<pEquation.get_var_count();i++)
 {
  switch (pEquation.get_var_type(i))
  {
   case NS_Formu::etUnknown:
        {
         strcpy(DataBuffer,pEquation.get_var_name(i));
         Error(IDERR_UNDEFVAR);
	 bOk=false;
         break;
        }
   case NS_Formu::etVar:
        {
         strcpy(DataBuffer,pEquation.get_var_name(i));
         Error(IDERR_INIUNKVAR);
	 bOk=false;
         break;
        }
   case NS_Formu::etParAsVar:
        {
         strcpy(DataBuffer,pEquation.get_var_name(i));
         Error(IDERR_INIPARVAR);
	 //Make this fatal, otherwise can cause Oops during integration (using unknown number of vars)
         bOk=false;
        }
   default: break;
  }
 }
#ifdef _DEBUG
qDebug("DEBUG: [BuildeqList] done");
#endif
 return bOk;
}

#ifndef SPINNER
void Equation::setKeepError(bool b)
{
 unsigned int i;
 
 bKeepError=b;
 if (bKeepError)
 {
  //clear old data
   for (i=0;i<vdvDataError.size();i++)
   	vdvDataError[i].erase(vdvDataError[i].begin(),vdvDataError[i].end());
   vdvDataError.erase(vdvDataError.begin(),vdvDataError.end());
   
   for (i=0;i<N;i++)
 	vdvDataError.push_back(NS_Formu::DblDeque());
}
else
 	dvDataError.assign(N,0.0);
}
#endif

bool Equation::ReadEquation(QStringList &list)
{
 bool bOk;
 QString qs;
 
 Flush();

 bOk=ParseEquation(list);

 if (bOk)
 	bOk=BuildEqList();

 if (nWarn)
 {
  if (wText)
	{
	 if (nWarn>1)
	 	qs.sprintf("<font color=#0000FF>There were </font><font color=#FF0010>%d</font><font color=#0000FF> errors.</font>",nWarn);
	 else
	 	qs.sprintf("<font color=#0000FF>There was </font><font color=#FF0010>%d</font><font color=#0000FF> error.</font>",nWarn);
	 wText->append(qs);
	}
  else
	perror("Found error(s) in the file\n");  
 }
 return bOk;
}

bool Equation::ParseEquation(QStringList &list)
{
#ifdef _DEBUG
qDebug("[Equation::ParseEquation] begin.");
#endif

 int mode;
 QString qs;
 QStringList::Iterator it;
 
 error=0;
 line=0;
 charnum=-1;
 mode=-1;
 N=0;
 nWarn=0;
 bEqBefore=false;
 sInfo.clear();
 sModelName.clear();
 ovOptions.clear();
 bvDoneHeader.assign(NUMHEADER,false);

 it=list.begin();
 while (it!=list.end())
 {
   strncpy(ReadBuffer,(*it).ascii(),MAXBUFFER);
   it++;
   line++;
   if (strlen(ReadBuffer)==MAXBUFFER-1)
  	{					//possible line too long
	  if (wText)
		{
	 	 qs.sprintf("<font color=#FF0010>Error: line %d is too long </font> (maximal length=%d characters)",line,MAXBUFFER-2);
                 wText->append(qs);
		}
	  else
	  	perror("Lines too long\t");
	  nWarn++;
          continue;
  	}
  if (mode>HEAD_DESC)
  	StripWhiteSpace(DataBuffer,ReadBuffer);
  else
  	strcpy(DataBuffer,ReadBuffer);
  
  StripComment(DataBuffer);
  if (strlen(DataBuffer)<1)
  	continue;
  if (IsHeader(DataBuffer))
  	{
  	 if (CheckHeader(&DataBuffer[1],&mode))
	 {
  	  //check if this header has already been done
  	  if (bvDoneHeader[mode-1])
  	 	{
  	 	 Error(IDERR_DOPPEL);
  	 	 mode=-1;
		  nWarn++;
  	 	}
	 else
		bvDoneHeader[mode-1]=1;
	  continue;
	 }
  	 Error(IDERR_WRONGHEAD);
	  nWarn++;
  	}
  //if we get here without a mode (ie under a header) print an error
  if (mode==0)
  	{
  	 Error(IDERR_NOMODE);
	 nWarn++;
  	 continue;
  	}
  //if in skip mode
  if (mode<0)
  	{
  	 continue;
  	}
  // check if the string is made up from valid characters
  if ((mode!=HEAD_DESC)&&(mode!=HEAD_NAM)&&(!isValidString(DataBuffer)))
   	   	{
   	   	 Error(IDERR_SYNTAX);
		 nWarn++;
		 return false;
		}
  switch (mode)
  {
   case HEAD_DESC: //description
   	  {
           if (strlen(DataBuffer)<2)
           	break;
	   sInfo.append(DataBuffer);
   	   break;
   	  }
   case HEAD_NAM: //name
   	  {
   	   sModelName=DataBuffer;
   	   mode=-1;				//skip rest until new header
   	   break;
   	  }
   case HEAD_OPT:
   	  {
   	   if (!GetOption(DataBuffer))
   	   	{
   	   	 Error(IDERR_WRONGOPT);
                 nWarn++;
		 return false;
		}
      	   break;
   	  }
   case HEAD_INI:
   	  {//initial
   	   int e=ExtractParam(DataBuffer,false);
   	   if (e)
   	   	{
   	   	 Error(e);
   	   	 if (e!=IDERR_ERRINI)	//critical error
                 {
                  nWarn++;
		  return false;
                 }
   	   	}
   	   break;
   	  }
   case HEAD_PAR:
   	  {
   	   //assume it is a parameter
   	   int e=ExtractParam(DataBuffer,true);
   	   if (e)
   	   	{
   	   	 Error(e);
   	   	 if (e!=IDERR_ERRINI)	//critical error
                 {
                  nWarn++;
		  return false;
                 }
   	   	}
#ifdef _DEBUG
 qDebug("DEBUG: Added param %s",DataBuffer);
#endif
   	   break;
   	  }
   case HEAD_EQ:
   	  {
	   if (!GetDeclare(DataBuffer))
           {
            nWarn++;
	    return false;
           }
	   pEquation.AddSource(DataBuffer);
   	   break;
	  }
   default: break;
  }
 }
#ifdef _DEBUG
qDebug("[Equation::ParseEquation] done.");
#endif
 return true;
}

void Equation::Error(int num)
{
 QString qs;
 nWarn++;
 if (wText)
	{
	 if (line>=0)
         {
          qs.sprintf("\n<font color=#FF2F2F>Error</font> : Line <font color=#0010FF>%d</font>: %s\n<font color=#FF0010>%s</font>\n",line,DataBuffer,Errors[num]);
	  wText->append(qs);
         }
 	 else
         {
          qs.sprintf("\n<font color=#FF2F2F>Error</font>: <font color=#FF0010>\"%s\"</font>:\n<font color=#0010FF>%s</font>\n",DataBuffer,Errors[num]);
	  wText->append(qs);
         }
	}
 else
	printf("Error in line #%d \"%s\":\10\t%s",line,DataBuffer,Errors[num]);
}

void Equation::FatalError(const char *err,const char *p)
{
 if (wText)
	{
	 QString qs;
	 qs.sprintf("\n<font color=#F01010>Fatal error :</font>\n<font color=#FF0010>%s</font>\n<font color=#0010FF>%s</font>\n",p,err);
	 if (charnum>0)
         {
          int w=qs.length()-(strlen(err)+8)+charnum;
	  qs.insert(w,"<font color=#F020F0><b>?</b></font>");
         }
	  wText->append(qs);
	}
 else
	 printf("Fatal error at \"%s\":\t%s",p,err);
}

void Equation::SetFunction(QTextEdit *memo)
{
 Q_ASSERT(memo!=NULL);
 wText=memo;
}

const char *Equation::GetName()
{
 return sModelName.c_str();
}

const char *Equation::GetInfo()
{
 return sInfo.c_str();
}

const char *Equation::GetEquation(unsigned int n)
{
 return pEquation.GetEquation(n);
}

const char *Equation::GetVarName(unsigned int n)
{
 return pEquation.GetVariableName(n);
}

NS_Formu::TEqTypes Equation::GetVariableType(unsigned int n)
{
 return pEquation.GetVariableType(n);
}

bool Equation::SetVarName(unsigned int n,const char *p)
{
 return pEquation.SetVariableName(n,p);
}

const char *Equation::GetParmName(unsigned int n)
{
 return pEquation.GetParameterName(n);
}

double Equation::GetVarValue(unsigned int n)
{
 return pEquation.GetVariableVal(n);
}

double Equation::GetVarIniValue(unsigned int n)
{
 return pEquation.GetVariableIniVal(n);
}

double Equation::GetParmValue(unsigned int n)
{
 return pEquation.GetParameterVal(n);
}

double Equation::GetConstValue(unsigned int n)
{
 return pEquation.GetConstantVal(n);
}

void Equation::SetParmValue(unsigned int n,double d)
{
 pEquation.MakeParamVal(n,d);
}

void Equation::SetParmValue(QString &qs,double d)
{
 pEquation.MakeParamVal(qs.ascii(),d);
}

void Equation::SetVarValue(unsigned int n,double d)
{
 pEquation.MakeVarVal(n,d);
}

void Equation::SetVarIniValue(unsigned int n,double d)
{
 pEquation.MakeVarIniVal(n,d);
}

void Equation::SetVarValue(QString &qs,double d)
{
 pEquation.make_var(qs.ascii(),d);
}

void Equation::SetVarIniValue(QString &qs,double d)
{
 pEquation.make_ini(qs.ascii(),d);
}

bool Equation::IsImmediate(unsigned int n)
{
 return pEquation.IsImmediate(n);
}

bool Equation::IsDiffMap(unsigned int n)
{
 return ((pEquation.IsDifferential(n))||(pEquation.IsMap(n)));
}

unsigned int Equation::GetNumDiffMap()
{
 unsigned int v,i;
 
 v=0;
 for (i=0;i<N;i++)
 {
  if (IsDiffMap(i))
  	v++;
 }
 return v;
}

void Equation::Print()
{
 //print info
 if (wText)
 {
  QString qs;
   if (!sModelName.empty())
  	{
          qs.sprintf("<font color=#0000F0>Model: </font>%s",sModelName.c_str());
          wText->append(qs);
	}
  if (!sInfo.empty())
  	{
          qs.sprintf("<font color=#0000F0>Information: </font>%s",sInfo.c_str());
          wText->append(qs);
	}
   pEquation.print(wText);
  }
}

void Equation::DoRegister()
{
 pEquation.RegisterData(&vdvData);
}

void Equation::Reset()
{
 unsigned int i;
 
 pEquation.reset();
 for (i=0;i<vdvData.size();i++)
    vdvData[i].erase(vdvData[i].begin(),vdvData[i].end());
 for (i=0;i<vdvDataError.size();i++)
    vdvDataError[i].erase(vdvDataError[i].begin(),vdvDataError[i].end());
  i=dvDataError.size();
 dvDataError.assign(i,0.0);
}

void Equation::setSocketData(TSockDataPoint &dp)
{
//qDebug("[Equation::setSocketData] begin");
  for (unsigned int i=0; i<pEquation.aSocks.size();i++)
  {
    if ((pEquation.aSocks[i].sid==(unsigned int)dp.nId)&&(pEquation.aSocks[i].num==(unsigned int)dp.nIdx))
    {
      pEquation.aSocks[i].value=dp.d;
    }
  }
// #ifdef _DEBUG
 	//qDebug("[Equation::setSocketData] size %u, id %u idx %d value %f",(unsigned int)pEquation.aSocks.size(),dp.nId,dp.nIdx,dp.d);
 	//fsync(0);
// #endif
//qDebug("[Equation::setSocketData] end");
}

void Equation::EvaluateAll(NS_Formu::DblVector &pIn, NS_Formu::DblVector &pOut, NS_Formu::DblVector &pNew)
{
 unsigned int i;
 const NS_Formu::DblVector *dv;

 for (i=0; i<N; i++)
 	{
// #ifdef _DEBUG
// *debug<<"In "<<i<<"="<<pIn[i]<<endl;
// #endif
        pEquation.MakeVarVal(i,pIn[i]);
 	}
  dv=pEquation.evalue();
//  for (i=0;i<N;i++)
  for (i=0;i<dv->size();i++)
  {
   if (!pEquation.IsImmediate(i))
    pOut[i]=dv->at(i);
   else
   {
    pOut[i]=0.0;
    pNew[i]=dv->at(i);
   }
// #ifdef _DEBUG
// *debug<<" Out "<<i<<"="<<pOut[i]<<", "<<pNew[i]<<endl;
// #endif
  }
}

void Equation::Evaluate(NS_Formu::DblVector &pIn, NS_Formu::DblVector &pOut)
{
 unsigned int i;
 const NS_Formu::DblVector *dv;

 for (i=0; i<N; i++)
 	{
// #ifdef _DEBUG
// *debug<<"In "<<i<<"="<<pIn[i]<<endl;
// #endif
        pEquation.MakeVarVal(i,pIn[i]);
 	}
  dv=pEquation.evalue();
//  for (i=0;i<N;i++)
  for (i=0;i<dv->size();i++)
  {
   if (!pEquation.IsImmediate(i))
      pOut[i]=dv->at(i);
   else
      pOut[i]=0.0;
// #ifdef _DEBUG
// *debug<<" Out "<<i<<"="<<pOut[i]<<endl;
// #endif
  }
}

void Equation::DoneStep(double nt, NS_Formu::DblVector &dv,NS_Formu::DblVector &de)
{
 unsigned int i;

// #ifdef _DEBUG
// *debug << "[Equation::DoneStep] begin" <<endl;
// #endif
 t=nt;
// #ifdef _DEBUG
// *debug << "[Equation::DoneStep] N="<<N<<" t="<<t <<endl;
// #endif
 if (bStoreData)
 {
  vdvData[0].push_back(t);
  for (i=0; i<N;i++)
     vdvData[i+1].push_back(dv[i]);
 }
 else
 {//save only the last data
  vdvData[0][0]=t;
  for (i=0; i<N;i++)
     vdvData[i+1][0]=dv[i];
 }
 if (bKeepError)
 {
   for (i=0; i<N;i++)
     vdvDataError[i].push_back(de[i]);
  }
  else
  {//estimate some running average:
    transform(dvDataError.begin(),dvDataError.end(),de.begin(),dvDataError.begin(),plus<double>());
    transform(dvDataError.begin(),dvDataError.end(),dvDataError.begin(),bind2nd(divides<double>(),2.0));
  }
 if (bSaveData)
    WriteData(dv);
// #ifdef _DEBUG
// *debug << "[Equation::DoneStep] done" <<endl;
// #endif
}

void Equation::WriteData(NS_Formu::DblVector &dv)
{
 float d;

 if (fp==NULL)
    return;
 if (bBinary)
 {
  fputc(DATABEGIN,fp);
  d=t;
  fwrite(&d,1,sizeof(float),fp);
  for (unsigned int j=0; j<N; j++)
  {
   d=dv[j];
   fwrite(&d,1,sizeof(float),fp);
  }
  fputc(DATAEND,fp);
 }
 else
 {
  fprintf(fp,"%0.10f, ",t);
  for (unsigned int j=0; j<N; j++)
  {
   fprintf(fp,"%0.10f",dv[j]);
   if (j==(N-1))
      fputc(10,fp);
   else
      fprintf(fp,", ");
  }
 }
}

