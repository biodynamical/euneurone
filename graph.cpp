// graph..cpp

#include <stdexcept>
#include <qwidget.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qstatusbar.h>
#include <qprinter.h>
#include <qfiledialog.h> 
#include <qinputdialog.h>
#include <qpicture.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include "controlobject.h"
#include "graph.h"

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#include <qwt5/qwt_plot_canvas.h>
#include <qwt5/qwt_plot_printfilter.h>
#else
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_canvas.h>
#include <qwt/qwt_plot_printfilter.h>
#endif

#define VALSTATUSSHOW 30000

extern QApplication *pApp;
extern NS_Control::Control *pControl;
extern QPrinter *pPrinter;
extern const char *achTime;
extern const char *achLyap;
extern const char *achRecur;
extern const char *achMaxLyap;
extern const char *achPeriod;
extern const char *achPower;
extern const char *achPoincare;
extern const char *achSpike;

const char *achNoLyapX="Can not plot Lyapunov exponents as ordinal.";
const char *achNoEvol="Can not plot this function versus arbitrary variables, only against time.";
const char *achFileExists="File Already Exists";

TPlotCanvas::TPlotCanvas(QwtPlot *plot) : QwtPlotCanvas( plot )
{
}

QPixmap * TPlotCanvas::getPixmap()
{
     return this->cache();
}

//=============================================================
TGraph::TGraph(QWidget *parent) : QwtPlot(parent,"QWTGraph")
{
 setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
 setMargin(5);
 type=ctiNone;
 enableAxis(QwtPlot::yLeft,true);
 enableAxis(QwtPlot::xBottom,true);
 //visibleAxis(QwtPlot::yLeft,true);
 //visibleAxis(QwtPlot::xBottom,true);
 enableGridX(true);
 enableGridY(true);
 setCanvasBackground(white);
 setAutoReplot(false);
 setAcceptDrops(true);
 setAutoLegend(true);
 setLegendPos(Qwt::Right);
 setLegendFrameStyle(QFrame::WinPanel|QFrame::Sunken);
 qsXLabel=achTime;
 gtX.setTitle(achTime);
 gtX.setPlot(this);
 setAxisTitle(QwtPlot::xBottom,achTime);
#ifdef _DEBUG
qDebug("[TGraph::TGraph] done");
#endif

 //setAxisFont(QwtPlot::yLeft,parent->font());
}

void TGraph::dragEnterEvent(QDragEnterEvent* event)
{
 bool b;
 
 b=NS_Control::TDragObject::canDecode(event);
 if (b)
 {
  QWidget *p=(QWidget *)parent();
  p->setActiveWindow();
  QTimer::singleShot(500,p,SLOT(raise()));
 }
 event->accept(b);    
}

void TGraph::dragMoveEvent(QDragMoveEvent* event)
{
 if (NS_Control::TDragObject::canDecode(event))
 {
  QRect rx,ry,rc;
  //only accept drops on the axes
  const QwtScaleDraw *pX,*pY;
   
   rc=canvas()->rect();
   pX=axisScaleDraw(QwtPlot::xBottom);
   rx.setLeft(pX->x());
   rx.setTop(pX->y());
   rx.setWidth(pX->maxWidth(QPen(),fontMetrics()));
   rx.setHeight(pX->maxHeight(QPen(),fontMetrics()));
   
   pY=axisScaleDraw(QwtPlot::yLeft);
   ry.setLeft(pY->x());
   ry.setTop(pY->y());
   ry.setWidth(pY->maxWidth(QPen(),fontMetrics()));
   ry.setHeight(pY->maxHeight(QPen(),fontMetrics()));
   
   ry.moveBy(pY->majTickLength()-ry.width(),rc.top());
   rx.moveBy(rc.left()+ry.right(),rc.bottom()+margin());
   if (rx.contains(event->pos(),true)||ry.contains(event->pos(),true))
  	event->accept();
   else
   	event->ignore();
 }
}

void TGraph::dragLeaveEvent(QDragLeaveEvent*)
{
}

