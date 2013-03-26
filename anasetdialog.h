/****************************************************************************
** Created: Wed Aug 25 21:38:55 2004
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.2   edited Nov 24 13:47 $)
****************************************************************************/

#ifndef ANASETDIALOG_H
#define ANASETDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
#include "controlobject.h"
#include "analysis.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QGroupBox;
class QTextEdit;
class QTable;

class AnaSetDialog : public QDialog
{
    Q_OBJECT

public:
    AnaSetDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AnaSetDialog();

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QGroupBox* groupBox;
    QTextEdit* textEdit;
    QTable* anaTable;

    void setupDialog(NS_Control::NS_Analysis::AnaSetT *, QString qs=0);
    
protected:
    QHBoxLayout* Layout1;
   NS_Control::NS_Analysis::AnaSetT *pSet;
   
protected slots:
    virtual void languageChange();
    virtual void currentChange(int,int);
    virtual void doOk();
};

#endif // ANASETDIALOG_H
