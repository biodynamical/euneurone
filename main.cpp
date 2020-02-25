#include <qapplication.h>
#include <qtextedit.h>
#include <qbuttongroup.h>
#include <qtabwidget.h>
#include <qscrollview.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qprinter.h>
#include <qmessagebox.h>
#include <qstringlist.h>
#include <qsettings.h>
#include <qlabel.h>
#include <exception>
#include "mainform.h"
#include "controlobject.h"
#ifdef Q_WS_X11
  #include <X11/Xlib.h>
#endif

MainForm *pMain;
QApplication *pApp;
NS_Control::Control *pControl;
QPrinter *pPrinter;
QSettings *pSettings;
QString qsApplication("EuNeurone");
QString qsProgramUrl("EuNeurone.co.uk");
QString qsProgramVersion("2.3.");
QFont fontGlobal( "Helvetica", 10 );

//define a generalized exception handler
class UnexpectedException
{
  std::unexpected_handler old;
 public:
  UnexpectedException(std::unexpected_handler f) { old=std::set_unexpected(f);}
  ~UnexpectedException() { std::set_unexpected(old);}
};

void throwUE() throw(std::exception) { throw std::exception(); }

int main( int argc, char ** argv ) throw (std::exception)
{
    int result=0;
    
#ifdef Q_WS_X11
     XInitThreads();
#endif
    
   static UnexpectedException ue(&throwUE);
   try
   {
#ifdef _DEBUG
  	qDebug("[Main] Start of programme");
#endif   
    //New application instance
    pApp=new QApplication( argc, argv );
    // Main window
    pMain=new MainForm();
    pApp->connect( pApp, SIGNAL( lastWindowClosed() ), pApp, SLOT( quit() ) );
    pApp->connect( pMain, SIGNAL( closed() ), pApp, SLOT( quit() ) );
    // Instance of the controlling object
    pControl=new NS_Control::Control(pMain->textEditComp,pMain->textEditLog,pMain->textEditFile);
    pApp->connect(pControl,SIGNAL(newGraphSignal(QStringList *)),pMain,SLOT(newControlGraph(QStringList *)));
    pApp->connect(pControl,SIGNAL(new3DGraphSignal(QStringList *)),pMain,SLOT(newControlGraph3D(QStringList *)));
    pApp->setFont(fontGlobal);
    // Instance of the settings object
    pSettings= new QSettings(QSettings::Ini); // Get settings from local ini file always
    pSettings->setPath(qsProgramUrl,qsApplication,QSettings::User);
#ifdef Q_WS_X11
    pSettings->insertSearchPath( QSettings::Unix,pApp->applicationDirPath() );
#endif
    pMain->Initialize(pSettings);
    NS_Control::TWidgetVector *pv=new NS_Control::TWidgetVector;
    pv->push_back(pMain->buttonGroupInt);
    pv->push_back(pMain->listViewVars);
    pv->push_back(pMain->progressBar);
    pv->push_back(pMain->lcdNumber);
    pv->push_back(pMain->lcdEstTime);
    pv->push_back(pMain->lcdElapTime);
    //pv->push_back(pMain->lcdUpdate);
    pv->push_back(pMain->statusBar());
    pv->push_back(pMain-> lblIntThread);
    pv->push_back(pMain-> lblStatThread);
    pv->push_back(pMain-> lblAnaThread);
    pControl->Initialize(*pv,pSettings);
    delete pv;
    delete pSettings;
    
    //setup a global printer
    pPrinter=new QPrinter(QPrinter::HighResolution);
    pMain->show();
    if (argc>1)
    {
    	QString qs(argv[1]);
    	pControl->ReadEquation(qs);
    }
    result=pApp->exec();
   // Attempt to save settings:
    pSettings= new QSettings(QSettings::Ini); // Get settings from local ini file always
    pSettings->setPath(qsProgramUrl,qsApplication,QSettings::User);
    pMain->saveSettings(pSettings);
    delete pSettings;
   }
   catch(std::exception &e)
   {
  #ifdef _DEBUG
  	qDebug("[Main] Caught exception!");
  #endif
    QString qs;
    qs="Caught an unhandled exception: ";
    qs.append(e.what());
    QMessageBox::critical( 0,"EuNeurone Fatal Error",qs,"Quit" );
   }
   catch(...)
   {
  #ifdef _DEBUG
  	qDebug("[Main] Caught exception!");
  #endif
    QMessageBox::critical( 0,"EuNeurone Fatal Error","Caught an unknown, unhandled exception, sorry.","Quit" );
   }
    delete pPrinter;
    delete pControl;
    delete pMain;
    delete pApp;
  #ifdef _DEBUG
  	qDebug("[Main] End of programme");
  #endif
    return result;
}
