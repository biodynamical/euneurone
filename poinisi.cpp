/****************************************************************************
** Created: Sat Aug 5 16:54:42 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
****************************************************************************/
#include <qapplication.h>
#include <qvariant.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtable.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <qtextedit.h>
#include <qprogressdialog.h>
#include <qdatetime.h>
#include <stdexcept>
 
#include "poinisi.h"

using namespace NS_Control;
using namespace NS_Analysis;
using namespace std;

extern NS_Control::Control *pControl;
extern QApplication *pApp;

extern const char *achIdentifier;
extern const char *achHeader;
extern const char *achComma;

const char *achIsiMask1="Spike time %1";
const char *achIsiMask2="{%1}";
const char *achIsiMask3="<%1>";

/*
 *  Constructs a Form as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
PoinIsiForm::PoinIsiForm( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "PoinIsiForm" );
    pBox=new QHBox(this);
    setCentralWidget(pBox);
    setIcon(QPixmap::fromMimeSource("euneurone.png"));

    groupBox = new QGroupBox( centralWidget(), "groupBox" );
    groupBox->setGeometry( QRect( 0, 0, 290, 340 ) );

    listBox = new QListBox( groupBox, "listBox" );
    listBox->setGeometry( QRect( 10, 20, 270, 170 ) );

    buttonGroup2 = new QButtonGroup( groupBox, "buttonGroup2" );
    buttonGroup2->setGeometry( QRect( 10, 200, 130, 120 ) );

    radioButton5 = new QRadioButton( buttonGroup2, "radioButton5" );
    radioButton5->setGeometry( QRect( 10, 40, 97, 22 ) );

    radioButton4 = new QRadioButton( buttonGroup2, "radioButton4" );
    radioButton4->setGeometry( QRect( 10, 20, 97, 22 ) );

    radioButton6 = new QRadioButton( buttonGroup2, "radioButton6" );
    radioButton6->setGeometry( QRect( 10, 60, 97, 22 ) );

//     lineEdit2 = new QLineEdit( buttonGroup2, "lineEdit2" );
//     lineEdit2->setGeometry( QRect( 30, 85, 90, 24 ) );

    buttonGroup1 = new QButtonGroup( groupBox, "buttonGroup1" );
    buttonGroup1->setGeometry( QRect( 150, 200, 130, 90 ) );

    radioButton1 = new QRadioButton( buttonGroup1, "radioButton1" );
    radioButton1->setGeometry( QRect( 10, 20, 60, 22 ) );

    radioButton2 = new QRadioButton( buttonGroup1, "radioButton2" );
    radioButton2->setGeometry( QRect( 10, 40, 60, 22 ) );

    radioButton3 = new QRadioButton( buttonGroup1, "radioButton3" );
    radioButton3->setGeometry( QRect( 10, 60, 60, 22 ) );

/*    lineEdit3 = new QLineEdit( groupBox, "lineEdit3" );
    lineEdit3->setGeometry( QRect( 160, 295, 103, 24 ) );*/
    
    progress = new QProgressBar( groupBox, "progressBar" );
    progress->setGeometry( QRect( 10, 325, 130, 30) );
    
    pushButtonCalc = new QPushButton( groupBox, "pushButtonCalc" );
    pushButtonCalc->setGeometry( QRect( 150, 290, 130, 26 ) );

    pushButtonOptions = new QPushButton( groupBox, "pushButtonOptions" );
    pushButtonOptions->setGeometry( QRect( 150, 325, 130, 26 ) );
    
    groupBox->setFixedWidth(290);

    table = new QTable( pBox, "table" );
    //table->setGeometry( QRect( 10, 20, 370, 310 ) );
    table->setNumRows( 0 );
    table->setNumCols( 0 );
   
   pBox->setSpacing(5);
   progress->setProgress(0,100);
   
    // actions
   fileSaveAction = new QAction( this, "fileSaveAction" );
   fileSaveAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "filesave.png" ) ) );
   filePrintAction = new QAction( this, "filePrintAction" );
   filePrintAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileprint.png" ) ) );
   fileCloseAction = new QAction( this, "fileCloseAction" );
   fileCloseAction->setIconSet( QIconSet( QPixmap::fromMimeSource( "fileclose.png" ) ) );

    // toolbars
    toolBar = new QToolBar( QString(""), this, DockTop ); 

    fileSaveAction->addTo( toolBar );
    filePrintAction->addTo( toolBar );
    fileCloseAction->addTo( toolBar );


    // menubar
    MenuBar = new QMenuBar( this, "MenuBar" );


    fileMenu = new QPopupMenu( this );
    fileSaveAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    filePrintAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileCloseAction->addTo( fileMenu );
    MenuBar->insertItem( QString(""), fileMenu, 1 );

    listBox->setSelectionMode(QListBox::Extended);
    languageChange();
    resize( QSize(685, 445).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

/*#ifdef HDF_FILE 
    nType=stHDF;
#else*/
    nType=stCSV;
//#endif
   
    connect( pushButtonCalc, SIGNAL( clicked() ), this, SLOT( calcIsi() ) );
    connect( pushButtonOptions, SIGNAL( clicked() ), this, SLOT( isiOptions() ) );
    connect( filePrintAction, SIGNAL( activated() ), this, SLOT( printData()));
    connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( saveData()));
    connect( fileCloseAction, SIGNAL( activated() ), this, SLOT( close()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
PoinIsiForm::~PoinIsiForm()
{
    // no need to delete child widgets, Qt does it all for us
}


/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PoinIsiForm::languageChange()
{
    setCaption( tr( "Interspike Interval Estimation" ) );
    groupBox->setTitle( tr( "Variable" ) );
    buttonGroup2->setTitle( tr( "Poincare Cut" ) );
    radioButton5->setText( tr( "Mi&nima" ) );
    radioButton5->setAccel( QKeySequence( tr( "Alt+N" ) ) );
    radioButton4->setText( tr( "Ma&xima" ) );
    radioButton4->setAccel( QKeySequence( tr( "Alt+X" ) ) );
    radioButton6->setText( tr( "&Value :" ) );
    radioButton6->setAccel( QKeySequence( tr( "Alt+V" ) ) );
    buttonGroup1->setTitle( tr( "Poincare Direction" ) );
    radioButton1->setText( tr( "&Up" ) );
    radioButton1->setAccel( QKeySequence( tr( "Alt+U" ) ) );
    radioButton2->setText( tr( "&Down" ) );
    radioButton2->setAccel( QKeySequence( tr( "Alt+D" ) ) );
    radioButton3->setText( tr( "&Both" ) );
    radioButton3->setAccel( QKeySequence( tr( "Alt+B" ) ) );
    pushButtonCalc->setText( tr( "&Calculate" ) );
    pushButtonCalc->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    pushButtonOptions->setText( tr( "&Options" ) );
    pushButtonOptions->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    fileSaveAction->setText( tr( "Save" ) );
    fileSaveAction->setMenuText( tr( "&Save" ) );
    fileSaveAction->setAccel( tr( "Ctrl+S" ) );
    filePrintAction->setText( tr( "Print" ) );
    filePrintAction->setMenuText( tr( "&Print..." ) );
    filePrintAction->setAccel( tr( "Ctrl+P" ) );
    fileCloseAction->setText( tr( "Close" ) );
    fileCloseAction->setMenuText( tr( "&Close" ) );
    fileCloseAction->setAccel( tr( "Alt+F4" ) );
    //toolBar->setLabel( tr( "Tools" ) );
    if (MenuBar->findItem(1))
        MenuBar->findItem(1)->setText( tr( "&File" ) );
}

void PoinIsiForm::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    emit closed();
}

void PoinIsiForm::doIsiProgress(unsigned int max, unsigned int cur)
{
 if (max==0)
 	 progress->hide();
 else
 	 progress->setProgress(max,cur);
 pApp->processEvents();
}

void PoinIsiForm::setup(const Analysis *p)
{
 Q_ASSERT(p!=NULL);
 pAna=const_cast<Analysis *>(p);
 listBox->clear();
 pControl->AddAllDataStore(listBox);
 //set default values
 radioButton1->setChecked(true);
 radioButton4->setChecked(true);
}

void PoinIsiForm::isiOptions()
{
}

void PoinIsiForm::calcIsi()
{
 unsigned int i,j,id,sz,szr;
 QString qs;
 
 pAna->purgeAnalysis(aotIsi);
 vAnaRefs.clear();
 
 for (i=0;i<listBox->count();i++)
 {
  if (listBox->isSelected(i))
  {
   	 id=pAna->addAnalysis(aotIsi,i,IDINVALID);
   	 if (id!=IDINVALID)
   	 	vAnaRefs.push_back(id);
	 if (!pAna->IsOk())
  		{ //some error occurred
		 pControl->Error(pAna->GetErrorStr());
		 return;
   		}
    }
 }
 sz=vAnaRefs.size();
 table->setNumCols(0);
 table->setNumRows((3*sz));
 szr=0;
 AnaTypeT *pType;
 for (i=0;i<sz;i++)
 {
  pType=pAna->get(vAnaRefs[i]);
  if (pType)
  {
   if (!pAna->doAnalysis(*pType))
   	break;
   //fill table with results:
    //table->insertColumns(i,2);
    QHeader *th = table->verticalHeader();
    th->setLabel( (3*i), pControl->GetVarTimeName(pType->nVar1));
    qs=QString(achIsiMask2).arg(pControl->GetVarTimeName(pType->nVar1));
    th->setLabel( (3*i)+1, qs);
    qs=QString(achIsiMask3).arg(pControl->GetVarTimeName(pType->nVar1));
    th->setLabel( (3*i)+2, qs);
    
    if (szr<pType->ddXResult.size())
    {
    szr=pType->ddXResult.size();
    table->setNumCols(szr);
    }
    for (j=0;j<pType->ddXResult.size();j++)
    	table->setText((3*i),j,QString::number(pType->ddXResult[j]));
    for (j=0;j<pType->ddYResult.size();j++)
    	table->setText((3*i)+1,j,QString::number(pType->ddYResult[j]));
    for (j=0;j<pType->ddResult.size();j++)
    	table->setText((3*i)+2,j,QString::number(pType->ddResult[j]));
  }
 }
 if (!pAna->IsOk())
 	 pControl->Error(pAna->GetErrorStr());
}

void PoinIsiForm::printData()
{
 // TODO: Print data in  color coded tables
 QMessageBox::information( this, NULL,  "This function has not been implemented yet", QMessageBox::Ok );
}

void PoinIsiForm::saveData()
{
 int i;
 QString qsFile,qs;
 
 TSaveDlg* fd = new TSaveDlg( this, nType);
 QString fn=pControl->getFileName();
 i=fn.findRev('.');
 if (i>=0)
	 fn.truncate(i);
 fd->setName(fn);
 if ( fd->exec() != QDialog::Accepted )
 	return;
 qsFile = fd->selectedFile();
 if (!qsFile.isEmpty())
  {
/*   if (fd->bgCompress!=NULL)
   {
    c=fd->bgCompress->selectedId();
    if (c<0) c=0;*/
/*#ifdef HDF_FILE
  writeHDF(qsFile,c);
#else*/
  if (!fd->leComment->text().isEmpty())
  	qs=fd->leComment->text();
  writeCSV(qsFile,fd->cbAppend->isChecked(),fd->cbVars->isChecked(),fd->cbRows->isChecked(),qs);
    
   }
//#endif
//   pControl->saveDataStore(qsFile,nType,c);
}

// void PoinIsiForm::writeHDF(QString qsFile,int compress)
// {
//  int sd_id=0, sds_id=0, nError=0, start[1]={0};
//  unsigned int vermin,vermaj,release,size1,size2,i,j;
//  char pLibInfo[80];
//  comp_coder_t comp_type;
//  comp_info c_info; /* Compression structure */
//  double *pd=NULL;
//  AnaTypeT *pType;
//  QString qs;
//  
// try
//  {
// 	if (vAnaRefs.size()==0)
// 		return;
// 	sd_id = SDstart (qsFile.ascii(), DFACC_CREATE);
// 	if (sd_id==FAIL)
// 			throw std::runtime_error(QString().sprintf("Error creating HDF file."));
// 		
// 	if (Hgetlibversion(&vermaj, &vermin, &release, pLibInfo)==FAIL)
// 			throw std::runtime_error(QString().sprintf("Error getting HDF library information"));
// 		
// 		// write out file info attributes
// 	if (SDsetattr(sd_id, achIdentifier, DFNT_CHAR8, strlen(achHeader), achHeader)==FAIL)
// 			throw std::runtime_error(QString().sprintf("Error creating global attribute Header"));
// 	//set optional compression level
// 	switch(compress)
// 		{
// 		 case 5: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 9; break; }
// 		 case 4: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 6; break; }
// 		 case 3: { comp_type = COMP_CODE_DEFLATE; c_info.deflate.level = 3; break; }
// 		 case 2: { comp_type =  COMP_CODE_SKPHUFF; c_info.skphuff.skp_size = 1; break; } //No documentation regarding good values for skp_size!
// 		 case 1: { comp_type =  COMP_CODE_RLE; break; }
// 		 default:
// 		 case 0: { comp_type =  COMP_CODE_NONE ; break; }
// 		}
//         pType=pAna->get(vAnaRefs[0]);
// 	//create vars
// 	 size2=pType->ddTime.size();
// 	 sds_id = SDcreate (sd_id, pControl->GetVarTimeName(0), DFNT_FLOAT64, 1, (int *)&size2);
// 	if (sds_id==FAIL)
// 		throw std::runtime_error(QString().sprintf("Error creating variable"));
// 	nError = SDsetcompress(sds_id, comp_type, &c_info);
// 	if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 			throw std::runtime_error(QString().sprintf("Error setting up compression\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 	size2=pType->ddTime.size();
// 	pd=new double[size2];
// 	for (j=0;j<size2;j++)
// 				pd[j]=pType->ddTime[j];
// 	nError = SDwritedata (sds_id, start, NULL, (int *)&size2, pd);
// 	if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 			throw std::runtime_error(QString().sprintf("Error writing values\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 		}
// 	delete[] pd;
// 	pd=NULL;
// 	//write other values
// 	size1=vAnaRefs.size();
// 	for (i=0;i<size1;i++)
// 		{
// 		  pType=pAna->get(vAnaRefs[i]);
//   		  if (pType)
//   		  {
//   		   //first do variable:
// 		   size2=pType->ddData.size();
// 		   if (size2>0)
// 		   {
// 			sds_id = SDcreate (sd_id, pControl->GetVarTimeName(pType->nVar1), DFNT_FLOAT64, 1, (int *)&size2);
// 			if (sds_id==FAIL)
// 				throw std::runtime_error(QString().sprintf("Error creating variable"));
// 			nError = SDsetcompress(sds_id, comp_type, &c_info);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error setting up compression\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			pd=new double[size2];
// 			for (j=0;j<size2;j++)
// 				pd[j]=pType->ddData[j];
// 			nError = SDwritedata (sds_id, start, NULL, (int *)&size2, pd);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error writing values\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			delete[] pd;
// 			pd=NULL;
// 		  }
// 		  //next do results: time cut
// 		  size2=pType->ddXResult.size();
// 		   if (size2>0)
// 		   {
// 		   	qs=QString(achIsiMask1).arg(pControl->GetVarTimeName(pType->nVar1));
// 			sds_id = SDcreate (sd_id, qs, DFNT_FLOAT64, 1, (int *)&size2);
// 			if (sds_id==FAIL)
// 				throw std::runtime_error(QString().sprintf("Error creating variable"));
// 			nError = SDsetcompress(sds_id, comp_type, &c_info);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error setting up compression\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			pd=new double[size2];
// 			for (j=0;j<size2;j++)
// 				pd[j]=pType->ddXResult[j];
// 			nError = SDwritedata (sds_id, start, NULL, (int *)&size2, pd);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error writing values\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			delete[] pd;
// 			pd=NULL;
// 		  }
// 		  //next do results: var cut
// 		  size2=pType->ddResult.size();
// 		   if (size2>0)
// 		   {
// 		   	qs=QString(achIsiMask2).arg(pControl->GetVarTimeName(pType->nVar1));
// 			sds_id = SDcreate (sd_id, qs, DFNT_FLOAT64, 1, (int *)&size2);
// 			if (sds_id==FAIL)
// 				throw std::runtime_error(QString().sprintf("Error creating variable"));
// 			nError = SDsetcompress(sds_id, comp_type, &c_info);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error setting up compression\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			pd=new double[size2];
// 			for (j=0;j<size2;j++)
// 				pd[j]=pType->ddResult[j];
// 			nError = SDwritedata (sds_id, start, NULL, (int *)&size2, pd);
// 			if (nError==FAIL)
// 			{
// #ifdef _DEBUG
// 			 HEprint(stderr,0);
// #endif
// 				throw std::runtime_error(QString().sprintf("Error writing values\n%s",HEstring((hdf_err_code_t)HEvalue(nError))));
// 			}
// 			delete[] pd;
// 			pd=NULL;
// 		  }
// 		 }
// 		}
// 		if (SDendaccess(sds_id)==FAIL)
// 			throw std::runtime_error(QString().sprintf("Error closing access to HDF file"));
// 		sds_id=0;
// 		if (SDend(sd_id)==FAIL)
// 			throw std::runtime_error(QString().sprintf("Error closing HDF file"));
// 		sd_id=0;
// 	}
// 	catch (std::exception &stdex)
// 	{
// 		if (sds_id!=0)
// 			SDendaccess(sds_id);
// 		if (sd_id!=0)
// 			SDend(sd_id);
// 		if (pd)
// 			delete[] pd;
// 		pControl->Error(stdex.what());
// #ifdef _DEBUG
//   qDebug("[Control::saveHDFFile] ERROR (error=%s).",stdex.what());
// #endif
// 	}
// }

void PoinIsiForm::writeCSV(QString qsFile,bool bAppend, bool bVars, bool bRows,QString qs)
{
 unsigned int i,j,sz;
 bool done;
 AnaTypeT *pType;
 
 QFile file(qsFile);
 
 try
 {
  if (bAppend)
 	file.open(IO_WriteOnly|IO_Append);
  else
 	file.open(IO_WriteOnly);
  if (file.status()!=IO_Ok)
 	throw std::runtime_error(QString().sprintf("Error opening file."));

  QTextStream stream(&file);
  if (!bAppend)
  {
  //write header
   stream << achHeader << endl;
  }
  stream  << QString("%1 - [%2]").arg(pControl->getName()).arg(pControl->getFileName()).ascii() << endl;
   stream << QDate::currentDate().toString() << achComma << QTime::currentTime().toString() << endl;
  if (!qs.isEmpty())
  	stream << qs.ascii() << endl;
  sz=vAnaRefs.size();
  if (bRows)
    {
     for (i=0;i<sz;i++)
     {
      pType=pAna->get(vAnaRefs[i]);
      if (pType)
      {
      stream << pControl->GetVarTimeName(pType->nVar1);
      for (j=0;j<pType->ddXResult.size();j++)
    	stream << achComma << QString::number(pType->ddXResult[j]);
      stream << endl;
    	
      stream << QString(achIsiMask2).arg(pControl->GetVarTimeName(pType->nVar1));
      for (j=0;j<pType->ddYResult.size();j++)
    	stream << achComma << QString::number(pType->ddYResult[j]);
      stream << endl;
     
      stream << QString(achIsiMask3).arg(pControl->GetVarTimeName(pType->nVar1));
      for (j=0;j<pType->ddResult.size();j++)
    	stream << achComma << QString::number(pType->ddResult[j]);
     stream << endl;
     }
   }
  }
  else
  {
     for (i=0;i<sz;i++)
     {
      pType=pAna->get(vAnaRefs[i]);
      if (pType)
      {
       if (i>0)
       		stream << achComma;
       stream << pControl->GetVarTimeName(pType->nVar1);
       stream << achComma << QString(achIsiMask2).arg(pControl->GetVarTimeName(pType->nVar1));
       stream << achComma << QString(achIsiMask3).arg(pControl->GetVarTimeName(pType->nVar1));
      }
     }
     stream << endl;
     j=0;
     done=false;
     while(!done)
     {
      for (i=0;i<sz;i++)
      {
       pType=pAna->get(vAnaRefs[i]);
       if (pType)
       {
        if (i>0)
       		stream << achComma;
        if (j<pType->ddXResult.size())
        	{
        	 done=false;
    	 	 stream << QString::number(pType->ddXResult[j]);
    	 	}
    	 else
    	 	done=true;
        if (j<pType->ddYResult.size())
        	{
        	 done=false;
    		 stream << achComma << QString::number(pType->ddYResult[j]);
    		}
    	else
    		done=true;
	if (j<pType->ddResult.size())
		{
		 done=false;
    		 stream << achComma << QString::number(pType->ddResult[j]);
    		}
    	else
    		done=true;
       }
      }
      j++;
     stream << endl;
    }
  }

/*  stream << achIndex << c;
  for (i=0;i<vdtStoreData.size();i++)
  {
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
   }  }

*/
  stream << endl;
  file.close();
 }
 catch (std::exception &stdex)
 {
  pControl->Error(stdex.what());
  file.close();
#ifdef _DEBUG
  qDebug("[Poinisi::saveCSV] ERROR (error=%s).",stdex.what());
#endif
 }
}
