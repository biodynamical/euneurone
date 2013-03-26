/****************************************************************************
** Form implementation generated from reading ui file 'form1.ui'
**
** Created: Mon Jul 4 14:03:30 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
****************************************************************************/

#include "savestat.h"

#include <qvariant.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

SaveStat::SaveStat( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "Saving data file..." );

    bCancel=true;
    
    textEdit = new QTextEdit( this, "textEdit" );
    textEdit->setGeometry( QRect( 10, 10, 270, 200 ) );

    progressBar = new QProgressBar( this, "progressBar" );
    progressBar->setGeometry( QRect( 10, 220, 270, 28 ) );

    pushButton = new QPushButton( this, "pushButton" );
    pushButton->setGeometry( QRect( 80, 260, 112, 29 ) );
    languageChange();
    resize( QSize(290, 303).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    connect( pushButton, SIGNAL( clicked() ), this, SLOT( doit()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
SaveStat::~SaveStat()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SaveStat::languageChange()
{
    setCaption( tr( "Saving..." ) );
    pushButton->setText( tr( "&Cancel" ) );
    pushButton->setAccel( QKeySequence( tr( "Alt+C" ) ) );
}

void SaveStat::doit()
{
	if (!bCancel)
	{
	 close();
	 return;
	}
	emit cancelSave();
}

void SaveStat::done()
{
	bCancel=false;
	pushButton->setText( tr( "&Close" ) ); 
}
