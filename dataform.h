/****************************************************************************
** Form interface generated from reading ui file 'dataform.ui'
**
** Created: Mon Aug 18 21:14:30 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
****************************************************************************/

#ifndef DATAFORM_H
#define DATAFORM_H
#include <qmainwindow.h>
#include <qvariant.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qgroupbox.h>
 #include <qhbox.h>
#include <qbuttongroup.h>
#include <qlistbox.h>
#include <qframe.h>
#include <qradiobutton.h>
#include <qfiledialog.h>
#include <qradiobutton.h>

typedef enum {stNone, stTSV, stCSV, stMat, stCDF, stHDF, stHDF5} TSaveTypes;

class TVarTable : public QListBox
{
public:
    TVarTable(QWidget*parent);
protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
};

class DataForm : public QMainWindow
{
    Q_OBJECT

    TVarTable* tableData;
    QButtonGroup *pGroup;
    //QFrame *pFrame;
    QHBox *pBox;
    QToolBar *toolBar;
    QAction* fileClearAction;
    QAction* fileAllAction;
    QAction* fileSaveAction;
    QAction* filePrintAction;
    QAction* fileCloseAction;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QWidget *Owner;

public:
    DataForm( QWidget* parent = 0);
    ~DataForm();
    void rename(QString &qs);

protected:
   void closeEvent(QCloseEvent*);
   QString qsFile;
   TSaveTypes nType;
signals:
    void closed();
    void newDataSignal();

public slots:
    void printData();
    void saveData();
    void clearList();
    void addAllList();
    void setType(int);
};

#endif // DATAFORM_H
