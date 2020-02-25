// ControlObject.h
#ifndef CONTROLOBJECT_H
#define CONTROLOBJECT_H

#include <qmutex.h>
#include <qsyntaxhighlighter.h>
#include <qlistview.h>
#include <qheader.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qdragobject.h>
#include <qdatetime.h>
#include <qstatusbar.h>
#include <qlistbox.h>
#include <qvariant.h>
#include <qfile.h>
#include <qtextstream.h>
#ifndef _WINDOWS
#include <qprocess.h>
#endif
#include <vector>
#include <time.h>
#include "equation.h"
#include "integrator.h"
#include "graphtype.h"
#include "graph3dtype.h"
#include "editseriesform.h"
#include "dataform.h"

#ifdef SPINNER
#include "dataspinner.h"
#endif

#ifdef MAT_FILE
#ifdef MATLIB
#include <extern/include/mat.h>
#endif
#endif

#ifdef CDF_FILE
#include <cdf.h>
#endif

#ifdef HDF_FILE
#include <mfhdf.h>
#endif

#ifdef HDF5_FILE
#include <hdf5.h>
#define CHUNK_SIZE 4096
#endif

#ifdef USE_SOCKETS
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_SOCKS 10
#endif

namespace NS_Control {

#define CONTROL_TRYMUTEX 200
#define CONTROL_LOCK_US 1000

#define ID_INVALID UINT_MAX
#define ID_LYAPPARSTEPS ID_INVALID-2
#define ID_LYAPPARDIST ID_INVALID-3
#define ID_LYAPPARDIM ID_INVALID-4
#define ID_LYAPPARNUM ID_INVALID-5

#define COLOUR_MULTIPLIER 5
#define COLUMN_VAR 0
#define COLUMN_VAL 1
#define COLUMN_INI 2
#define COLUMN_COL 3
#define COLUMN_MEAN 4
#define COLUMN_VARIANCE 5

typedef std::vector<QLineEdit *> TEditVector;
typedef std::vector<QWidget *> TWidgetVector;

#ifdef HDF5_FILE
struct Param
{
  QString name;
  double value;
  Param(char *p, double d) {name=p; value=d;}
};

typedef std::queue<Param> TParamQueue;
#endif

class EqHighlighter : public QSyntaxHighlighter
{
 NS_Equation::Equation *EquationObject;
 int highlightParagraph(const QString& text, int endStateOfLastPara);
public:
 EqHighlighter(QTextEdit* textEdit, NS_Equation::Equation *pEquation);
};

class TDragObject : public QStoredDrag
{
 public:
    TDragObject( unsigned int n, QWidget * parent = 0, const char * name = 0 );
    ~TDragObject() {};

    static bool canDecode( QDragMoveEvent* e );
    static bool decode( QDropEvent* e, unsigned int *n);
};

class TListView : public QListView
{
public:
 TListView(QWidget* parent, const char* name);
private:
    unsigned int nId;
protected:
	QDragObject *dragObject();
};
//---------------------------------------------------------------------------
typedef struct TVarDataT
{
 unsigned int nIdx;
 unsigned int nVarId;
 QPen pen;
 QString Title;
 dblDequeT *pData;
 QListViewItem *pItem;
} TVarData;

typedef std::vector<TVarData> TVarDataVector;
#include "stats.h"
#include "analysis.h"
//---------------------------------------------------------------------------
#define ID_RTTIVAR 300000
#define ID_RTTISTAT 350000
#define ID_RTTIPAR 400000
#define ID_RTTICONST 500000
#define ID_RTTICALC 600000
#define ID_RTTICOP 650000
#define ID_RTTIREADONLY 299999

class VarListViewItem : public QListViewItem
{
public:
 unsigned int nId;
 QColor colour;
 VarListViewItem(QListViewItem* parent, TVarData &vd, double d);
 int rtti () const;
};
//---------------------------------------------------------------------------
class StatListViewItem : public QListViewItem
{
public:
 unsigned int nId;
 QColor colour;
 StatListViewItem(QListViewItem* parent, TVarData &vd);
 int rtti () const;
};
//---------------------------------------------------------------------------
class ParListViewItem : public QListViewItem
{
public:
 unsigned int nId;
 ParListViewItem(QListViewItem* parent, NS_Equation::Equation *pEq, unsigned int n);
 int rtti () const;
};
//---------------------------------------------------------------------------
class ConstListViewItem : public QListViewItem
{
public:
 unsigned int nId;
 ConstListViewItem(QListViewItem* parent, NS_Equation::Equation *pEq, unsigned int n);
 int rtti () const;
};
//---------------------------------------------------------------------------
class CalcListViewItem : public QListViewItem
{
public:
 TCalcTypeItem type;
 CalcListViewItem(QListViewItem* parent, QString qs,TCalcTypeItem t,bool b);
 int rtti () const;
};

class CalcOptionListViewItem : public QListViewItem
{
public:
 unsigned int nId;
 QVariant varValue;
 double dr1,dr2;
 unsigned int nr1,nr2;
 bool bReadOnly;
 CalcOptionListViewItem(QListViewItem* parent, QString qs, QVariant v, unsigned int n,bool b);
 int rtti () const;
 void setRange(double d1, double d2);
 void setRange(unsigned int n1, unsigned int n2);
};
//---------------------------------------------------------------------------
class TLogFile
{
 QString file;
 bool bOk;
  
