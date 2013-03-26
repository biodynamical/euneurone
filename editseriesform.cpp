/****************************************************************************
** Form implementation generated from reading ui file 'editseriesform.ui'
**
** Created: Tue Aug 19 10:03:24 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "editseriesform.h"

#include <qvariant.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcolordialog.h>
#include "controlobject.h"

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#include <qwt5/qwt_data.h>
#else
#include <qwt/qwt_plot.h>
#include <qwt/qwt_data.h>
#endif

const char *achLineNames[6]=
{
 "Points",
 "Solid",
 "Dashed",
 "Dotted",
 "Dash Dot Dash",
 "Dash Dot Dot"
};

const char *achSymNames[11]=
{
 "No symbol",
 "Ellipse",
 "Rectangle",
 "Diamond",
 "Triangle upwards",
 "Triangle downwards",
 "Triangle upwards",
 "Triangle left",
 "Triangle right",
 "Cross",
 "Diagonal cross"
};

extern NS_Control::Control *pControl;

/* 
 *  Constructs a EditSeriesForm as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
EditSeriesForm::EditSeriesForm( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    if ( !name )
	setName( "EditSeriesForm" );

    pushButtonOk = new QPushButton( this, "pushButtonOk" );
    pushButtonOk->setGeometry( QRect( 20, 260, 70, 28 ) );

    pushButtonRemove = new QPushButton( this, "pushButtonRemove" );
    pushButtonRemove->setGeometry( QRect( 100, 260, 80, 28 ) );

    pushButtonCancel = new QPushButton( this, "pushButtonCancel" );
    pushButtonCancel->setGeometry( QRect( 190, 260, 80, 28 ) );

    groupBox = new QGroupBox( this, "groupBox" );
    groupBox->setGeometry( QRect( 0, 0, 290, 250 ) );

    textLabel3 = new QLabel( groupBox, "textLabel3" );
    textLabel3->setGeometry( QRect( 20, 210, 50, 20 ) );

    checkBoxLines = new QCheckBox( groupBox, "checkBoxLines" );
    checkBoxLines->setGeometry( QRect( 20, 50, 80, 21 ) );

    checkBoxPoints = new QCheckBox( groupBox, "checkBoxPoints" );
    checkBoxPoints->setGeometry( QRect( 20, 110, 80, 21 ) );
    
    checkBoxVisible = new QCheckBox( groupBox, "checkBoxVisible" );
    checkBoxVisible->setGeometry( QRect( 20, 170, 80, 21 ) );

    textLabel2 = new QLabel( groupBox, "textLabel2" );
    textLabel2->setGeometry( QRect( 130, 170, 68, 20 ) );

    textLabel5 = new QLabel( groupBox, "textLabel5" );
    textLabel5->setGeometry( QRect( 20, 140, 80, 20 ) );

    pixmapButton = new QToolButton( groupBox, "pixmapButton" );
    pixmapButton->setGeometry( QRect( 190, 170, 22, 22 ) );
//    pixmapLabel->setScaledContents( TRUE );

    spinBoxMarkSize = new QSpinBox( groupBox, "spinBoxMarkSize" );
    spinBoxMarkSize->setGeometry( QRect( 110, 140, 90, 23 ) );
    
    textLabel6 = new QLabel( groupBox, "textLabel6" );
    textLabel6->setGeometry( QRect( 20, 80, 80, 20 ) );
    
    spinBoxLineSize = new QSpinBox( groupBox, "spinBoxLineSize" );
    spinBoxLineSize->setGeometry( QRect( 110, 80, 90, 23 ) );

    comboBoxPoint = new QComboBox( FALSE, groupBox, "comboBoxPoint" );
    comboBoxPoint->setGeometry( QRect( 110, 110, 150, 23 ) );

    comboBoxLine = new QComboBox( FALSE, groupBox, "comboBoxLine" );
    comboBoxLine->setGeometry( QRect( 110, 50, 150, 23 ) );

    spinBoxBegin = new QSpinBox( groupBox, "spinBoxBegin" );
    spinBoxBegin->setGeometry( QRect( 70, 210, 80, 23 ) );

    textLabel4 = new QLabel( groupBox, "textLabel4" );
    textLabel4->setGeometry( QRect( 155, 210, 40, 20 ) );

    spinBoxEnd = new QSpinBox( groupBox, "spinBoxEnd" );
    spinBoxEnd->setGeometry( QRect( 190, 210, 90, 23 ) );

    textLabel = new QLabel( groupBox, "textLabel" );
    textLabel->setGeometry( QRect( 10, 20, 270, 20 ) );
    textLabel->setAlignment(AlignHCenter | AlignVCenter | ExpandTabs);
    
    languageChange();
    resize( QSize(291, 296).expandedTo(minimumSizeHint()) );
    
    addSymbols(comboBoxPoint);
    addLines(comboBoxLine);
    setColorPixmap(pixmapButton,black);
    connect( pushButtonCancel, SIGNAL( clicked() ), this, SLOT( close()));
    connect( pushButtonOk, SIGNAL( clicked() ), this, SLOT( doOk()));
    connect( pushButtonRemove, SIGNAL( clicked() ), this, SLOT( doRemove()));
    connect( pixmapButton, SIGNAL( clicked() ), this, SLOT( setColor()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
EditSeriesForm::~EditSeriesForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void EditSeriesForm::languageChange()
{
    setCaption( tr( "Edit Series" ) );
    pushButtonOk->setText( tr( "&Ok" ) );
    pushButtonOk->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    pushButtonRemove->setText( tr( "&Remove" ) );
    pushButtonRemove->setAccel( QKeySequence( tr( "Alt+R" ) ) );
    pushButtonCancel->setText( tr( "&Cancel" ) );
    pushButtonCancel->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    groupBox->setTitle( tr( "Series" ) );
    textLabel3->setText( tr( "Begin" ) );
    checkBoxLines->setText( tr( "Lines" ) );
    checkBoxPoints->setText( tr( "Markers" ) );
    checkBoxVisible->setText( tr( "Invisible" ) );
    textLabel2->setText( tr( "Colour" ) );
    textLabel4->setText( tr( "End" ) );
    textLabel5->setText( tr( "Marker Size" ) );
    textLabel6->setText( tr( "Line Size" ) );
    textLabel->setText( tr( "Unknown variable" ) );
}

void EditSeriesForm::addSymbols(QComboBox *pBox)
{
 QPainter *pPaint;
 QPixmap *pPix;
 QwtSymbol sym;
 int i;
 const int size=16;
 
 sym.setSize(12);
 pPix=new QPixmap( size, size);
 pPaint=new QPainter(pPix);
 for (i=0;i<11;i++)
 {
  sym.setStyle(QwtSymbol::Style(i));
  pPix->fill(pBox->paletteBackgroundColor());  // fill with widget background
  pPaint->setBackgroundMode(Qt::TransparentMode);
  //create masked image
  pPaint->setPen(black);
  pPaint->setBrush(black);
  sym.draw(pPaint,size/2,size/2);
  pPix->setMask( pPix->createHeuristicMask() );
  //create color selector
  sym.draw(pPaint,size/2,size/2);
  pBox->insertItem(*pPix,achSymNames[i]);
 }
 pPaint->end();
 delete pPaint;
 delete pPix;
}

void EditSeriesForm::addLines(QComboBox *pBox)
{
 QPainter *pPaint;
 QPixmap *pPix;
 QPen pen;
 int i;
 
 pPix=new QPixmap( 16, 16);
 pPaint=new QPainter(pPix);
 pen.setColor(black);
 pPix->fill(pBox->paletteBackgroundColor());  // fill with widget background
 pPaint->setBackgroundMode(Qt::TransparentMode);
 //create masked image
 pPaint->setPen(pen);
 pPaint->drawPoint(7,7);
 pPix->setMask( pPix->createHeuristicMask() );
 pBox->insertItem(*pPix,achLineNames[0]);
 for (i=1;i<6;i++)
 {
  pen.setStyle(Qt::PenStyle(i));
#ifndef WINDOWS
  pen.setWidth(2);
#else
  pen.setWidth(1);
#endif
  pPix->fill(pBox->paletteBackgroundColor());  // fill with widget background
  pPaint->setBackgroundMode(Qt::TransparentMode);
  //create masked image
  pPaint->setPen(pen);
  pPaint->drawLine(0,7,15,7);
  pPix->setMask( pPix->createHeuristicMask() );
  pBox->insertItem(*pPix,achLineNames[i]);
 }
 pPaint->end();
 delete pPaint;
 delete pPix;
}

void EditSeriesForm::setColorPixmap(QToolButton *p,QColor c)
{
 QPainter *pPaint;
 QPixmap *pPix;
 
 pPix=new QPixmap( 20, 20);
 pPaint=new QPainter(pPix);
 pPix->fill(c);
 pPaint->setPen(black);
 pPaint->drawRect(0,0,20,20);
 p->setPixmap(*pPix);
 pPaint->end();
 delete pPaint;
 delete pPix;
}

void EditSeriesForm::setColor()
{
  QColor c;
  QPen p;
  p=sg.getPen();
  c=p.color();
  c=QColorDialog::getColor(c);
  if (c.isValid())
  {
   p.setColor(c);
   sg.setPen(p);
   setColorPixmap(pixmapButton,c);
  }
}

void EditSeriesForm::setupForm(graphType &sgl)
{
 sg=sgl;
 QPen p;
 p=sg.getPen();
 textLabel->setText(sg.getTitle());
 setColorPixmap(pixmapButton,p.color());
 spinBoxBegin->setMinValue(0);
 spinBoxBegin->setMaxValue(sg.getSizeY());
 spinBoxBegin->setValue(sg.getStart());
 spinBoxEnd->setMinValue(0);
 spinBoxEnd->setMaxValue(sg.getSizeY());
 spinBoxEnd->setValue(sg.getStop());
 comboBoxLine->setCurrentItem(p.style());
 comboBoxPoint->setCurrentItem(sg.getStyle());
 spinBoxMarkSize->setMinValue(1);
 spinBoxMarkSize->setMaxValue(50);
 spinBoxMarkSize->setValue(sg.getSymbolSize());
 spinBoxLineSize->setMinValue(0);
 spinBoxLineSize->setMaxValue(50);
 spinBoxLineSize->setValue(sg.getLineWidth());
 switch(sg.getType())
 {
  case stInvisible:
	{
	 checkBoxVisible->setChecked(true);
	 checkBoxLines->setChecked(false);
	 checkBoxPoints->setChecked(false);
	 break;
	}
  case stLine:
	{
	 checkBoxVisible->setChecked(false);
	 checkBoxLines->setChecked(true);
	 break;
	}
  case stSymbols:
	{
	 checkBoxVisible->setChecked(false);
	 checkBoxPoints->setChecked(true);
	 break;
	}
   case stBoth:
	{
	 checkBoxVisible->setChecked(false);
	 checkBoxLines->setChecked(true);
	 checkBoxPoints->setChecked(true);
	 break;
	}
 }
}

void EditSeriesForm::doOk()
{
 QPen p;
 seriesType sType;
 
 p=sg.getPen();
 sType=sg.getType();
 sg.setBounds(spinBoxBegin->value(),spinBoxEnd->value());
 sg.setSymbolSize(spinBoxMarkSize->value());
 p.setStyle(Qt::PenStyle(comboBoxLine->currentItem()));
 sg.setPen(p);
 sg.setStyle(QwtSymbol::Style(comboBoxPoint->currentItem()));
 sg.setLineWidth(spinBoxLineSize->value());
 if (checkBoxVisible->isChecked())
 	sType=stInvisible;
 else
 	{
	 if (checkBoxLines->isChecked())
	 	sType=stLine;
	 if (checkBoxPoints->isChecked())
	 	{
		 if (sType==stLine)
		 	sType=stBoth;
		 else
		 	sType=stSymbols;
		}
	}
 sg.setType(sType);
 pControl->editSeries(sg);
 close();
}

void EditSeriesForm::doRemove()
{
 pControl->removeSeries(sg.getPlot(),sg.getCurve());
 pControl->doRecaption(sg.getPlot(),sg.getPlot()->axisTitle(QwtPlot::xBottom),false);
 close();
}
