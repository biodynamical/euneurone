// ControlObject.cpp

#include <qapplication.h>
#include <qtextedit.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qradiobutton.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qcolordialog.h>
#include <qprogressdialog.h>
#include <qcursor.h>
#include <qvalidator.h>
#include <qstringlist.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <unistd.h>
#include <stdexcept>
#include <time.h> 
#include <float.h>

#ifdef USE_GSL
#include <gsl/gsl_errno.h>
#include <gsl/gsl_rng.h>
#else
#include "random250.h"
#endif

#ifndef __BORLANDC__
#include <sys/utsname.h> 
#endif

#include "controlobject.h"
#include "mainform.h"
#include "anasetdialog.h"
#include "savestat.h"

using namespace NS_Control;
using namespace NS_Integrator;
using namespace NS_Equation;
using namespace NS_Stats;
using namespace NS_Analysis;

//try for 30s
#define INT_LOCK_TRY 600
#define INT_LOCK_MS 50

#define SOCKSTART "S"
#define SOCKSTOP "P"
#define SOCKNEXT "N"
#define SOCKHELLO "H"

extern Control *pControl;
extern QApplication *pApp;
extern QString qsApplication;
extern QString qsProgramVersion;
extern MainForm *pMain;

extern const char *achAnalysis;
extern const char *achRecur;
extern const char *achMaxLyap;
extern const char *achPeriod;
extern const char *achPower;
extern const char *achPoincare;
extern const char *achSpike;
extern const char* achCut;
extern const char* achThres;

#ifdef USE_GSL
//random generator
gsl_rng *ranGenerator=NULL;
const gsl_rng_type *ranType=NULL;
#endif

const char *pNoLoaded="No model loaded yet.";
const char *achVars="Variables";
const char *achPars="Parameters";
const char *achConsts="Constants";
const char *achTime="Evolution";
const char *achMimeType="application/vnd.euneurone.var";
const char *achUnknown="Unknown";
const char *achTimerFormat="hh:mm:ss";
const char *achDone="Integration complete";
const char *achStopping="Stopping...";
const char *achRuntimeOptions="Runtime Options";
const char *achLyap="Lyapunov";
const char *achStats="Statistics";
const char *achIntError="Integration error";
const char *achMean="Mean";
const char *achStddev="Std dev";
const char *achRunAvg="Only running average";
const char *achAllVal="All error estimates";
const char *achErrorMean="Error estimate";
const char *achInvalidVal="Invalid real value";
const char *achTrue="True";
const char *achFalse="False";
const char *achOn="on";
const char *achLogExt="log";
const char *achLyapDist="Initial distance";
const char *achLyapStep="# iterations";
const char *achLyapDim="Lyapunov dimension";
const char *achLyapNum="# of systems";
const char *achPoinAt1d="Poincare %1 (%2)";
const char *achPoinAt2d="Poincare %1 (%2, %3)";
const char *achSpikeEst="Spike estimate %1 (%2)";
const char *achHeader="EuNeurone Version 2.5 - Copyright Tjeerd olde Scheper";
const char *achMatHeader="EuNeurone 2.5 - (c) Tjeerd olde Scheper";
const char *achMatlabHeader="MATLAB 5.0 MAT-file";
const char *achPlatform="Platform: ";
const char *achWindows="Windows";
const char *achCreated="Created: ";
const char *achIndex="Index";
const char *achSuccess="File compiled successfully.";
const char *achCompError="Errors found whilst compiling.";
const char *achOpened="Opened file";
const char *achClosed="Closed file and log.";
const char *achSeparator="[========================================]";
const char *achNL="\n";
const char *achComma=", ";
const char *achFmtFont="<font color=#%06X>%s</font>";
const char *achOK="OK";
const char *achFile="File";
const char *achIdentifier="Identifier";
const char *achModel="Model";
const char *achName="Name";
const char *achInfo="Info";
const char *achEqExt=".eq";
//---------------------------------------------------------------------------
static bool bLibError=false;
static QString qsLibError;

#ifdef __BORLANDC__ 
int std::_matherr(struct math_exception *e)
#else
int matherr (struct __exception *e)
#endif
{
 const char *pt;
 if (bLibError)
 	return 1;
 bLibError=true;
 switch (e->type)
 	{
         case DOMAIN: pt="DOMAIN"; break;
         case SING: pt="SINGULARITY"; break;
         case OVERFLOW: pt="OVERFLOW"; break;
	 case UNDERFLOW: pt="UNDERFLOW"; break;
	 case TLOSS: pt="TOTAL LOSS"; break;
	 default: pt="UNKNOWN"; break;
	}
 qsLibError.sprintf("Math library error: %s in function \"%s\" (Arg1=%f, Arg2=%f)",pt,e->name,e->arg1,e->arg2);
 //#ifdef _DEBUG
 qDebug(qsLibError);
//#endif
 //segfaults:
//QMessageBox::warning( pMain, qsApplication,qsLibError,QMessageBox::Ok,QMessageBox::NoButton);
 return 1;
}

//============================================
union hextodbl 
      {
	double  d;
	unsigned char  b[sizeof(double)];
      };

//============================================
TDragObject::TDragObject( unsigned int n, QWidget * parent, const char * name )
    : QStoredDrag( achMimeType, parent, name )
{
   unsigned int i;
     //QByteArray is not an array of bytes (ie unsigned chars) but an 
    //array of chars, therefore one can not simply store an int
    // as four bytes, alternatively we store it as 8 chars of 4 bits
   QByteArray data(sizeof(unsigned int)*2);
   for (i=0;i<(sizeof(unsigned int)*2);i++)
   	data[i]=(n>>(i*4))&0xF;
   setEncodedData( data );
}

bool TDragObject::canDecode( QDragMoveEvent* e )
{
    return e->provides( achMimeType );
}

bool TDragObject::decode( QDropEvent* e, unsigned int *n )
{
    unsigned int i,v;
    //see comment in constructor
    QByteArray data = e->data( achMimeType );
    if ( data.size() )
    {
	e->accept();
	v=0;
	for (i=0;i<data.size();i++)
   		v+=(data[i]<<(i*4));
	*n=v;
	return TRUE;
    }
    return FALSE;
}
//============================================
TListView::TListView(QWidget* parent, const char* name) : QListView(parent,name)
{
 nId=IDINVALID;
 setMultiSelection(false);
#ifdef _DEBUG
 qDebug("[TListView::constructor]");
#endif
}
//---------------------------------------------------------------------------
QDragObject *TListView::dragObject()
{
 QListViewItem *pItem;
 
 pItem=selectedItem();
 if (!pItem)
  return NULL;
 switch(pItem->rtti())
 {
  case ID_RTTIVAR:  nId=(((VarListViewItem *)pItem)->nId); break;
  case ID_RTTICALC: nId=(IDINVALID-(unsigned int) ((CalcListViewItem *)pItem)->type); break;
  case ID_RTTICOP: nId=(IDINVALID-(unsigned int) ((CalcOptionListViewItem *)pItem)->nId); break;
  default:  return NULL;
 }
 TDragObject *pDrag=new TDragObject(nId,this,"VarDrag");
 pDrag->setPixmap(QPixmap::fromMimeSource( "dragdrop.png" ));
 return pDrag;
}
//============================================
EqHighlighter::EqHighlighter(QTextEdit * textEdit, NS_Equation::Equation *pEquation):QSyntaxHighlighter(textEdit)
{
 EquationObject=pEquation;
}

int EqHighlighter::highlightParagraph(const QString& text, int endStateOfLastPara)
{
 int idx,n;
/*#ifdef _DEBUG
 qDebug("[EqHighlighter::highlight] begin %s",text.ascii());
#endif*/
 idx=endStateOfLastPara;
 n=text.length()-1;
 //reset former formatting
 setFormat(0,n,black);
 //first check comment
 idx=text.find('%');
 if (idx>=0)
  {
   setFormat(idx,(n-idx),darkBlue);
/*#ifdef _DEBUG
 qDebug("[EqHighlighter::highlight] done comment.");
#endif */
   return 0;
  }
  //check for header
  if (EquationObject->IsHeader(text.ascii()))
  {
   QString qs;
   if (n>3)
    qs=text.mid(1,n-2);
/*#ifdef _DEBUG
 qDebug("[EqHighlighter::highlight] header [%s],[%s].",text.ascii(),qs.ascii());
#endif */
   if (EquationObject->CheckHeader(qs.ascii(),&idx))
   {
    setFormat(0,n,darkGreen);
    return 0;
   }
   setFormat(0,n,darkRed);
   return 0;
  }
/*#ifdef _DEBUG
 qDebug("[EqHighlighter::highlight] end");
#endif */
 return 0;
}
//---------------------------------------------------------------------------
VarListViewItem::VarListViewItem(QListViewItem *parent,TVarData &vd, double d) : QListViewItem(parent)
{
 QString qs;
 
 nId=vd.nIdx;
 colour=vd.pen.color();
 setText(COLUMN_VAR,vd.Title);
 if (vd.nVarId!=IDINVALID)
 	setText(COLUMN_COL,colour.name());
 qs.setNum(d);
 setText(COLUMN_VAL,qs);
 setText(COLUMN_INI,qs);
 setPixmap(COLUMN_VAR,QPixmap::fromMimeSource("dd.png"));
 setDragEnabled(true);
}
//---------------------------------------------------------------------------
int VarListViewItem::rtti() const
{
 return ID_RTTIVAR;
}
//---------------------------------------------------------------------------
StatListViewItem::StatListViewItem(QListViewItem *parent,TVarData &vd) : QListViewItem(parent)
{
 QString qs;
 
 nId=vd.nIdx;
 colour=vd.pen.color();
 setText(COLUMN_VAR,vd.Title);
 if (vd.nVarId!=IDINVALID)
 	setText(COLUMN_COL,colour.name());
 setPixmap(COLUMN_VAR,QPixmap::fromMimeSource("dd.png"));
 qs.setNum(0.0);
 setText(COLUMN_MEAN,qs);
 setText(COLUMN_VARIANCE,qs);
 setDragEnabled(true);
}
//---------------------------------------------------------------------------
int StatListViewItem::rtti() const
{
 return ID_RTTISTAT;
}
//---------------------------------------------------------------------------
ParListViewItem::ParListViewItem(QListViewItem *parent,Equation *pEq, unsigned int n) : QListViewItem(parent)
{
 QString qs;
 
 nId=n;
 setText(COLUMN_VAR,pEq->GetParmName(n));
 qs.setNum(pEq->GetParmValue(n));
 setText(COLUMN_VAL,qs);
}
//---------------------------------------------------------------------------
int ParListViewItem::rtti() const
{
 return ID_RTTIPAR;
}
//---------------------------------------------------------------------------
ConstListViewItem::ConstListViewItem(QListViewItem *parent,Equation *pEq, unsigned int n) : QListViewItem(parent)
{
 QString qs;
 
 nId=n;
 qs.setNum(n);
 setText(COLUMN_VAR,qs);
 qs.setNum(pEq->GetConstValue(n));
 setText(COLUMN_VAL,qs);
}
//---------------------------------------------------------------------------
int ConstListViewItem::rtti() const
{
 return ID_RTTICONST;
}
//---------------------------------------------------------------------------
CalcListViewItem::CalcListViewItem(QListViewItem *parent,QString qs,TCalcTypeItem t,bool b)  : QListViewItem(parent)
{
 type=t;
 setText(COLUMN_VAR,qs);
 if (b)
 {
  setPixmap(COLUMN_VAR,QPixmap::fromMimeSource("dd.png"));
  setDragEnabled(true);
 }
 else
  setDragEnabled(false);
}
//---------------------------------------------------------------------------
int CalcListViewItem::rtti() const
{
 return ID_RTTICALC;
}
//---------------------------------------------------------------------------
CalcOptionListViewItem::CalcOptionListViewItem(QListViewItem *parent,QString qs,QVariant v, unsigned int n,bool b)  : QListViewItem(parent)
{
 nId=n;
 varValue=v;
 bReadOnly=b;
 setText(COLUMN_VAR,qs);
 setText(COLUMN_VAL,varValue.toString());
}
//---------------------------------------------------------------------------
int CalcOptionListViewItem::rtti() const
{
  return ID_RTTICOP;
}
//---------------------------------------------------------------------------
void CalcOptionListViewItem::setRange(double d1,double d2)
{
  dr1=d1;
  dr2=d2;
}
//---------------------------------------------------------------------------
void CalcOptionListViewItem::setRange(unsigned int n1,unsigned int n2)
{
  nr1=n1;
  nr2=n2;
}
//===============================================================
TLogFile::TLogFile()
{
 pfLog=NULL;
 psLog=NULL;
 pText=NULL;
 bOk=false;
 col=0xFF;
}

TLogFile::~TLogFile()
{
 close();
}

void TLogFile::close()
{
 if (!pfLog)
 	return;
 timeStamp();
 qsFmt.append(achClosed);
 qsFmt.append(achNL);
 qsFmt.append(achSeparator);
 if (pText)
	pText->append(QString().sprintf(achFmtFont,col,qsFmt.ascii()));
  if (psLog)
 	{
	 qsFmt.append(achNL);
	 *psLog << qsFmt;
 	 qsFmt.truncate(0);
	}
 if (pfLog->isOpen())
  	pfLog->close();
 delete psLog;
 delete pfLog;
 psLog=NULL;
 pfLog=NULL;
 file.truncate(0);
 bOk=false;
}
void TLogFile::setLogFilename(QString &qs)
{
  QFileInfo info(qs);
  qs=info.baseName(true);
  qs.append(achLogExt);
  open(qs);
}

void TLogFile::saveLog()
{//write QTextEdit to stream:
 if (psLog)
 	*psLog << pText->text();
}

bool TLogFile::open(std::string &s)
{
 QString qs=s;
 return (open(qs));
}

bool TLogFile::open(char *s)
{
 QString qs=s;
 return (open(qs));
}

bool TLogFile::open(QString &qs)
{
 file=qs;
 if (pfLog)
 	close();
 if (qs.section('.',-1)!=achLogExt)
 	return false; 
 pfLog=new QFile(qs);
 if (!pfLog->open(IO_WriteOnly | IO_Append))
 	{
	 delete pfLog;
 	 return false;
	}
  psLog=new QTextStream(pfLog);
  bOk=true;
  loadLog();
  qsFmt.truncate(0);
  qsFmt.append(achSeparator);
  if (pText)
 		pText->append(QString().sprintf(achFmtFont,0x0,qsFmt.ascii()));
  qsFmt.append(achNL);
  *psLog << qsFmt;
  qsFmt.truncate(0);
  timeStamp();
  qsFmt.append(achOpened);
  if (pText)
 		pText->append(QString().sprintf(achFmtFont,col,qsFmt.ascii()));
  qsFmt.append(achNL);
  *psLog << qsFmt;
  qsFmt.truncate(0);
  qsFmt.append("\t{ ");
  qsFmt.append(file);
  qsFmt.append(" }");
  if (pText)
 		pText->append(QString().sprintf(achFmtFont,col,qsFmt.ascii()));
  qsFmt.append(achNL);
  *psLog << qsFmt;
  qsFmt.truncate(0);
  *psLog << ::flush;
  return true;
}

void TLogFile::setTextEdit(QTextEdit *pt)
{
  pText=pt;
  bOk=true;
}

void TLogFile::loadLog()
{
  if ((!bOk)||(!pText))
  	return;
  QFile f(file);
  if (f.open(IO_ReadOnly))
   {
    QTextStream ts(&f);
    QString text;
    text.append("<font color=#777777>");
    text.append(ts.read());
   // text.append(" </font>\n");
    f.close();
    pText->setText(text);
    pText->scrollToBottom();
   }
}

void TLogFile::timeStamp()
{
  qsFmt.append("[");
  qsFmt.append(QDate::currentDate().toString());
  qsFmt.append(" ");
  qsFmt.append(QTime::currentTime().toString());
  qsFmt.append("] ");
}

// This procedure is to enable the call of switch functions (eg newl,dec,hex...)
// within the stream:  log << "Test:" << i << endl;
TLogFile & TLogFile::operator <<(TLogFile & (*_f)(TLogFile &))
{
 if (bOk)
 	return (*_f)(*this);
 return (*this);
}

TLogFile & TLogFile::operator<<(int i)
{
  if (!bOk)
  	return *this;
  QString qs;
  qs.setNum(i);
  qsFmt.append(qs);
 return *this;
}

TLogFile & TLogFile::operator<<(char *p)
{
  if (!bOk)
  	return *this;
 qsFmt.append(p);
 return *this;
}

TLogFile & TLogFile::operator<<(const char *p)
{
  if (!bOk)
  	return *this;
 qsFmt.append(p);
 return *this;
}

TLogFile & TLogFile::operator<<(char c)
{
  if (!bOk)
  	return *this;
 qsFmt.append(QChar(c));
 return *this;
}

TLogFile & TLogFile::operator<<(QString &qs)
{
  if (!bOk)
  	return *this;
 qsFmt.append(qs);
 return *this;
}

TLogFile & TLogFile::operator<<(double d)
{
  if (!bOk)
  	return *this;
  QString qs;
  qs.setNum(d);
  qsFmt.append(qs);
 return *this;
}

TLogFile & TLogFile::operator<<(QRgb c)
{
  if (!bOk)
  	return *this;
 col=c;
 return *this;
}

TLogFile & endl(TLogFile &l)
{
 // the textedit adds its own newline
 if (l.pText)
 	l.pText->append(QString().sprintf(achFmtFont,l.col,l.qsFmt.ascii()));
  // save to file
 if (l.psLog)
 	*(l.psLog) << l.qsFmt << achNL;
 l.qsFmt.truncate(0);
 return l;
}

TLogFile & time(TLogFile &l)
{
 l.timeStamp();
 return l;
}

TLogFile & tab(TLogFile &l)
{
 l.qsFmt.append("\t");
 return l;
}

TLogFile & flush(TLogFile &l)
{
 if (l.pfLog)
 	l.pfLog->flush();
 return l;
}
//===============================================================
// Socket implementation
// March 2012
// to be continued
TSocket::TSocket(vectorStreamProps *p)
{
  nSocket=0;
  socketFD=-1;
  nNumClients=0;
  step=0.0;
  pProperties=p;
  bStop=true;
  pMutex =new QMutex(false);
}

TSocket::~TSocket()
{
  if(socketFD!= -1)
    close(socketFD);
  delete pMutex;
}

void TSocket::stopThreads()
{
  bStop=false;
  pMutex->lock();
  for (unsigned int i=0;i<vData.size();i++)
    vData[i]->stopThread();
  pMutex->unlock();
  terminate();
}

void TSocket::freeSocket()
{
  if(socketFD!= -1)
    close(socketFD);
  pMutex->lock();
  for (unsigned int i=0;i<vData.size();i++)
    delete vData[i];
  vData.erase(vData.begin(),vData.end());
  nNumClients=0;
  socketFD=-1;
  pMutex->unlock();
}

