/****************************************************************************
** Form implementation generated from reading ui file 'dataform.ui'
**
** Created: Mon Aug 18 21:15:43 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)

****************************************************************************/
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtimer.h>
 #include <qmessagebox.h>
#include <vector>

#include "controlobject.h"
#include "dataform.h"

using namespace NS_Control;

const char *achCSV="Comma Separated Values (CSV)";
const char *achTSV="Tab Separated Values (TSV)";
const char *achMat="Matlab file (MAT)";
const char *achCDF="Common Data Format (CDF)";
const char *achHDF="Hierarchical Data Format (HDF)";
const char *achHDF5="Hierarchical Data Format 5 (H5)";

extern NS_Control::Control *pControl;

TVarTable::TVarTable( QWidget *parent ) : QListBox(parent)
{
setAcceptDrops(true);
}

void TVarTable::dragEnterEvent(QDragEnterEvent* event)
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

void TVarTable::dragMoveEvent(QDragMoveEvent* event)
{
 if (NS_Control::TDragObject::canDecode(event))
 	event->accept();
 else
 	event->ignore();
}

void TVarTable::dragLeaveEvent(QDragLeaveEvent*)
{
}

void TVarTable::dropEvent(QDropEvent* event)
{
 unsigned int nId;
 
 try
 {
  if ( NS_Control::TDragObject::decode(event, &nId) ) 
 	pControl->AddDataStore(this, nId);
 }
  catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
     }

}

