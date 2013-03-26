// graph3dform.h

#ifndef GRAPH3DFORM_H
#define GRAPH3DFORM_H
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qframe.h>
#include <qlabel.h>
#include <qcombobox.h>

#include "tglgraph.h"

class TGraph3DForm : public QMainWindow
{
    Q_OBJECT

    QToolBar *toolBar;
    QAction* fileSaveAction;
    QAction* filePrintAction;
    QAction* fileCloseAction;
    QMenuBar *menubar;
    QPopupMenu *fileMenu;
    QComboBox *pcbMouse;
    QLabel *plbMouse;
    QWidget *Owner;
    TGlGraph *pGraph;
public:
    TGraph3DForm( QWidget* parent = 0);
    ~TGraph3DForm();
    void rename(QString &qs);
    void addVariables(QStringList *p);
     TGlGraph *getGraph() { return pGraph;}
protected:
   void closeEvent(QCloseEvent*);
   void keyPressEvent(QKeyEvent *);
   QString qsFile;
   int nType;
   int nMouse;
signals:
    void closed();
    void newGraphSignal();

public slots:
    void printData();
    void saveData();
    void setType(int);
    void selectMouseCombo(int index);
};

#endif // GRAPH3DFORM_H
