/****************************************************************************
** Created: Sat Aug 5 16:54:37 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
****************************************************************************/

#ifndef POINISI_H
#define POINISI_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>
#include <qhbox.h>
#include <qprogressbar.h>

#include "controlobject.h"
#include "analysis.h"
#include "dataform.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QGroupBox;
class QListBox;
class QListBoxItem;
class QButtonGroup;
class QRadioButton;
class QLineEdit;
class QPushButton;
class QTable;

class PoinIsiForm : public QMainWindow
{
    Q_OBJECT

public:
    PoinIsiForm( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~PoinIsiForm();

    QHBox *pBox;
    QGroupBox* groupBox;
    QListBox* listBox;
    QButtonGroup* buttonGroup2;
    QRadioButton* radioButton5;
    QRadioButton* radioButton4;
    QRadioButton* radioButton6;
/*    QLineEdit* lineEdit2;
    QLineEdit* lineEdit3;*/
    QButtonGroup* buttonGroup1;
    QRadioButton* radioButton1;
    QRadioButton* radioButton2;
    QRadioButton* radioButton3;
    QPushButton* pushButtonCalc;
    QPushButton* pushButtonOptions;
    QProgressBar* progress;
    QTable* table;
    QMenuBar *MenuBar;
    QPopupMenu *fileMenu;
    QToolBar *toolBar;
    QAction* fileSaveAction;
    QAction* filePrintAction;
    QAction* fileCloseAction;
    
    NS_Control::NS_Analysis::Analysis *pAna;
    void setup(const NS_Control::NS_Analysis::Analysis *p);
protected:
    NS_Control::NS_Analysis::UIntVector vAnaRefs;
    TSaveTypes nType;
    void closeEvent(QCloseEvent*);
    void writeCSV(QString qsFile,bool,bool,bool,QString);
    //void writeHDF(QString qsFile, int);
signals:
    void closed();
protected slots:
    virtual void languageChange();
public slots:
    void printData();
    void saveData();
    void calcIsi();
    void isiOptions();
     void doIsiProgress(unsigned int max, unsigned int cur);
};

#endif // POINISI_H
