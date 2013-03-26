/****************************************************************************
** Created: Wed Aug 25 21:39:06 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
****************************************************************************/

#include "anasetdialog.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qtextedit.h>
#include <qtable.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <stdexcept>

using namespace NS_Control;
using namespace NS_Analysis;

extern const char *achInvalidVal;
extern Control *pControl;

/*
 *  Constructs a AnaSetDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AnaSetDialog::AnaSetDialog( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "AnaSetDialog" );
    setSizeGripEnabled( FALSE );

    QWidget* privateLayoutWidget = new QWidget( this, "Layout1" );
    privateLayoutWidget->setGeometry( QRect( 120, 290, 250, 33 ) );
    Layout1 = new QHBoxLayout( privateLayoutWidget, 0, 6, "Layout1"); 

    buttonOk = new QPushButton( privateLayoutWidget, "buttonOk" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    Layout1->addWidget( buttonOk );

    buttonCancel = new QPushButton( privateLayoutWidget, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    groupBox = new QGroupBox( this, "groupBox" );
    groupBox->setGeometry( QRect( 20, 10, 480, 270 ) );

    textEdit = new QTextEdit( groupBox, "textEdit" );
    textEdit->setGeometry( QRect( 10, 20, 460, 90 ) );
    textEdit->setReadOnly(true);
    
    anaTable = new QTable( groupBox, "anaTable" );
    anaTable->setGeometry( QRect( 10, 120, 460, 140 ) );
    anaTable->setNumRows( 0 );
    anaTable->setNumCols( 3 );
    anaTable->setSelectionMode( QTable::SingleRow );
    anaTable->setSorting(false);
    
    languageChange();
    resize( QSize(511, 333).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( doOk() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    // set signal for selection change of anatable
    connect( anaTable, SIGNAL( currentChanged(int,int) ), this, SLOT( currentChange(int,int) ));
    //set pointer to Null
    pSet=NULL;
}

/*
 *  Destroys the object and frees any allocated resources
 */
AnaSetDialog::~AnaSetDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AnaSetDialog::languageChange()
{
    QHeader *pHeader;
    
    setCaption( tr( "Analysis Settings" ) );
    buttonOk->setText( tr( "&OK" ) );
    buttonOk->setAccel( QKeySequence( QString::null ) );
    buttonCancel->setText( tr( "&Cancel" ) );
    buttonCancel->setAccel( QKeySequence( QString::null ) );
    groupBox->setTitle( tr( "Analysis" ) );
    pHeader=anaTable->horizontalHeader();
    pHeader->setLabel(0,tr("Parameter"));
    pHeader->setLabel(1,tr("Value"));
    pHeader->setLabel(2,tr("Description"));
}

void AnaSetDialog::setupDialog(AnaSetT *p, QString qsExtra)
{
 unsigned int i;
 QString qs;
 
 if (!p) return;
 pSet=p;
 groupBox->setTitle(p->qsTitle);
 if (qsExtra)
 	qs=QString("%1\nVariable: %2").arg(p->qsDescr).arg(qsExtra);
 else
 	qs=p->qsDescr;
 textEdit->setText(qs);
 anaTable->setNumRows( p->slNames.size() );

 for (i=0;i<p->slNames.size();i++)
	 anaTable->setText(i,0,p->slNames[i]);
 for (i=0;i<p->ddVals.size();i++)
 	{
	 qs.setNum(p->ddVals[i]);
	 anaTable->setText(i,1,qs);
   	}
 for (i=0;i<p->slDescr.size();i++)
	 anaTable->setText(i,2,p->slDescr[i]);
  for (i=0;i<3;i++)
  	 anaTable->adjustColumn(i);
   anaTable->setColumnReadOnly(0,true);
   anaTable->setColumnReadOnly(1,false);
   anaTable->setColumnReadOnly(2,true);
}

void AnaSetDialog::currentChange(int r, int c)
{
 //ensure that the focus is always on the editable value
  if (c!=1)
  	anaTable->setCurrentCell(r,1);
}

void AnaSetDialog::doOk()
{//store data
 unsigned int i;
 double d;
 bool b;
 QString qs;
 
 try
 {
   if (pSet)
 	{
	 for (i=0;i<pSet->ddVals.size();i++)
 	 {
	  qs=anaTable->text(i,1);
	  d=qs.toDouble(&b);
	  if (!b)
	  	throw std::logic_error(achInvalidVal);
	  pSet->ddVals[i]=d;
   	 }
	}
  }
  catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
      return;
     }
  accept();
}
