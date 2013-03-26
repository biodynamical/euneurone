/****************************************************************************
** Form implementation generated from reading ui file 'mainform.ui'
**
** Created: Mon Aug 18 21:28:42 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
****************************************************************************/
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qheader.h>
#include <qlcdnumber.h>
#include <qlistview.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qfiledialog.h> 
#include <qstatusbar.h> 
#include <qgroupbox.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qhbox.h>
#include <qvalidator.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include "mainform.h"
#include "about.h"

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#else
#include <qwt_plot.h>
#endif

#define HISTORY_SIZE 10
#define STATUSSHOW 2000

extern NS_Control::Control *pControl;
extern QString qsApplication;
extern QFont fontGlobal;

extern const char *achMean;
extern const char *achStddev;

const char *achRecent="Recent Files";
const char *achFileTypes="Equation files (*.eq)";
const char *achH5FileTypes="HDF5 (*.h5)";

//===============================================================
TGraphAction::TGraphAction(QObject *parent,int n) : QAction(parent, "TGraphAction")
{
 QWidget *pw=(QWidget *)parent;
 int w;
 
 nIdx=n;
 pGraph=new TGraphForm(pw);
 pGraph->setFont(fontGlobal);
 connect( pControl, SIGNAL( reCaption(QwtPlot *,QString)), this, SLOT( reCaption(QwtPlot *,QString)));
 connect( pControl, SIGNAL( doProgress(QwtPlot *, unsigned int, unsigned int)),
 		 this, SLOT( doProgress(QwtPlot *, unsigned int, unsigned int)));
 connect( pGraph, SIGNAL( closed() ), this, SLOT( getKilled() ) );
 connect( pGraph, SIGNAL( newGraphSignal() ), this, SLOT( sendNGS() ) );
 rename(n);
 setIconSet( QIconSet(QPixmap::fromMimeSource( "graph.png" )));
 connect( this, SIGNAL( activated() ) , this, SLOT( isActivated() ) );
 w=pw->frameGeometry().width();
 if (w<pw->width())
 	w=pw->width()+6;
 pGraph->move(w+pw->pos().x()+((n-1)*6),((n-1)*32));
 pGraph->show();
}

TGraphAction::~TGraphAction()
{
 delete pGraph;
}

void TGraphAction::isActivated()
{
 pGraph->raise();
 pGraph->setActiveWindow();
 emit Clicked(nIdx);
}

void TGraphAction::getKilled()
{
 emit killMe(nIdx);
}

void TGraphAction::sendNGS()
{
 emit newGraphSignal();
}

void TGraphAction::rename(int id)
{
 QString qs;
 int key;
 
 nIdx=id;
 qs.sprintf("Graph %d",id);
 setText(qs);
 pGraph->rename(qs);
 qs.sprintf("Graph &%d",id);
 setMenuText(qs);
 if (id>9)
  key=0;
 else
  key=ALT|(Key_0+id);
 setAccel(key);
}

void TGraphAction::add(QStringList *p)
{
 pGraph->addVariables(p);
}

void TGraphAction::reCaption(QwtPlot *p,QString qsn)
{
 if (pGraph->getGraph()!=p)
 	return;
 QString qs;
 
 qs.sprintf("Graph %d [%s]",nIdx,qsn.ascii());
 setText(qs);
 pGraph->rename(qs);
 qs.sprintf("Graph &%d [%s]",nIdx,qsn.ascii());
 setMenuText(qs);
}

void TGraphAction::doProgress(QwtPlot *p,unsigned int max, unsigned int cur)
{
 if (pGraph->getGraph()!=p)
 	return;
 if (max==0)
 	pGraph->hideProgress();
 else
 	pGraph->setProgress(max,cur);
}
//===============================================================
TGraph3DAction::TGraph3DAction(QObject *parent,int n) : QAction(parent, "TGraph3DAction")
{
 nIdx=n;
 pGraph=new TGraph3DForm((QWidget *)parent);
 connect( pGraph, SIGNAL( closed() ), this, SLOT( getKilled() ) );
 connect( pGraph, SIGNAL( newGraphSignal() ), this, SLOT( sendNGS() ) );
 rename(n);
 setIconSet( QIconSet(QPixmap::fromMimeSource( "graph3d.png" )));
 connect( this, SIGNAL( activated() ) , this, SLOT( isActivated() ) );
 pGraph->move((n*5),(n*32));
 pGraph->show();
 pGraph->resize(300,350);
}

TGraph3DAction::~TGraph3DAction()
{
 delete pGraph;
}

void TGraph3DAction::isActivated()
{
 pGraph->raise();
 pGraph->setActiveWindow();
 emit Clicked(nIdx);
}

void TGraph3DAction::getKilled()
{
 emit killMe(nIdx);
}

void TGraph3DAction::sendNGS()
{
 emit newGraph3DSignal();
}

void TGraph3DAction::rename(int id)
{
 QString qs;
 int key;
 
 nIdx=id;
 qs.sprintf("Graph 3D %d",id);
 setText(qs);
 pGraph->rename(qs);
 qs.sprintf("Graph 3D &%d",id);
 setMenuText(qs);
 if (id>9)
  key=0;
 else
  key=ALT|(Key_0+id);
 setAccel(key);
}