void TGraph::dropEvent(QDropEvent* event)
{
 unsigned int nId;
 
 try
 {
  if ( NS_Control::TDragObject::decode(event, &nId) ) 
  {
   QRect rx,ry,rc;
   //find where the drop is
   const QwtScaleDraw *pX,*pY;
  
   rc=canvas()->rect();
   pX=axisScaleDraw(QwtPlot::xBottom);
   rx.setLeft(pX->x());
   rx.setTop(pX->y());
   rx.setWidth(pX->maxWidth(QPen(),fontMetrics()));
   rx.setHeight(pX->maxHeight(QPen(),fontMetrics()));
   
   pY=axisScaleDraw(QwtPlot::yLeft);
   ry.setLeft(pY->x());
   ry.setTop(pY->y());
   ry.setWidth(pY->maxWidth(QPen(),fontMetrics()));
   ry.setHeight(pY->maxHeight(QPen(),fontMetrics()));
   
   ry.moveBy(pY->majTickLength()-ry.width(),rc.top());
   rx.moveBy(rc.left()+ry.right(),rc.bottom()+margin());
   if (rx.contains(event->pos(),true))
   { // xaxis
    //check if it is a analysis element
#ifdef _DEBUG
qDebug("[TGraph::dropEvent]xaxis nid=%x",nId);
#endif
    switch(nId)
    {
     case IDINVALID: return;
     case IDSTATS: break;
     case IDLYAPUNOV:
     		{
		 if (type!=ctiNone)
		 	break;
		 throw std::logic_error(achNoLyapX);
		}
     case IDRECURRENCE:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiRecur;
		 addRecur();
		 break;
		}
     case IDMAXLYAP:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiMaxLyap;
		 addMaxLyap();
		 break;
		}
     case IDPERIOD:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiPeriod;
		 addPeriod();
		 break;
		}
     case IDPOWER:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiPower;
		 addPower();
		 break;
		}
     default:
     		{
		 if (type==ctiNone)
		 	type=ctiNormal;
		 setXVariable(nId);
		 break;
		}
    }
   }
   else if (ry.contains(event->pos(),true))
   {//y axis
#ifdef _DEBUG
qDebug("[TGraph::dropEvent] yaxis nid=%x",nId);
#endif
    //check if it is a analysis element
    switch(nId)
    {
     case IDINVALID: return;
     case IDSTATS: break;
     case IDLYAPUNOV: 
     		{
		 if (!isEvolution())
		 	throw std::logic_error(achNoEvol);
		 if (type!=ctiNone)
		 	break;
		 type=ctiLyapunov;
		 addLyapunov();
		 break;
		}
     case IDRECURRENCE:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiRecur;
		 addRecur();
		 break;
		}
     case IDMAXLYAP:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiMaxLyap;
		 addMaxLyap();
		 break;
		}
     case IDPOINCARE1D:
     		{
		 if (type!=ctiNormal)
		 	break;
		 addPoincare();
		 break;
		}
     case IDSPIKE: 
     		{
		 if (!isEvolution())
		 	throw std::logic_error(achNoEvol);
		 if (type!=ctiNormal)
		 	break;
		 type=ctiSpike;
		 addSpike();
		 break;
		}
     case IDPERIOD:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiPeriod;
		 addPeriod();
		 break;
		}
     case IDPOWER:
     		{
		 if (type!=ctiNone)
		 	break;
		 type=ctiPower;
		 addPower();
		 break;
		}
     default:
     		{
		 if (type==ctiNone)
		 	type=ctiNormal;
		 addYVariable(nId);
		 break;
		}
    }
   }
   replot();
  }
 }
  catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
     }
}

void TGraph::addVariables(QStringList *psl)
{ 
 unsigned int i,id;
 
 type=ctiNormal;
 if (psl->size()>0)
 {
  id=pControl->GetVarIdx(*psl->at(0));
  if (id!=IDINVALID)
  	setXVariable(id);
 }
 for (i=1;i<psl->size();i++)
 {
  id=pControl->GetVarIdx(*psl->at(i));
  if (id!=IDINVALID)
  	addYVariable(id);
 }
 pControl->doRecaption(this,gtX.getTitle(),false);
}

void TGraph::setXVariable(unsigned int n)
{ 
  if (!pControl->GetVarStruct(n,gtX))
    return;
  switch(type)
  {
   case ctiLyapunov:
   case ctiStats:
   case ctiNone: break;
   case ctiPeriod:
   case ctiPower:
   case ctiMaxLyap:
   case ctiRecur:
   	{
	 addYVariable(n);
	 break;
	}
   default:
	{
  	 qsXLabel=gtX.getTitle();
  	 setAxisTitle(QwtPlot::xBottom,qsXLabel);
  	 pControl->UpdateXVars(gtX);
	// replot();
#ifdef _DEBUG
qDebug("[TGraph::setXVariable] n=%x (%s)",n,qsXLabel.ascii());
#endif
	 break;
	}
 }
 pControl->doRecaption(this,gtX.getTitle(),false);
}