 public:
 QString qsFmt;
 QTextStream *psLog;
 QFile *pfLog;
 QTextEdit *pText;
 QRgb col;
  
  TLogFile();
  ~TLogFile();
  void close();
  bool open(QString &qs);
  bool open(std::string &s);
  bool open(char *s);
  void saveLog();
  void setTextEdit(QTextEdit *);
  void loadLog();
  void timeStamp();
  bool needLogFilename() {return (file.isEmpty()||file.isNull());}
  void setLogFilename(QString &qs);
  QString &getLogFilename() {return file;}
  TLogFile & operator <<(int);
  TLogFile & operator <<(double);
  TLogFile & operator <<(QString &);
  TLogFile & operator <<(char *);
  TLogFile & operator <<(const char *);
  TLogFile & operator <<(char);
  TLogFile & operator <<(QRgb);  
  TLogFile &operator <<(TLogFile & (*_f)(TLogFile &));
  TLogFile & endl(TLogFile &l);
  TLogFile & time(TLogFile &l);
  TLogFile & tab(TLogFile &l);
  TLogFile & flush(TLogFile &l);
  bool doLog() { return bOk;}
};

//---------------------------------------------------------------------------

class TSaveDlg : public QFileDialog
{
	TSaveTypes type;
	public:
		TSaveDlg( QWidget *parent, TSaveTypes nType);
		void setName(QString &qs);
		QButtonGroup *bgCompress;
		QButtonGroup *bgAppend;
		QRadioButton *rb1, *rb2, *rb3, *rb4, *rb5, *rb6;
		QCheckBox *cbCompress;
		QCheckBox *cbAppend;
		QCheckBox *cbVars;
		QCheckBox *cbRows;
		QLineEdit *leComment;
		QLabel *lblComment;
};
//---------------------------------------------------------------------------
struct TStreamProperties
{
  std::string sName;
  int nId;
  int nNum;
  int nSize;
  float fStep;
  bool isEqual(std::string &s) { return sName==s;}
  TStreamProperties(QStringList &sl);
};

typedef std::vector<TStreamProperties> vectorStreamProps;
typedef vectorStreamProps::iterator streamIterator;

enum TSocketState {soNone, soStart, soData, soFill, soStop, soError};

//---------------------------------------------------------------------------
class TDataStream : public QThread
{
  bool bReady;
  int clientFD;
  int nId;
  int nNum;
  double step;
  unsigned int nSize;
  unsigned int qSize;
  std::string sName;
  NS_Equation::dataDeque deck;
  TSocketState state;
  struct sockaddr_in stClientAddr;
  char buff[81];
  QMutex *pMutex;
  void prefill();
public:
  TDataStream();
  ~TDataStream();
  void Start();
  void Stop();
  void Next();
  void resetStream();
  void stopThread();
  void setStep(double d);
  std::string &Accept(int socketFD);
  void setupStream(std::string &s, int id, int n, float step, int size);
  bool isReady() { return bReady;}
  bool getDataPoints(double time,NS_Equation::dataDeque &dq);
  virtual void run();
};
//---------------------------------------------------------------------------
typedef std::vector<TDataStream *> vectorDataStreams;

//---------------------------------------------------------------------------
class TSocket : public QThread
{
   int nSocket;
   vectorDataStreams vData;
   struct sockaddr_in stSockAddr;
   int socketFD;
   int nNumClients;
   bool bStop;
   vectorStreamProps *pProperties;
   QMutex *pMutex;
   double step;
public:
  TSocket(vectorStreamProps *p);
  ~TSocket();
  void Stop() { bStop=true;}
  void setSocket(int n) { nSocket=n;}
  int getSocket() { return nSocket; }
  bool setupSocket();
  void setStep(double d);
  int getNumClients();
  bool getStreamData(int client,double time,NS_Equation::dataDeque &dq);
  void stopThreads();
  void resetStreams();
  void freeSocket();
  virtual void run();
};
//---------------------------------------------------------------------------
typedef std::vector<QRgb> RgbVector;
typedef std::deque<unsigned int> TUIntDeque;
//---------------------------------------------------------------------------
class Control : public QObject
{
 Q_OBJECT
private:
 //declare an integrator object
  NS_Integrator::Integrator *IntegratorObject;
  NS_Equation::Equation *EquationObject;
#ifdef SPINNER
  NS_DataSpinner::TDataSpinner *spinObject;
#else
  QMutex *pIntMutex;
#endif
  NS_Control::EqHighlighter *pEqHigh;
  NS_Stats::Statistics *pStats;
  NS_Analysis::Analysis *pAna;
  NS_Equation::dataDeque sockDataDeque;
  TListView *pListView;
  QListViewItem *pListItem;
  QLineEdit *pLineEdit;
  RgbVector cvRgb;
  QLCDNumber *pLcd,*pLcdElapTime,*pLcdEstTime, *pLcdUpdate;
  QProgressBar *pProgress;
  QButtonGroup *pButtonGroup;
  QStatusBar *pStatusBar;
  QLabel *pPixIntLabel;
  QPixmap pixThreadOn, pixThreadOff, pixThreadWait;
  TSeriesVector vsgSeries;
  TSeriesVector vsgAnalysis;
  T3DSeriesVector vsg3DSeries;
  TPSeriesVector vpsUpdate;
  TVarDataVector vvdVars;
  TVarDataVector vvdError;
  TUIntDeque vdtStoreData;
  TLogFile log;
  TSocket *pSocket;
  vectorStreamProps streamProps;
  time_t theTime;
  QTime qtATimer;
  QString qsFile;
  QString qsEqFile;
  QMutex *pStatMutex;
  QMutex *pAnaMutex;
  QTextEdit *pLog,*pEdit, *pComp;
  int nSocket;
  int nCol;
  bool bError;
  bool bReady;
  bool bRunning;
  bool bLocked;
  bool bUpdating;
  bool bStats;
  bool bStatError;
  bool bStatStopped;
  bool bAna;
  bool bAnaError;
  bool bAnaStopped;
  bool bSaving;
  void setupControl();
  void initRgb();
  QColor getRgb(int n);
  bool ReadEquation(QStringList &list);
  bool isInDataStore(unsigned int);
  void doOptions();
  bool Update();
  void UpdateVariables();
  void UpdatePlots();
  void Update3DPlots();
  void UpdateAnalysis();
  void UpdateLyapunovPlots();
  void UpdateAnalysisPlot(unsigned int n);
  void UpdateTimers();
  void startStats();
  void startAnalysis();
  void setPixmap(QListViewItem *pItem,const QColor &c);
  void changeVarName(QString &qs,unsigned int n);
  void changeValue(QString qs,unsigned int n);
  void changeParValue(QString qs,unsigned int n);
  void changeIniValue(QString qs,unsigned int n);
  void changeCopValue(QString& qs,unsigned int n);
  void doDataSubset(graphType &);
  int doAnaSettings(TCalcTypeItem);
  int doAnaSettings(TCalcTypeItem,unsigned int n, QString qs=0);
#ifdef HDF5_FILE
  herr_t writeStringAttrHDF5(hid_t h, const char *name, const char *str);
  herr_t writeDoubleAttrHDF5(hid_t h, const char *name, double d);
  bool LoadH5File(QString &qs);
  bool ReadH5Equation(QStringList &list);
  TParamQueue qParams;
#endif
protected:
   bool lockMutex();
   void unlockMutex();
   void timerEvent( QTimerEvent * );
   void customEvent( QCustomEvent * e );
   bool GetVarType(unsigned int n,TVarData &);
   bool GetStatErrorType(unsigned int n,TVarData &,bool);
   int nTimerId;
   int nUpdateInt;
public:
  Control();
  Control(QTextEdit*pc,QTextEdit *pl, QTextEdit *pe);
  ~Control();
  void Error(const char *);
  bool ReadEquation(QString &qs);
  void LoadEquation(QString &qs);
#ifdef HDF5_FILE
  bool LoadH5Equation(QString &qs);
#endif  
  void NewEquation();
  void saveEquation(QString &qs);
  void saveEquation();
  bool needLogFilename() {return (log.needLogFilename());}
  void setLogFilename(QString &qs) {log.setLogFilename(qs);}
  QString &getLogFilename() {return log.getLogFilename();}
  void saveLog();
  bool needFilename() {return (qsFile.isEmpty()||qsFile.isNull());}
  void setFilename(QString &qs) {qsFile=qs;}
  QString &getFileName() { return qsFile;}
  void Initialize(TWidgetVector &, QSettings *);
  void Done();
  void SetOptions(TEditVector &);
  const char *GetVarName(unsigned int n);
  const char *GetVarTimeName(unsigned int n);
  const char *GetVarDiffName(unsigned int n);
  const char *getName();
  unsigned int GetVarIdx(QString &);
  bool GetVarStruct(unsigned int n,graphType &);
  bool GetVarStruct3D(unsigned int n, struct3DGraph &sg,int axis);
  void addSeries(graphType &);
  void add3DSeries(struct3DGraph &);
  void AddDataStore(QListBox *pList,unsigned int nId);
  void AddAllDataStore(QListBox *pList);
  void UpdateXVars(graphType &);
  const char *getSeriesTitle(QwtPlot *p,long key);
  void toggleStats(bool b);
  void toggleLyap(bool b);
  void addLyapunov(graphType &);
  void addRecur(graphType &);
  void addMaxLyap(graphType &sg);
  void addPeriod(graphType &sg);
  void addPower(graphType &sg);
  void addPoincare(QwtPlot *p);
  void addSpike(QwtPlot *p);
  void clearPlots();
  void killSeries(QwtPlot *p);
  void killSeries(TGlGraph *p);
  void editSeries(QwtPlot *p,long key);
  void editSeries(graphType &);
  void removeSeries(QwtPlot *p,long key);
  void doRecaption(QwtPlot *p, QString qsx, bool bSpecial);
  void clearDataStore();
  void saveDataStore(QString &qs,TSaveTypes type,int c);
#ifdef MAT_FILE
  void saveMatFile(QString &qs);
#endif
#ifdef CDF_FILE
  void saveCDFFile(QString &qs);
  bool statusCDF(CDFstatus status);
  char errorMsg[CDF_STATUSTEXT_LEN+1];
#endif
#ifdef HDF_FILE
  void saveHDFFile(QString &qs,int compress);
#endif
#ifdef HDF5_FILE
  void saveHDF5File(QString &qs,int compress);
#endif
  void UpdatePlot(QwtPlot *p);
  void stopAnalysis(QwtPlot *p);
  double getStep();
  void setStep(double);
  int getBegin();
  void setBegin(int);
  int getEnd();
  void setEnd(int);
  int getInterval();
  void setInterval(int);
  bool logOk();
  void addUserLog(QString &);
  void logState();
  bool doStats() { return bStats;}
  bool doLyap();
  void updateSockets(double time);
  void resetSockets();
#ifndef SPINNER
  void lockedMutex(bool b);
#endif
  const NS_Analysis::Analysis *getAnalysis() { return pAna;}
signals:
    void newGraphSignal(QStringList *);
    void new3DGraphSignal(QStringList *);
    void reCaption(QwtPlot *,QString);
    void doProgress(QwtPlot *,unsigned int, unsigned int);
    void doIsiProgress(unsigned int, unsigned int);
//private slots:
public slots:
  void slotListSelect(QListViewItem *, const QPoint&, int );
  void slotChangeValue();
  void slotEditLostFocus();
  void slotSetIntegrator(int);
  void Reset();
  void Start();
  void Stop();
  void StopStats();
  void StopAnalysis();
  void toggleMean(bool b);
  void cancelSave();
};

}; //end of Namespace NS_Control
#endif
