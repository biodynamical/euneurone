/****************************************************************************
** Form interface generated from reading ui file 'editseriesform.ui'
**
** Created: Tue Aug 19 10:03:24 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef EDITSERIESFORM_H
#define EDITSERIESFORM_H

#include <qvariant.h>
#include <qdialog.h>
#include "graphtype.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QToolButton;
class QSpinBox;

class EditSeriesForm : public QDialog
{
    Q_OBJECT

public:
    EditSeriesForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~EditSeriesForm();
   void setupForm(graphType &sg);

    QPushButton* pushButtonOk;
    QPushButton* pushButtonRemove;
    QPushButton* pushButtonCancel;
    QGroupBox* groupBox;
    QLabel* textLabel3;
    QCheckBox* checkBoxLines;
    QCheckBox* checkBoxPoints;
    QCheckBox* checkBoxVisible;
    QLabel* textLabel2;
    QToolButton* pixmapButton;
    QComboBox* comboBoxPoint;
    QComboBox* comboBoxLine;
    QSpinBox* spinBoxBegin;
    QLabel* textLabel4;
    QLabel* textLabel5;
    QLabel* textLabel6;
    QSpinBox* spinBoxEnd;
    QSpinBox* spinBoxMarkSize;
    QSpinBox* spinBoxLineSize;
    QLabel* textLabel;

protected:
   graphType sg;
   void addSymbols(QComboBox *pBox);
   void addLines(QComboBox *pBox);
   void setColorPixmap(QToolButton *p,QColor c);
protected slots:
    virtual void languageChange();
    virtual void setColor();
    virtual void doOk();
    virtual void doRemove();
};

#endif // EDITSERIESFORM_H