void TGraph::addYVariable(unsigned int n)
{ 
 graphType gtY;
 
 if (!pControl->GetVarStruct(n,gtY))
   return;
  switch(type)
  {
   case ctiLyapunov:
   case ctiStats: break;
   case ctiRecur:
   	{
  	 gtY.setPlot(this);
  	 pControl->addRecur(gtY);
	 break;
	}
   case ctiMaxLyap:
   	{
  	 gtY.setPlot(this);
  	 pControl->addMaxLyap(gtY);
	 break;
	}
   case ctiPeriod:
   	{
  	 gtY.setPlot(this);
  	 pControl->addPeriod(gtY);
	 break;
	}
   case ctiPower:
   	{
  	 gtY.setPlot(this);
  	 pControl->addPower(gtY);
	 break;
	}
/*   case ctiPoincare:
   	{
  	 gtY.setPlot(this);
  	 pControl->addPoincare(gtY);
	 break;
	}*/
   case ctiNone:
   	{
	 type=ctiNormal;
	}
   default:
   	{
  	 gtY.setPlot(this);
   	 //create curve
	 gtY.newCurve(gtX.idY(),gtY.idY());
  	 pControl->addSeries(gtY);
 	 pControl->doRecaption(this,gtX.getTitle(),false);
	 break;
	}
 }
}

void TGraph::addLyapunov()
{ 
 graphType gtY;
  
 if (type!=ctiLyapunov)
  return;
  
 gtY.setPlot(this);
 pControl->addLyapunov(gtY);
 qsTitle=achLyap;
 enableTitle(true);
 pControl->doRecaption(this,qsTitle,true);
}

void TGraph::addRecur()
{ 
  if (type!=ctiRecur)
   return;
 
 qsTitle=achRecur;
 enableTitle(true);
 enableXLabel(false);
 pControl->doRecaption(this,qsTitle,true);
}

void TGraph::addMaxLyap()
{ 
  if (type!=ctiMaxLyap)
   return;
 
 qsTitle=achMaxLyap;
 enableTitle(true);
 enableXLabel(false);
 pControl->doRecaption(this,qsTitle,true);
}

void TGraph::addPoincare()
{ 
 pControl->addPoincare(this);
 qsTitle=achPoincare;
 pControl->doRecaption(this,qsTitle,false);
}

void TGraph::addSpike()
{ 
 pControl->addSpike(this);
 qsTitle=achSpike;
 pControl->doRecaption(this,qsTitle,false);
}

void TGraph::addPeriod()
{ 
  if (type!=ctiPeriod)
   return;
 
 qsTitle=achPeriod;
 enableTitle(true);
 enableXLabel(false);
 changeAxisOptions(QwtPlot::yLeft,QwtAutoScale::Logarithmic, true);
 pControl->doRecaption(this,qsTitle,true);
}

void TGraph::addPower()
{ 
  if (type!=ctiPower)
   return;
 
 qsTitle=achPower;
 enableTitle(true);
 enableXLabel(false);
 changeAxisOptions(QwtPlot::yLeft,QwtAutoScale::Logarithmic, true);
 pControl->doRecaption(this,qsTitle,true);
}

bool TGraph::isEvolution()
{
 if (gtX.idX()==0)
 	return true;
 return false;
}

void TGraph::enableTitle(bool b)
{
 if (b)
 {
   setTitle(qsTitle);
   updateLayout();
 }
 else
   setTitle(QString::null);
}

