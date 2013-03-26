/****************************************************************************
** Form implementation generated from reading ui file 'about.ui'
** About dialog
** Created: Sun Dec 21 17:49:09 2003
****************************************************************************/

#include "about.h"

#include <qvariant.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qprocess.h> 

#include "gpl.h"

extern QString qsApplication;
extern QString qsProgramVersion;
extern QString qsProgramUrl;

/* 
 *  Constructs a AboutForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AboutForm::AboutForm( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AboutForm" );

    pVBox=new QVBox(this);
    pVBox->setGeometry(0,0,450,370);
    pHBox=new QHBox(pVBox);
    pixmapLabel = new QLabel( pHBox, "pixmapLabel" );
    pixmapLabel->setPixmap( QPixmap::fromMimeSource( "euneurone.png" ) );
    titleLabel = new QLabel( pHBox, "titleLabel" );

    tabWidget = new QTabWidget( pVBox, "tabWidget" );
    tabWidget->setGeometry( QRect( 0, 0, 470, 180 ) );
    tabWidget->setTabPosition( QTabWidget::Top );
    
    tab = new QGroupBox(2, Qt::Horizontal, tabWidget, "tab" );

    textLabel4 = new QLabel( tab, "textLabel4" );
    tab->addSpace(0);
            
    textLabel5 = new QLabel( tab, "textLabel5" );
    textLabel3 = new QLabel( tab, "textLabel3" );
    textLabel2 = new QLabel( tab, "textLabel2" );
    textLabel1 = new QLabel( tab, "textLabel1" );
    textLabel6 = new QLabel( tab, "textLabel6" );
    textLabel7 = new QLabel( tab, "textLabel7" );
    
    tabWidget->insertTab( tab, "" );
    
    textEdit = new QTextEdit( tabWidget, "textEdit" );
    textEdit->setGeometry( QRect( 0, 0, 192, 128 ) );
    textEdit->setReadOnly(true);
    tabWidget->insertTab( textEdit, "" );
    
    urlLabel = new QLabel( pVBox, "urlLabel" );
    urlLabel->setAlignment(Qt::AlignHCenter);
    urlLabel->setGeometry( QRect( 20, 190, 430, 30 ) );
    urlLabel->setCursor(Qt::PointingHandCursor);
    
    warrentyLabel = new QLabel( pVBox, "warrentyLabel" );
    warrentyLabel->setAlignment(Qt::AlignHCenter|Qt::WordBreak);
    warrentyLabel->setGeometry( QRect( 20, 220, 430, 180 ) );
    
    pHBox=new QHBox(pVBox);
    QLabel *pl1=new QLabel(pHBox);
     pl1->setGeometry( QRect( 20, 220, 0, 0 ) );
    closeButton = new QPushButton( pHBox, "closeButton" );
    closeButton->setFixedSize( 100, 30 );
    closeButton->setDefault( TRUE );
    QLabel *pl2=new QLabel(pHBox);
    pl2->setGeometry( QRect( 20, 220, 0, 0 ) );
    
    languageChange();
    setMinimumSize(450, 380);
    setMaximumSize(450, 380);
    resize( QSize(450, 380).expandedTo(minimumSizeHint()) );
    
    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();     // returns desktop width
    int h = d->height();    // returns desktop height
    move((w/2)-(width()/2),(h/2)-(height()/2)); //center dialog
    
    connect( closeButton, SIGNAL( clicked() ), this, SLOT( clickClose() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AboutForm::~AboutForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AboutForm::languageChange()
{
    setCaption( tr( "About " ) + qsApplication);
    closeButton->setText( tr( "&Close" ) );
    closeButton->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    textEdit->setText( pGPL );
    titleLabel->setText( "<font size=+1><b>" + qsApplication + "</b></font>");
    textLabel4->setText(  "Copyright Tjeerd olde Scheper" );
    textLabel5->setText( "Department of Computing" );
    textLabel3->setText( tr( "Version: " ) + qsProgramVersion);
    textLabel2->setText( "School of Technology" );
    textLabel1->setText( __DATE__  );
    textLabel6->setText(  "Oxford Brookes University" );
#ifdef _DEBUG
    textLabel7->setText( tr( "Development Version (alpha.1)" ) );
#else
    textLabel7->setText( tr( "Release Version (beta.2)" ) );
#endif
    urlLabel->setText( "<font color=blue><u>"+qsProgramUrl+"</u></font>" );
    warrentyLabel->setText( qsApplication + tr( " comes with ABSOLUTELY NO WARRENTY. This is free software, and you are welcome to redistribute it under certain conditions, refer to the licence for details." ) );
   tabWidget->changeTab( tab, "About" );
   tabWidget->changeTab( textEdit, tr( "Licence" ) );
}

void AboutForm::clickClose()
{
 close();
}

void AboutForm::mousePressEvent(QMouseEvent*)
{
 if (urlLabel->hasMouse())
 	{
#ifdef _DEBUG
	qDebug("\t[AboutForm::mousePressEvent] Clicked on url.");
#endif
	//launch a webbrowser to go to the url:
	QProcess *proc = new QProcess( this );
#ifdef Q_WS_X11
        proc->addArgument( "konqueror");
#endif
      //  proc->addArgument( qsProgramUrl );
        
	if (!proc->start()) 
		urlLabel->setText( "<font color=red>Can not start external webbrowser</font>" );
	}
}