void TGraph3DAction::add(QStringList *p)
{
 pGraph->addVariables(p);
}
//============================================
/* 
 *  Constructs a MainForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
MainForm::MainForm( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "MainForm" );

    setFont(fontGlobal);

    setIcon(QPixmap::fromMimeSource("euneurone.png"));
 
    tabWidget = new QTabWidget( this, "tabWidget" );
    tabWidget->setGeometry( QRect( 0, 0, 560, 270 ) );
    tabWidget->setTabPosition( QTabWidget::Bottom );
    tabWidget->setTabShape( QTabWidget::Rounded );
    setCentralWidget( tabWidget );

    textEditComp = new QTextEdit( tabWidget, "textEditComp" );
    textEditComp->setGeometry( QRect( 0, 0, 550, 240 ) );
    textEditComp->setPaletteBackgroundColor( QColor( 170, 255, 127 ) );
    textEditComp->setResizePolicy( QTextEdit::AutoOneFit );
    textEditComp->setReadOnly( TRUE );
    textEditComp->setTextFormat( Qt::LogText );
    tabWidget->insertTab( textEditComp, "" );

    vboxLog=new QVBox(tabWidget);
    textEditLog = new QTextEdit( vboxLog, "textEditLog" );
    textEditLog->setGeometry( QRect( 0, 0, 550, 240 ) );
    textEditLog->setResizePolicy( QTextEdit::AutoOneFit );
//    textEditLog->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 1, 1, textEditLog->sizePolicy().hasHeightForWidth() ) );
    textEditLog->setPaletteBackgroundColor( QColor( 255, 255, 127 ) );
    textEditLog->setReadOnly( TRUE );
    textEditLog->setTextFormat( Qt::LogText );
    hboxLog=new QHBox(vboxLog);
    lineEditLog = new QLineEdit( hboxLog, "lineEditLog" );
    pushButtonLog1 = new QPushButton( hboxLog, "pushButtonLog1" );
//    pushButtonLog1->setGeometry( QRect( 35, 195, 125, 26 ) );
    pushButtonLog2 = new QPushButton( hboxLog, "pushButtonLog2" );
    
    tabWidget->insertTab( vboxLog, "" );

    textEditFile = new QTextEdit( tabWidget, "textEditFile" );
    textEditFile->setGeometry( QRect( 0, 0, 560, 240 ) );
    textEditFile->setResizePolicy( QTextEdit::AutoOneFit );
    tabWidget->insertTab( textEditFile, "" );

    tabControl = new QScrollView( tabWidget, "tabControl" );
    boxControl = new QHBox(tabControl->viewport());
    boxControl->setGeometry( QRect( 0, 0,  tabControl->visibleWidth(), tabControl->visibleHeight() ));
//    boxControl = new QHBox(tabWidget);
    tabControl->addChild(boxControl);
    
    groupBoxButs = new QGroupBox( boxControl, "groupBoxButs" );
    groupBoxButs->setGeometry( QRect( 0, 0, tabControl->visibleWidth(), tabControl->visibleHeight() ) );
    tabControl->viewport()->setEraseColor(groupBoxButs->eraseColor());
    
    lcdNumber = new QLCDNumber( groupBoxButs, "lCDNumber" );
    lcdNumber->setGeometry( QRect( 10, 17, 150, 30 ) );
    lcdNumber->setFrameShape( QLCDNumber::Box );
    lcdNumber->setFrameShadow( QLCDNumber::Plain );
    lcdNumber->setSmallDecimalPoint( FALSE );
    lcdNumber->setNumDigits( 10 );
    lcdNumber->setSegmentStyle( QLCDNumber::Flat );

    progressBar = new QProgressBar( groupBoxButs, "progressBar" );
    progressBar->setGeometry( QRect( 10, 50, 150, 30 ) );
    progressBar->setFrameShape( QProgressBar::WinPanel );
    progressBar->setFrameShadow( QProgressBar::Plain );
    progressBar->setMidLineWidth( 0 );
    progressBar->setTotalSteps( 100 );
    progressBar->setProgress( 0 );
    progressBar->setCenterIndicator( TRUE );

    textLabelElapsed = new QLabel( groupBoxButs, "textLabelElapsed" );
    textLabelElapsed->setGeometry( QRect( 10, 80, 70, 20 ) );

    textLabelEstimated = new QLabel( groupBoxButs, "textLabelEstimated" );
    textLabelEstimated->setGeometry( QRect( 85, 80, 70, 20 ) );

    lcdElapTime = new QLCDNumber( groupBoxButs, "lCDElapTime" );
    lcdElapTime->setGeometry( QRect( 10, 100, 70, 30 ) );
    lcdElapTime->setFrameShape( QLCDNumber::Box );
    lcdElapTime->setFrameShadow( QLCDNumber::Plain );
    lcdElapTime->setSmallDecimalPoint( FALSE );
    lcdElapTime->setNumDigits( 8 );
    lcdElapTime->setSegmentStyle( QLCDNumber::Flat );

    lcdEstTime = new QLCDNumber( groupBoxButs, "lCDEstTime" );
    lcdEstTime->setGeometry( QRect( 85, 100, 70, 30 ) );
    lcdEstTime->setFrameShape( QLCDNumber::Box );
    lcdEstTime->setFrameShadow( QLCDNumber::Plain );
    lcdEstTime->setSmallDecimalPoint( false );
    lcdEstTime->setNumDigits( 8 );
    lcdEstTime->setSegmentStyle( QLCDNumber::Flat );
    
    pushButtonStart = new QPushButton( groupBoxButs, "pushButtonStart" );
    pushButtonStart->setGeometry( QRect( 35, 135, 125, 26 ) );

    pushButtonReset = new QPushButton( groupBoxButs, "pushButtonReset" );
    pushButtonReset->setGeometry( QRect( 35, 165, 125, 26 ) );
    
    pushButtonStop = new QPushButton( groupBoxButs, "pushButtonStop" );
    pushButtonStop->setGeometry( QRect( 35, 195, 125, 26 ) );

    lblIntThread = new QLabel( groupBoxButs, "lblIntThread" );
    lblIntThread->setGeometry( QRect(12,197,20,20) );
    lblIntThread->setPixmap( QPixmap::fromMimeSource( "threadoff.png" ));
    
    pushButtonStatStop = new QPushButton( groupBoxButs, "pushButtonStatStop" );
    pushButtonStatStop->setGeometry( QRect( 35, 225, 125, 26 ) );

    lblStatThread = new QLabel( groupBoxButs, "lblStatThread" );
    lblStatThread->setGeometry( QRect(12,227,20,20) );
    lblStatThread->setPixmap( QPixmap::fromMimeSource( "threadoff.png" ));

    pushButtonAnaStop = new QPushButton( groupBoxButs, "pushButtonAnaStop" );
    pushButtonAnaStop->setGeometry( QRect( 35, 255, 125, 26 ) );

    lblAnaThread = new QLabel( groupBoxButs, "lblAnaThread" );
    lblAnaThread->setGeometry( QRect(12,257,20,20) );
    lblAnaThread->setPixmap( QPixmap::fromMimeSource( "threadoff.png" ));
    
    //--
    groupBoxIntVals = new QGroupBox( boxControl, "groupBoxIntVals" );
    groupBoxIntVals->setGeometry( QRect( 0, 0,  tabControl->visibleWidth(), tabControl->visibleHeight() ) );

    QWidget* privateLayoutWidget = new QWidget( groupBoxIntVals, "privlayout" );
    privateLayoutWidget->setGeometry( QRect( 8, 20,  150, 250 ) );
    layoutVals = new QVBoxLayout( privateLayoutWidget, 5, 5, "layoutVals"); 

    textLabelBegin = new QLabel( privateLayoutWidget, "textLabelBegin" );
    layoutVals->addWidget( textLabelBegin );

    lineEditBegin = new QLineEdit( privateLayoutWidget, "lineEditBegin" );
    layoutVals->addWidget( lineEditBegin );
    textLabelBegin->setBuddy(lineEditBegin);
    lineEditBegin->setValidator(new QIntValidator(lineEditBegin));

    textLabelEnd = new QLabel( privateLayoutWidget, "textLabelEnd" );
    layoutVals->addWidget( textLabelEnd );

    lineEditEnd = new QLineEdit( privateLayoutWidget, "lineEditEnd" );
    layoutVals->addWidget( lineEditEnd );
    textLabelEnd->setBuddy(lineEditEnd);
    lineEditEnd->setValidator(new QIntValidator(lineEditEnd));

    textLabelStep = new QLabel( privateLayoutWidget, "textLabelStep" );
    layoutVals->addWidget( textLabelStep );

    lineEditStep = new QLineEdit( privateLayoutWidget, "lineEditStep" );
    layoutVals->addWidget( lineEditStep );
    textLabelStep->setBuddy(lineEditStep);
    lineEditStep->setValidator(new QDoubleValidator(lineEditStep));

    textLabelInterval = new QLabel( privateLayoutWidget, "textLabelInterval" );
    layoutVals->addWidget( textLabelInterval );

    lineEditInterval = new QLineEdit( privateLayoutWidget, "lineEditInterval" );
    layoutVals->addWidget( lineEditInterval );
    textLabelInterval->setBuddy(lineEditInterval);
    lineEditInterval->setValidator(new QIntValidator(lineEditInterval));
    
    buttonGroupInt = new QButtonGroup( boxControl, "buttonGroupInt" );
    buttonGroupInt->setGeometry( QRect( 0, 0,  175, 220 ) );
    buttonGroupInt->setAlignment( int( QButtonGroup::AlignAuto ) );
    
    tabWidget->insertTab( tabControl, "" );

    listViewVars = new NS_Control::TListView( tabWidget, "listViewVars" );
    listViewVars->addColumn( tr( "Elements" ) );
    listViewVars->addColumn( tr( "Value" ) );
    listViewVars->addColumn( tr( "Initial value" ) );
    listViewVars->addColumn( tr( "Colour" ) );
    listViewVars->addColumn( tr( achMean ) );
    listViewVars->addColumn( tr( achStddev ) );
    listViewVars->setGeometry( QRect( 0, 0, 560, 240 ) );
    listViewVars->setResizePolicy( QListView::AutoOneFit );
    listViewVars->setAllColumnsShowFocus( TRUE );
    tabWidget->insertTab( listViewVars, "" );

    // actions
    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filenew.png" ) ) );
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileopen.png" ) ) );
#ifdef HDF5_FILE    
    fileLoadAction = new QAction( this, "fileLoafAction" );
    fileLoadAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileload.png" ) ) );
#endif
    fileSaveAction = new QAction( this, "fileSaveAction" );
    fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave.png" ) ) );
    fileSaveAsAction = new QAction( this, "fileSaveAsAction" );
    fileSaveAsAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesaveas.png" ) ) );
    filePrintAction = new QAction( this, "filePrintAction" );
    filePrintAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileprint.png" ) ) );
    fileExitAction = new QAction( this, "fileExitAction" );
    fileExitAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "exit.png" ) ) );
    helpContentsAction = new QAction( this, "helpContentsAction" );
    helpIndexAction = new QAction( this, "helpIndexAction" );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    
    anaLyapAction = new QAction( this, "anaLyapAction" );
    anaLyapAction->setToggleAction(true);
    anaLyapAction->setOn(true);
    anaStatsAction = new QAction( this, "anaStatsAction" );
    anaStatsAction->setToggleAction(true);
    anaStatsAction->setOn(true);

    winDataAction = new QAction( this, "newDataAction" );
    winDataAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "anadata.png" ) ) );
    winNewGraphAction = new QAction( this, "winNewGraphAction" );
    winNewGraphAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "newgraph.png" ) ) );
    winNew3DAction = new QAction( this, "winNew3DAction" );
    winNew3DAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "new3dgraph.png" ) ) );
    winIsiAction = new QAction( this, "winIsiAction" );
    winIsiAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "isi.png" ) ) );
    
    // toolbars
    toolBar = new QToolBar( "", this, DockTop ); 

    fileNewAction->addTo( toolBar );
    toolBar->addSeparator();
    fileOpenAction->addTo( toolBar );
#ifdef HDF5_FILE    
    fileLoadAction->addTo( toolBar );
#endif
    fileSaveAction->addTo( toolBar );
    fileSaveAsAction->addTo( toolBar );
    toolBar->addSeparator();
    filePrintAction->addTo( toolBar );


    // menubar
    menubar = new QMenuBar( this, "menubar" );

    menubar->setGeometry( QRect( 0, 0, 565, 24 ) );
    fileMenu = new QPopupMenu( this );
    fileMenu->setFont(fontGlobal);
    fileNewAction->addTo( fileMenu );
    fileOpenAction->addTo( fileMenu );
#ifdef HDF5_FILE        
    fileLoadAction->addTo( fileMenu );
#endif    
    historyMenu = new QPopupMenu(this);
    historyMenu->setFont(fontGlobal);
    fileMenu->insertItem(QIconSet( QPixmap::fromMimeSource( "filehistory.png" ) ), achRecent, historyMenu );
    fileSaveAction->addTo( fileMenu );
    fileSaveAsAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    filePrintAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    menubar->insertItem( "", fileMenu, 0 );

    AnaMenu = new QPopupMenu( this );
    AnaMenu->setFont(fontGlobal);
    anaLyapAction->addTo( AnaMenu );
    anaStatsAction->addTo( AnaMenu );
    menubar->insertItem( "", AnaMenu, 1 );
        
    WindowsMenu = new QPopupMenu( this );
    WindowsMenu->setFont(fontGlobal);
    winDataAction->addTo( WindowsMenu );
    winNewGraphAction->addTo( WindowsMenu );
    winNew3DAction->addTo( WindowsMenu );
    winIsiAction->addTo( WindowsMenu );
    WindowsMenu->insertSeparator();
    menubar->insertItem( "", WindowsMenu, 2 );
    
    helpMenu = new QPopupMenu( this );
    helpMenu->setFont(fontGlobal);
    helpContentsAction->addTo( helpMenu );
    helpIndexAction->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    menubar->insertItem( "", helpMenu, 3 );

    languageChange();
    resize( QSize(575, 410).expandedTo(minimumSizeHint()) );
    pData=NULL;
    pIsiForm=NULL;
     
     // signals and slots connections
    connect( tabWidget, SIGNAL( currentChanged(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );
     //menu
    connect( fileNewAction, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
#ifdef HDF5_FILE    
    connect( fileLoadAction, SIGNAL( activated() ), this, SLOT( fileLoad() ) );
#endif    
    connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    connect( fileSaveAsAction, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    connect( filePrintAction, SIGNAL( activated() ), this, SLOT( filePrint() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    connect( anaLyapAction, SIGNAL( toggled( bool )), this, SLOT( toggleLyap( bool)));
    connect( anaStatsAction, SIGNAL( toggled( bool )), this, SLOT( toggleStats( bool)));
    connect( winDataAction, SIGNAL( activated() ), this, SLOT( newData() ) );
    connect( winNewGraphAction, SIGNAL( activated() ), this, SLOT( newGraph() ) );
    connect( winNew3DAction, SIGNAL( activated() ), this, SLOT( new3DGraph() ) );
    connect( winIsiAction, SIGNAL( activated() ), this, SLOT( newIsi() ) );
    connect( historyMenu, SIGNAL( activated(int) ), this, SLOT( fileHistory(int) ) );
    //buttons
    connect( pushButtonStart, SIGNAL( clicked() ), this, SLOT( startButtonClicked() ) );
    connect( pushButtonReset, SIGNAL( clicked() ), this, SLOT( resetButtonClicked() ) );
    connect( pushButtonStop, SIGNAL( clicked() ), this, SLOT( stopIntButtonClicked() ) );
    connect( pushButtonStatStop, SIGNAL( clicked() ), this, SLOT( stopStatButtonClicked() ) );
    connect( pushButtonAnaStop, SIGNAL( clicked() ), this, SLOT( stopAnaButtonClicked() ) );
    
    connect( pushButtonLog1, SIGNAL( clicked() ), this, SLOT( addLogButtonClicked() ) );
    connect( pushButtonLog2, SIGNAL( clicked() ), this, SLOT( stateLogButtonClicked() ) );
    //lineedits
    connect( lineEditStep, SIGNAL(textChanged(const QString &)),this, SLOT(leStepChanged(const QString &)));
    connect( lineEditBegin, SIGNAL(textChanged(const QString &)),this, SLOT(leBeginChanged(const QString &)));
    connect( lineEditEnd, SIGNAL(textChanged(const QString &)),this, SLOT(leEndChanged(const QString &)));
    connect( lineEditInterval, SIGNAL(textChanged(const QString &)),this, SLOT(leIntervalChanged(const QString &)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MainForm::languageChange()
{
    setCaption( tr( qsApplication ) );
    tabWidget->changeTab( textEditComp, tr( "Compilation" ) );
    tabWidget->changeTab( vboxLog, tr( "Log" ) );
    tabWidget->changeTab( textEditFile, tr( "Edit" ) );
    groupBoxIntVals->setTitle( tr( "Integration values" ) );
    textLabelBegin->setText( tr( "&Initial stepvalue" ) );
    textLabelEnd->setText( tr( "Fina&l stepvalue" ) );
    textLabelStep->setText( tr( "&Step size" ) );
    textLabelInterval->setText( tr( "&Update interval" ) );
    groupBoxButs->setTitle( tr( "Integration control" ) );
    textLabelEstimated->setText( tr( "Estimated" ) );
    textLabelElapsed->setText( tr( "Elapsed" ) );
    pushButtonStart->setText( tr( "Start Integration" ) );
    pushButtonStop->setText( tr( "Stop Integration" ) );
    pushButtonReset->setText( tr( "Reset Integration" ) );
    pushButtonStatStop->setText( tr( "Stop Statistics" ) );
    pushButtonAnaStop->setText( tr( "Stop Analysis" ) );
    pushButtonLog1->setText( tr("Add"));    
    pushButtonLog2->setText( tr("State"));    
    
    buttonGroupInt->setTitle( tr( "Integrators" ) );
    tabWidget->changeTab( tabControl, tr( "Control" ) );
    listViewVars->header()->setLabel( 0, tr( "Elements" ) );
    listViewVars->header()->setLabel( 1, tr( "Value" ) );
    listViewVars->header()->setLabel( 2, tr( "Initial value" ) );
    listViewVars->header()->setLabel( 3, tr( "Colour" ) );
    listViewVars->clear();
    QListViewItem * item = new QListViewItem( listViewVars, 0 );
    item->setText( 0, tr( "No file loaded" ) );

    tabWidget->changeTab( listViewVars, tr( "Variables" ) );
    fileNewAction->setText( tr( "New" ) );
    fileNewAction->setMenuText( tr( "&New" ) );
    fileNewAction->setAccel( tr( "Ctrl+N" ) );
    fileOpenAction->setText( tr( "Open" ) );
    fileOpenAction->setMenuText( tr( "&Open..." ) );
    fileOpenAction->setAccel( tr( "Ctrl+O" ) );
#ifdef HDF5_FILE
    fileLoadAction->setText( tr( "Load" ) );
    fileLoadAction->setMenuText( tr( "&Load..." ) );
    fileLoadAction->setAccel( tr( "Ctrl+L" ) );
#endif    
     fileSaveAction->setText( tr( "Save" ) );
     fileSaveAction->setMenuText( tr( "&Save" ) );
     fileSaveAction->setAccel( tr( "Ctrl+S" ) );
    fileSaveAsAction->setText( tr( "Save As" ) );
    fileSaveAsAction->setMenuText( tr( "Save &As..." ) );
    fileSaveAsAction->setAccel( tr( "Ctrl+A" ) );
//    fileSaveAsAction->setAccel( QString::null );
    filePrintAction->setText( tr( "Print" ) );
    filePrintAction->setMenuText( tr( "&Print..." ) );
    filePrintAction->setAccel( tr( "Ctrl+P" ) );
    fileExitAction->setText( tr( "Exit" ) );
    fileExitAction->setMenuText( tr( "E&xit" ) );
    fileExitAction->setAccel( tr("Ctrl+Q") );
    helpContentsAction->setText( tr( "Contents" ) );
    helpContentsAction->setMenuText( tr( "&Contents..." ) );
    helpContentsAction->setAccel( QString::null );
    helpIndexAction->setText( tr( "Index" ) );
    helpIndexAction->setMenuText( tr( "&Index..." ) );
    helpIndexAction->setAccel( QString::null );
    helpAboutAction->setText( tr( "About" ) );
    helpAboutAction->setMenuText( tr( "&About" ) );
    helpAboutAction->setAccel( QString::null );
    
    anaLyapAction->setText( tr( "Lyapunov" ) );
    anaLyapAction->setMenuText( tr( "&Lyapunov" ) );
    anaLyapAction->setAccel( tr("Ctrl+L") );
    anaStatsAction->setText( tr( "Statistics" ) );
    anaStatsAction->setMenuText( tr( "S&tatistics" ) );
    anaStatsAction->setAccel( tr("Ctrl+T") );
    
    winDataAction->setText( tr( "Data Store" ) );
    winDataAction->setMenuText( tr( "&Data Store" ) );
    winDataAction->setAccel( tr("Ctrl+D") );
    winNewGraphAction->setText( tr( "New Graph" ) );
    winNewGraphAction->setMenuText( tr( "New &Graph" ) );
    winNewGraphAction->setAccel( tr("Ctrl+G") );
    winNew3DAction->setText( tr( "New &3D Graph" ) );
    winNew3DAction->setMenuText( tr( "New &3D Graph" ) );
    winNew3DAction->setAccel( tr("Ctrl+F") );
    winIsiAction->setText( tr( "&Interspike Window" ) );
    winIsiAction->setMenuText( tr( "Show Interspike Interval Window" ) );
    winIsiAction->setAccel( tr("Ctrl+I") );
    
    toolBar->setLabel( tr( "Tools" ) );
    menubar->findItem( 0 )->setText( tr( "&File" ) );
    menubar->findItem( 1 )->setText( tr( "&Analysis" ) );
    menubar->findItem( 2 )->setText( tr( "&Windows" ) );
    menubar->findItem( 3 )->setText( tr( "&Help" ) );
}
//------------------------------------------------------------------------------------
void MainForm::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    emit closed();
}
//------------------------------------------------------------------------------------
void MainForm::Initialize(QSettings *ps)
{ 
 //read in history list
 ps->beginGroup(qsApplication);
 slHistory=ps->readListEntry( achRecent );
 ps->endGroup();
 for (unsigned int i=0;i<slHistory.size();i++)
 {
  historyMenu->insertItem(QIconSet( QPixmap::fromMimeSource( "fileedit.png" ) ), slHistory[i],i);
 }
}
//------------------------------------------------------------------------------------
void MainForm::saveSettings(QSettings *ps)
{ 
 // save history list
 ps->beginGroup(qsApplication);
 ps->writeEntry( achRecent, slHistory );
 ps->endGroup();
}
//------------------------------------------------------------------------------------
void MainForm::tabChanged(QWidget *p)
{
 if ((p==textEditFile)||(p==vboxLog))
 {
  fileSaveAction->setEnabled(true);
  fileSaveAsAction->setEnabled(true);
 }
 else
 {
  fileSaveAction->setEnabled(false);
  fileSaveAsAction->setEnabled(false);
 }
}
//------------------------------------------------------------------------------------
void MainForm::fileNew()
{
 pControl->NewEquation();
 tabWidget->setCurrentPage(2); 
 killGraphs();
}
//------------------------------------------------------------------------------------
void MainForm::fileOpen()
{
 QString fn = QFileDialog::getOpenFileName( QString::null,achFileTypes, this, NULL, "Open an equation file" );
 doLoad(fn);
 addToHistory(fn);
}
//------------------------------------------------------------------------------------
void MainForm::doLoad(QString &fn)
{
  if ( !fn.isEmpty() )
   {
       textEditComp->clear();
       textEditFile->clear();
       tabWidget->setCurrentPage(0);
       killGraphs();
       killData();
       killIsi();
       if (pControl->ReadEquation(fn))
       {
        if (pControl->logOk())
		{
		 lineEditLog->setEnabled(true);
		 pushButtonLog1->setEnabled(true);
		 pushButtonLog2->setEnabled(true); 
		}
	else
		{
		 lineEditLog->setEnabled(false);
		 pushButtonLog1->setEnabled(false);
		 pushButtonLog2->setEnabled(false); 
		}
      	statusBar()->message( "Loading successful", STATUSSHOW );
	setCaption(QString("%1 - [%2]").arg(tr( qsApplication )).arg(fn));
        setOptions();
       }
   }
   else
        statusBar()->message( "Loading aborted", STATUSSHOW );
}
//------------------------------------------------------------------------------------
#ifdef HDF5_FILE
void MainForm::fileLoad()
{
 QString fn = QFileDialog::getOpenFileName( QString::null,achH5FileTypes, this, NULL, "Load from a HDF5 file" );
 doH5Load(fn);
}

void MainForm::doH5Load(QString &fn)
{
  if ( !fn.isEmpty() )
   {
       textEditComp->clear();
       textEditFile->clear();
       tabWidget->setCurrentPage(0);
       killGraphs();
       killData();
       killIsi();
       if (pControl->LoadH5Equation(fn))
       {
        if (pControl->logOk())
		{
		 lineEditLog->setEnabled(true);
		 pushButtonLog1->setEnabled(true);
		 pushButtonLog2->setEnabled(true); 
		}
	else
		{
		 lineEditLog->setEnabled(false);
		 pushButtonLog1->setEnabled(false);
		 pushButtonLog2->setEnabled(false); 
		}
      	statusBar()->message( "Loading successful", STATUSSHOW );
	setCaption(QString("%1 - [%2]").arg(tr( qsApplication )).arg(fn));
        setOptions();
       }
   }
   else
        statusBar()->message( "Loading aborted", STATUSSHOW );
}
#endif
//------------------------------------------------------------------------------------
void MainForm::fileSave()
{
 switch(tabWidget->indexOf(tabWidget->currentPage()))
 {
  default: break;
  case 1: //save log
  	{ 
	 if (pControl->needLogFilename())
	 {
	   QString qs=QFileDialog::getSaveFileName(QString::null, "Log files (*.log)", this, NULL, "Save as...");
	   if (qs.isNull())
	    break;
	   pControl->setLogFilename(qs);
	 }
	 pControl->saveLog();
	 break;
	}
  case 2: //edit page
  	{
	 if (pControl->needFilename())
	 {
	   QString qs=QFileDialog::getSaveFileName(QString::null, achFileTypes, this, NULL, "Save as...");
	   if (qs.isNull())
	    break;
	   pControl->setFilename(qs);
	 }
	 pControl->saveEquation();
	 if (QMessageBox::information( this, qsApplication, "Reload equation?", "Yes", "No",0, 0, 1 )==0)
	 	doLoad(pControl->getFileName());
	 break;
	}
 }
}
//------------------------------------------------------------------------------------
void MainForm::fileHistory(int id)
{
#ifdef _DEBUG
	qDebug("[MainForm::fileHistory] id=%d",id);
#endif
 if ((id>=0)&&((unsigned int)id<slHistory.size()))
      {
 	doLoad(slHistory[id]);
	moveHistory(id);
      }
}
//------------------------------------------------------------------------------------
void MainForm::fileSaveAs()
{
 switch(tabWidget->indexOf(tabWidget->currentPage()))
 {
  default: break;
  case 1: //save log
  	{ 
	 QString qs=QFileDialog::getSaveFileName(QString::null, "Log files (*.log)", this, NULL, "Save as...");
	 if (qs.isNull())
	    break;
	 pControl->setLogFilename(qs);
	 pControl->saveLog();
	 break;
	}
  case 2: //edit page
  	{
	 QString qs=QFileDialog::getSaveFileName(QString::null,achFileTypes, this, NULL, "Save as...");
	 if (qs.isNull())
	    break;
	 pControl->setFilename(qs);
	 pControl->saveEquation();
	 if (QMessageBox::information( this, qsApplication, "Reload equation?", "Yes", "No",0, 0, 1 )==0)
	 	doLoad(pControl->getFileName());
	 break;
	}
 }
}
//------------------------------------------------------------------------------------
void MainForm::filePrint()
{

}
//------------------------------------------------------------------------------------
void MainForm::fileExit()
{
 QApplication::exit();
}
//------------------------------------------------------------------------------------
void MainForm::helpIndex()
{

}
//------------------------------------------------------------------------------------
void MainForm::helpContents()
{
QMessageBox::aboutQt( this, "Qt Application Example" );
}
//------------------------------------------------------------------------------------
void MainForm::helpAbout()
{
 AboutForm *pAbout=new AboutForm(this);
 pAbout->setFont(fontGlobal);
 pAbout->exec();
 delete pAbout;
}
//------------------------------------------------------------------------------------
void MainForm::addToHistory(QString &qs)
{
  int idx;
  unsigned int i;
  
  if (qs.isNull()||qs.isEmpty())
  	return;
  idx=slHistory.findIndex(qs);
  if (idx<0) //not found
  	{
	 if (slHistory.size()>=HISTORY_SIZE)
	 	{
	 	 slHistory.pop_back(); //remove last item
		 historyMenu->removeItemAt(HISTORY_SIZE-1);
		}
	 slHistory.prepend(qs);
         historyMenu->insertItem(QIconSet( QPixmap::fromMimeSource( "fileedit.png" ) ), qs,0,0);
	}
 // renumber ids
 for (i=0;i<slHistory.size();i++)
  	historyMenu->setId(i,i);
}
//------------------------------------------------------------------------------------
void MainForm::moveHistory(int idx)
{
 //return if it is invalid or already the first item:
  if ((idx<1)|(idx>=static_cast<int>(slHistory.size())))
  	return;
  //swap
  QString qs;
  int i;
  for (i=idx;i>0;i--)
  {
   qs=slHistory[i-1];
   slHistory[i-1]=slHistory[i];
   slHistory[i]=qs;
  }
 // renumber ids
  for (i=0;i<static_cast<int>(slHistory.size());i++)
  	{
  	 historyMenu->changeItem(i,slHistory[i]);
  	 historyMenu->setId(i,i);
  	}
}
//------------------------------------------------------------------------------------
void MainForm::leBeginChanged(const QString &qsin)
{
 int n;
 bool bok;
 
 n=qsin.toInt(&bok);
 if (bok)
 	pControl->setBegin(n);
}
//------------------------------------------------------------------------------------
void MainForm::leEndChanged(const QString &qsin)
{
 int n;
 bool bok;
 
 n=qsin.toInt(&bok);
 if (bok)
 	pControl->setEnd(n);
}
//------------------------------------------------------------------------------------
void MainForm::leStepChanged(const QString &qsin)
{
 QIntValidator *pv;
 QString qs;
 double d;
 bool bok;
 int n;
 
 pv=(QIntValidator *)lineEditStep->validator();
 qs=qsin;
 n=0;
 if (pv->validate(qs,n)!=QValidator::Acceptable)
 {
  qs.setNum(pControl->getStep());
  lineEditStep->setText(qs);
 }
 else
 {
  d=qs.toDouble(&bok);
  if (bok)
   pControl->setStep(d);
 }
}
//------------------------------------------------------------------------------------
void MainForm::leIntervalChanged(const QString &qsin)
{
 int n;
 bool bok;
 
 n=qsin.toInt(&bok);
 if (bok)
 	pControl->setInterval(n);
}
//------------------------------------------------------------------------------------
void MainForm::setOptions()
{
  NS_Control::TEditVector ve;
  
  ve.push_back(lineEditBegin);
  ve.push_back(lineEditEnd);
  ve.push_back(lineEditStep);
  ve.push_back(lineEditInterval);
  pControl->SetOptions(ve);
  anaLyapAction->setOn(pControl->doLyap());
  anaStatsAction->setOn(pControl->doStats());
}
//------------------------------------------------------------------------------------
void MainForm::toggleLyap(bool b)
{
 pControl->toggleLyap(b);
}
//------------------------------------------------------------------------------------
void MainForm::toggleStats(bool b)
{
 pControl->toggleStats(b);
}
//------------------------------------------------------------------------------------
void MainForm::newData()
{
 if (pData!=NULL)
 {
  pData->setActiveWindow();
  pData->raise();
  return;
 }
 pData=new DataForm(this);
 pData->setFont(fontGlobal);
 connect( pData, SIGNAL( closed() ), this, SLOT( killData() ) );
 pData->show();
}
//------------------------------------------------------------------------------------
void MainForm::killData()
{
 #ifdef _DEBUG
qDebug("[MainForm::killData]");
#endif
 if (pData)
 	delete pData;
 pData=NULL;
}
//------------------------------------------------------------------------------------
void MainForm::newControlGraph(QStringList *psl)
{
#ifdef _DEBUG
qDebug("[MainForm::newControlGraph]");
#endif
 int n=vGraphAction.size()+vGraph3DAction.size()+1;
 TGraphAction *p= new TGraphAction(this, n);
#ifdef _DEBUG
qDebug("[MainForm::newControlGraph] done action");
#endif
 p->addTo( WindowsMenu );
 connect( p, SIGNAL( killMe(int) ), this, SLOT( killGraph(int) ) );
 connect( p, SIGNAL( newGraphSignal() ), this, SLOT( newGraph() ) );
 vGraphAction.push_back(p);
 p->add(psl);
#ifdef _DEBUG
qDebug("[MainForm::newControlGraph] done");
#endif
}
//------------------------------------------------------------------------------------
void MainForm::newGraph()
{
#ifdef _DEBUG
qDebug("[MainForm::newGraph]");
#endif
 int n=vGraphAction.size()+vGraph3DAction.size()+1;
 TGraphAction *p= new TGraphAction(this, n);
 p->addTo( WindowsMenu );
 connect( p, SIGNAL( killMe(int) ), this, SLOT( killGraph(int) ) );
 connect( p, SIGNAL( newGraphSignal() ), this, SLOT( newGraph() ) );
 vGraphAction.push_back(p);
}
//------------------------------------------------------------------------------------
void MainForm::killGraph(int id)
{
 unsigned int i;
 QwtPlot *pp;
#ifdef _DEBUG
qDebug("[MainForm::killGraph]%d",id);
#endif
 
 for (i=0;i<vGraphAction.size();i++)
 {
  if (vGraphAction[i]->nIdx==id)
  {
   pp=vGraphAction[i]->pGraph->getGraph();
   pControl->killSeries(pp);
   TGraphActionVector::iterator it(&vGraphAction[i]);
   delete vGraphAction[i];
   vGraphAction.erase(it);
  }
 }
 // relabel the graphs
 for (i=0;i<vGraphAction.size();i++)
  vGraphAction[i]->rename(i+1);
 for (i=0;i<vGraph3DAction.size();i++)
  vGraph3DAction[i]->rename(i+1+vGraphAction.size());
 // relabel the graphs caption with the series
 for (i=0;i<vGraphAction.size();i++)
 {
   pp=vGraphAction[i]->pGraph->getGraph();  
   pControl->doRecaption(pp,pp->axisTitle(QwtPlot::xBottom),false);
 }
}
//------------------------------------------------------------------------------------
void MainForm::killGraphs()
{
 unsigned int i;
#ifdef _DEBUG
qDebug("[MainForm::killGraphs] begin.");
#endif
 
 for (i=0;i<vGraphAction.size();i++)
 {
   pControl->killSeries(vGraphAction[i]->pGraph->getGraph());
   delete vGraphAction[i];
  }
#ifdef _DEBUG
qDebug("[MainForm::killGraphs] done 2d");
#endif
 vGraphAction.clear();
 for (i=0;i<vGraph3DAction.size();i++)
 {
   pControl->killSeries(vGraph3DAction[i]->pGraph->getGraph());
   delete vGraph3DAction[i];
  }
 vGraph3DAction.clear();
#ifdef _DEBUG
qDebug("[MainForm::killGraphs] done.");
#endif
}
//------------------------------------------------------------------------------------
void MainForm::new3DGraph()
{
#ifdef _DEBUG
qDebug("[MainForm::new3DGraph]");
#endif
 int n=vGraphAction.size()+vGraph3DAction.size()+1;
 TGraph3DAction *p= new TGraph3DAction(this, n);
 p->addTo( WindowsMenu );
 connect( p, SIGNAL( killMe(int) ), this, SLOT( killGraph3D(int) ) );
 connect( p, SIGNAL( newGraph3DSignal() ), this, SLOT( new3DGraph() ) );
 vGraph3DAction.push_back(p);
}
//------------------------------------------------------------------------------------
void MainForm::newControlGraph3D(QStringList *psl)
{
#ifdef _DEBUG
qDebug("[MainForm::newControlGraph]");
#endif
 int n=vGraphAction.size()+vGraph3DAction.size()+1;
 TGraph3DAction *p= new TGraph3DAction(this, n);
 p->addTo( WindowsMenu );
 connect( p, SIGNAL( killMe(int) ), this, SLOT( killGraph3D(int) ) );
 connect( p, SIGNAL( newGraph3DSignal() ), this, SLOT( new3DGraph() ) );
 vGraph3DAction.push_back(p);
 p->add(psl);
}
//------------------------------------------------------------------------------------
void MainForm::killGraph3D(int id)
{
 unsigned int i;
#ifdef _DEBUG
qDebug("[MainForm::killGraph3D]%d",id);
#endif
 
 for (i=0;i<vGraph3DAction.size();i++)
 {
  if (vGraph3DAction[i]->nIdx==id)
  {
   pControl->killSeries(vGraph3DAction[i]->pGraph->getGraph());
   TGraph3DActionVector::iterator it(&vGraph3DAction[i]);
   delete vGraph3DAction[i];
   vGraph3DAction.erase(it);
  }
 }
 for (i=0;i<vGraphAction.size();i++)
  vGraphAction[i]->rename(i+1);
 for (i=0;i<vGraph3DAction.size();i++)
  vGraph3DAction[i]->rename(i+1+vGraphAction.size());
}
//------------------------------------------------------------------------------------
void MainForm::newIsi()
{
#ifdef _DEBUG
qDebug("[MainForm::newIsi]");
#endif
 if (pIsiForm!=NULL)
 {
  pIsiForm->setup(pControl->getAnalysis());
  pIsiForm->setActiveWindow();
  pIsiForm->raise();
  return;
 }
 pIsiForm=new PoinIsiForm(this);
 pIsiForm->setFont(fontGlobal);
 connect( pIsiForm, SIGNAL( closed() ), this, SLOT( killIsi() ) );
 connect( pControl, SIGNAL( doIsiProgress(unsigned int, unsigned int)),
 		 pIsiForm, SLOT( doIsiProgress(unsigned int, unsigned int)));
 pIsiForm->setup(pControl->getAnalysis());
 pIsiForm->show();
}
//---------------------------------------------------------------------------------------
void MainForm::killIsi()
{
 if (pIsiForm)
 	delete pIsiForm;
 pIsiForm=NULL;
}
//---------------------------------------------------------------------------------------
void MainForm::startButtonClicked()
{
 statusBar()->message( "Started model...", STATUSSHOW );
 pControl->Start();
}
//---------------------------------------------------------------------------------------
void MainForm::stopIntButtonClicked()
{
 pControl->Stop();
 statusBar()->message( "Model stopped.", STATUSSHOW );
}
//---------------------------------------------------------------------------------------
void MainForm::resetButtonClicked()
{
 pControl->Reset();
 statusBar()->message( "Model reset.", STATUSSHOW );
}
//------------------------------------------------------------------------------------
void MainForm::stopStatButtonClicked()
{
 pControl->StopStats();
 statusBar()->message( "Stopped Statistics.", STATUSSHOW );
}
//---------------------------------------------------------------------------------------
void MainForm::stopAnaButtonClicked()
{
 pControl->StopAnalysis();
 statusBar()->message( "Stopped Analysis.", STATUSSHOW );
}
//---------------------------------------------------------------------------------------
void MainForm::addLogButtonClicked()
{
 QString qs=lineEditLog->text();
 if (qs.length()>0)
 	pControl->addUserLog(qs);
}
//---------------------------------------------------------------------------------------
void MainForm::stateLogButtonClicked()
{
 pControl->logState();
}
//---------------------------------------------------------------------------------------