void TGraph::enableXLabel(bool b)
{
 if (b)
 {
  if (!qsXLabel.isNull())
  	setAxisTitle(QwtPlot::xBottom,qsXLabel);
 }
 else
 {
   setAxisTitle(QwtPlot::xBottom,QString::null);
 }
}
//===============================================================
TGraphForm::TGraphForm(QWidget *parent) : QMainWindow(NULL,"Graph",0)
{
 Owner=parent;
 
 QStatusBar *pStatus=statusBar();
 pGraph=new TGraph(this);
 setGeometry(0,0,475,350);
 setCentralWidget(pGraph);
 
 setIcon(QPixmap::fromMimeSource("euneurone.png"));

  // toolbars
 fileSaveAction = new QAction( this, "fileSaveAction" );
 fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave.png" ) ) );
 filePrintAction = new QAction( this, "filePrintAction" );
 filePrintAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileprint.png" ) ) );
 fileCloseAction = new QAction( this, "fileCloseAction" );
 fileCloseAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileclose.png" ) ) );
 updateAction = new QAction( this, "updateAction" );
 updateAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "update.png" ) ) );

 viewLegendAction = new QAction( this, "viewLegendAction" );
 viewLegendAction->setToggleAction(true);
 viewLegendAction->setOn(true);
 viewTitleAction = new QAction( this, "viewTitleAction" );
 viewTitleAction->setToggleAction(true);
 viewTitleAction->setOn(false);
 viewXLabelAction = new QAction( this, "viewXLabelAction" );
 viewXLabelAction->setToggleAction(true);
 viewXLabelAction->setOn(true);
 viewXAxisAction = new QAction( this, "viewXAxisAction" );
 viewXAxisAction->setToggleAction(true);
 viewXAxisAction->setOn(true);
 viewYAxisAction = new QAction( this, "viewYAxisAction" );
 viewYAxisAction->setToggleAction(true);
 viewYAxisAction->setOn(true);
 viewGridAction = new QAction( this, "viewGridAction" );
 viewGridAction->setToggleAction(true);
 viewGridAction->setOn(true);

 unZoomAction = new QAction( this, "unZoomAction" );
 unZoomAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "undozoom.png" ) ) );
 
 menubar = new QMenuBar( this, "menubar" );

 menubar->setGeometry( QRect( 0, 0, 150, 24 ) );
 fileMenu = new QPopupMenu( this );
 fileSaveAction->addTo( fileMenu );
 filePrintAction->addTo( fileMenu );
 fileMenu->insertSeparator();
 fileCloseAction->addTo( fileMenu );
 viewMenu = new QPopupMenu( this );
 viewLegendAction->addTo(viewMenu);
 viewTitleAction->addTo(viewMenu);
 viewGridAction->addTo(viewMenu);
 viewXLabelAction->addTo(viewMenu);
 viewXAxisAction->addTo(viewMenu);
 viewYAxisAction->addTo(viewMenu);

 menubar->insertItem( "&File", fileMenu ); 
 menubar->insertItem( "&View", viewMenu); 

 toolBar = new QToolBar( "", this); 
 toolBar->setLabel("Graph tools");
 plbMouse=new QLabel(toolBar);
 pcbMouse=new QComboBox(toolBar);
 plbMouse->setBuddy(pcbMouse);
 unZoomAction->addTo(toolBar);
 toolBar->addSeparator();
 fileSaveAction->addTo( toolBar );
 filePrintAction->addTo( toolBar );
 toolBar->addSeparator();
 updateAction->addTo( toolBar );
 
 plbMouse->setText( tr("&Mouse"));
 
 fileSaveAction->setText( tr( "Save" ) );
 fileSaveAction->setMenuText( tr( "&Save..." ) );
 fileSaveAction->setAccel( tr( "Ctrl+S" ) );
 filePrintAction->setText( tr( "Print" ) );
 filePrintAction->setMenuText( tr( "&Print..." ) );
 filePrintAction->setAccel( tr( "Ctrl+P" ) );
 fileCloseAction->setText( tr( "Close" ) );
 fileCloseAction->setMenuText( tr( "&Close" ) );
 fileCloseAction->setAccel( tr( "Alt+F4" ) );
 updateAction->setText( tr( "Update" ) );
 updateAction->setMenuText( tr( "&Update" ) );
 updateAction->setAccel( tr( "Ctrl+U" ) );
 
 viewLegendAction->setText( tr( "Legend" ) );
 viewLegendAction->setMenuText( tr( "&Legend" ) );
 viewLegendAction->setAccel( tr( "Ctrl+L" ) ); 
 viewTitleAction->setText( tr( "Title" ) );
 viewTitleAction->setMenuText( tr( "&Title" ) );
 viewTitleAction->setAccel( tr( "Ctrl+T" ) );
 viewXLabelAction->setText( tr( "Label X axis" ) );
 viewXLabelAction->setMenuText( tr( "La&bel X axis" ) );
 viewXLabelAction->setAccel( tr( "Ctrl+B" ) );
 viewXAxisAction->setText( tr( "X axis" ) );
 viewXAxisAction->setMenuText( tr( "&X axis" ) );
 viewXAxisAction->setAccel( tr( "Ctrl+X" ) );
 viewYAxisAction->setText( tr( "Y axis" ) );
 viewYAxisAction->setMenuText( tr( "&Y axis" ) );
 viewYAxisAction->setAccel( tr( "Ctrl+Y" ) );
 viewGridAction->setText( tr( "Grid" ) );
 viewGridAction->setMenuText( tr( "&Grid" ) );
 viewGridAction->setAccel( tr( "Ctrl+R" ) );

 unZoomAction->setEnabled(false);

 pcbMouse->insertItem("Values");
 pcbMouse->insertItem("Zoom");
 pcbMouse->insertItem("Reset");
 
 pProgress=new QProgressBar(pStatus);
 pStatus->addWidget(pProgress,0,true);
 pProgress->hide();
 pStop=new QToolButton(pStatus);
 pStop->setIconSet( QIconSet( QPixmap::fromMimeSource( "stop.png" ) ) );
 pStatus->addWidget(pStop,0,true); 
 pStop->hide();
 
 nMouse=0;
 bLegend=true;
 bXLabel=true;
 bXAxis=true;
 bYAxis=true;
 bTitle=false;
 bGrid=true;
 //setup signals
 connect( pGraph, SIGNAL(legendClicked(long)) , this, SLOT( editSeries(long)) );
 connect( pGraph, SIGNAL(plotMouseMoved(const QMouseEvent &)) , this, SLOT( mouseMove(const QMouseEvent &)) );
 connect( pGraph, SIGNAL(plotMousePressed(const QMouseEvent &)) , this, SLOT( mousePressed(const QMouseEvent &)) );
 connect( pGraph, SIGNAL(plotMouseReleased(const QMouseEvent &)) , this, SLOT( mouseReleased(const QMouseEvent &)) );
 connect( pStop, SIGNAL( clicked() ), this, SLOT( clickedStop()));
 connect( pcbMouse, SIGNAL( activated(int)) , this, SLOT( selectMouseCombo(int)) );
 connect( filePrintAction, SIGNAL( activated() ), this, SLOT( printGraph()));
 connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( saveGraph()));
 connect( fileCloseAction, SIGNAL( activated() ), this, SLOT( close()));
 connect( updateAction, SIGNAL( activated() ), this, SLOT( updatePlot()));
 connect( viewLegendAction, SIGNAL( toggled( bool )), this, SLOT( toggleLegend( bool)));
 connect( viewTitleAction, SIGNAL( toggled( bool )), this, SLOT( toggleTitle( bool)));
 connect( viewXLabelAction, SIGNAL( toggled( bool )), this, SLOT( toggleXLabel( bool)));
 connect( viewXAxisAction, SIGNAL( toggled( bool )), this, SLOT( toggleXAxis( bool)));
 connect( viewYAxisAction, SIGNAL( toggled( bool )), this, SLOT( toggleYAxis( bool)));
 connect( viewGridAction, SIGNAL( toggled( bool )), this, SLOT( toggleGrid( bool)));
 connect( unZoomAction, SIGNAL( activated() ), this, SLOT( doUnzoom()));
}

