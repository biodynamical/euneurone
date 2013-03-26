//---------------------------------------------------------------------------
#ifndef serverH
#define serverH

#include <vector>
//---------------------------------------------------------------------------
#include "plot3d.h"
#include "graph.h"
#include "integrator.h"
#include "netintegrator.h"
#include "Unitclass.h"
#include "Series.hpp"
#include "Settings.h"
#include "grphstr.h"
#include "rchart.h"
namespace NS_Server {

#define EXCEPTION_FAILED_MUTEX 0x14150020
//---------------------------------------------------------------------------
class Server
{
 void __fastcall PopupMenuItemClick(TObject *Sender);
private:
 //declare an integrator object
  NS_Integrator::Integrator *IntegratorObject;
  NS_NetIntegrator::NetIntegrator *NetIntegratorObject;
  NS_UnitWeb::UnitWeb *NetObject;
  NS_Equation::Equation *EquationObject;
  NS_NetEq::NetEquation *NetEquationObject;
  gSeriesVector vSeries;
  s3DVector v3Data;
  boolVector vBool;
  NS_Settings::Settings *Set;
  TTimer *Timer;
#ifdef MUTEX
  HANDLE hMutex;
#endif
  TComponent *Owner;
  AnsiString SaveName;
  FILE *fp;
  double t;
  double tau;
  double tbegin;
  double tend;
  double h;
  UINT nOutput;
  UINT nUpdate;
  UINT nGraphAdd;
  UINT nUnit;
  int nLag;
  double dLag;
  int nArrowMod;
  int nXVgrid;
  int nYVgrid;
  bool bGraph;
  bool bMarker;
  bool bSave;
  bool bBinary;
  bool bArrows;
  bool bNet;
  bool bReg;
  bool bError;
  bool bVectorRefresh;
  bool bSaveNetOk;
  UINT nGraphType;
  TCursor SaveCursor;
  unsigned long ulFPos;
  UINT nException;
  void __fastcall Output();
  void __fastcall OutputDone();
  void __fastcall Update();
  void __fastcall EqUpdate();
  void __fastcall Eq3DUpdate();
  void __fastcall NetUpdate();
  void __fastcall EqGraphUpdate();
  void __fastcall NetGraphUpdate();
  void __fastcall OpenData();
  void __fastcall CloseData();
//  void __fastcall VectorField(TGraphForm *);
  bool __fastcall QuerySeries(TGraphForm *tf, structGraph &);
  int __fastcall DoEditSeries(TRChartForm *tf, structGraph &);
  void __fastcall RefreshVectorfield();
  UINT __fastcall FilterException(UINT);
  void __fastcall ExceptionMessage();
  int __fastcall FindXLine(TRChart *chart,const AnsiString &as);
  void __fastcall SetChartX(TRChartForm *graph,const AnsiString &as, structGraph &sg,TRichEdit *pmemo);
  float __fastcall Calc3DScale(float f,const NS_Formu::DblVector *dv);
  void __fastcall AppendRichEdit(TRichEdit *tr,AnsiString &as);
  void __fastcall AppendRichEdit(TRichEdit *tr,const char *p);
public:
  bool bReady;
  __fastcall Server(TComponent *anOwner);
  __fastcall ~Server();
  void __fastcall StlException(std::exception &e);
  void __fastcall ExceptionMessage(UINT n);
  void __fastcall FlagException(UINT);
  void __fastcall ErrorMessage(char *);
  void __fastcall DoAfterDraw(TObject *Sender);
  void __fastcall SetupWindows();
  void __fastcall SaveWindows();
  bool __fastcall ReadNetFile(const AnsiString &);
  char *__fastcall GetSetFilename();
  char *__fastcall GetEqFilename();
  char *__fastcall GetNetFilename();
  bool __fastcall ReadEquation(char *);
  void __fastcall SetFunction(TRichEdit *);
  void __fastcall ClearFunction();
  bool __fastcall LoadNet();
  bool __fastcall LoadNetEquation();
  int __fastcall GetNetWarnings();
  int __fastcall GetEqWarnings();
  bool __fastcall SaveNet();
  bool __fastcall SaveLog();
  void __fastcall LoadLog(char *p,TRichEdit *pMemo);
  void __fastcall LogVarChange(AnsiString &as,double d,double dold);
  void __fastcall Initialise();
  void __fastcall DoOptions(TRichEdit *);
  void __fastcall NetOptions(TRichEdit *);
  const NS_Equation::dblVectorOfVector * __fastcall GetDataVector();
  void __fastcall GetDataVectors(UINT,const NS_Formu::DblVector **,const NS_Formu::DblVector **,const NS_Formu::DblVector **);
  struct3D * __fastcall Get3DVector(UINT);
  UINT __fastcall GetNum3D();
  struct3D *__fastcall New3DSeries(TGlControl *,char *,char *,char *);
  void __fastcall Server::Append3DSeries(TGlControl *pgl,char *p1,enumDrop ed);
  void __fastcall New3DSeries(char *p);
  void __fastcall Toggle3D(UINT,bool);
  void __fastcall Del3D(UINT);
  void __fastcall Flush3D();
  void __fastcall WriteRawData(char *filename);
  void __fastcall WriteData(char *filename);
  void __fastcall NewSeries(TRChart *t, const AnsiString &s);
  UINT __fastcall NewSeries(TRChart *chart,const AnsiString &asx, const AnsiString &asy,TRichEdit *pMemo);
  UINT __fastcall NewSeries(TRChart *chart,UINT xunit,UINT xidx,const AnsiString &asy);
  void __fastcall DeleteSeries(UINT idx);
  void __fastcall ChangeChartX(TRChartForm *graph,const AnsiString &as);
  void __fastcall SelectUnit(const AnsiString &s);
  void __fastcall SetSeriesType(structGraph &, structGraph &, int );
  structGraph * __fastcall Server::GetSeriesPtr(UINT n);
  AnsiString __fastcall GetIntegrator();
  bool __fastcall SetIntegrator(int);
  void __fastcall SetUpdateSpeed(UINT);
  void __fastcall SetGraphSpeed(UINT);
  void __fastcall Set3DAutoScale(bool);
  void __fastcall SetArrows(bool);
  void __fastcall SetGraphOutput(bool);
  void __fastcall SetProgressMarker(bool);
  void __fastcall SetColor(UINT,TColor);
  void __fastcall SetInitValue(UINT, double);
  void __fastcall SetVarValue(UINT, double);
  void __fastcall SetInitValue(AnsiString &, AnsiString &, double);
  void __fastcall SetVarValue(AnsiString &,AnsiString &, double);
  void __fastcall SetNetPosRan(AnsiString &as, bool b);
  void __fastcall SetStart(double);
  void __fastcall SetStop(double);
  void __fastcall SetStep(double);
  void __fastcall SetSave(bool);
  void __fastcall SetBinary(bool);
  void __fastcall SetSaveName(AnsiString &);
  void __fastcall SetLineColour(AnsiString& s, TColor c);
  void __fastcall SetLineColour(AnsiString& as,AnsiString& s, TColor c);
  bool __fastcall IsImmediate(UINT idx);
  double __fastcall GetVarValue(UINT);
  double __fastcall GetUnitVarValue(UINT,UINT);
  double __fastcall GetUnitVarValue(UINT,AnsiString &);
  const char *__fastcall GetVarName(UINT);
  const char *__fastcall GetVarTimeName(UINT n);
  const char *__fastcall GetEquationName();
  const char *__fastcall GetUnitName(UINT);
  const char *__fastcall GetUnitVarName(UINT,UINT);
  AnsiString __fastcall GetUnitAndVarName(UINT,UINT);
  UINT __fastcall GetNumberParam();
  UINT __fastcall GetNumUnitParam(UINT);
  const char *__fastcall GetParamName(UINT);
  double __fastcall GetParamValue(UINT);
  const char *__fastcall GetUnitParamName(UINT,UINT);
  double __fastcall GetUnitParamValue(UINT,UINT);
  void __fastcall GetUnitCoord(UINT, float *,float *,float *,bool *);
  void __fastcall SetUnitCoord(UINT, float,float,float,bool);
  const char *__fastcall GetEquationFileName();
  void __fastcall SetNetOptionStore(AnsiString &as, bool b);
  void __fastcall SetNetOptionSave(AnsiString &as, bool b);
  void __fastcall SetLag(double f);
  void __fastcall SetVectorField(TGraphForm *,bool);
  void __fastcall SetVectorGrid(int ,bool);
  bool __fastcall CalcVectorField(UINT idx, DblVector &dvv);
  int __fastcall CalcArrows(UINT idx,IntVector &ivv);
  void __fastcall ShowVectors();
  void __fastcall HideVectors();
  void __fastcall DrawVectors(TCanvas *);
  void __fastcall Reset();
  void __fastcall Start();
  void __fastcall Stop();
  UINT __fastcall GetN();
  UINT __fastcall GetNNet();
  UINT __fastcall GetNUnit(UINT);
  UINT __fastcall GetUnitNumConnect(UINT);
  UINT __fastcall GetUnitConnection(UINT,UINT);
  UINT __fastcall GetUnitIdx(AnsiString &as);
  void __fastcall Print();
  void __fastcall PrintModelInfo(TRichEdit *memo);
  double __fastcall GetBeginTime() {return tbegin;};
  double __fastcall GetEndTime() {return tend;};
  double __fastcall GetStep() {return h;};
  double __fastcall GetLag() { return dLag; };
  AnsiString __fastcall GetXAxisName(TChart *t);
  bool __fastcall AskFor3D(AnsiString &,TColor *); 
  void __fastcall DeleteGraphs();
  void __fastcall RepaintSeries();
  void __fastcall EditSeries(TRChartForm *tf,int idx);
  void __fastcall SetGraphStyle(UINT n) { nGraphType=n; }
  UINT __fastcall GetGraphStyle() { return nGraphType; }
  void __fastcall AskChangeColour(AnsiString &);
  void __fastcall AddHistory(const AnsiString &as);
  std::string &__fastcall GetHistory(UINT n);
  UINT __fastcall GetNumHistory();
  AnsiString &__fastcall GetCurDir();
  void __fastcall FillPowerData(AnsiString &asu, AnsiString &as,PowerData &pd);
  void __fastcall FillPowerData(TGraphForm *tf,TChartSeries *Series,PowerData &pd);
  void __fastcall UpdatePowerData(PowerData &pd);
};

};

extern NS_Server::Server *ServerObject;

#endif
