/****************************************************************************
* 
****************************************************************************/
#include <qapplication.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qfiledialog.h>
#include <qwidgetlist.h>
#include <vector>

#include "controlobject.h"
#include "graph3dform.h"

extern NS_Control::Control *pControl;

//========================================================
TGraph3DForm::TGraph3DForm( QWidget* parent) : QMainWindow(NULL,"TGraph3DForm",0)
{
 
 Owner=parent;
 statusBar();
  //check for other glwidgets
 bool b=false;
 QWidgetList  *list = QApplication::allWidgets();
 QWidgetListIt it( *list );         // iterate over the widgets
 QWidget * w;
 while ( (w=it.current()) != 0 )
  {
    ++it;
    if (w->inherits("QGLWidget"))
    {//attempt to share display list
      b=true;
      pGraph=new TGlGraph(this,"TGlGraph",(QGLWidget *)w);
      break;
    }
   }
 delete list;
 if (!b)
 	pGraph=new TGlGraph(this,"TGlGraph");
 setCentralWidget(pGraph);
 
 setIcon(QPixmap::fromMimeSource("euneurone.png"));
 
 fileSaveAction = new QAction( this, "fileSaveAction" );
 fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave.png" ) ) );
 filePrintAction = new QAction( this, "filePrintAction" );
 filePrintAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileprint.png" ) ) );
 fileCloseAction = new QAction( this, "fileCloseAction" );
 fileCloseAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileclose.png" ) ) );

 toolBar = new QToolBar( "", this); 
 toolBar->setLabel("3D graph tools");
 fileSaveAction->addTo( toolBar );
 filePrintAction->addTo( toolBar );
 toolBar->addSeparator();
 
 menubar = new QMenuBar( this, "menubar" );
 menubar->setGeometry( QRect( 0, 0, 150, 24 ) );
 fileMenu = new QPopupMenu( this );
 fileMenu->insertSeparator();
 fileSaveAction->addTo( fileMenu );
 filePrintAction->addTo( fileMenu );
 fileMenu->insertSeparator();
 fileCloseAction->addTo( fileMenu );
 
 menubar->insertItem( "&File", fileMenu ); 
 
 fileSaveAction->setText( tr( "Save" ) );
 fileSaveAction->setMenuText( tr( "&Save..." ) );
 fileSaveAction->setAccel( tr( "Ctrl+S" ) );
 filePrintAction->setText( tr( "Print" ) );
 filePrintAction->setMenuText( tr( "&Print..." ) );
 filePrintAction->setAccel( tr( "Ctrl+P" ) );
 fileCloseAction->setText( tr( "Close" ) );
 fileCloseAction->setMenuText( tr( "&Close" ) );
 fileCloseAction->setAccel( tr( "Alt+F4" ) );
  
 plbMouse=new QLabel(toolBar);
 pcbMouse=new QComboBox(toolBar);
 plbMouse->setBuddy(pcbMouse);
 plbMouse->setText( tr("&Mouse"));
 pcbMouse->insertItem("Select");
 pcbMouse->insertItem("Rotate");
 pcbMouse->insertItem("Zoom");
 pcbMouse->insertItem("Reset");
 
 nMouse=0;

 setCaption("3D Graph");
 nType=0;

 resize( QSize(300, 275).expandedTo(minimumSizeHint()) );
 
 connect( pcbMouse, SIGNAL( activated(int)) , this, SLOT( selectMouseCombo(int)) );
 connect( filePrintAction, SIGNAL( activated() ), this, SLOT( printData()));
 connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( saveData()));
 connect( fileCloseAction, SIGNAL( activated() ), this, SLOT( close()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
TGraph3DForm::~TGraph3DForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void TGraph3DForm::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    emit closed();
}

void TGraph3DForm::keyPressEvent(QKeyEvent* e)
{
#ifdef _DEBUG
 qDebug("[TGraph3DForm::keypresssevent] %d",e->isAccepted());
#endif
 if ( (e->key() == Key_F) && ( e->state() & ControlButton ) )
    {
      e->accept();
      emit newGraphSignal();
    }
    else
    {
      e->ignore();
      QMainWindow::keyPressEvent( e );
    }
}

void TGraph3DForm::rename(QString &qs)
{
  setCaption(qs);
}

void TGraph3DForm::setType(int n)
{
 nType=n;
}


void TGraph3DForm::addVariables(QStringList *psl)
{ 
 pGraph->addVariables(psl);
}

void TGraph3DForm::printData()
{
}

void TGraph3DForm::saveData()
{
}

void TGraph3DForm::selectMouseCombo(int index)
{
 switch(index)
 {
  default:
  case 0:	pGraph->setMouseAction(emSelect); break;
  case 1:	pGraph->setMouseAction(emRotate); break;
  case 2:	pGraph->setMouseAction(emZoom); break;
 }
 nMouse=index;
}