void TSocket::setStep(double d)
{
  step=d;
 pMutex->lock();
  for (unsigned int i=0;i<vData.size();i++)
  {
    vData[i]->setStep(d);
  }
 pMutex->unlock();
}

bool TSocket::setupSocket()
{
  if (!nSocket)
    return false;

  socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(socketFD == -1)
      return false;
 
   memset(&stSockAddr, 0, sizeof(stSockAddr));
 
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(nSocket);
  stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
   if(bind(socketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr))==-1)
     return false;
   bStop=false;
   return true;
}

void TSocket::run()
{
  while(!bStop)
  {
    if (socketFD!=-1)
    {
       if (listen(socketFD, MAX_SOCKS)!=-1)
       {
#ifdef _DEBUG
qDebug("[TSocket::run] get connection.");
#endif
	 TDataStream *pStream=new TDataStream();
	 //check in properties
	 std::string &s=pStream->Accept(socketFD);
#ifdef _DEBUG
qDebug("[TSocket::run] get accept %s.",s.c_str());
#endif
	 for (unsigned int i=0;i!=pProperties->size();i++)
	 {
	   if ((*pProperties)[i].isEqual(s))
	   {
	     pStream->setStep(step);
	     pStream->setupStream((*pProperties)[i].sName,(*pProperties)[i].nId,(*pProperties)[i].nNum, (*pProperties)[i].nSize);
#ifdef _DEBUG
qDebug("[TSocket::run] set stream.");
#endif
	   }
	 }
	 if (!pStream->isReady())
	 {
	   delete pStream;
	 }
	 else
	 {
	    pMutex->lock();
	    vData.push_back(pStream);
	    nNumClients++;
	    pStream->start();
	    pMutex->unlock();
#ifdef _DEBUG
qDebug("[TSocket::run] start stream.");
#endif
	 }
       }
    }
  }
}

int TSocket::getNumClients()
{
  volatile int n;
  
  pMutex->lock();
  n=nNumClients;
  pMutex->unlock();
  return n;
}

bool TSocket::getStreamData(int client, double t, NS_Equation::dataQueue &dq)
{
  volatile bool b=false;
  pMutex->lock();
  if (client<(int)vData.size())
    b=vData[client]->getDataPoints(t,dq);
  pMutex->unlock();
  return b;
}

void TSocket::resetStreams()
{
 pMutex->lock();
  for (unsigned int i=0;i<vData.size();i++)
  {
    vData[i]->resetStream();
  }
 pMutex->unlock();
}
//===============================================================
TStreamProperties::TStreamProperties(QStringList &sl)
{
  bool b;
  QString qs;
  
  qs=sl.first();
  sName=qs.ascii();
  sl.pop_front();
  qs=sl.first();
  nId=qs.toInt(&b);
  sl.pop_front();
  qs=sl.first();
  nNum=qs.toInt(&b);
  sl.pop_front();
  qs=sl.first();
  nSize=qs.toInt(&b);
  if (nSize<=0)
    nSize=1024;
}
//===============================================================
// DataStreams implementation
// March 2012
TDataStream::TDataStream()
{
  bReady=false;
  nId=IDINVALID;
  nSize=0;
  qSize=0;
  clientFD=-1;
  memset(buff,0,81);
  pMutex =new QMutex(false);
}

TDataStream::~TDataStream()
{
  Stop();
  if (clientFD!=-1)
  {
    shutdown(clientFD, SHUT_RDWR);
    close(clientFD);
  }
  delete pMutex;
}

void TDataStream::stopThread()
{
  Stop();
  terminate();
}

std::string &TDataStream::Accept(int sfd)
{
  socklen_t len;
  
  len=sizeof(stClientAddr);
  clientFD=accept(sfd,(struct sockaddr *)&stClientAddr, &len);
   if (clientFD==-1)
	throw std::runtime_error(QString().sprintf("Failed to connect to client"));
  write(clientFD,SOCKHELLO,1);   
  if (read(clientFD,buff,80)==-1)
	throw std::runtime_error(QString().sprintf("Failed to read response from client"));
  if (strnlen(buff,80)<1)
    	throw std::runtime_error(QString().sprintf("Failed to get a response at all"));
  sName=buff;
  return sName;
}

void TDataStream::setupStream(std::string &s, int id, int n, int size)
{
  state=soNone;
  sName=s;
  nId=id;
  nNum=n;
  nSize=size;
  sprintf(buff,"%s {%s}",qsApplication.ascii(),qsProgramVersion.ascii());
  write(clientFD,buff,strlen(buff));
  fsync(clientFD);
  sprintf(buff,"[%d,%d,%d]",id,n,size);
  write(clientFD,buff,strlen(buff));
  fsync(clientFD);
//qDebug("[TDataStream::setup] waiting.");
  int len=read(clientFD,buff,80);
  if (len>0)
  {
    if (strstr(buff,achOK)!=NULL)
      bReady=true; 
  }
//#ifdef _DEBUG
//qDebug("[TDataStream::setup] send client info: %s.",buff);
//#endif
}

void TDataStream::run()
{
  hextodbl h;
  int remoteId, remoteNum;
  double dt;
  TSockDataPoint dp;
  
  if (clientFD==-1)
    return;
  resetStream();
  while(state!=soStop)
  {
    if (read(clientFD,&remoteId,sizeof(int))==-1)
	throw std::runtime_error(QString().sprintf("Failed to read id data from client"));
    if (read(clientFD,&remoteNum,sizeof(int))==-1)
	throw std::runtime_error(QString().sprintf("Failed to read number data from client"));
    if (read(clientFD,&dt,sizeof(double))==-1)
	throw std::runtime_error(QString().sprintf("Failed to read time stamp from client"));
    if (read(clientFD,&h.b,sizeof(double))==-1)
	throw std::runtime_error(QString().sprintf("Failed to read data from client"));
     pMutex->lock();
     dp.nId=remoteId;
     dp.nIdx=remoteNum;
     dp.tstamp=dt;
     dp.d=h.d;
     queue.push(dp);
//#ifdef _DEBUG
//qDebug("DEBUG: Read from socket id %d [%d], stream %d (size %u): %f.",nId,remoteId,remoteNum,(unsigned int)queue.size(),h.d);
//#endif
     pMutex->unlock();
  }
}

void TDataStream::Start()
{
  write(clientFD,SOCKSTART,1);
  fsync(clientFD);
  sprintf(buff,"T%f",step);
  write(clientFD,buff,strlen(buff));
  fsync(clientFD);
  state=soStart;
}

void TDataStream::Stop()
{
  write(clientFD,SOCKSTOP,1);
  fsync(clientFD);
  state=soStop;
}

void TDataStream::Next()
{
  if (state!=soStop)
  {
    state=soData;
    write(clientFD,SOCKNEXT,1);
    fsync(clientFD);
  }
}

void TDataStream::prefill()
{
    state=soFill;
    for (unsigned int i=0;i<nSize;i++)
      write(clientFD,SOCKNEXT,1);
    fsync(clientFD);
}

void TDataStream::resetStream()
{
  pMutex->lock();
  while (queue.size())
      queue.pop();
  qSize=0;
  Start();
  prefill();
  pMutex->unlock();
}

void TDataStream::setStep(double d)
{
  step=d;
}

bool TDataStream::getDataPoints(double time, NS_Equation::dataQueue &dq)
{
  bool b;
  NS_Equation::TSockDataPoint dp;
  
  pMutex->lock();

  if (queue.size()==0)
  {
    pMutex->unlock();
    return false;
  }
  
  for (int i=0;i<nNum;i++)
  {
    b=false;
    do
    {
     dp=queue.front();
     queue.pop();
     qSize=queue.size();
     //check the time:
     if (!isnan(dp.tstamp))
     {
      if (time>dp.tstamp)
	b=true;
      else
	b=false;
     }
    } while((b)&&(qSize>0));
    dq.push(dp);
    if (qSize==0)
      break;
  }
//#ifdef _DEBUG
//	qDebug("[TDataStream::getData(%p)] time=%f [%f], nId= %d [%d, %d], d %f, qsize %u, %d (nsize %u)",this,time,dp.tstamp,nId,dp.nId,dp.nIdx,dp.d,qSize,(int)queue.size(),nSize);
//#endif
 if (qSize<nSize)
   	Next();
 pMutex->unlock();
 return true;
}
//===============================================================
/*
 Modified save file dialog
*/
TSaveDlg::TSaveDlg(QWidget *parent, TSaveTypes nType) : QFileDialog(parent, "SaveData", TRUE)
{ 
 setMode( QFileDialog::AnyFile );
 bgCompress=NULL;
 cbCompress=NULL;
 bgAppend=NULL;
 cbAppend=NULL;
 cbRows=NULL;
 cbVars=NULL;
 lblComment=NULL;
 leComment=NULL;
 type=nType;
 switch (nType)
	{
		default:
			case stNone: break;
			case stTSV: addFilter( "TSV files (*.tsv)" ); break;
			case stCSV: {
				addFilter( "CSV files (*.csv)" );
				bgAppend=new QButtonGroup("Options",this);
				cbAppend=new QCheckBox("Append",bgAppend);
				cbAppend->move(5,25);
				bgAppend->insert(cbAppend);
				cbVars=new QCheckBox("Variables",bgAppend);
				cbVars->move(5,50);
				bgAppend->insert(cbVars);
				cbRows=new QCheckBox("Data in Rows",bgAppend);
				cbRows->move(5,75);
				bgAppend->insert(cbRows);
				addRightWidget(bgAppend);
				bgAppend->resize(100,100);
				lblComment = new QLabel( "Comment", this );
        			leComment = new QLineEdit( this );
			        addWidgets( lblComment, leComment, NULL );
				break;
				}
			case stMat:  {
				addFilter( "Matlab files (*.mat)" );
				cbCompress=new QCheckBox("Compress data",this);
				addWidgets(NULL,cbCompress,NULL);
				break;
				}
			case stCDF: addFilter( "Common Data Format (*.cdf)" ); break;
			case stHDF: {
				addFilter( "Hierarchical Data Format (*.hdf)" );
				bgCompress=new QButtonGroup("Compression",this);
				rb1=new QRadioButton("None", bgCompress);
				rb1->move(5,25);
				bgCompress->insert(rb1);
				rb2=new QRadioButton("RLE", bgCompress);
				rb2->move(5,50);
				bgCompress->insert(rb2);
				rb3=new QRadioButton("Huffman", bgCompress);
				rb3->move(5,75);
				bgCompress->insert(rb3);
				rb4=new QRadioButton("GZIP Low (3)", bgCompress);
				rb4->move(5,100);
				bgCompress->insert(rb4);
				rb5=new QRadioButton("GZIP Medium (6)", bgCompress);
				rb5->move(5,125);
				bgCompress->insert(rb5);
				rb6=new QRadioButton("GZIP High (9)", bgCompress);
				rb6->move(5,150);
				bgCompress->insert(rb6);
				bgCompress->setExclusive( TRUE );
				bgCompress->setButton(0);
				addRightWidget(bgCompress);
				bgCompress->resize(100,100);
				break;
			}
			case stHDF5: {
				addFilter( "Hierarchical Data Format 5 (*.h5)" );
				bgCompress=new QButtonGroup("Compression",this);
				rb1=new QRadioButton("None", bgCompress);
				rb1->move(5,25);
				bgCompress->insert(rb1);
				rb2=new QRadioButton("GZIP", bgCompress);
				rb2->move(5,50);
				bgCompress->insert(rb2);
				rb3=new QRadioButton("SZIP", bgCompress);
				rb3->move(5,75);
				bgCompress->insert(rb3);
				bgCompress->setExclusive( TRUE );
				bgCompress->setButton(1);
				addRightWidget(bgCompress);
				bgCompress->resize(100,120);
				break;
			}
	}
 setSelectedFilter(1);
 setCaption( tr( "Save data as" ) );
}

void TSaveDlg::setName(QString &qs)
{
 if (qs.isEmpty())
	return;
 switch (type)
	{
	default:
	case stNone: break;
	case stTSV: qs.append(".tsv"); break;
	case stCSV: qs.append(".csv"); break;
	case stMat: qs.append(".mat"); break;
	case stCDF: qs.append(".cdf"); break;
	case stHDF: qs.append(".hdf"); break;
	case stHDF5: qs.append(".h5"); break;
	}
	setSelection(qs);
}
//===============================================================
Control::Control() : QObject()
{
 EquationObject =new Equation();
 pIntMutex =new QMutex(false);
 pStatMutex =new QMutex(false);
 pAnaMutex =new QMutex(false);
 IntegratorObject =new Integrator();
 IntegratorObject->SetLyapunov(true);
 pStats=new Statistics(pIntMutex,pStatMutex);
 pAna=new Analysis(pIntMutex,pAnaMutex);
 pSocket=new TSocket(&streamProps);
 pixThreadOn=QPixmap(QPixmap::fromMimeSource( "threadon.png" ));
 pixThreadOff=QPixmap(QPixmap::fromMimeSource( "threadoff.png" ));
 pixThreadWait=QPixmap(QPixmap::fromMimeSource( "threadwait.png" ));
 bReady=false;
 bLocked=false;
 bUpdating=false;
 bStats=true;
 bStatError=false;
 bStatStopped=false;
 bAna=true;
 bAnaError=false;
 bAnaStopped=false;
 pComp=NULL;
 pLog=NULL;
 pEdit=NULL;
 pListItem=NULL;
 pLcd=NULL;
 pLcdElapTime=NULL;
 pLcdEstTime=NULL;
 pLcdUpdate=NULL;
 pProgress=NULL;
 pButtonGroup=NULL;
 pStatusBar=NULL;
 pLineEdit=NULL;
 initRgb();
 //init random generators
 #ifdef USE_GSL
 //turn off gsl's error handling
 gsl_set_error_handler_off();
 gsl_rng_env_setup();
 if (gsl_rng_default_seed==0)
 {//do something reasonable
  gsl_rng_default_seed=time(NULL);
 }
 if (ranType==NULL)
 	ranType=gsl_rng_default;
 if (ranGenerator==NULL)
 	ranGenerator=gsl_rng_alloc(ranType);
#else
 rnd_init();
#endif
}
//---------------------------------------------------------------------------
Control::Control(QTextEdit *pc, QTextEdit *pl,QTextEdit *pe) : QObject()
{
 Q_ASSERT(pc!=NULL);
 Q_ASSERT(pl!=NULL);
 Q_ASSERT(pe!=NULL);
 EquationObject = new Equation();
#ifdef SPINNER
 spinObject=new TDataSpinner();
#else
 pIntMutex =new QMutex(false);
#endif
 pStatMutex =new QMutex(false);
 pAnaMutex =new QMutex(false);
 IntegratorObject =new Integrator();
 IntegratorObject->SetLyapunov(true);
 pStats=new Statistics(pIntMutex,pStatMutex);
 pAna=new Analysis(pIntMutex,pAnaMutex);
 pSocket=new TSocket(&streamProps);
 EquationObject->SetFunction(pc);
 pEqHigh=new EqHighlighter(pe,EquationObject);
 pixThreadOn=QPixmap(QPixmap::fromMimeSource( "threadon.png" ));
 pixThreadOff=QPixmap(QPixmap::fromMimeSource( "threadoff.png" ));
 pixThreadWait=QPixmap(QPixmap::fromMimeSource( "threadwait.png" ));
 bReady=false;
 bLocked=false;
 bStats=true;
 bStatError=false;
 bStatStopped=false;
 bAna=true;
 bAnaError=false;
 bStatStopped=false;
 bAnaStopped=false;
 initRgb();
 pListItem=NULL;
 pComp=pc;
 pLog=pl;
 pEdit=pe;
 pLcd=NULL;
 pLcdElapTime=NULL;
 pLcdEstTime=NULL;
 pLcdUpdate=NULL;
 pProgress=NULL;
 pButtonGroup=NULL;
 pStatusBar=NULL;
 pLineEdit=NULL;
 log.setTextEdit(pl);
 //init random generators
#ifdef USE_GSL
 gsl_rng_env_setup();
 if (gsl_rng_default_seed==0)
 {//do something reasonable
  gsl_rng_default_seed=time(NULL);
 }
 if (ranType==NULL)
 	ranType=gsl_rng_default;
 if (ranGenerator==NULL)
 	ranGenerator=gsl_rng_alloc(ranType);
#else
 rnd_init();
#endif
}
//---------------------------------------------------------------------------
Control::~Control()
{
 if (IntegratorObject->running())
 	{
  	 IntegratorObject->Stop();
	 IntegratorObject->wait();
	}
 if (pStats->running())
 	{
  	 pStats->Stop();
	 pStats->wait();
	}
 if (pAna->running())
 	{
  	 pAna->Stop();
	 pAna->wait();
	}
 //try to lock each mutex to make sure that they are unlocked properly
 if (pAnaMutex->tryLock())
 	pAnaMutex->unlock();
 if (pStatMutex->tryLock())
 	pStatMutex->unlock();
#ifndef SPINNER
  if (pIntMutex->tryLock())
	pIntMutex->unlock();
#endif
  log.close();
 pSocket->stopThreads();
 delete pSocket;
 delete pEqHigh;
 delete pAna;
 delete pStats;
 delete IntegratorObject;
 delete pAnaMutex;
 delete pStatMutex;
#ifdef SPINNER
 delete spinObject;
#else
 delete pIntMutex;
#endif
 delete EquationObject;
#ifdef USE_GSL
 if (ranGenerator!=NULL)
 	gsl_rng_free(ranGenerator);
#endif
}
//---------------------------------------------------------------------------
void Control::initRgb()
{
  int g,r,b;
  cvRgb.clear();
    for ( g = 0; g < 4; g++ )
	for ( r = 0;  r < 4; r++ )
	    for ( b = 0; b < 3; b++ )
		cvRgb.push_back(qRgb( r*255/3, g*255/3, b*255/2 ));
}
//---------------------------------------------------------------------------
QColor Control::getRgb(int n)
{
  QColor c;
  
  do
  {
   c=cvRgb[(n*COLOUR_MULTIPLIER)%cvRgb.size()];
   n++;
  } while (c==white);
  return c;
}
//---------------------------------------------------------------------------
void Control::Error(const char *ps)
{
 Q_ASSERT(ps!=NULL);
 
 QString qs;

 bError=true;
 if (pStatusBar)
 {
  qs.sprintf("Error: %s",ps);
  pStatusBar->message(qs,5000);
 }
 qs.sprintf("Error:\n%s\n",ps);
 QMessageBox::warning( pMain, qsApplication,qs,QMessageBox::Ok,QMessageBox::NoButton);
}
//----------------------------------------------------------------------------------------------------
bool Control::ReadEquation(QString &qs)
{
  if (EquationObject==NULL)
    return false;
   qsFile=qs;
  if (pComp)
   	pComp->append(QString().sprintf("<font color=#0000FF>Opening file : </font>%s",qs.ascii()));
  // read equation file
  LoadEquation(qs);
  //Split into lines and parse:
  QStringList list=QStringList::split('\n', qsEqFile);
  return ReadEquation(list);
}
//----------------------------------------------------------------------------------------------------
bool Control::ReadEquation(QStringList &list)
{
 bool b=false;
  //close log if opened
  log.close();
  if (pLog)
  	pLog->clear();
  //flush databuffers
#ifdef SPINNER
  spinObject->flush();
#endif
  b=EquationObject->ReadEquation(list);
  if (b)
  {
   bReady=true;
#ifdef SPINNER
   spinObject->initData(EquationObject->GetN());
#endif
   if (!EquationObject->GetNumWarnings())
   {
    EquationObject->Print();
    if (pComp)
   	pComp->append(QString().sprintf(achFmtFont,0xFF,achSuccess));
   }
   setupControl();
   doOptions();
   //start socket thread
   pSocket->start();
  Reset();
  }
  else
  {
   bReady=false;
    if (pComp)
   	pComp->append(QString().sprintf(achFmtFont,0xFF,achCompError));
   }
 if (pEdit)
    pEdit->setText(qsEqFile);
 return b;
}
//-----------------------------------------------------------------------------------------------------------
bool Control::logOk()
{
 return log.doLog();
}
//-----------------------------------------------------------------------------------------------------------
void Control::addUserLog(QString &qs)
{
  if (log.doLog())
  	{
  	  log << QRgb(0x707F00) << time << "User log item" << endl;
	  log << tab << "( " << qs << " )" << endl << QRgb(0xFF);
	}
}
//-----------------------------------------------------------------------------------------------------------
void Control::logState()
{
  if (log.doLog())
  	{
	 unsigned int i,n;
	 char c;
	 
  	  log << QRgb(0x648743) << time << "Equation State" << endl;
	  log << tab << "[Integrator]" << endl;
	  log << tab << "{ " << IntegratorObject->GetIntegratorName() << achComma << "begin=" << IntegratorObject->GetBegin() << achComma << "step=" << IntegratorObject->GetStep() << achComma << "end=" << IntegratorObject->GetEnd() << achComma << "current=" << IntegratorObject->Gett() << " }" << endl << endl;
	  
	  log << tab << "[Parameters]" << endl;
	  n=EquationObject->GetNPar();
	  for (i=0;i<n;i++)
	  {
	   log << tab << "{ " << EquationObject->GetParmName(i) << "=" << EquationObject->GetParmValue(i) << " }";
	   if (i==(n-1))
	   	log << endl;
	   else
	   	log << achComma << endl;
	  }
	  log << endl;
	  log << tab << "[Variables]" << endl;
	  n=EquationObject->GetN();
	  for (i=0;i<n;i++)
     	 {
      	  switch(EquationObject->GetVariableType(i))
	    {
	       default:
	       case NS_Formu::etUnknown: c='?'; break;
	       case NS_Formu::etParAsVar: c='!'; break;
	       case NS_Formu::etParam: c=' '; break;
	       case NS_Formu::etVar: c=' '; break;
	       case NS_Formu::etImm: c=' '; break;
	       case NS_Formu::etDeriv: c='\''; break;
	       case NS_Formu::etMap: c='~'; break;
      	   }
	   log << tab << "{ " << EquationObject->GetVarName(i) << c << "=" << EquationObject->GetEquation(i) << endl;
	   log << tab << "  initial=" << EquationObject->GetVarIniValue(i) << achComma << "current=" << EquationObject->GetVarValue(i) << " }";
	   if (i==(n-1))
	   	log << endl;
	   else
	   	log << achComma << endl;
     	  }
	  log << QRgb(0xFF);
	}
}
//-----------------------------------------------------------------------------------------------------------
void Control::LoadEquation(QString &qs)
{
  QFile f(qs);
   if (f.open(IO_ReadOnly))
   {
    QTextStream ts(&f);
    qsEqFile = ts.read();
    f.close();
   }
}
//-----------------------------------------------------------------------------------------------------------
#ifdef HDF5_FILE
bool Control::ReadH5Equation(QStringList &list)
{
 bool b=false;
  //close log if opened
  log.close();
  if (pLog)
  	pLog->clear();
  //flush databuffers
#ifdef SPINNER
  spinObject->flush();
#endif
  b=EquationObject->ReadEquation(list);
  if (b)
  {
   bReady=true;
#ifdef SPINNER
   spinObject->initData(EquationObject->GetN());
#endif
  //set parameter values
  while(!qParams.empty())
  {
    EquationObject->SetParmValue(qParams.front().name,qParams.front().value);
    qParams.pop();
  }
   if (!EquationObject->GetNumWarnings())
   {
    EquationObject->Print();
    if (pComp)
   	pComp->append(QString().sprintf(achFmtFont,0xFF,achSuccess));
   }
   setupControl();
   doOptions();
   //start socket thread
   pSocket->start();
  Reset();
  }
  else
  {
   bReady=false;
    if (pComp)
   	pComp->append(QString().sprintf(achFmtFont,0xFF,achCompError));
   }
 if (pEdit)
    pEdit->setText(qsEqFile);
 return b;
}