TGraphForm::~TGraphForm()
{//--
}

void TGraphForm::closeEvent(QCloseEvent* e)
{
    QMainWindow::closeEvent(e);
    emit closed();
}

void TGraphForm::keyPressEvent(QKeyEvent* e)
{
#ifdef _DEBUG
 qDebug("[TGraphForm::keypresssevent] %d",e->isAccepted());
#endif
 if ( (e->key() == Key_G) && ( e->state() & ControlButton ) )
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

void TGraphForm::rename(QString &qs)
{
  setCaption(qs);
}

void TGraphForm::addVariables(QStringList *psl)
{ 
 pGraph->addVariables(psl);
}

void TGraphForm::mousePressed(const QMouseEvent &e)
{
 switch(nMouse)
 {
  default:
  case 0:
  	{
	 pGraph->setOutlineStyle(Qwt::Cross);
	 pGraph->enableOutline(true);
	 showValues(e.x(),e.y());
	 break;
	}
  case 1:
  	{
	  pGraph->setOutlineStyle(Qwt::Rect);
	  pGraph->enableOutline(true);
	  rZoom.setCoords(e.x(),e.y(),e.x(),e.y());
	 break;
	}
  case 2:
  	{
	 break;
	}
 }
}

void TGraphForm::mouseMove(const QMouseEvent &e)
{
 switch(nMouse)
 {
  default:
  case 0:
  	{
	 showValues(e.x(),e.y());
	 break;
	}
  case 1:
  	{
	 rZoom.setRight(e.x());
	 rZoom.setBottom(e.y());
	 break;
	}
  case 2:
  	{
	 break;
	}
 }
}

void TGraphForm::mouseReleased(const QMouseEvent &)
{
 switch(nMouse)
 {
  default:
  case 0:
  	{
	 pGraph->enableOutline(false);
	 break;
	}
  case 1:
  	{
	 pGraph->enableOutline(false);
	 doZoom();
	 break;
	}
  case 2:
  	{
	 break;
	}
 }
}

void TGraphForm::doZoom()
{
  double dx1,dy1,dx2,dy2;

 //first check is it is empty
 if (rZoom.isEmpty())
 	return;
 rZoom=rZoom.normalize(); //swap left and right if necessary
 dx1=pGraph->invTransform(QwtPlot::xBottom,rZoom.left());
 dx2=pGraph->invTransform(QwtPlot::xBottom,rZoom.right());
 dy1=pGraph->invTransform(QwtPlot::yLeft,rZoom.top());
 dy2=pGraph->invTransform(QwtPlot::yLeft,rZoom.bottom());
 pGraph->setAxisScale(QwtPlot::xBottom,dx1,dx2);
 pGraph->setAxisScale(QwtPlot::yLeft,dy1,dy2);
 pGraph->replot();
 dsZoom.push(dx1);
 dsZoom.push(dy1);
 dsZoom.push(dx2);
 dsZoom.push(dy2);
 if (!unZoomAction->isEnabled())
 	unZoomAction->setEnabled(true);
}

void TGraphForm::showValues(int x, int y)
{  
  int dist,idx;
  double dx,dy;
  long lc;

  lc=pGraph->closestCurve(x,y,dist,dx,dy,idx);
  if (lc==0)
   return;  
  qsStatus.sprintf("%s: X=%g, Y=%g",pControl->getSeriesTitle(pGraph,lc),dx,dy);
  statusBar()->message( qsStatus, VALSTATUSSHOW );
}

void TGraphForm::selectMouseCombo(int index)
{
 if (index==2) 
 {//reset
  pGraph->setAxisAutoScale(QwtPlot::xBottom);
  pGraph->setAxisAutoScale(QwtPlot::yLeft);
  pGraph->replot();
  index=0;
  pcbMouse->setCurrentItem(index);
 }
 nMouse=index;
}

void TGraphForm::doUnzoom()
{
  double dx1,dy1,dx2,dy2;

  if (dsZoom.size()<=4)
 	{
 	 pGraph->setAxisAutoScale(QwtPlot::xBottom);
 	 pGraph->setAxisAutoScale(QwtPlot::yLeft);
	 pGraph->replot();
 	 unZoomAction->setEnabled(false);
	 while(dsZoom.size()>0)
	 	dsZoom.pop();
	 return;
	}
 for (int i=0;i<4;i++) dsZoom.pop(); //remove top 4 elements
 dy2=dsZoom.top();
 dsZoom.pop(); 
 dx2=dsZoom.top();
 dsZoom.pop(); 
 dy1=dsZoom.top();
 dsZoom.pop(); 
 dx1=dsZoom.top();
 dsZoom.pop(); 
 pGraph->setAxisScale(QwtPlot::xBottom,dx1,dx2);
 pGraph->setAxisScale(QwtPlot::yLeft,dy1,dy2);
 pGraph->replot();
}

void TGraphForm::toggleLegend(bool b)
{
 if (bLegend!=b)
 {
  pGraph->enableLegend(b);
  bLegend=b;
  viewLegendAction->setOn(b);
  pGraph->replot();
 }
}

void TGraphForm::toggleTitle(bool b)
{
 if (bTitle!=b)
 {
  if (b)
  {
   bool ok;
   QString text = QInputDialog::getText(caption(), "Enter a title:", QLineEdit::Normal,pGraph->qsTitle, &ok, this );
   if ((!ok) || text.isEmpty())
   {
    viewTitleAction->setOn(false);
    bTitle=false;
    return;
   }
   pGraph->qsTitle=text;
  }
  pGraph->enableTitle(b);
  bTitle=b;
  viewTitleAction->setOn(b);
  pGraph->replot();
 }
}

void TGraphForm::toggleXLabel(bool b)
{
 if (bXLabel!=b)
 {
  pGraph->enableXLabel(b);
  bXLabel=b;
  viewXLabelAction->setOn(b);
  pGraph->replot();
}
}

void TGraphForm::toggleXAxis(bool b)
{
 if (bXAxis!=b)
 {
//  pGraph->visibleAxis(QwtPlot::xBottom,b);
  bXAxis=b;
  viewXAxisAction->setOn(b);
  pGraph->replot();
 }
}

void TGraphForm::toggleYAxis(bool b)
{
 if (bYAxis!=b)
 {
//  pGraph->visibleAxis(QwtPlot::yLeft,b);
  bYAxis=b;
  viewYAxisAction->setOn(b);
  pGraph->replot();
 }
}

void TGraphForm::toggleGrid(bool b)
{
 if (bGrid!=b)
 {
  pGraph->enableGridX(b);
  pGraph->enableGridY(b);
  bGrid=b;
  viewGridAction->setOn(b);
  pGraph->replot();
 }
}

void TGraphForm::printGraph()
{
 pPrinter->setOrientation(QPrinter::Landscape);
 pPrinter->setColorMode(QPrinter::Color);
 pPrinter->setOutputToFile(false);
 if ( pPrinter->setup(this) )
 	pGraph->print(*pPrinter);
}
 
void TGraphForm::saveGraph()
{
 QString fn,fc;
 int i;
 //try to get a reasonable filename
 fn=pControl->getFileName();
 i=fn.findRev('.');
 if (i>=0)
 	fn.truncate(i);
 fc=caption().section(' ',2,2);
 // append part of caption
 fn.append(fc);
 fn.append(".eps");
#ifdef _DEBUG
 qDebug("name=%s, cap=%s",fn.ascii(),fc.ascii());
#endif
 fn = QFileDialog::getSaveFileName( fn, "Encapsulated Postscript (*.eps);;Portable Network Graphic (*.png)", this, NULL, "Open a file" );
 if ( !fn.isEmpty() )
 {
#ifdef _DEBUG
 qDebug("name=%s",fn.ascii());
#endif
 if ( QFile::exists(fn))
 {
  if (QMessageBox::question(this,achFileExists,QString().sprintf("The file\n%s\nalready exists. Overwrite ?",fn.ascii()),QMessageBox::Yes,QMessageBox::No)!=QMessageBox::Yes)
  	return;
 }
 if (fn.contains(".ps") || fn.contains(".eps"))
 {
  pPrinter->setOrientation(QPrinter::Landscape);
  pPrinter->setColorMode(QPrinter::Color);
  pPrinter->setOutputToFile(true);
  pPrinter->setOutputFileName(fn);
  pPrinter->setPageSize(QPrinter::A6); //This produces reasonable results for the font sizes
  pGraph->print(*pPrinter);
 }
 else
 {
 //FIXME: This ought to work but doesn't, don't know why not.
/* QPicture pict;
 pGraph->print(pict);
 pict.save(fn,"svg");
 pict.save(fn);*/
/* QImage qi(640,480,32);
 QPainter qp;
 qp.begin(&qi);
 pGraph->print(&qi,640,480);
 qp.end();
 qi.save("Test.png");*/
     QPixmap *image;
     TPlotCanvas *canvas;
     
     canvas = (TPlotCanvas *)pGraph->canvas();
     image = canvas->getPixmap();
     image->save(fn, "PNG"); 
 }
 }
}

void TGraphForm::editSeries(long key)
{
 pControl->editSeries(pGraph,key);
}

void TGraphForm::hideProgress()
{
 pProgress->hide();
 pStop->hide();
}

void TGraphForm::setProgress(unsigned int max, unsigned int cur)
{
 if (!pStop->isVisible())
   pStop->show();
 if (!pProgress->isVisible())
   pProgress->show();
 pProgress->setProgress(cur,max);
 pApp->processEvents();
}

void TGraphForm::clickedStop()
{
 pControl->stopAnalysis(pGraph);
 hideProgress();
}

void TGraphForm::updatePlot()
{
 pControl->UpdatePlot(pGraph);
}