/* 
 *  Constructs a DataForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
DataForm::DataForm( QWidget* parent) : QMainWindow(NULL,"DataForm",0)
{
 QRadioButton *prb;
 int c=0;
 
 Owner=parent;
 statusBar();
 pBox=new QHBox(this);
 setCentralWidget(pBox);
 setIcon(QPixmap::fromMimeSource("euneurone.png"));

 pGroup = new QButtonGroup(pBox);
 pGroup->setGeometry( QRect( 10, 5, 150, 160 ) );
 
 prb=new QRadioButton(achCSV, pGroup);
 prb->setGeometry( 5, 20, 280, 25 );
 prb->setChecked( TRUE );
 c++;
 prb=new QRadioButton(achTSV, pGroup);
 prb->setGeometry( 5, 45, 280, 25 );
 c++;
#ifdef MAT_FILE
 prb=new QRadioButton(achMat, pGroup);
 prb->setGeometry( 5, 20+c*25, 280, 25 );
 c++;
#endif
#ifdef CDF_FILE
 prb=new QRadioButton(achCDF, pGroup);
 prb->setGeometry( 5, 20+c*25, 280, 25 );
 c++;
#endif
#ifdef HDF_FILE
 prb=new QRadioButton(achHDF, pGroup);
 prb->setGeometry( 5, 20+c*25, 280, 25 );
 c++;
#endif
#ifdef HDF5_FILE
 prb=new QRadioButton(achHDF5, pGroup);
 prb->setGeometry( 5, 20+c*25, 280, 25 );
 c++;
#endif
 pGroup->setTitle("Save type");
 tableData = new TVarTable( pBox );
 tableData->setGeometry( QRect( 160, 5, 120, 160 ) );
 tableData->setAcceptDrops(true);
 
 pBox->setStretchFactor(pGroup,3);
 pBox->setStretchFactor(tableData,2);
 pBox->setSpacing(5);
 
 fileClearAction = new QAction( this, "fileClearAction" );
 fileClearAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "newdata.png" ) ) );
 fileAllAction = new QAction( this, "fileAllAction" );
 fileAllAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "anadata.png" ) ) );
 fileSaveAction = new QAction( this, "fileSaveAction" );
 fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave.png" ) ) );
 filePrintAction = new QAction( this, "filePrintAction" );
 filePrintAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileprint.png" ) ) );
 fileCloseAction = new QAction( this, "fileCloseAction" );
 fileCloseAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileclose.png" ) ) );

 toolBar = new QToolBar( "", this); 
 toolBar->setLabel("Data tools");
 fileSaveAction->addTo( toolBar );
 filePrintAction->addTo( toolBar );
 toolBar->addSeparator();
 fileClearAction->addTo( toolBar );
 fileAllAction->addTo( toolBar );
  
 menubar = new QMenuBar( this, "menubar" );

 menubar->setGeometry( QRect( 0, 0, 150, 24 ) );
 fileMenu = new QPopupMenu( this );
 fileClearAction->addTo( fileMenu );
 fileAllAction->addTo( fileMenu );
 fileMenu->insertSeparator();
 fileSaveAction->addTo( fileMenu );
 filePrintAction->addTo( fileMenu );
 fileMenu->insertSeparator();
 fileCloseAction->addTo( fileMenu );
 
 menubar->insertItem( "&File", fileMenu ); 
 
 fileClearAction->setText( tr( "Clear list" ) );
 fileClearAction->setMenuText( tr( "C&lear list" ) );
 fileClearAction->setAccel( tr( "Ctrl+C" ) );
 fileAllAction->setText( tr( "Add all" ) );
 fileAllAction->setMenuText( tr( "&Add all" ) );
 fileAllAction->setAccel( tr( "Ctrl+A" ) );
 fileSaveAction->setText( tr( "Save" ) );
 fileSaveAction->setMenuText( tr( "&Save..." ) );
 fileSaveAction->setAccel( tr( "Ctrl+S" ) );
 filePrintAction->setText( tr( "Print" ) );
 filePrintAction->setMenuText( tr( "&Print..." ) );
 filePrintAction->setAccel( tr( "Ctrl+P" ) );
 fileCloseAction->setText( tr( "Close" ) );
 fileCloseAction->setMenuText( tr( "&Close" ) );
 fileCloseAction->setAccel( tr( "Alt+F4" ) );
 
 resize( QSize(500, 260).expandedTo(minimumSizeHint()) );
 setCaption("Data Store");
 nType=stNone;
    
 connect( pGroup, SIGNAL( clicked(int) ), SLOT( setType(int) ));
 connect( fileClearAction, SIGNAL( activated() ), this, SLOT( clearList()));
 connect( fileAllAction, SIGNAL( activated() ), this, SLOT( addAllList()));
 connect( filePrintAction, SIGNAL( activated() ), this, SLOT( printData()));
 connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( saveData()));
 connect( fileCloseAction, SIGNAL( activated() ), this, SLOT( close()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DataForm::~DataForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void DataForm::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    emit closed();
}

void DataForm::rename(QString &qs)
{
  setCaption(qs);
}

void DataForm::setType(int n)
{
 if (pGroup->selected()->text()==achCSV)
		nType=stCSV;
 if (pGroup->selected()->text()==achTSV)
	 nType=stTSV;
 if (pGroup->selected()->text()==achMat)
	 nType=stMat;
 if (pGroup->selected()->text()==achCDF)
	 nType=stCDF;
 if (pGroup->selected()->text()==achHDF)
	 nType=stHDF;
 if (pGroup->selected()->text()==achHDF5)
	 nType=stHDF5;
}

void DataForm::clearList()
{
 pControl->clearDataStore();
 tableData->clear();
}

void DataForm::addAllList()
{
 tableData->clear();
 pControl->AddAllDataStore(tableData);
}

void DataForm::printData()
{
 // TODO: Print data in  color coded tables
 QMessageBox::information( this, NULL,  "This function has not been implemented yet", QMessageBox::Ok );
}

void DataForm::saveData()
{
 int c=0,i;
 TSaveDlg* fd = new TSaveDlg( this, nType);
  QString fn=pControl->getFileName();
  i=fn.findRev('.');
  if (i>=0)
	 fn.truncate(i);
  fd->setName(fn);
  if ( fd->exec() == QDialog::Accepted )
	  qsFile = fd->selectedFile();
  if (!qsFile.isEmpty())
  {
   if (fd->bgCompress!=NULL)
   {
    c=fd->bgCompress->selectedId();
    if (c<0) c=0;
   }
   pControl->saveDataStore(qsFile,nType,c);
  }
}