bool Control::LoadH5Equation(QString &qs)
{
  if (EquationObject==NULL)
    return false;
  if (pComp)
   	pComp->append(QString().sprintf("<font color=#0000FF>Loading from HDF5 file : </font>%s",qs.ascii()));
  qsFile=QString::null;
  // read equation file
  if (!LoadH5File(qs))
    return false;
  //Split into lines and parse:
  QStringList list=QStringList::split('\n', qsEqFile);
  return (ReadH5Equation(list));
}
#endif
//-----------------------------------------------------------------------------------------------------------
void Control::saveEquation()
{
 if ((!qsFile.isNull())&&(!qsFile.isEmpty()))
 	saveEquation(qsFile);
}
//-----------------------------------------------------------------------------------------------------------
void Control::saveEquation(QString &qs)
{
  QString qsOld;
  
  QFileInfo info(qs);
  qs=info.baseName(true);
  qs.append(achEqExt);
  QFile f(qs);
   if (pEdit->length()>0)
   {
#ifndef _WINDOWS
    //change old file 
    qsOld=qs+".old";
    //rename the old file
    if (rename(qs,qsOld))
    	{
	 if (log.doLog())
  	 {
  	  log << time << "Error renaming old file, will overwrite" << endl;
	  log << tab << "{ " << strerror(errno)  << " }"  << endl;
	 }
#ifdef _DEBUG
	qDebug("[Control::SaveEquation] Failed to rename the file %s: %s",qs.ascii(),strerror(errno));
#endif
	}
#endif
    if (f.open(IO_WriteOnly))
   {
    QTextStream ts(&f);
    ts << pEdit->text();
    f.close();
    qsEqFile=pEdit->text();
   }
  }
}
//-----------------------------------------------------------------------------------------------------------
void Control::saveLog()
{
 log.saveLog();
}
//-----------------------------------------------------------------------------------------------------------
void Control::NewEquation()
{
 if (pComp)
 	pComp->clear();
 if (pEdit)
 	pEdit->clear();
 qsFile="";
#ifdef SPINNER
 spinObject->flush();
#endif
 EquationObject->newEquation(pEdit);
 bReady=false;
}
//-------------------------------------------------------------------------------------------------------------
void Control::Initialize(TWidgetVector &v, QSettings *ps)
{
 int i,n;
 unsigned int j,type;
 QRadioButton *prb;
 
 nUpdateInt=1000; 
 bError=false;
 bReady=false;
 bRunning=false;
 type=0;
 qtATimer.setHMS(0,0,0,0);
 
 for (j=0;j<v.size();j++)
 {
  if (v[j]->isA("QListView"))
  	type=1;
  if (v[j]->isA("QProgressBar"))
	type=2;
  if (v[j]->isA("QButtonGroup"))
  	type=3;
  if (v[j]->isA("QLCDNumber"))
   	type=4;
  if (v[j]->isA("QStatusBar"))
   	type=5;
  if (v[j]->isA("QLabel"))
   	type=6;

  switch(type)
  {
   default: break;
   case 1: //listview
   	{
	 pListView=(TListView *)v[j];
	 break;
	}
   case 2: //progressbar
   	{
	 pProgress=(QProgressBar *)v[j];
	 break;
	}
   case 3: //buttongroup
   	{
	 pButtonGroup=(QButtonGroup *)v[j];
	 break;
	}
   case 4: //lcd's
   	{
	 QLCDNumber *p=(QLCDNumber *)v[j];
	 if (strcmp(p->name(),"lCDNumber")==0)
	 	pLcd=p;
	 if (strcmp(p->name(),"lCDElapTime")==0)
	 	pLcdElapTime=p;
	 if (strcmp(p->name(),"lCDEstTime")==0)
	 	pLcdEstTime=p;
	 if (strcmp(p->name(),"lCDUpdate")==0)
	 	pLcdUpdate=p;
	 break;
	}
   case 5: //statusbar
   	{
	 pStatusBar=(QStatusBar *)v[j];
	 break;
	}
   case 6: //label
   	{
	 QLabel *p=(QLabel *)v[j];
	 if (strcmp(p->name(),"lblIntThread")==0)
	 	pPixIntLabel=p;
	 if (strcmp(p->name(),"lblStatThread")==0)
	 	pStats->setLabel(p);
	 if (strcmp(p->name(),"lblAnaThread")==0)
	 	pAna->setLabel(p);
	 break;
	}
  }
 }
 n=IntegratorObject->GetNumIntegrators();
  if (pButtonGroup)
  {
 //empty the buttongroup if necessary
   if (pButtonGroup->count()>0)
   {
    while(pButtonGroup->count()>0)
    {
     QButton *pb=pButtonGroup->find(0);
     if (pb!=NULL)
      pButtonGroup->remove(pb);
    }
   }
   QRect r=pButtonGroup->geometry();
   r.setBottom((n+1)*25+10);
   pButtonGroup->setGeometry(r);
   for (i=0;i<n;i++)
	{
	  prb = new QRadioButton(IntegratorObject->GetIntegratorName(i), pButtonGroup);
          prb->setGeometry( 10, i*25+20, pButtonGroup->width()-15, 25 );
          if ( i == IntegratorObject->GetIntegrator() )
            prb->setChecked( TRUE );
	}
   connect( pButtonGroup, SIGNAL( clicked(int) ), SLOT( slotSetIntegrator(int) ));
 }
 //set default values for progress bar
 if (pProgress)
 {
  pProgress->setTotalSteps(IntegratorObject->GetMaxEstimate());
  pProgress->reset();
 }
 if (pLcdElapTime)
 {
  pLcdElapTime->display(qtATimer.toString(achTimerFormat));
 }
 if (pLcdEstTime)
 {
  pLcdEstTime->display(qtATimer.toString(achTimerFormat));
 }
/* if (pLcdUpdate)
 {
  pLcdUpdate->display(achNoTime);
 }*/
 
 //load settings for analysis
  pAna->loadSettings(ps);
}
//-------------------------------------------------------------------------------------------------------------
void Control::doOptions()
{
 unsigned int i,j,n;
 structOptionT *pOp;
 QStringList slVars; 
 QString qs;
 
 for (i=0;i<EquationObject->GetNumOptions();i++) 
 {
  pOp=EquationObject->GetOption(i);
  if (pOp)
  	{
	 switch(pOp->type)
	 {
	  default: break;
	  case opIntegrator:
	  	{//set integrator
		 n=IntegratorObject->GetNumIntegrators();
		 for (j=0;j<n;j++)
		 	{
			 if (pOp->s==IntegratorObject->GetIntegratorName(j))
			 	{
			 	 IntegratorObject->SetIntegrator(j);
				 pButtonGroup->setButton(j);
				 if (pComp)
				 {
				  qs.sprintf("Set integrator to %s.",pOp->s.c_str());
				  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
				 }
				 break;
				}
			}
		break;
		}
	  case opLength:
	  	{//set integrator length
		 IntegratorObject->SetStop(pOp->d);
		 if (pComp)
		 {
		  qs.sprintf("Set integration stop to %f.",pOp->d);
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
		 break;
		}
	  case opStepSize:
	  	{//set integrator stepsize
		 IntegratorObject->SetStep(pOp->d);
		 if (pComp)
		 {
		  qs.sprintf("Set integration step to %f.",pOp->d);
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
		 break;
		}
	  case opOutput:
	  	{//set interval
		 n=(int)ceil(pOp->d);
		 setInterval(n);
		 if (pComp)
		 {
		  qs.sprintf("Set update interval to %f.",pOp->d);
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
		 break;
		}
	  case opSocket:
	  	{
		 pSocket->freeSocket();
		 n=(int)ceil(pOp->d);
		 pSocket->setSocket(n);
		 if (pSocket->setupSocket())
		 {
		   pSocket->setStep(IntegratorObject->GetStep());
		  if (pComp)
		  {
		   qs.sprintf("Set socket number %u.",n);
		   pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		  }
		 }
		 else
		 {
		  qs.sprintf("FAILED to bind socket number %u!",n);
		  if (pComp)
		      pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		  Error(qs);
		 }
		 break;
		}
	  case opClient:
	  	{
		 slVars =QStringList::split(',',pOp->s.c_str());
		 if (slVars.size())
		 {//parse into strings, id numbers and buffer size
		  TStreamProperties props(slVars);
		  streamProps.push_back(props);
		 }
		 if (pComp)
		 {
		  qs.sprintf("Set client to %s.",pOp->s.c_str());
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
		 break;
		}
	  case opGraph:
	  	{
		 slVars =QStringList::split(',',pOp->s.c_str());
		 if (slVars.size())
		 	emit newGraphSignal(&slVars);
		 if (pComp)
		 {
		  qs.sprintf("Created graph: %s.",pOp->s.c_str());
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
		 break;
		}
	  case op3D:
	  	{
#ifdef DO3D		
		 slVars =QStringList::split(',',pOp->s.c_str());
		 if (slVars.size())
		 	emit new3DGraphSignal(&slVars);
		 if (pComp)
		 {
		  qs.sprintf("Created 3D graph: %s.",pOp->s.c_str());
		  pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 }
#endif
		 break;
		}
	 case opLyap:
	 	{
		 if (pOp->s==achOn)
		 	toggleLyap(true);
		 else
		 	toggleLyap(false);
		 break;
		}
	 case opStats:
	 	{
		 if (pOp->s==achOn)
		 	toggleStats(true);
		 else
		 	toggleStats(false);
		 break;
		}
	  case opLog:
	  	{
		 if (pOp->s.find('/')==pOp->s.npos)
		 	{
		 	 qs=qsFile.section('/',0,-2,QString::SectionSkipEmpty|QString::SectionIncludeLeadingSep|QString::SectionIncludeTrailingSep );
			 qs.append(pOp->s);
			}
		 else
		 	qs=pOp->s;
#ifdef _DEBUG
  qDebug("[Control::doOptions] opening file %s at %s",pOp->s.c_str(),qs.ascii());
#endif
		 if (!log.open(qs))
			 	{
			 	 if (pComp)
		 		 {
		  	 	  qs.sprintf("Error opening Log file: %s.",pOp->s.c_str());
		  	  	  pComp->append(QString().sprintf(achFmtFont,0xFF0010,qs.ascii()));
#ifdef _DEBUG
  qDebug("[Control::doOptions] Could not open log file %s",pOp->s.c_str());
#endif
		 	 	 }
				 break;
				}
		 if (pComp)
		 	{
		  	 qs.sprintf("Opened Log file: %s.",pOp->s.c_str());
		  	 pComp->append(QString().sprintf(achFmtFont,0x70FF,qs.ascii()));
		 	 }
		 break;
		}
	 }
	}
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::timerEvent(QTimerEvent*)
{
 //first stop the timer, it may otherwise be triggered while we're still updating
 killTimer(nTimerId);
 //check if we are not already busy updating
 if (!bUpdating)
 	Update();
 //all done updating, but not finished, then restart timer:
 if (bRunning)
 	nTimerId=startTimer(nUpdateInt);
}
//-------------------------------------------------------------------------------------------------------------
void Control::Done()
{
 //first stop the timer, it may otherwise be triggered while we're still updating
 killTimer(nTimerId);
 if (IntegratorObject->running())
 	{
  	 IntegratorObject->Stop();
 	 if (IntegratorObject->running())
	 	IntegratorObject->wait();
	}
 if (!bUpdating)
 	Update();
  if (pPixIntLabel)
 	 pPixIntLabel->setPixmap(pixThreadOff);
}
//-------------------------------------------------------------------------------------------------------------
// update socket data structures in formuClass, take from datastreams
void Control::updateSockets(double time)
{
  unsigned int i, num;
  TSockDataPoint dp;

  //to accomodate time stamp, check for valid time in each stream and 
  num=pSocket->getNumClients();
  if (num<1)
    return;
  for (i=0;i<num;i++)
  {
     pSocket->getStreamData(i,time,sockDataQueue);
   }
   //i=0;
   while(sockDataQueue.size())
   {
     dp=sockDataQueue.front();
     sockDataQueue.pop();
     EquationObject->setSocketData(dp);
//#ifdef _DEBUG
//	qDebug("[Control::updateSockets] client %d, client id %d, stream %d of %d, data %f",dp.nId,dp.nIdx,i,num,dp.d);
	//i++;
//#endif
   }
}
//-------------------------------------------------------------------------------------------------------------
void Control::resetSockets()
{
  pSocket->resetStreams();
}
//-------------------------------------------------------------------------------------------------------------
#ifndef SPINNER
void Control::lockedMutex(bool bIntlock)
{
 // Checking with Valgrind showed that using the main Qt thread for the mime type pixmap could cause
 // unexplained race conditions, using a local copy of the pixmap, not updated locally should prevent this.
  if (bIntlock)
  	{
  	 if ((bRunning)&&(pPixIntLabel))
 	 	pPixIntLabel->setPixmap(pixThreadWait);
  	}
  else
  	{
  	 if ((!bRunning)&&(pPixIntLabel))
 	 	pPixIntLabel->setPixmap(pixThreadOff);
 	 else
  	  if ((bRunning)&&(pPixIntLabel))
 	 	pPixIntLabel->setPixmap(pixThreadOn);
  	}
  // FIXED: It seems that ControlObject runs as a separate thread from which Integrator is branched
  // this means that sometimes the main Qt thread is waiting for this thread to complete even though
  // nothing is happening...
  //qApp->processEvents();
}
#endif
//-------------------------------------------------------------------------------------------------------------
bool Control::Update()
{
 try
 {
  bUpdating=true;
  UpdateTimers();
  UpdateVariables();
#ifndef SPINNER
 if (!lockMutex())
  {
#ifdef _DEBUG
  qDebug("[Control::Update] Failed lock int mutex, will try again..");
#endif
  bUpdating=false;
  return false;
  }
#endif
 //check to see if thread has stopped
 if (IntegratorObject->finished())
 {
  bRunning=false;
  if (!IntegratorObject->IsOk())
  { //some error occurred
   Error(IntegratorObject->GetErrorStr());
  }
 }
 QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
 if (pLcdUpdate)
 	theTime=time(NULL);
 if (pLcd)
 	pLcd->display(IntegratorObject->Gett());
 if (pProgress)
 	pProgress->setProgress(IntegratorObject->GetSteps(),IntegratorObject->GetMaxEstimate());
#ifndef SPINNER
 lockedMutex(true);
#endif
  UpdatePlots();
  Update3DPlots();
  UpdateLyapunovPlots();
  UpdateAnalysis();
#ifndef SPINNER
  unlockMutex();
#endif
  if (bStats)
  	startStats();
  if (bAna)
  {
   pAna->clearFlags();
   startAnalysis();
  }
  if (vpsUpdate.size())
  	{
  	  graphType *psg;
  	  for (unsigned int i=0;i<vpsUpdate.size();i++)
  	  	{
  	  	 psg=vpsUpdate[i];
  	  	 psg->replot();
  	  	}
  	  vpsUpdate.clear();
  	}
 if (pLcdUpdate)
 	{
	 QTime t;
	 t.addSecs(static_cast<int>(difftime(time(NULL),theTime)));
 	pLcdUpdate->display(t.toString(achTimerFormat));
	}
  if ((!bRunning)&&(pStatusBar))
 	pStatusBar->message(achDone,5000);
   bUpdating=false;
   QApplication::restoreOverrideCursor();
  }
 catch (std::exception &stdex)
     {
       QApplication::restoreOverrideCursor();
       Error(stdex.what());
       bUpdating=false;
 #ifdef _DEBUG
  qDebug("[Control::Update] ERROR (error=%s).",stdex.what());
 #endif
  } 
 return true;
}
//-------------------------------------------------------------------------------------------------------------
void Control::UpdateVariables()
{ //update the variables in the listview
 unsigned int i,id;
 double d;
 QString qs;
 
 if (!pStatMutex->tryLock())
  return;
 for (i=0;i<vvdVars.size();i++)
 {
   id=vvdVars[i].nVarId;
   if (id==IDINVALID)
   	d=IntegratorObject->Gett();
   else
   	d=EquationObject->GetVarValue(id);
   qs.setNum(d);
   vvdVars[i].pItem->setText(COLUMN_VAL,qs);
 }
 pStatMutex->unlock();
}
//-------------------------------------------------------------------------------------------------------------
// This procedure updates the various plots, it can take quite some time
// especially when copying data and drawing on the canvas
// Sometimes this may cause the integrator to wait for a long, long
// time to lock the mutex so it can do another step. Therefore,
// we'll allow the integrator some time in between updating to
// lock the mutex...
void Control::UpdatePlots()
{
 dblDequeT *pdX,*pdY;
 graphType *psg;
 unsigned int i=0;
 
 while(i<vsgSeries.size())
 {
  psg=&vsgSeries[i];
  if (!psg->isValid(vvdVars.size()))
 	continue;
  pdX=vvdVars[psg->idX()].pData;
  pdY=vvdVars[psg->idY()].pData;
#ifdef _DEBUG
	qDebug("[Control::UpdatePlot] Updating %d: xsize=%u (%p), ysize=%u (%p)",i,(unsigned int)pdX->size(),pdX,(unsigned int)pdY->size(),pdY);
#endif
  if (psg->appendData(pdX,pdY))
	vpsUpdate.push_back(psg);
//  	psg->replot();
  i++;
#ifdef _DEBUG
	qDebug("[Control::UpdatePlot] Pausing mutex");
#endif
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::Update3DPlots()
{
 dblDequeT *pdX,*pdY,*pdZ;
 struct3DGraph *psg;
 unsigned int i;
 
 for (i=0;i<vsg3DSeries.size();i++)
 {
  psg=&vsg3DSeries[i];
  if ((psg->nXVarId>=vvdVars.size())||(psg->nYVarId>=vvdVars.size())||(psg->nZVarId>=vvdVars.size()))
 	continue;
  pdX=vvdVars[psg->nXVarId].pData;
  pdY=vvdVars[psg->nYVarId].pData;
  pdZ=vvdVars[psg->nZVarId].pData;
#ifdef _DEBUG
	qDebug("[Control::Update3DPlots] title=%s,x=%d,y=%d,z=%d",psg->Title.ascii(),psg->nXVarId,psg->nYVarId,psg->nZVarId);
#endif
  psg->pPlot->appendPoints(psg->uCurve,*pdX,*pdY,*pdZ,psg->nSizeX,psg->nSizeY,psg->nSizeZ);
  psg->nSizeX=pdX->size();
  psg->nSizeY=pdY->size();
  psg->nSizeZ=pdZ->size();
  psg->pPlot->replot();
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::UpdateAnalysisPlot(unsigned int n)
{
 dblDequeT *pdX,*pdY;
 graphType *psg;
 AnaTypeT *pat;

  #ifdef _DEBUG
  qDebug("[Control::UpdateAnlysisPlot] start %d.",n);
 #endif
 pat=pAna->get(n);
 if (pat==NULL)
 	return;
 if (pat->nGraph>=vsgAnalysis.size())
 	return;
 if (!pAnaMutex->tryLock())
 	return;
 psg=&vsgAnalysis[pat->nGraph];
 switch(psg->idX())
  {
   case IDLYAPUNOV:
   default: return;
   case IDRECURRENCE:
   {
    pdX=&pat->ddXResult;
    pdY=&pat->ddYResult;
    psg->setData(pdX,pdY);
    psg->addData(pdY,pdX);
    break;
   }
   case IDMAXLYAP:
   {
    pdX=&pat->ddXResult;
    pdY=&pat->ddYResult;
    psg->setData(pdX,pdY);
    break;
   }
   case IDPERIOD:
   {
    pdX=&pat->ddXResult;
    pdY=&pat->ddYResult;
    psg->setData(pdX,pdY);
    break;
   }
   case IDPOWER:
   {
    pdX=&pat->ddXResult;
    pdY=&pat->ddYResult;
    psg->setData(pdX,pdY);
    break;
   }
   case IDPOINCARE1D:
   case IDPOINCARE2D:
   case IDSPIKE:
   {
    pdX=&pat->ddXResult;
    pdY=&pat->ddYResult;
    psg->setData(pdX,pdY);
    break;
   }
  }
  pAnaMutex->unlock();
  #ifdef _DEBUG
  qDebug("[Control::UpdateAnlysisPlot] done.");
 #endif
 psg->replot();
}
//-------------------------------------------------------------------------------------------------------------
void Control::UpdateLyapunovPlots()
{
 dblDequeT *pdX,*pdY;
 graphType *psg;
 unsigned int i;
 
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if (psg->idX()!=IDLYAPUNOV)
  	continue;
   if (psg->idY()>=IntegratorObject->getLyapSize())
 	continue;
  pdX=IntegratorObject->getLyapData(0);
  pdY=IntegratorObject->getLyapData(psg->idY());
  psg->appendData(pdX,pdY);
  psg->replot();
#ifdef _DEBUG
	qDebug("[Control::UpdateLyapunovPlot] Pausing mutex");
#endif
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::UpdateAnalysis()
{ //update the variables in the listview
 unsigned int idx;
 double d;
 QListViewItem *pItem;
 QString qs;
 
 if (!pStatMutex->tryLock())
  return;
 if (IntegratorObject->hasLyapunov())
 {
  pItem=pListView->findItem(achLyap,0);
  if (pItem==NULL)
 	return;
  pItem=pItem->firstChild();
  while(pItem!=NULL)
  {
   if (pItem->rtti()==ID_RTTICOP)
   {
    CalcOptionListViewItem *p=((CalcOptionListViewItem *)pItem);
    idx=p->nId;
    if (idx==ID_LYAPPARDIM)
    {
     d=IntegratorObject->GetLyapDim();
     qs.setNum(d);
     p->setText(COLUMN_VAL,qs);
    }
    if (idx<ID_LYAPPARNUM)
    {
     d=IntegratorObject->getLyapAvg(idx);
     qs.setNum(d);
     p->setText(COLUMN_VAL,qs);
    }
   }
   pItem=pItem->nextSibling();
  }
 }
 pStatMutex->unlock();
}
 //-------------------------------------------------------------------------------------------------------------
void Control::UpdatePlot(QwtPlot *p)
{
 unsigned int i;
 graphType *psg;
 dblDequeT *pdX,*pdY;
 bool b=false;
 
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if (psg->isPlot(p))
  	{
  	 pAna->clearFlag(i);
	 b=true;
	 break;
	}
 }
 if (b)
 {
  startAnalysis();
  return;
 }
 
 if (!pIntMutex->tryLock())
 {
#ifdef _DEBUG
  qDebug("[Control::UpdatePlot] Failed lock int mutex");
#endif
  return;
 }
 //locked
 bLocked=true;

 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if (!(psg->isPlot(p))||(!psg->isValid(vvdVars.size())))
 	continue;
  pdX=vvdVars[psg->idX()].pData;
  pdY=vvdVars[psg->idY()].pData;
  psg->appendData(pdX,pdY);
  psg->replot();
 }
 pIntMutex->unlock();
 bLocked=false;
}
//-------------------------------------------------------------------------------------------------------------
void Control::UpdateTimers()
{
 unsigned int lnow,ltotal,lsofar;
 double df;
 QTime time(0,0,0,0);
  lnow=qtATimer.elapsed();
 
 if (pLcdElapTime)
 {//set the elapsed time
  pLcdElapTime->display(time.addMSecs(lnow).toString(achTimerFormat));  
 }
 if (pLcdEstTime)
 {
  ltotal=IntegratorObject->GetMaxEstimate();
  lsofar=IntegratorObject->GetSteps();
  if (lsofar>0)
   df=(double)ltotal/(double)lsofar;
  else
   df=0.0;
  df=(double)lnow*df;
  lnow=(unsigned int)ceil(df);
  pLcdEstTime->display(time.addMSecs(lnow).toString(achTimerFormat));  
 }
}
//---------------------------------------------------------------------------
void Control::startStats()
{
 if (bStatError||bStatStopped)
 	return;
 if (!pStats->running())
 {
  if (!pStats->IsOk())
  { //some error occurred
   bStatError=true;
   Error(pStats->GetErrorStr());
  }
  else
  	pStats->Start();
 }
}
//---------------------------------------------------------------------------
void Control::startAnalysis()
{
 if (bAnaError||bAnaStopped)
 	return;
 if (!pAna->running())
 {
  if (!pAna->IsOk())
  { //some error occurred
   bAnaError=true;
   Error(pAna->GetErrorStr());
  }
  else
  	pAna->Start();
 }
}
//---------------------------------------------------------------------------
void Control::stopAnalysis(QwtPlot *p)
{
 unsigned int i;
 graphType *psg;
 
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if (psg->isPlot(p))
  	 pAna->setFlag(i);
 }
}
//---------------------------------------------------------------------------
void Control::clearPlots()
{
 graphType *psg;
 struct3DGraph *psg3d;
 unsigned int i;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  psg->clearData();
  psg->replot();
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  psg->clearData();
  psg->replot();
 }
 for (i=0;i<vsg3DSeries.size();i++)
 {
  psg3d=&vsg3DSeries[i];
  if (!psg3d->pPlot)
 	continue;
  psg3d->pPlot->clear();
 }
}
//---------------------------------------------------------------------------
bool Control::lockMutex()
{
 if (bLocked)
	return true;
 bLocked=pIntMutex->tryLock();
 if (!bLocked)
 {//could not lock at this point,try for INT_LOCK_TRY times for INT_LOCK_MS (=n*m s) to lock it
   int i=0;
#ifdef _DEBUG
  qDebug("[Control::lockMutex] Failed lock int mutex, will try again..");
#endif
  timespec req;
  req.tv_sec=0;
  req.tv_nsec=INT_LOCK_MS;
  timespec rem;
   while((i<INT_LOCK_TRY)&&(!bLocked))
   {
    //wait for 50ms
    //msleep(INT_LOCK_MS);
    nanosleep(&req,&rem);
    bLocked=pIntMutex->tryLock();
    i++;
   }
  }
#ifdef _DEBUG
  qDebug("[Control::lockMutex] mutex lock=%d",bLocked);
#endif
  return bLocked;
}
//---------------------------------------------------------------------------
void Control::unlockMutex()
{
#ifndef SPINNER
  if (bLocked)
 	pIntMutex->unlock();
  bLocked=false;
  lockedMutex(false);
#ifdef _DEBUG
  qDebug("[Control::unlockMutex] mutex lock=%d",bLocked);
#endif
#endif
}
//-------------------------------------------------------------------------------------------------------------
void Control::setPixmap(QListViewItem *pItem,const QColor &c)
{
 QPainter *pPaint;
 QPixmap *pPix;
 
  pPix=new QPixmap( 18, 16);
  pPix->fill(pListView->viewport(),0,0);  // fill with widget background
  pPaint=new QPainter(pPix);
  pPaint->setBackgroundMode(Qt::TransparentMode);
  //create masked image
  pPaint->setPen(black);
  pPaint->setBrush(black);
  pPaint->drawEllipse(1, 0, 16, 16);
  pPix->setMask( pPix->createHeuristicMask() );
  //create color selector
  pPaint->setPen(black);
  pPaint->setBrush(c);
  pPaint->drawEllipse(1, 0, 16, 16);
  pItem->setPixmap(COLUMN_COL,*pPix);//QPixmap::fromMimeSource( "var.png" ) );
  pPaint->end();
  delete pPaint;
  delete pPix;
 }
//-------------------------------------------------------------------------------------------------------------
void Control::changeVarName(QString &qsNew,unsigned int n)
{
 QString qsOld;
 unsigned int id;
 
 id=vvdVars[n].nVarId; 
 qsOld=EquationObject->GetVarName(id);
 if (EquationObject->SetVarName(n,qsNew.ascii()))
 {
 // TODO: add code to change the name of a variable...
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::changeValue(QString qsEdit, unsigned int n)
{
 double d;
 unsigned int id;
 
 id=vvdVars[n].nVarId;
 d=EquationObject->GetVarValue(id);
 if (EquationObject->NumericalTermValue(const_cast<char *>(qsEdit.ascii()),&d))
 	EquationObject->SetVarValue(id,d);
 if (pListItem)
 	{
  	 QString qs;
	 qs.setNum(d);
 	 pListItem->setText(COLUMN_VAL,qs);
	}
if (log.doLog())
  	{
  	 log << time << "Changed current value" << endl;
	 log << tab << "{ " << GetVarName(id) <<"=" << d << " }"  << endl;
	}
}
//-------------------------------------------------------------------------------------------------------------
void Control::changeParValue(QString qsEdit, unsigned int n)
{
 double d;
 
 d=EquationObject->GetParmValue(n);
 if (EquationObject->NumericalTermValue(const_cast<char *>(qsEdit.ascii()),&d))
 	EquationObject->SetParmValue(n,d);
 if (pListItem)
 	{
  	 QString qs;
	 qs.setNum(d);
 	 pListItem->setText(COLUMN_VAL,qs);
	}
 if (log.doLog())
  	{
  	 log << time << "Changed parameter value" << endl;
	 log << tab << "{ " << EquationObject->GetParmName(n) <<"=" << d << " }"  << endl;
	}
}
//-------------------------------------------------------------------------------------------------------------
void Control::changeIniValue(QString qsEdit, unsigned int n)
{
 double d;
 unsigned int id;
 
 id=vvdVars[n].nVarId;
 d=EquationObject->GetVarIniValue(id);
 if (EquationObject->NumericalTermValue(const_cast<char *>(qsEdit.ascii()),&d))
 	EquationObject->SetVarIniValue(id,d);
 if (pListItem)
 	{
  	 QString qs;
	 qs.setNum(d);
 	 pListItem->setText(COLUMN_INI,qs);
	}
 if (log.doLog())
  	{
  	 log << time << "Changed initial value" << endl;
	 log << tab << "{ " << GetVarName(id) <<"=" << d << " }"  << endl;
	}
}
//-------------------------------------------------------------------------------------------------------------
void Control::changeCopValue(QString &qs, unsigned int n)
{
 double d;
 unsigned int v;
 bool bok;
 
 switch(n)
 {
  case ID_LYAPPARSTEPS:
  	{
	 v=qs.toUInt(&bok);
	 if (!bok)
	 	return;
	 IntegratorObject->SetLyapSteps(v); 
	 break;
	}
  case ID_LYAPPARDIST:
  	{
	 d=qs.toDouble(&bok);
	 if (!bok)
	 	return;
	 IntegratorObject->SetLyapDistance(d); 
	 break;
	}
  case ID_LYAPPARNUM:
  	{
	 v=qs.toUInt(&bok);
	 if (!bok)
	 	return;
	 IntegratorObject->SetNumSystems(v); 
	 break;
	}
  default: return;
 }
 if (pListItem)
 	 pListItem->setText(COLUMN_VAL,qs);
}
//-------------------------------------------------------------------------------------------------------------
void Control::customEvent( QCustomEvent * e )
{
  if ( e->type() == QEvent::User+10 )
  {  // It must be an AnalysisCompleteEvent
     AnalysisCompleteEvent* ace = (AnalysisCompleteEvent*)e;
     unsigned int n=ace->getIdx();
#ifdef _DEBUG
 qDebug("[Control::customEvent] AnalysisComplete idx=%d",n);
#endif
     if (n!=IDINVALID) //end of thread
     	UpdateAnalysisPlot(n);
     else
     {  //All done, hide progress bars
       for (unsigned int i=0;i<vsgAnalysis.size();i++)
          emit doProgress(vsgAnalysis[i].getPlot(),0,0);
     }
     if (!pAna->IsOk())
  	{ //some error occurred
   	 bAnaError=true;
   	 Error(pAna->GetErrorStr());
  	}
   }
  if ( e->type() == QEvent::User+11 )
  {  // It must be an AnalysisProgressEvent
     AnalysisProgressEvent* ape = (AnalysisProgressEvent*)e;
     unsigned int n=ape->getIdx();
     unsigned int max=ape->getMax();
     unsigned int cur=ape->getCur();
// #ifdef _DEBUG
//  qDebug("[Control::customEvent] AnalysisProgress max=%d,cur=%d",max,cur);
// #endif
     if (n<vsgAnalysis.size())
     	emit doProgress(vsgAnalysis[n].getPlot(),max,cur);
   }
  if ( e->type() == QEvent::User+12 )
  {  // It must be an AnalysisProgressEvent for ISI
     AnalysisProgressEvent* ape = (AnalysisProgressEvent*)e;
     //unsigned int n=ape->getIdx();
     unsigned int max=ape->getMax();
     unsigned int cur=ape->getCur();
// #ifdef _DEBUG
//  qDebug("[Control::customEvent] AnalysisProgress max=%d,cur=%d",max,cur);
// #endif
      emit doIsiProgress(max,cur);
   }
}
//-------------------------------------------------------------------------------------------------------------
void Control::setupControl()
{
 unsigned int i,n;
 QListViewItem *pItem;
 VarListViewItem *pVItem;
 StatListViewItem *pSItem;
 CalcListViewItem *pCItem;
 QCheckListItem *pCLItem, *pCLItem2, *pCLItem3;
 CalcOptionListViewItem *pCOPitem;
 TVarData vd;
 double d;
 
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin.");
#endif

 // first create list of graph structs for each variable
 vvdVars.clear();
 // create list for error estimates
 vvdError.clear();
 vd.pen.setWidth(1);
 //setup the listview
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin setup listview.");
#endif
 n=EquationObject->GetN();
 pListView->clear();
 pListView->setItemMargin(2);
 //setup values
 pListView->setRootIsDecorated( TRUE );
 pItem=new QListViewItem(pListView,achVars);
 pItem->setExpandable( TRUE );
 //add evolution to listview
 GetVarType(IDINVALID,vd);
 vd.nIdx=vvdVars.size();
 d=IntegratorObject->GetBegin();
 pVItem=new VarListViewItem(pItem,vd,d);
 vd.pItem=pVItem;
 vvdVars.push_back(vd);
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin add vars.");
#endif
 // add variables
 for (i=0;i<n;i++)
 {
  GetVarType(i,vd);
  vd.nIdx=vvdVars.size();
  d=EquationObject->GetVarValue(i);
  pVItem=new VarListViewItem(pItem,vd,d);
  setPixmap(pVItem,vd.pen.color());
  vd.pItem=pVItem;
  vvdVars.push_back(vd);
 }
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin add pars.");
#endif
// add parameters
 pItem=new QListViewItem(pListView,achPars);
 pItem->setExpandable( TRUE );
 n=EquationObject->GetNPar();
 for (i=0;i<n;i++)
 	new ParListViewItem(pItem,EquationObject,i);
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin add consts.");
#endif
 //add constants
 pItem=new QListViewItem(pListView,achConsts);
 pItem->setExpandable( TRUE );
 n=EquationObject->GetNConsts();
 for (i=0;i<n;i++)
 	new ConstListViewItem(pItem,EquationObject,i);

 pItem=new QListViewItem(pListView,achRuntimeOptions);
 pItem->setExpandable( TRUE );

#ifdef _DEBUG
 qDebug("[Control::setupControl] begin add lyapunov.");
#endif
 //lyapunov
 pCItem=new CalcListViewItem(pItem,achLyap,ctiLyapunov,true);
 pCOPitem=new CalcOptionListViewItem(pCItem,achLyapStep,QVariant(IntegratorObject->GetLyapSteps()),ID_LYAPPARSTEPS,false);
 pCOPitem->setRange((unsigned int)IntegratorObject->GetBegin(),(unsigned int)IntegratorObject->GetEnd());
 pCOPitem=new CalcOptionListViewItem(pCItem,achLyapDist,QVariant(IntegratorObject->GetLyapDistance()),ID_LYAPPARDIST,false);
 pCOPitem->setRange(0.0,1.0);
 pCOPitem=new CalcOptionListViewItem(pCItem,achLyapNum,QVariant((unsigned int)1),ID_LYAPPARNUM,false);
 pCOPitem->setRange((unsigned int)1,(unsigned int)(10000000));
// pCOPitem->setRange((unsigned int)1,(unsigned int)(UINT_MAX-10));
 pCOPitem=new CalcOptionListViewItem(pCItem,achLyapDim,QVariant(0.0),ID_LYAPPARDIM,true);
 pCItem->setVisible(IntegratorObject->hasLyapunov());
 n=EquationObject->GetNumDiffMap();
 for (i=0;i<n;i++)
  {
   QString qs;
   qs.sprintf("Lambda %d (%s)",i+1,GetVarDiffName(i));
   pCOPitem=new CalcOptionListViewItem(pCItem,qs,QVariant(0.0),i,true);
  }
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin add stats.");
#endif
 
 //stats
 pCItem=new CalcListViewItem(pItem,achStats,ctiStats,false);
 pCLItem=new QCheckListItem(pCItem,achMean,QCheckListItem::CheckBox);
 pCLItem->setOn(pStats->mean());
 //connect( pCLItem, SIGNAL( /*state*/Change(bool) ), SLOT( toggleMean(bool) ) );
 pCLItem=new QCheckListItem(pCItem,achStddev,QCheckListItem::CheckBox);
 pCLItem->setOn(pStats->stddev());
 
 // Integration error estimates
 pCItem=new CalcListViewItem(pItem,achIntError,ctiErrors,false);
 pCLItem=new QCheckListItem(pCItem,achErrorMean,QCheckListItem::RadioButtonController);
 pCLItem2=new QCheckListItem(pCLItem,achRunAvg,QCheckListItem::RadioButton);
 pCLItem3=new QCheckListItem(pCLItem,achAllVal,QCheckListItem::RadioButton);
 if (pStats->error())
 	pCLItem2->setOn(true);
 else
 	pCLItem3->setOn(true);
 for (i=0;i<n;i++)
 {
  GetStatErrorType(i,vd,EquationObject->getKeepError());
  vd.nIdx=vvdError.size();
  pSItem=new StatListViewItem(pCItem,vd);
  setPixmap(pSItem,vd.pen.color());
  vd.pItem=pSItem;
  vvdError.push_back(vd);
 }

  //add calc methods
 pItem=new QListViewItem(pListView,achAnalysis);
 pItem->setExpandable( TRUE );
 
 //recurrence plot parameter
 pCItem=new CalcListViewItem(pItem,achRecur,ctiRecur,true);
 //maximal lyapunov parameter
 pCItem=new CalcListViewItem(pItem,achMaxLyap,ctiMaxLyap,true);
 //Periodogram
 pCItem=new CalcListViewItem(pItem,achPeriod,ctiPeriod,true);
 //Powerspectrum
 pCItem=new CalcListViewItem(pItem,achPower,ctiPower,true);
 //Poincare section
 pCItem=new CalcListViewItem(pItem,achPoincare,ctiPoincare1d,true);
 //Spike estimate
 pCItem=new CalcListViewItem(pItem,achSpike,ctiSpike,true);
 
 connect( pListView, SIGNAL( rightButtonClicked( QListViewItem *, const QPoint&, int ) ), SLOT( slotListSelect(QListViewItem *, const QPoint&, int ) ) );
 
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin create qeditline.");
#endif
//create edit line
 if (pLineEdit==NULL)
 	pLineEdit=new QLineEdit(pListView);
 pLineEdit->hide();
 connect( pLineEdit, SIGNAL( returnPressed() ), SLOT( slotChangeValue() ) );
 connect( pLineEdit, SIGNAL( lostFocus() ), SLOT( slotEditLostFocus() ) );
//TODO: add other slots e.g. pagedown or down arrow
#ifdef _DEBUG
 qDebug("[Control::setupControl] begin register.");
#endif 
  //register it with the integrator:
#ifdef SPINNER
 IntegratorObject->registerObjects(EquationObject,spinObject);
#else
 IntegratorObject->registerObjects(EquationObject,pIntMutex);
#endif
 EquationObject->Reset();
 //setup statistics
 pStats->RegisterData(vvdVars,vvdError);
 //setup analysis
 pAna->RegisterData(vvdVars);
 //setup progressbar
 if (pProgress)
  pProgress->setProgress(IntegratorObject->GetSteps(),IntegratorObject->GetMaxEstimate());
#ifdef _DEBUG
 qDebug("[Control::setupControl] done.");
#endif
}
//-------------------------------------------------------------------------------------------------------------
void Control::SetOptions(TEditVector &ve)
{
 unsigned int i,n;
 QString qs;
 QIntValidator *pv;
 
 n=ve.size();
 for (i=0;i<n;i++)
 {
  switch(i)
  {
   case 0:
   {
    if (ve[i]->isA("QLineEdit"))
    {
     qs.setNum(IntegratorObject->GetBegin());
//      pv=(QIntValidator *)ve[i]->validator();
//      if (pv)
//      	pv->setRange(0,0x7FFFFFFF);
     ve[i]->setText(qs);
    }
    break;
   }
   case 1:
   {
    if (ve[i]->isA("QLineEdit"))
    {
     qs.setNum(IntegratorObject->GetEnd());
//      pv=(QIntValidator *)ve[i]->validator();
//      if (pv)
//      	pv->setRange((int)IntegratorObject->GetBegin(),0x7FFFFFFF);
     ve[i]->setText(qs);
    }
    break;
   }
   case 2:
   {
    if (ve[i]->isA("QLineEdit"))
    {
     qs.setNum(IntegratorObject->GetStep());
//      QDoubleValidator *pdv=(QDoubleValidator *)ve[i]->validator();
//      if (pdv)
//      	pdv->setRange(1e-6,1.0,6);
     ve[i]->setText(qs);
    }
    break;
   }
   case 3:
   {
    if (ve[i]->isA("QLineEdit"))
    {
     qs.setNum(nUpdateInt);
     pv=(QIntValidator *)ve[i]->validator();
     if (pv)
     	pv->setRange(50,0x7FFFFFFF);
     ve[i]->setText(qs);
    }
    break;
   }
   default: break;
  }
 }
}
//-------------------------------------------------------------------------------------------------------------
bool Control::doLyap()
{
 return IntegratorObject->hasLyapunov();
}
//-------------------------------------------------------------------------------------------------------------
void Control::slotListSelect(QListViewItem *pl, const QPoint&, int col)
{
 int n,i,size;
 
 if (pl==NULL) return; //No object available...
 
 pListItem=NULL;
 QRect r(pListView->itemRect(pl));
 nCol=col;
 switch (col)
 {
  default:
  case COLUMN_COL:
  {
   if (pl->rtti()!=ID_RTTIVAR) return;
   QColor c;
   c.setNamedColor(pl->text(col));
   c=QColorDialog::getColor(c);
   if (c.isValid())
   {
    pl->setText(col,c.name());
    setPixmap(pl,c);
    ((VarListViewItem *)pl)->colour=c;
   }
   return;
  }
  case COLUMN_VAR:
  {
   if (pl->depth()==0) return; //don't edit root elements
   if (pl->text(COLUMN_VAR)==achRecur)
   	{
   	 doAnaSettings(ctiRecur);
	 return;
	}
   if (pl->text(COLUMN_VAR)==achMaxLyap)
   	{
   	 doAnaSettings(ctiMaxLyap);
	 return;
	}
   if (pl->text(COLUMN_VAR)==achPeriod)
   	{
   	 doAnaSettings(ctiPeriod);
	 return;
	}
   if (pl->text(COLUMN_VAR)==achPower)
   	{
   	 doAnaSettings(ctiPower);
	 return;
	}
   if (pl->text(COLUMN_VAR)==achPoincare)
   	{
   	 doAnaSettings(ctiPoincare1d);
	 return;
	}
   if (pl->text(COLUMN_VAR)==achPower)
   	{
   	 doAnaSettings(ctiPower);
	 return;
	}
   return;
  }
  case COLUMN_INI:
  {
      if (pl->rtti()!=ID_RTTIVAR) return;
      break;
  }
  case COLUMN_VAL: break;
  }
  if (pl->rtti()==ID_RTTICOP)
  	{//calculation parameters
	 CalcOptionListViewItem *pcitem=(CalcOptionListViewItem *)pl;
	 if (pcitem->bReadOnly)
	 	return;
#ifdef _DEBUG
qDebug("[slotlistselect] %s",pcitem->varValue.typeName());
#endif
  	 if ((pcitem->varValue.type()==QVariant::UInt)||(pcitem->varValue.type()==QVariant::Int))
	 {
	  QIntValidator *pv=new QIntValidator(pLineEdit);
	  pLineEdit->setValidator(pv);
	  pv->setRange(pcitem->nr1,pcitem->nr2);
	 }
  	 if (pcitem->varValue.type()==QVariant::Double)
	 {
	  QDoubleValidator *pv=new QDoubleValidator(pLineEdit);
	  pLineEdit->setValidator(pv);
	  pv->setRange(pcitem->dr1,pcitem->dr2);
	 }
	}
  size=r.left();
  r.moveTop(r.top()+pListView->viewport()->y());
  for (i=0;i<col;i++)
   {
    n=pListView->columnWidth(i);
    size+=n;
   }
  r.setLeft(size);
  r.setRight(size+pListView->columnWidth(col));
  pLineEdit->setGeometry(r);
  pLineEdit->setText(pl->text(col)); 
  pLineEdit->show();
  pLineEdit->setFocus();
  pListItem=pl;
}
//-------------------------------------------------------------------------------------------------------------
void Control::slotChangeValue()
{
 Q_ASSERT(pListItem!=NULL);
 
 if (nCol>=0)
 {
#ifdef _DEBUG
 qDebug("[Control::slotChangeValue]");
#endif
  switch (nCol)
   {
    default:
    case COLUMN_COL: break;
    case COLUMN_VAR:
  	{
	 break;
  	}
    case COLUMN_INI: 
    {
     if (pListItem->rtti()==ID_RTTIVAR) 
     	changeIniValue(pLineEdit->text(),((VarListViewItem *)pListItem)->nId);
     break;
    }
    case COLUMN_VAL:
    {
     switch(pListItem->rtti())
     {
      case ID_RTTIVAR:
      {
     	changeValue(pLineEdit->text(),((VarListViewItem *)pListItem)->nId);
	break;
      }
      case ID_RTTIPAR:
      {
     	changeParValue(pLineEdit->text(),((ParListViewItem *)pListItem)->nId);
	break;
      }
      case ID_RTTICONST:
      {
     	//changeParValue(pLineEdit->text(),((ParListViewItem *)pListItem)->nId);
	break;
      }
      case ID_RTTICOP:
      {
       QString qs=pLineEdit->text();
       if (pLineEdit->validator())
       	{
	 int n=0;
	 if (pLineEdit->validator()->validate(qs,n)!=QValidator::Acceptable)
	 	break;
	}
	changeCopValue(qs,((CalcOptionListViewItem *)pListItem)->nId);
       break;
      }
      default: break;
     }
     break;
    }
   }
  nCol=-1;
  pLineEdit->hide();
  pLineEdit->clearValidator();
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::slotEditLostFocus()
{
 if (nCol>=0)
 {
  slotChangeValue();
  pLineEdit->hide();
 #ifdef _DEBUG
 qDebug("[Control::slotEditLostFocus]");
#endif
 }
}
//-------------------------------------------------------------------------------------------------------------
void Control::slotSetIntegrator(int id)
{
 if (log.doLog())
  	{
  	 log << time << "Changed Integrator" << endl;
	 log << tab << "{ from: " << IntegratorObject->GetIntegratorName()  << " }"  << endl;
	 log << tab << "{ to: " << IntegratorObject->GetIntegratorName(id)  << " }"  << endl;
	}
 if (!IntegratorObject->SetIntegrator(id))
 	 Error("No such integrator available");
}
//-------------------------------------------------------------------------------------------------------------
int Control::doAnaSettings(TCalcTypeItem t, unsigned int n, QString qs)
{
 AnaSetDialog *pDlg;
 AnaSetT *pSet;
 
 switch(t)
 {
  case ctiSpike:
  	{
	 pSet=pAna->get1(aotSpike,n);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiPoincare1d:
  	{
	 pSet=pAna->get1(aotPoincare1d,n);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiPower:
  	{
	 pSet=pAna->get1(aotPower,n);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiRecur:
  	{
	 pSet=pAna->get1(aotRecur,n);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiMaxLyap:
  	{
	 pSet=pAna->get1(aotMaxLyap,n);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  default: return QDialog::Rejected;
 }
 pDlg=new AnaSetDialog(0,0,true);
 pDlg->setupDialog(pSet,qs);
 return pDlg->exec();
}
//-------------------------------------------------------------------------------------------------------------
int Control::doAnaSettings(TCalcTypeItem t)
{
 AnaSetDialog *pDlg;
 AnaSetT *pSet;
 
 switch(t)
 {
  case ctiPoincare1d:
  	{
	 pSet=pAna->get(aotPoincare1d);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiRecur:
  	{
	 pSet=pAna->get(aotRecur);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiPower:
  	{
	 pSet=pAna->get(aotPower);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  case ctiMaxLyap:
  	{
	 pSet=pAna->get(aotMaxLyap);
	 if (!pSet) return QDialog::Rejected;
	 break;
	}
  default: return QDialog::Rejected;
 }
 pDlg=new AnaSetDialog(0,0,true);
 pDlg->setupDialog(pSet);
 return pDlg->exec();
}
//-------------------------------------------------------------------------------------------------------------
void Control::Start()
{
 if (!bReady)
 {
  Error(pNoLoaded);
  return;
 }
 if (bRunning) //already started
 	return;
  // log the event
  if (log.doLog())
  	{
  	 log << time << "Started" << endl;
	 log << tab << "{ begin=" << IntegratorObject->Gett() << achComma << "end="<< IntegratorObject->GetEnd() << achComma << "step=" << IntegratorObject->GetStep() << " }" << endl;
	 log << flush;
	}
  //first setup a timer
 nTimerId=startTimer(nUpdateInt);
 bRunning=true;
 qtATimer.restart();
 if (pPixIntLabel)
	pPixIntLabel->setPixmap(pixThreadOn); 
  IntegratorObject->Start();
}

//-------------------------------------------------------------------------------------------------------------
void Control::Stop()
{
 if (!bReady)
{
  Error(pNoLoaded);
  return;
 }
 pStatusBar->message(achStopping);
 //kill the time
 killTimer(nTimerId);
 IntegratorObject->Stop();
 if (pPixIntLabel)
	pPixIntLabel->setPixmap(pixThreadOff);
  if (log.doLog())
  	{
  	 log << time << "Stopped" << endl;
	 log << tab << "{ current=" << IntegratorObject->Gett() << " }"  << endl;
	}
 //check if we need to update again..
 if (bRunning&&(!bUpdating))
 	Update();
 bRunning=false;
 pStatusBar->clear();
}
//-------------------------------------------------------------------------------------------------------------
void Control::Reset()
{
 if (!bReady)
{
  Error(pNoLoaded);
  return;
 }
 bError=false;
 bStatError=false;
 bStatStopped=false;
 bAnaStopped=false;
 if (bStats)
 	pStats->Reset();
 if (bAna)
 	pAna->Reset();
 IntegratorObject->Reset();
 qtATimer.setHMS(0,0,0,0);
 clearPlots();
 Update();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::StopStats()
{
 bStatStopped=true;
 pStats->Stop();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::StopAnalysis()
{
 bAnaStopped=true;
 pAna->Stop();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::toggleLyap(bool b)
{
 QListViewItem *pItem;
 
 IntegratorObject->SetLyapunov(b);
 pItem=pListView->findItem(achLyap,0);
 if (pItem!=NULL)
 	pItem->setVisible(b);
}
//-----------------------------------------------------------------------------------------------------------------
void Control::toggleMean(bool b)
{
 pStats->setMean(b);
 if (b&&bStats)
 {
  pListView->setColumnWidthMode(COLUMN_MEAN,QListView::Maximum);
  pListView->setColumnWidth(COLUMN_MEAN,30);
 }
 else
 {
   pListView->setColumnWidthMode(COLUMN_MEAN,QListView::Manual);
   pListView->setColumnWidth(COLUMN_MEAN,0);
 }
}
//-----------------------------------------------------------------------------------------------------------------
void Control::toggleStats(bool b)
{
 bStats=b;
 if (b)
 {//show stats columns
  if (pStats->mean())
  	{
  	 pListView->setColumnWidthMode(COLUMN_MEAN,QListView::Maximum);
  	 pListView->setColumnWidth(COLUMN_MEAN,30);
	}
  if (pStats->stddev())
  	{
  	 pListView->setColumnWidthMode(COLUMN_VARIANCE,QListView::Maximum);
  	 pListView->setColumnWidth(COLUMN_VARIANCE,30);
	}
 }
 else
 {
   pListView->setColumnWidthMode(COLUMN_MEAN,QListView::Manual);
   pListView->setColumnWidth(COLUMN_MEAN,0);
   pListView->setColumnWidthMode(COLUMN_VARIANCE,QListView::Manual);
   pListView->setColumnWidth(COLUMN_VARIANCE,0);
 }
 pListView->triggerUpdate();
}
//-----------------------------------------------------------------------------------------------------------------
double Control::getStep()
{
  return IntegratorObject->GetStep();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::setStep(double d)
{
 IntegratorObject->SetStep(d);
 pSocket->setStep(d);
 if (pLcd)
 	pLcd->display(IntegratorObject->Gett());
 if (pProgress)
 	pProgress->setProgress(IntegratorObject->GetSteps(),IntegratorObject->GetMaxEstimate());
 if (log.doLog())
  	{
  	 log << time << "Changed step" << endl;
	 log << tab << "{ step=" << IntegratorObject->GetStep() << " }" << endl;
	}
}
//-----------------------------------------------------------------------------------------------------------------
int Control::getBegin()
{
  return (int)IntegratorObject->GetBegin();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::setBegin(int n)
{
 IntegratorObject->SetBegin((double)n);
 if (pLcd)
 	pLcd->display(IntegratorObject->Gett());
 if (pProgress)
 	pProgress->setProgress(IntegratorObject->GetSteps(),IntegratorObject->GetMaxEstimate());
 UpdateTimers();
 if (log.doLog())
  	{
  	 log << time << "Changed begin" << endl;
	 log << tab << "{ begin=" << IntegratorObject->GetBegin() << " }" << endl;
	}
}
//-----------------------------------------------------------------------------------------------------------------
int Control::getEnd()
{
  return (int)IntegratorObject->GetEnd();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::setEnd(int n)
{
 IntegratorObject->SetEnd((double)n);
 if (pProgress)
 	pProgress->setProgress(IntegratorObject->GetSteps(),IntegratorObject->GetMaxEstimate());
 UpdateTimers();
 if (log.doLog())
  	{
  	 log << time << "Changed end" << endl;
	 log << tab << "{ end=" << IntegratorObject->GetEnd() << " }" << endl;
	}
}
//-----------------------------------------------------------------------------------------------------------------
int Control::getInterval()
{
  return nUpdateInt;
}
//-----------------------------------------------------------------------------------------------------------------
void Control::setInterval(int n)
{
 if (n>=50)
 	nUpdateInt=n;
}
//-----------------------------------------------------------------------------------------------------------------
const char *Control::getName()
{
 return EquationObject->GetName();
}
/*//-----------------------------------------------------------------------------------------------------------------
const char *Control::getFileName()
{
 return EquationObject->GetFileName();
}*/
//-----------------------------------------------------------------------------------------------------------------
const char *Control::GetVarName(unsigned int n)
{
 return EquationObject->GetVarName(n);
}
//-----------------------------------------------------------------------------------------------------------------
const char *Control::GetVarTimeName(unsigned int n)
{
 if (n==0)
  return achTime;
 return EquationObject->GetVarName(n-1);
}
//-----------------------------------------------------------------------------------------------------------------
const char *Control::GetVarDiffName(unsigned int n)
{
 unsigned int i,v;
 
 v=0;
 for (i=0;i<EquationObject->GetN();i++)
 {
  if (EquationObject->IsDiffMap(i))
  {
   if (v==n)
   	return EquationObject->GetVarName(i);
   v++;
  }
 }
 return NULL;
}
//-----------------------------------------------------------------------------------------------------------------
unsigned int Control::GetVarIdx(QString &s)
{//returns the index within the vvdvars array
 unsigned int i,n;
 
 n=IDINVALID;
 for (i=0;i<vvdVars.size();i++)
 {
  if (vvdVars[i].Title==s)
  {
   n=vvdVars[i].nIdx;
   break;
  }
 }
 return n;
}
//-----------------------------------------------------------------------------------------------------------------
bool Control::GetVarType(unsigned int n, TVarData &vd)
{ //fill the struct with info
 dblDequeT *pd;
 
 if (n==IDINVALID) //evolution
 {
  pd=&EquationObject->vdvData[0];
  vd.Title=achTime;
 }
 else
 {
  if (n>=EquationObject->GetN())
   return false;
  pd=&EquationObject->vdvData[n+1];
  vd.pen.setColor(getRgb(n));
  vd.Title=EquationObject->GetVarName(n);
 }
 vd.nVarId=n;
 vd.nIdx=IDINVALID;
 vd.pData=pd;
 return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool Control::GetStatErrorType(unsigned int n, TVarData &vd, bool bkeep)
{ //fill the struct with info
 dblDequeT *pd;
 
 if ((n==IDINVALID)|| (n>=EquationObject->GetN()))
   return false;
  //use nvarid to flag if error data is being stored
  vd.nVarId=(int)bkeep;
  if (bkeep)
  	pd=&EquationObject->vdvDataError[n];
  else
  	pd=&EquationObject->dvDataError;
  vd.Title=EquationObject->GetVarName(n);
  //scale colour beyond the number of vars
  n+=EquationObject->GetN();
  vd.pen.setColor(getRgb(n));
  vd.nIdx=IDINVALID;
  vd.pData=pd;
  return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool Control::GetVarStruct(unsigned int n, graphType &sg)
{ //copy the struct from list
 if (n>=vvdVars.size())
   return false;
  
 sg.setTitle(vvdVars[n].Title);
 sg.setPen(vvdVars[n].pen);
  //the var id of structgraph is an index into the var array whose var id is an index into equation array
 sg.setIdY(vvdVars[n].nIdx);
 sg.setBounds(0,vvdVars[n].pData->size());
 return true;
}
//-----------------------------------------------------------------------------------------------------------------
bool Control::GetVarStruct3D(unsigned int n, struct3DGraph &sg,int axis)
{ //copy the struct from list
 if (n>=vvdVars.size())
   return false;
   
 QString qs;  
  //the var id of structgraph is an index into the var array whose var id is an index into equation array
 switch(axis)
 { 
  default: break;
  case 0: sg.nXVarId=vvdVars[n].nIdx; break;
  case 1: sg.nYVarId=vvdVars[n].nIdx; break;
  case 2: sg.nZVarId=vvdVars[n].nIdx; break;
 }
 sg.color=vvdVars[n].pen.color();
 //create title string
 qs.sprintf("%s,%s,%s",GetVarTimeName(sg.nXVarId),GetVarTimeName(sg.nYVarId),GetVarTimeName(sg.nZVarId));
 sg.iStart=0;
 sg.iStop=0;
 sg.bEnabled=true;
 sg.Title=qs;
 return true;
}
//-----------------------------------------------------------------------------------------------------------------
void Control::add3DSeries(struct3DGraph &sg)
{ //copy the struct from list
 if (sg.pPlot==NULL)
 	return; 
 if ((sg.nXVarId>=vvdVars.size())||(sg.nYVarId>=vvdVars.size())||(sg.nZVarId>=vvdVars.size()))
 	return;

 dblDequeT *pdX,*pdY, *pdZ;
 
 pdX=vvdVars[sg.nXVarId].pData;
 pdY=vvdVars[sg.nYVarId].pData;
 pdZ=vvdVars[sg.nZVarId].pData;
 sg.pPlot->addPoints(sg.uCurve,*pdX,*pdY,*pdZ);
 sg.nSizeX=pdX->size();
 sg.nSizeY=pdY->size();
 sg.nSizeZ=pdZ->size();
 vsg3DSeries.push_back(sg);
}
//-----------------------------------------------------------------------------------------------------------------
void Control::doRecaption(QwtPlot *p, QString qsx, bool bSpecial)
{
 unsigned int i;
 graphType *psg;
 QStringList qsl;
 
 qsl.append(qsx);
 for (i=0;i<vsgSeries.size();i++)
 {
   psg=&vsgSeries[i];
   if (!psg->isPlot(p))
  	continue;
   qsl.append(psg->getTitle());
 }
 if (!bSpecial)
 {
  if (qsl.size()<2)
   qsl.append("?");
  qsx=qsl.join(",");
#ifdef _DEBUG
  qDebug("[Control::doRecaption] done");
#endif
 }
 emit reCaption(p,qsx);
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addSeries(graphType &sg)
{ 
  if (!sg.isValid(vvdVars.size()))
 	return;

 dblDequeT *pdX,*pdY;
 
 pdX=vvdVars[sg.idX()].pData;
 pdY=vvdVars[sg.idY()].pData;
 sg.setData(pdX,pdY);
 vsgSeries.push_back(sg);
 sg.setStyles();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::UpdateXVars(graphType &sg)
{
 unsigned int i;
 graphType *psg;
 dblDequeT *pdX,*pdY;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if (!psg->isPlot(sg.getPlot()))
  	continue;
  psg->setIdX(sg.idY());
  pdX=vvdVars[psg->idX()].pData;
  pdY=vvdVars[psg->idY()].pData;
  psg->setData(pdX,pdY);
 }
}
//-----------------------------------------------------------------------------------------------------------------
const char *Control::getSeriesTitle(QwtPlot *p,long key)
{
 unsigned int i;
 graphType *psg;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
   return psg->getTitle().ascii();
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
   return psg->getTitle().ascii();
 }
 return achUnknown;
}
//---------------------------------------------------------------------------
void Control::killSeries(QwtPlot *p)
{
 graphType *psg;
 unsigned int i;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if (psg->isPlot(p))
  {
   psg->deleteCurve();
   TSeriesVector::iterator it(&vsgSeries[i]);
   vsgSeries.erase(it);
   i--; //reset search
  }
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if (psg->isPlot(p))
  {
   psg->deleteCurve();
   TSeriesVector::iterator it(&vsgAnalysis[i]);
   pAna->remove(i);
   vsgAnalysis.erase(it);
   i--; //reset search
  }
 }
}
//---------------------------------------------------------------------------
void Control::killSeries(TGlGraph *p)
{
 struct3DGraph *psg;
 unsigned int i;
 
 for (i=0;i<vsg3DSeries.size();i++)
 {
  psg=&vsg3DSeries[i];
  if (psg->isPlot(p))
  {
   psg->pPlot->clear();
   T3DSeriesVector::iterator it(&vsg3DSeries[i]);
   vsg3DSeries.erase(it);
   i--; //reset search
  }
 }
}
//---------------------------------------------------------------------------
void Control::removeSeries(QwtPlot *p,long key)
{
 graphType *psg;
 unsigned int i;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
  {
   psg->deleteCurve();
   TSeriesVector::iterator it(&vsgSeries[i]);
   vsgSeries.erase(it);
   p->replot();
   return;
  }
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
  {
   psg->deleteCurve();
   TSeriesVector::iterator it(&vsgAnalysis[i]);
   vsgAnalysis.erase(it);
   p->replot();
   return;
  }
 }
}
//---------------------------------------------------------------------------
void Control::editSeries(QwtPlot *p,long key)
{
 graphType *psg;
 unsigned int i;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
  {
   EditSeriesForm *p=new EditSeriesForm(0,0,true);
   p->setupForm(*psg);
   p->show();
   return;
  }
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if ((psg->isPlot(p))&&(psg->isCurve(key)))
  {
   EditSeriesForm *p=new EditSeriesForm(0,0,true);
   p->setupForm(*psg);
   p->show();
   return;
  }
 }
}
//---------------------------------------------------------------------------
void Control::editSeries(graphType &sg)
{
 graphType *psg;
 unsigned int i;
 
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if ((psg->isPlot(sg.getPlot()))&&(psg->isCurve(sg.getCurve())))
  {
   sg.setStyles();
   if ((psg->getStart()!=sg.getStart())||(psg->getStop()!=sg.getStop()))
   	doDataSubset(sg);
   vsgSeries[i]=sg;
   sg.replot();
   return;
  }
 }
 for (i=0;i<vsgAnalysis.size();i++)
 {
  psg=&vsgAnalysis[i];
  if ((psg->isPlot(sg.getPlot()))&&(psg->isCurve(sg.getCurve())))
  {
   sg.setStyles();
   if ((psg->getStart()!=sg.getStart())||(psg->getStop()!=sg.getStop()))
   	doDataSubset(sg);
   vsgAnalysis[i]=sg;
   sg.replot();
   return;
  }
 }
}
//-----------------------------------------------------------------------------------------------------------------
void Control::doDataSubset(graphType &sg)
{
 unsigned int start,stop;
 //first check if a subset exists:
 start=sg.getStart();
 stop=sg.getStop();

 if (sg.isSubset(start,stop))
 {
  dblDequeT *pdX,*pdY; 
  switch(sg.idX())
  {
   default:
   {
    pdX=vvdVars[sg.idX()].pData;
    pdY=vvdVars[sg.idY()].pData;
    break;
   }
   case IDLYAPUNOV:
   {
    pdX=IntegratorObject->getLyapData(0);
    pdY=IntegratorObject->getLyapData(sg.idY());
    break;
   }
  }
  if (stop>pdX->size())
  	stop=pdX->size();
  if (stop>pdY->size())
	stop=pdY->size();
  if (sg.subsetData(pdX,pdY,start,stop))
  	sg.replot();
 }
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addLyapunov(graphType &sg)
{
 unsigned int i,n;
 
 QString qs;
 dblDequeT *pdX,*pdY;
 
 if (sg.getPlot()==NULL)
 	return;
 n=IntegratorObject->getLyapSize(); //number of exponents, including time itself
 for (i=1;i<n;i++)
 {
  pdX=IntegratorObject->getLyapData(0);
  pdY=IntegratorObject->getLyapData(i);
  qs.sprintf("Lambda %d (%s)",i,GetVarDiffName(i-1));
  sg.setTitle(qs);
  sg.setIdY(i);
  sg.setIdX(IDLYAPUNOV);
  QColor c=getRgb(EquationObject->GetN()+i);
  QPen p(c);
  sg.setPen(p);
  sg.newCurve(IDLYAPUNOV,i);
  vsgAnalysis.push_back(sg);
  sg.setData(pdX,pdY);
  sg.setStyles();
 }
  sg.replot();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addPoincare(QwtPlot *p)
{ 
 bool bAccept,bAll;
 graphType sg;
 graphType *psg;
 QString qs;
 unsigned int i,id,idx;
 double d;
 
 if (p==NULL)
 	return;
 bAccept=false;
 bAll=false;
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if (psg->isPlot(p))
  {
   id=psg->idY();
   sg.setIdX(IDPOINCARE1D);
   sg.setIdY(id);
   sg.setPlot(p);
   idx=pAna->addAnalysis(aotPoincare1d,id,vsgAnalysis.size());
   bAccept=(doAnaSettings(ctiPoincare1d,idx,GetVarTimeName(id))==QDialog::Accepted);
   bAll|=bAccept;
   if (bAccept)
   {
    d=pAna->getValue(idx,achCut);
    qs=QString(achPoinAt1d).arg(GetVarTimeName(id)).arg(d);
    sg.setTitle(qs);
    sg.newCurve(IDPOINCARE1D,id);
    QColor c=getRgb(id);
    QPen p(c);
    sg.setPen(p);
    sg.setType(stSymbols);
    sg.setStyles();
    vsgAnalysis.push_back(sg);
   }
   else
   {
    pAna->remove(idx);
   }
  }
 }
 if (bAll)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addSpike(QwtPlot *p)
{ 
 bool bAccept,bAll;
 graphType sg;
 graphType *psg;
 QString qs;
 unsigned int i,id,idx;
 double d;
 
 if (p==NULL)
 	return;
 bAccept=false;
 bAll=false;
 for (i=0;i<vsgSeries.size();i++)
 {
  psg=&vsgSeries[i];
  if (psg->isPlot(p))
  {
   id=psg->idY();
   sg.setIdX(IDSPIKE);
   sg.setIdY(id);
   sg.setPlot(p);
   idx=pAna->addAnalysis(aotSpike,id,vsgAnalysis.size());
   bAccept=(doAnaSettings(ctiSpike,idx,GetVarTimeName(id))==QDialog::Accepted);
   bAll|=bAccept;
   if (bAccept)
   {
    d=pAna->getValue(idx,achThres);
    qs=QString(achSpikeEst).arg(GetVarTimeName(id)).arg(d);
    sg.setTitle(qs);
    sg.newCurve(IDSPIKE,id);
    QColor c=getRgb(id);
    QPen p(c);
    sg.setPen(p);
    sg.setType(stSymbols);
    sg.setStyles();
    vsgAnalysis.push_back(sg);
   }
   else
   {
    pAna->remove(idx);
   }
  }
 }
 if (bAll)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addRecur(graphType &sg)
{
 if (sg.getPlot()==NULL)
 	return;
	
 sg.setIdX(IDRECURRENCE);
 pAna->addAnalysis(aotRecur,sg.idY(),vsgAnalysis.size());
 sg.newCurve(IDRECURRENCE,sg.idY());
 vsgAnalysis.push_back(sg);
 QPen p(Qt::NoPen);
 sg.setPen(p);
 sg.setStyles();
 if (doAnaSettings(ctiRecur)==QDialog::Accepted)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addMaxLyap(graphType &sg)
{
 if (sg.getPlot()==NULL)
 	return;
	
 sg.setIdX(IDMAXLYAP);
 pAna->addAnalysis(aotMaxLyap,sg.idY(),vsgAnalysis.size());
 sg.newCurve(IDMAXLYAP,sg.idY());
 vsgAnalysis.push_back(sg);
 //QPen p(Qt::NoPen);
 QColor c=getRgb(sg.idY());
 QPen p(c);
 sg.setPen(p);
 sg.setStyles();
 if (doAnaSettings(ctiMaxLyap)==QDialog::Accepted)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addPeriod(graphType &sg)
{
 if (sg.getPlot()==NULL)
 	return;
	
 sg.setIdX(IDPERIOD);
 pAna->addAnalysis(aotPeriod,sg.idY(),vsgAnalysis.size());
 sg.newCurve(IDPERIOD,sg.idY());
 vsgAnalysis.push_back(sg);
 QColor c=getRgb(sg.idY());
 QPen p(c);
 sg.setPen(p);
 sg.setStyles();
 //sg.getPlot()->setCurveStyle(sg.getCurve(),QwtCurve::Sticks);
// if (doAnaSettings(ctiRecur)==QDialog::Accepted)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::addPower(graphType &sg)
{
 if (sg.getPlot()==NULL)
 	return;
	
 sg.setIdX(IDPOWER);
 pAna->addAnalysis(aotPower,sg.idY(),vsgAnalysis.size());
 sg.newCurve(IDPOWER,sg.idY());
 vsgAnalysis.push_back(sg);
 QColor c=getRgb(sg.idY());
 QPen p(c);
 sg.setPen(p);
 sg.setStyles();
 //sg.getPlot()->setCurveStyle(sg.getCurve(),QwtCurve::Sticks);
// if (doAnaSettings(ctiRecur)==QDialog::Accepted)
 	startAnalysis();
}
//-----------------------------------------------------------------------------------------------------------------
bool Control::isInDataStore(unsigned int nId)
{
 //find if an entry already exists
 for (unsigned int i=0;i<vdtStoreData.size();i++)
 {
  if (vdtStoreData[i]==nId)
  	return true;
 }
 return false;
}
//-----------------------------------------------------------------------------------------------------------------
void Control::AddDataStore(QListBox *pList,unsigned int nId)
{
 if (pList==NULL)
 	return; 
 if (nId>=vvdVars.size())
 {
   if ((nId==IDLYAPUNOV)&&(!isInDataStore(nId)))
   {
    vdtStoreData.push_back(nId);
    pList->insertItem(achLyap);
   }
  return;
 }
 if (!isInDataStore(nId))
 {
  vdtStoreData.push_back(nId);
  pList->insertItem(vvdVars[nId].Title);
 }
}
//-----------------------------------------------------------------------------------------------------------------
void Control::AddAllDataStore(QListBox *pList)
{
 if (pList==NULL)
	return; 
 vdtStoreData.clear();
 unsigned int i;
 for (i=0;i<vvdVars.size();i++)
	{
	 vdtStoreData.push_back(i);
	 pList->insertItem(vvdVars[i].Title);
	}
}
//-----------------------------------------------------------------------------------------------------------------
void Control::clearDataStore()
{
 vdtStoreData.clear();
}
//-----------------------------------------------------------------------------------------------------------------
void Control::saveDataStore(QString &qs, TSaveTypes type, int compress)
{
 unsigned int i,j,id,size;
 char c;
 
 if (vdtStoreData.size()==0)
 	return;
 switch(type)
 	{
		default:
		case stCSV: c=','; break;
		case stTSV: c='\t'; break;
		case stMat: 
		{
#ifdef MAT_FILE
	 	saveMatFile(qs);
#endif
	 	 return;
		}
		case stCDF:
		{
#ifdef CDF_FILE
	 	saveCDFFile(qs);
#endif
	 	 return;
		}
		case stHDF:
		{
#ifdef HDF_FILE
	 	saveHDFFile(qs,compress);
#endif
	 	 return;
		}
		case stHDF5:
		{
#ifdef HDF5_FILE
	 	saveHDF5File(qs,compress);
#endif
	 	 return;
		}
	}

 size=vvdVars[0].pData->size();
 QProgressDialog progressDlg( "Writing values...", "Abort", size, NULL, "progress", TRUE );
 QFile file(qs);
 if (file.open(IO_WriteOnly))
 {
  QTextStream stream(&file);
  //set precision
  stream.precision(12);
  //write header
  stream << achHeader << endl;
  stream << achIndex << c;
  for (i=0;i<vdtStoreData.size();i++)
  {//TODO: implement Lyapunov export
    if (i>0)
     stream << c;
   id=vdtStoreData[i];
   if (id>=vvdVars.size())
   	continue;
   stream << vvdVars[id].Title;
  }
  stream << endl;
  //write data sizes
  stream << size << c;
  for (i=0;i<vdtStoreData.size();i++)
  {
    if (i>0)
     stream << c;
   id=vdtStoreData[i];
   if (id>=vvdVars.size())
   	continue;
   stream << vvdVars[id].pData->size();
  }
  stream << endl << endl;
  file.flush();
  //here comes the data
  //one line at the time
  for (j=0;j<size;j++)
  {
   stream << j << c;
   for (i=0;i<vdtStoreData.size();i++)
   {
    if (i>0)
     stream << c;
    id=vdtStoreData[i];
    if (id>=vvdVars.size())
   	continue;
    stream << vvdVars[id].pData->at(j);
   }
   stream << endl;
   if (progressDlg.wasCancelled())
        break;
   if (j%100)
   {
    progressDlg.setProgress(j);
    qApp->processEvents();
   }
  }
  progressDlg.setProgress(size);
  file.close();
 }
}

#ifdef MAT_FILE
#ifndef MATLIB
void pad64bit(QFile &file,QDataStream &data)
{
  //add 64bit padding:
  QIODevice::Offset of;
  of=file.at()%8;
  if (of)
 	 {
   	 for (QIODevice::Offset c=of;c<8;c++)
   		data << 0;
  	 }
}

 void Control::saveMatFile(QString &qs)
 {
  QFile file(qs);
  QIODevice::Offset of;
  SaveStat *pDlg=NULL;
  unsigned int size1,size2,i,j,idx,len,tot;
  double *pd=NULL;
  
  try
  {
   //create data stream:
   file.open(IO_WriteOnly);
   QTextStream stream(&file);
   //write out matlab header:
   stream << achMatlabHeader << achNL;
   //write out platform:
#ifdef __BORLANDC__
	stream << achPlatform << achWindows << achNL;
#else
	struct utsname u;
	uname(&u);
	stream << achPlatform << u.sysname << achNL;
#endif
  //write out time and date:
   stream << achCreated << QDate::currentDate().toString() << achComma << QTime::currentTime().toString() << achNL;
   //program stamp:
   stream << achMatHeader << achNL;
   //now pad it out to 116 bytes if necessary:
   of=file.at();
   if (of<116)
   	{
   	 for (QIODevice::Offset c=of;c<116;c++)
   	 	stream << '\000';
   	}
   //next write out bytes 117 to 124 with zero to indicate no platform specific data:
   for (QIODevice::Offset c=117;c<=124;c++)
   	 	stream << '\000';
   //next two bytes are the version, should be set to 0x0100:
   stream << '\000' << '\001';
   //lastly, write out the byte swap flags in the order "IM":
   stream << "IM";
   //switch to binary data mode
   QDataStream data(&file);
   data.setByteOrder(QDataStream::LittleEndian);
   size1=vdtStoreData.size();
   pDlg=new SaveStat(0,0,true);
   connect( pDlg, SIGNAL( cancelSave() ), this, SLOT( cancelSave()));
  bSaving=true;
  pDlg->show();
  pDlg->progressBar->setProgress(0,size1);
  pApp->processEvents();
  for (i=0;i<size1;i++)
		{
		 if (!bSaving)
				throw std::runtime_error(QString().sprintf("User cancelled saving data."));
		  pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing variable %d.</font>",i+1));
		  idx=vdtStoreData[i];
		  if (idx>=vvdVars.size())
				continue;
		 size2=vvdVars[idx].pData->size();
		 pd=new double[size2];
		 len=vvdVars[idx].Title.length()+1;
		 len+=(8-len%8);
		 tot=size2;
		 tot+=(8-tot%8);
		 //first write a matrix element
		 data<<0x0000000e;
		 //size is size2 + size of all the other tags below: +16 flags, +16 dimensions +8 name tag +size of string to 8 byte boundary:
//		 data<<(size2+16+16+8+len);
		data<<(tot+64);
		 //do flags:
		 data<<0x00000006;
		 data<<0x00000008;
		 data<<0x00000006;
		 data<<0x00000000;
		 //write out dimensionality:
		 data<<0x00000005;
		 data<<0x00000008;
		 data<<0x00000001;
		 data<<(tot/8);
		 //write out name:
		 data<<0x00000001;
		 data<<len;
		 data<<vvdVars[idx].Title.ascii();
 		 pad64bit(file,data);
		 //now write out each variable, first 32 bit data type, imDOUBLE=0x9 then size in 32 bit, followed by data itself:
		 //write out double specifier:
	  	 data<<0x00000009;
	  	 //write out size:
	  	 data<<tot;
		 for (j=0;j<size2;j++)
			{
			 if (!bSaving)
					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
			  pd[j]=vvdVars[idx].pData->at(j);
			}
		 for (j=0;j<size2;j++)
			{
			 if (!bSaving)
					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
			  data<<pd[j];
			}
		 delete[] pd;
		 pad64bit(file,data);
		 pd=NULL;
		 pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
		 pDlg->progressBar->setProgress(i);
		 pApp->processEvents();
		}
  file.close();
  pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done saving.</font>"));
  pDlg->progressBar->setProgress(size1,size1);
  pDlg->done();
  pDlg->setModal(true);
  pDlg->exec();
  }
 catch (std::exception &stdex)
     {
      file.close();
      if (pd)
		delete[] pd;
      Error(stdex.what());
      if (pDlg)
		pDlg->close();
#ifdef _DEBUG
  qDebug("[Control::saveMatFile] ERROR (error=%s).",stdex.what());
 #endif
  }
}
#else
void Control::saveMatFile(QString &qs)
{
 MATFile* fp=NULL;
 mxArray *pmx=NULL;
 double *pd=NULL;
 unsigned int size1,size2,i,j,id;
 int status;
 
 try
 {
  fp=matOpen(qs.ascii(),"wc");
  if (fp==NULL)
    	 throw std::runtime_error(QString().sprintf("[Control::saveMatFile]: Failed opening file %s.",qs.ascii()));  
  size1=vdtStoreData.size();
  QProgressDialog progressDlg( "Writing values...", "Abort", size1, NULL, "progress", TRUE );

   for (i=0;i<size1;i++)
   {
    id=vdtStoreData[i];
    if (id>=vvdVars.size())
   	continue;
    size2=vvdVars[id].pData->size();
    pmx=mxCreateDoubleMatrix(1,size2,mxREAL);
    if (pmx==NULL)
    	 throw std::runtime_error(QString().sprintf("[Control::saveMatFile]: Error creating data buffer."));    
    pd=mxGetPr(pmx);
    for (j=0;j<size2;j++)
    	pd[j]=vvdVars[id].pData->at(j);
    status= matPutVariable(fp,vvdVars[id].Title.ascii(),pmx);
    if (status!=0)
    	 throw std::runtime_error(QString().sprintf("[Control::saveMatFile]: Error writing Mat file: %s (%d)",strerror(status),status));
   mxDestroyArray(pmx);
   if (progressDlg.wasCancelled())
        break;
    progressDlg.setProgress(i);
    qApp->processEvents();
  }
  progressDlg.setProgress(size1);
 matClose(fp);
 }
 catch (std::exception &stdex)
     {
      if (fp!=NULL)
 		matClose(fp);
       mxDestroyArray(pmx);
       Error(stdex.what());
 #ifdef _DEBUG
  qDebug("[Control::saveMatFile] ERROR (error=%s).",stdex.what());
 #endif
  }
}
#endif
#endif

#ifdef CDF_FILE
bool Control::statusCDF(CDFstatus status)
{
	if (status!=CDF_OK)
	{
	 CDFerror(status,errorMsg);
	 if (status>CDF_OK)
	 {
#ifdef _DEBUG
	qDebug("Information CDF: %s",errorMsg);
#endif
	 }
	else
	{
	 if (status<CDF_WARN)
		{
#ifdef _DEBUG
	qDebug("Error CDF: %s",errorMsg);
#endif
		return true;
		}
		else
		{
#ifdef _DEBUG
	qDebug("Warning CDF: %s",errorMsg);
#endif
		}
	}
 }
 return false;
}

void Control::saveCDFFile(QString &qs)
{
	SaveStat *pDlg=NULL;
	unsigned int size1,size2,i,j,idx;
	unsigned int dimSize[1]={1};
	CDFid id;
	long attrId, varId;
	int ver, rel;
	char copyright[CDF_DOCUMENT_LEN+1];
	long l1dVary= {VARY};
	long l2dVary[2]= {VARY, VARY};
	long indices[2]={0,0};
	double *pd=NULL;
		
	try
	{
		size1=vdtStoreData.size();
		pDlg=new SaveStat(0,0,true);
		connect( pDlg, SIGNAL( cancelSave() ), this, SLOT( cancelSave()));
		bSaving=true;
		id=0;
		pDlg->show();
		pDlg->progressBar->setProgress(0,size1);
		pApp->processEvents();
		if (statusCDF(CDFcreate(qs.ascii(),1, dimSize, NETWORK_ENCODING, ROW_MAJOR, &id)))
			throw std::runtime_error(QString().sprintf("Error creating CDF file\n%s",errorMsg));
		if (statusCDF(CDFdoc(id,&ver,&rel,copyright)))
			throw std::runtime_error(QString().sprintf("Error getting CDF file information\n%s",errorMsg));
		//write out doc info:
		pDlg->textEdit->append(QString().sprintf(achFmtFont,0x10FF,copyright));
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Version %d, Release %d</font>",ver,rel));
		// write out file info
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing attributes.</font>"));
		if (statusCDF(CDFattrCreate(id, achIdentifier, GLOBAL_SCOPE,&attrId)))
			throw std::runtime_error(QString().sprintf("Error creating global attribute\n%s",errorMsg));
		if (statusCDF(CDFattrPut(id, attrId, 0, CDF_CHAR, strlen(achHeader), const_cast<char *>(achHeader))))
			throw std::runtime_error(QString().sprintf("Error putting global attribute\n%s",errorMsg));
		if (statusCDF(CDFattrCreate(id, achModel, GLOBAL_SCOPE,&attrId)))
			throw std::runtime_error(QString().sprintf("Error creating global attribute\n%s",errorMsg));
		if (statusCDF(CDFattrPut(id, attrId, 0, CDF_CHAR, qsFile.length(), qsFile.ascii())))
			throw std::runtime_error(QString().sprintf("Error putting global attribute\n%s",errorMsg));
		if (statusCDF(CDFattrPut(id, attrId, 1, CDF_CHAR, strlen(EquationObject->GetName()), const_cast<char *>(EquationObject->GetName()))))
			throw std::runtime_error(QString().sprintf("Error putting global attribute\n%s",errorMsg));
		if (statusCDF(CDFattrPut(id, attrId, 2, CDF_CHAR, strlen(EquationObject->GetInfo()), const_cast<char *>(EquationObject->GetInfo()))))
			throw std::runtime_error(QString().sprintf("Error putting global attribute\n%s",errorMsg));
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
		//create vars
		for (i=0;i<size1;i++)
		{
		//TODO: implement Lyapunov export
			if (!bSaving)
				throw std::runtime_error(QString().sprintf("User cancelled saving data."));
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing variable %d.</font>",i+1));
			idx=vdtStoreData[i];
			if (idx>=vvdVars.size())
				continue;
			size2=vvdVars[idx].pData->size();
			if (statusCDF(CDFvarCreate(id, vvdVars[idx].Title.ascii(), CDF_DOUBLE, 1, l1dVary, l2dVary, &varId)))
				throw std::runtime_error(QString().sprintf("Error creating variable\n%s",errorMsg));
			pd=new double[size2];
			for (j=0;j<size2;j++)
			{
				if (!bSaving)
					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
				pd[j]=vvdVars[idx].pData->at(j);
/*				indices[1]=j;
				if (statusCDF(CDFvarPut(id, varId, 0, indices, &d)))
					throw std::runtime_error(QString().sprintf("[Control::saveCDFFile]: Error writing values: %s",errorMsg));*/
			}
			if (statusCDF(CDFvarPut(id, varId, 0, indices, &pd)))
				throw std::runtime_error(QString().sprintf("Error writing values\n%s",errorMsg));
			delete[] pd;
			pd=NULL;
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
			pDlg->progressBar->setProgress(i);
			pApp->processEvents();
		}
		if (statusCDF(CDFclose(id)))
			throw std::runtime_error(QString().sprintf("Error closing CDF file\n%s",errorMsg));
		id=0;
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done saving.</font>"));
		pDlg->progressBar->setProgress(size1,size1);
		pDlg->done();
		pDlg->setModal(true);
		pDlg->exec();
	}
	catch (std::exception &stdex)
	{
   	if (id!=0)
		CDFclose(id);
	if (pd)
		delete[] pd;
	Error(stdex.what());
	if (pDlg)
		pDlg->close();
#ifdef _DEBUG
  qDebug("[Control::saveCDFFile] ERROR (error=%s).",stdex.what());
#endif
	}
}
#endif

#ifdef HDF_FILE
void Control::saveHDFFile(QString &qs, int compress)
{
	SaveStat *pDlg=NULL;
	unsigned int size1,size2,i,j,idx;
	int sd_id=0, sds_id=0, nError=0, start[1]={0};
	unsigned int vermin,vermaj,release;
	char pLibInfo[80];
	comp_coder_t comp_type;
	comp_info c_info; /* Compression structure */
	double *pd=NULL;
		
	try
	{
		size1=vdtStoreData.size();
		pDlg=new SaveStat(0,0,true);
		connect( pDlg, SIGNAL( cancelSave() ), this, SLOT( cancelSave()));
		bSaving=true;
		pDlg->show();
		pDlg->progressBar->setProgress(0,size1);
		pApp->processEvents();
		
	  //TODO: implement Lyapunov export
		
		sd_id = SDstart (qs.ascii(), DFACC_CREATE);
		if (sd_id==FAIL)
			throw std::runtime_error(QString().sprintf("Error creating HDF file."));
		
		if (Hgetlibversion(&vermaj, &vermin, &release, pLibInfo)==FAIL)
			throw std::runtime_error(QString().sprintf("Error getting HDF library information"));
		//write out doc info:
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>HDF Library version %d.%d.%d</font>",vermaj,vermin,release));
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>%s</font>",pLibInfo));
		
		// write out file info attributes
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing attributes.</font>"));
		if (SDsetattr(sd_id, achIdentifier, DFNT_CHAR8, strlen(achHeader), achHeader)==FAIL)
			throw std::runtime_error(QString().sprintf("Error creating global attribute Header"));
		if (SDsetattr(sd_id, achModel, DFNT_CHAR8, qsFile.length(), qsFile.ascii())==FAIL)
			throw std::runtime_error(QString().sprintf("Error creating global attribute Model"));
		if (SDsetattr(sd_id, achName, DFNT_CHAR8, strlen(EquationObject->GetName()), EquationObject->GetName())==FAIL)
			throw std::runtime_error(QString().sprintf("Error creating global attribute Name"));
		if (SDsetattr(sd_id, achInfo, DFNT_CHAR8, strlen(EquationObject->GetInfo()), EquationObject->GetInfo())==FAIL)
			throw std::runtime_error(QString().sprintf("Error creating global attribute Info"));
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
		//write vars
		for (i=0;i<size1;i++)
		{
			if (!bSaving)
				throw std::runtime_error(QString().sprintf("User cancelled saving data."));
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing variable %d.</font>",i+1));
			idx=vdtStoreData[i];
			if (idx>=vvdVars.size())
				continue;
			size2=vvdVars[idx].pData->size();
			sds_id = SDcreate (sd_id, vvdVars[idx].Title.ascii(), DFNT_FLOAT64, 1, (int *)&size2);
			if (sds_id==FAIL)
				throw std::runtime_error(QString().sprintf("Error creating variable"));
			//set optional compression level
			switch(compress)
			{
				case 5: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 9; break; }
				case 4: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 6; break; }
				case 3: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 3; break; }
				case 2: { comp_type =  COMP_CODE_SKPHUFF; c_info.skphuff.skp_size = 1; break; } //No documentation regarding good values for skp_size!
				case 1: { comp_type =  COMP_CODE_RLE; break; }
				default:
				case 0: { comp_type =  COMP_CODE_NONE ; break; }
			}
#ifdef _DEBUG
  qDebug("[Control::saveHDFFile] Set compression %d (%d).",comp_type,c_info.deflate.level);
#endif
			nError = SDsetcompress(sds_id, comp_type, &c_info);
			if (nError==FAIL)
			{
#ifdef _DEBUG
			 HEprint(stderr,0);
#endif
				throw std::runtime_error(QString().sprintf("Error setting up compression\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
			}
			pd=new double[size2];
			for (j=0;j<size2;j++)
			{
				if (!bSaving)
					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
				pd[j]=vvdVars[idx].pData->at(j);
			}
			nError = SDwritedata (sds_id, start, NULL, (int *)&size2, pd);
			if (nError==FAIL)
			{
#ifdef _DEBUG
			 HEprint(stderr,0);
#endif
				throw std::runtime_error(QString().sprintf("Error writing values\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
			}
			delete[] pd;
			pd=NULL;
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
			pDlg->progressBar->setProgress(i);
			pApp->processEvents();
		}
		if (SDendaccess(sds_id)==FAIL)
			throw std::runtime_error(QString().sprintf("Error closing access to HDF file"));
		sds_id=0;
		if (SDend(sd_id)==FAIL)
			throw std::runtime_error(QString().sprintf("Error closing HDF file"));
		sd_id=0;
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done saving.</font>"));
		pDlg->progressBar->setProgress(size1,size1);
		pDlg->done();
		pDlg->setModal(true);
		pDlg->exec();
	}
	catch (std::exception &stdex)
	{
		if (sds_id!=0)
			SDendaccess(sds_id);
		if (sd_id!=0)
			SDend(sd_id);
		if (pd)
			delete[] pd;
		Error(stdex.what());
		if (pDlg)
			pDlg->close();
#ifdef _DEBUG
  qDebug("[Control::saveHDFFile] ERROR (error=%s).",stdex.what());
#endif
	}
}
#endif

#ifdef HDF5_FILE
bool Control::LoadH5File(QString &qs)
{ //open the file and load the equations from it:
 unsigned int i,vermin,vermaj,release;
 herr_t status;
 hid_t hFile=0, hGroup=0, attr=0, atype=0, atype_mem;
 hsize_t size;
 H5O_info_t oinfo;
 char *p=NULL;
 double d;
 
 try
	{
	  //clear queue
	  while(!qParams.empty())
	    qParams.pop();
	 //open library:
	 status=H5open();
	 if (status<0)
		throw std::runtime_error(QString().sprintf("Error opening HDF5 library"));
	 status=H5get_libversion(&vermaj, &vermin, &release);
	 if (status<0)
		throw std::runtime_error(QString().sprintf("Failed getting library version info"));
	 hFile = H5Fopen(qs.ascii(), H5F_ACC_RDONLY, H5P_DEFAULT);
	 if (hFile<0)
			throw std::runtime_error(QString().sprintf("Error opening HDF5 file."));
	 hGroup = H5Gopen (hFile, "/Information", H5P_DEFAULT);
	 if (hGroup<0)
			throw std::runtime_error(QString().sprintf("Error getting group in HDF5 file."));
	 //check if the attribute exists:
	 if (!H5Aexists(hGroup,achFile))
			throw std::runtime_error(QString().sprintf("Equation file not found in HDF5 file."));
	 attr=H5Aopen(hGroup,achFile,H5P_DEFAULT);
	 atype=H5Aget_type(attr);
	 atype_mem = H5Tget_native_type(atype, H5T_DIR_ASCEND);
	 //get the size:
	 size=H5Aget_storage_size(attr);
	 if (size==0)
		throw std::runtime_error(QString().sprintf("Failed getting valid size from HDF5 file."));
	 p=new char[size+1];
         status= H5Aread(attr, atype_mem, p);
	 if (status<0)
		throw std::runtime_error(QString().sprintf("Failed reading equations from HDF5 file."));
	 qsEqFile=p;
	 if (p!=NULL)
	    delete[] p;
         p=NULL;
	 H5Tclose(atype_mem);
	 H5Tclose(atype);
	 H5Aclose(attr);
	 H5Gclose(hGroup);
	 //now open parameters group
	 hGroup = H5Gopen (hFile, "/Parameters", H5P_DEFAULT);
	 if (hGroup<0)
			throw std::runtime_error(QString().sprintf("Error getting parameters group in HDF5 file."));
	 status= H5Oget_info(hGroup, &oinfo);
	 if (status<0)
		throw std::runtime_error(QString().sprintf("Failed reading group info from HDF5 file."));
	 //iterate through attributes and extract values:
	 for(i = 0; i < (unsigned)oinfo.num_attrs; i++)
	    {
              attr = H5Aopen_by_idx(hGroup, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, (hsize_t)i, H5P_DEFAULT, H5P_DEFAULT);
	      //get name size
	      size=H5Aget_name(attr,0,NULL);
	      if (size==0)
		continue;
	      //incr for zero terminator
	      size++;
	      //allocate mem:
	      p=new char[size];
	      memset(p,0,size);
	      //get name
	      H5Aget_name(attr,size,p);
	      //get attribute value
	      status=H5Aread(attr, H5T_NATIVE_DOUBLE, &d);
	      if (status<0)
		throw std::runtime_error(QString().sprintf("Failed to read parameter value from HDF5 file."));
	      qParams.push(Param(p,d));
	      delete[] p;
	      p=NULL;
	      H5Aclose(attr);
	    }
	 H5Gclose(hGroup);
	 H5Fclose(hFile);
	 H5close();
	}
	catch (std::exception &stdex)
	{
	 if (hFile!=0)
	    H5Fclose(hFile);
	 H5close();
	 if (p!=NULL)
	    delete[] p;
  	 Error(stdex.what());
	 return false;
	}
  return true;
}

herr_t Control::writeStringAttrHDF5(hid_t h, const char *name, const char *str)
{
  hid_t hs, atype,attr;
  herr_t status;
  
  hs= H5Screate(H5S_SCALAR);
  atype = H5Tcopy(H5T_C_S1);
  H5Tset_size(atype,strlen(str)+1);
  H5Tset_strpad(atype,H5T_STR_NULLTERM);
  attr = H5Acreate2(h, name, atype, hs, H5P_DEFAULT, H5P_DEFAULT);
  status = H5Awrite(attr, atype, str);
  H5Sclose(hs);
  H5Tclose(atype);
  H5Aclose(attr);
  return status;
}

herr_t Control::writeDoubleAttrHDF5(hid_t h, const char *name, double d)
{
  hid_t hs,attr;
  herr_t status;

  hs= H5Screate(H5S_SCALAR);
  attr = H5Acreate2(h, name, H5T_NATIVE_DOUBLE, hs, H5P_DEFAULT, H5P_DEFAULT);
  status= H5Awrite(attr, H5T_NATIVE_DOUBLE, &d);
  H5Sclose(hs);
  H5Aclose(attr);
  return status;
}

void Control::saveHDF5File(QString &qs, int compress)
{
	SaveStat *pDlg=NULL;
	unsigned int size1,size2,i,j,idx,N;
	unsigned int vermin,vermaj,release;
	double *pd=NULL;
	herr_t status;
	hid_t hFile=0, hGroup=0, hSpace=0, hData=0, hProp=0;
	htri_t  avail;
	unsigned int filter_info;
	hsize_t  dim[1] ={0};
	hsize_t chunk[1]={CHUNK_SIZE}; //4K chunk size as default
	
	try
	{
		size1=vdtStoreData.size();
		pDlg=new SaveStat(0,0,true);
		connect( pDlg, SIGNAL( cancelSave() ), this, SLOT( cancelSave()));
		bSaving=true;
		pDlg->show();
		pDlg->progressBar->setProgress(0,size1);
		pApp->processEvents();
		
		//open library:
		status=H5open();
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error opening HDF5 library"));
		status=H5get_libversion(&vermaj, &vermin, &release);
		if (status<0)
			throw std::runtime_error(QString().sprintf("Failed getting library version info"));
		//create error stack
		H5Ecreate_stack();
		//check for compression availability
		avail=0;
		switch(compress)
		{
		  case 1:
		  {
		    avail = H5Zfilter_avail(H5Z_FILTER_DEFLATE);
		    status = H5Zget_filter_info (H5Z_FILTER_DEFLATE, &filter_info);
		    if ((!avail)||(!(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED)) || (!(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ))
			throw std::runtime_error(QString().sprintf("H5Z_FILTER_DEFLATE not available"));
		    break;
		  }
		  case 2:
		  {
		    avail = H5Zfilter_avail(H5Z_FILTER_SZIP);
		    status = H5Zget_filter_info (H5Z_FILTER_SZIP, &filter_info);
		    if ((!avail)||(!(filter_info & H5Z_FILTER_CONFIG_ENCODE_ENABLED)) || (!(filter_info & H5Z_FILTER_CONFIG_DECODE_ENABLED) ))
			throw std::runtime_error(QString().sprintf("H5Z_FILTER_SZIP not available"));
		    break;
		  }
		  default: break; //no compression
		}
		
		//open file
		hFile = H5Fcreate (qs.ascii(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
		if (hFile<0)
			throw std::runtime_error(QString().sprintf("Error creating HDF5 file."));

		//write out doc info:
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>HDF5 Library version %d.%d.%d</font>",vermaj,vermin,release));
		//write file info:
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing file attributes.</font>"));

		//create a group
		hGroup = H5Gcreate (hFile, "/Information", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 		// write out file info attributes:
		status=writeStringAttrHDF5(hGroup,achIdentifier,achHeader);
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing string to HDF5 file"));
		status=writeStringAttrHDF5(hGroup,achModel,qsFile.ascii());
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing string to HDF5 file"));
		status=writeStringAttrHDF5(hGroup,achName,EquationObject->GetName());
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing string to HDF5 file"));
		status=writeStringAttrHDF5(hGroup,achInfo,EquationObject->GetInfo());
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing string to HDF5 file"));
		//write file content
		if (qsEqFile.length()>0)
		{
		  status=writeStringAttrHDF5(hGroup,achFile,qsEqFile.ascii());
		  if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing equation file to HDF5 file"));
		}
		status = H5Gclose (hGroup);
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error closing group"));
		hGroup=0;
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));

 		//create a group
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing parameters.</font>"));
		hGroup = H5Gcreate (hFile, "/Parameters", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		N=EquationObject->GetNPar();
		for (i=0;i<N;i++)
		{
		  status=writeDoubleAttrHDF5(hGroup,EquationObject->GetParmName(i),EquationObject->GetParmValue(i));
		  if (status<0)
			throw std::runtime_error(QString().sprintf("Error writing double to HDF5 file"));
		}
		status = H5Gclose (hGroup);
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error closing group"));
		hGroup=0;
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));

		//create a group
		hGroup = H5Gcreate (hFile, "/Variables", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 		//create vars
 		for (i=0;i<size1;i++)
 		{
 			if (!bSaving)
 				throw std::runtime_error(QString().sprintf("User cancelled saving data."));
 			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing variable %d.</font>",i+1));
 			idx=vdtStoreData[i];
 			if (idx>=vvdVars.size())
 				continue;
 			size2=vvdVars[idx].pData->size();
			dim[0]=size2;
			hSpace = H5Screate_simple(1, dim, NULL);
			hProp = H5Pcreate (H5P_DATASET_CREATE);
			switch(compress)
			{
			  default: break;
			  case 1:
			  {
			    status=H5Pset_deflate(hProp,9);
			    if (status<0)
				throw std::runtime_error(QString().sprintf("Error setting GZIP compression level"));
			    break;
			  }
			  case 2:
			  {
			    status = H5Pset_szip (hProp, H5_SZIP_NN_OPTION_MASK, 8);
			    if (status<0)
				throw std::runtime_error(QString().sprintf("Error setting SZIP options"));
			    break;
			  }
			}
			//check if chunk size is larger than data size:
			if (size2<CHUNK_SIZE)
			  chunk[0]=size2;
			else
			  chunk[0]=CHUNK_SIZE;
			status = H5Pset_chunk (hProp, 1, chunk);
			hData = H5Dcreate (hGroup, vvdVars[idx].Title.ascii(), H5T_IEEE_F64LE, hSpace, H5P_DEFAULT, hProp,H5P_DEFAULT);
 			pd=new double[size2];
 			for (j=0;j<size2;j++)
 			{
 				if (!bSaving)
 					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
 				pd[j]=vvdVars[idx].pData->at(j);
 			}
  		        status = H5Dwrite (hData, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pd);
		        if (status<0)
			    throw std::runtime_error(QString().sprintf("Error writing data"));
 			delete[] pd;
			pd=NULL;
			status = H5Pclose (hProp);
			status = H5Dclose (hData);
			status = H5Sclose (hSpace);			
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
			pDlg->progressBar->setProgress(i);
			pApp->processEvents();
		}
		status = H5Gclose (hGroup);
		if (status<0)
			throw std::runtime_error(QString().sprintf("Error closing group"));
		hGroup=0;
		//Write Lyapunov if requested
		if (isInDataStore(IDLYAPUNOV))
		{
		  //create a group
		  hGroup = H5Gcreate (hFile, "/Lyapunov", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		  size1=IntegratorObject->getLyapSize();

		  for (i=0;i<size1;i++)
		  {
		    if (!bSaving)
			throw std::runtime_error(QString().sprintf("User cancelled saving data."));
		    pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Writing Lyapunov estimate %d.</font>",i+1));
		    dblDeque *pdl=IntegratorObject->getLyapData(i);
		    size2=pdl->size();
		    dim[0]=size2;
		    hSpace = H5Screate_simple(1, dim, NULL);
		    hProp = H5Pcreate (H5P_DATASET_CREATE);
		    switch(compress)
			{
			  default: break;
			  case 1:
			  {
			    status=H5Pset_deflate(hProp,9);
			    if (status<0)
				throw std::runtime_error(QString().sprintf("Error setting GZIP compression level"));
			    break;
			  }
			  case 2:
			  {
			    status = H5Pset_szip (hProp, H5_SZIP_NN_OPTION_MASK, 8);
			    if (status<0)
				throw std::runtime_error(QString().sprintf("Error setting SZIP options"));
			    break;
			  }
			}
			//check if chunk size is larger than data size:
			if (size2<CHUNK_SIZE)
			  chunk[0]=size2;
			else
			  chunk[0]=CHUNK_SIZE;
			status = H5Pset_chunk (hProp, 1, chunk);
			qs.sprintf("Lambda%d",i+1);
			hData = H5Dcreate (hGroup,qs, H5T_IEEE_F64LE, hSpace, H5P_DEFAULT, hProp,H5P_DEFAULT);
 			pd=new double[size2];
 			for (j=0;j<size2;j++)
 			{
 				if (!bSaving)
 					throw std::runtime_error(QString().sprintf("User cancelled saving data."));
 				pd[j]=pdl->at(j);
 			}
  		        status = H5Dwrite (hData, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, pd);
		        if (status<0)
			    throw std::runtime_error(QString().sprintf("Error writing data"));
 			delete[] pd;
			pd=NULL;
			status = H5Pclose (hProp);
			status = H5Dclose (hData);
			status = H5Sclose (hSpace);			
			pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done.</font>"));
			pDlg->progressBar->setProgress(i);
			pApp->processEvents();
		}
		 status = H5Gclose (hGroup);
		 if (status<0)
			throw std::runtime_error(QString().sprintf("Error closing group"));
		 hGroup=0;
		}
		//close stuff
		if (H5Fclose(hFile)<0)
		  throw std::runtime_error(QString().sprintf("Error closing HDF5 file"));			  
		hFile=0;
		if (H5close()<0)
		  throw std::runtime_error(QString().sprintf("Error closing HDF5 library"));
		pDlg->textEdit->append(QString().sprintf("<font color=#0010FF>Done saving.</font>"));
		pDlg->progressBar->setProgress(size1,size1);
		pDlg->done();
		pDlg->setModal(true);
		pDlg->exec();
	}
	catch (std::exception &stdex)
	{
		if (hFile!=0)
		  H5Fclose(hFile);
		if (pd)
			delete[] pd;
		pDlg->textEdit->append(QString().sprintf("<font color=#F010FF>Error:\n%s</font>",stdex.what()));
		pDlg->textEdit->append(QString().sprintf("<font color=#F010FF>Errors reported by HDF5 library:</font>"));
		hid_t h=H5Eget_current_stack();
		char buf[256];
		H5E_type_t t;
		for (ssize_t k=0;k<H5Eget_num(h);k++)
		{
		  if (H5Eget_msg(k,&t,buf,255)>0)
		    pDlg->textEdit->append(QString(buf));
		}
		pDlg->textEdit->append(QString().sprintf("<font color=#F010FF>Errors done.</font>"));
		pDlg->progressBar->setProgress(size1,size1);
		pDlg->done();
		pDlg->setModal(true);
		pDlg->exec();
		H5Eclear1();
		H5close();
#ifdef _DEBUG
  qDebug("[Control::saveHDF5File] ERROR (error=%s).",stdex.what());
#endif
	}
}
#endif

// Cancel saving data 
void Control::cancelSave()
{
	if (bSaving)
		bSaving=false;
}
