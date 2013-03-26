/****************************************************************************
** Form interface generated from reading ui file 'mainform.ui'
**
** Created: Mon Aug 18 21:27:50 2003
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.1.1   edited Nov 21 17:40 $)
****************************************************************************/

#ifndef MAINFORM_H
#define MAINFORM_H

#include <qvariant.h>
#include <qmainwindow.h>
#include <qaction.h>
#include <qlistview.h>
#include <qscrollview.h>
#include <qhbox.h>
#include <vector>
#include "controlobject.h"
#include "graph.h"
#include "dataform.h"
#include "poinisi.h"
#include "graph3dform.h"
#include "editseriesform.h"

class QVBoxLayout;
class QHBoxLayout;
class QVBox;
class QGridLayout;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QFrame;
class QGroupBox;
class QButtonGroup;
class QLCDNumber;
class QLineEdit;
class QListView;
class QListViewItem;
class QProgressBar;
class QPushButton;
class QTabWidget;
class QTextEdit;
class QSettings;
class QWidget;

class TGraphAction : public QAction
{
 Q_OBJECT
public:
  TGraphForm *pGraph;
  int nIdx;
 TGraphAction(QObject *parent, int n);
 ~TGraphAction();
 void rename(int idx);
 void add(QStringList *p);
public slots:
 void isActivated();
 void getKilled();
 void sendNGS();
 void reCaption(QwtPlot *,QString);
 void doProgress(QwtPlot *p,unsigned int max, unsigned int cur);
signals:
 void Clicked(int n);
 void killMe(int id);
 void newGraphSignal();
};

class TGraph3DAction : public QAction
{
 Q_OBJECT
public:
  TGraph3DForm *pGraph;
  int nIdx;
 TGraph3DAction(QObject *parent, int n);
 ~TGraph3DAction();
 void rename(int idx);
 void add(QStringList *p);
public slots:
 void isActivated();
 void getKilled();
 void sendNGS();
signals:
 void Clicked(int n);
 void killMe(int id);
 void newGraph3DSignal();
};

typedef std::vector<TGraphAction *> TGraphActionVector;
typedef std::vector<TGraph3DAction *> TGraph3DActionVector;

class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );
    ~MainForm();
    void Initialize(QSettings *);
    void addToHistory(QString &);
    void moveHistory(int);
    void saveSettings(QSettings *);
    
    QTabWidget* tabWidget;
    QHBox* boxControl;
    QTextEdit* textEditComp;
    QTextEdit* textEditLog;
    QTextEdit* textEditFile;
    QLineEdit *lineEditLog;
    QVBox *vboxLog;
    QHBox *hboxLog;
    QPushButton *pushButtonLog1;
    QPushButton *pushButtonLog2;
    QScrollView* tabControl;
    QGroupBox* groupBoxIntVals;
    QLabel* textLabelBegin;
    QLineEdit* lineEditBegin;
    QLabel* textLabelEnd;
    QLineEdit* lineEditEnd;
    QLabel* textLabelStep;
    QLineEdit* lineEditStep;
    QLabel* textLabelInterval;
    QLineEdit* lineEditInterval;
    QGroupBox* groupBoxButs;
    QLCDNumber* lcdNumber;
    QLCDNumber* lcdEstTime;
    QLCDNumber* lcdElapTime;
    QLabel* textLabelElapsed;
    QLabel* textLabelEstimated;
    QProgressBar* progressBar;
    QPushButton* pushButtonStart;
    QPushButton* pushButtonStop;
    QPushButton* pushButtonReset;
    QPushButton* pushButtonAnaStop;
    QPushButton* pushButtonStatStop;
    QLabel *lblIntThread;
    QLabel *lblStatThread;
    QLabel *lblAnaThread;
    QButtonGroup* buttonGroupInt;
    QWidget* tabVars;
    NS_Control::TListView* listViewVars;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QPopupMenu *historyMenu;
    QPopupMenu *WindowsMenu;
    QPopupMenu *AnaMenu;
    QPopupMenu *helpMenu;
    QToolBar *toolBar;
    QAction* fileNewAction;
    QAction* fileOpenAction;
//    QAction* fileHistoryAction;
#ifdef HDF5_FILE
    QAction* fileLoadAction;
#endif    
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* filePrintAction;
    QAction* fileExitAction;
    QAction* helpContentsAction;
    QAction* helpIndexAction;
    QAction* helpAboutAction;
    QAction* anaLyapAction;
    QAction* anaStatsAction;
    QAction* winDataAction;
    QAction* winNewGraphAction;
    QAction* winNew3DAction;
    QAction* winIsiAction;

    TGraphActionVector vGraphAction;
    TGraph3DActionVector vGraph3DAction;
    DataForm *pData;
    PoinIsiForm *pIsiForm;
public slots:
    virtual void fileNew();
    virtual void fileOpen();
#ifdef HDF5_FILE
    virtual void fileLoad();
#endif    
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void filePrint();
    virtual void fileExit();
    virtual void fileHistory(int);
    virtual void helpIndex();
    virtual void helpContents();
    virtual void helpAbout();
    virtual void newGraph();
    virtual void new3DGraph();
    virtual void newIsi();
    virtual void killIsi();
    virtual void newControlGraph(QStringList *);
    virtual void newControlGraph3D(QStringList *);
    virtual void killGraph(int id);
    virtual void killGraph3D(int id);
    virtual void newData();
    virtual void killData();
    virtual void toggleLyap(bool);
    virtual void toggleStats(bool);
    
protected:
    void closeEvent(QCloseEvent*);
    void setOptions();
    void killGraphs();
    QVBoxLayout* layoutVals;
    QStringList slHistory;
    void doLoad(QString &fn);
#ifdef HDF5_FILE
        void doH5Load(QString &fn);
#endif
protected slots:
    virtual void languageChange();
    virtual void startButtonClicked();
    virtual void resetButtonClicked();
    virtual void stopIntButtonClicked();
    virtual void stopStatButtonClicked();
    virtual void stopAnaButtonClicked();
    virtual void leStepChanged(const QString &);
    virtual void leBeginChanged(const QString &);
    virtual void leEndChanged(const QString &);
    virtual void leIntervalChanged(const QString &);
    virtual void tabChanged(QWidget *);
    virtual void addLogButtonClicked();
    virtual void stateLogButtonClicked();
signals:
    void closed();
};

#endif // MAINFORM_H
