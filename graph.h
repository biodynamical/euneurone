// graph.h

#ifndef GRAPH_H
#define GRAPH_H
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcolor.h>
#include <qmenubar.h>
#include <qstringlist.h>
#include <qprogressbar.h>
#include <qtoolbutton.h>
#include <vector>
#include <stack>

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#include <qwt5/qwt_plot_canvas.h>
#else
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#endif

#include "graphtype.h"

class TPlotCanvas :public QwtPlotCanvas
{
public:
 TPlotCanvas(QwtPlot *);
 QPixmap *getPixmap();
};


//---------------------------------------------------------------------------
class TGraph : public QwtPlot
{
 private:
  graphType gtX;
 public:
  TGraph(QWidget *);
  void addVariables(QStringList *p);
  void setXVariable(unsigned int n);
  void addYVariable(unsigned int n);
  void addLyapunov();
  void addRecur();
  void addMaxLyap();
  void addPeriod();
  void addPower();
  void addPoincare();
  void addSpike();
  bool isEvolution();
  void enableTitle(bool);
  void enableXLabel(bool);
  QString qsTitle, qsXLabel;
  TCalcTypeItem type;
 protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dropEvent(QDropEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dragLeaveEvent(QDragLeaveEvent* event);
};

typedef std::stack<double> TDblStack;

class TGraphForm : public QMainWindow
{
 Q_OBJECT
 TGraph *pGraph;
 QToolBar *toolBar;
 QAction* fileSaveAction;
 QAction* filePrintAction;
 QAction* fileCloseAction;
 QAction* unZoomAction;
 QAction* viewLegendAction;
 QAction* viewTitleAction;
 QAction* viewXLabelAction;
 QAction* viewXAxisAction;
 QAction* viewYAxisAction;
 QAction* viewGridAction;
 QAction* updateAction;
 QComboBox *pcbMouse;
 QLabel *plbMouse;
 QProgressBar *pProgress;
 QToolButton *pStop;
 QMenuBar *menubar;
 QPopupMenu *fileMenu;
 QPopupMenu *viewMenu;
 QWidget *Owner;
 QString qsStatus;
 TDblStack dsZoom;
 QRect rZoom;
 int nMouse;
public:
 TGraphForm(QWidget *parent=0);
 ~TGraphForm();
 void rename(QString &qs);
 void addVariables(QStringList *p);
 QwtPlot *getGraph() { return pGraph;}
 void hideProgress();
 void setProgress(unsigned int,unsigned int);
signals:
    void closed();
    void newGraphSignal();

protected:
    bool bLegend, bXLabel, bXAxis, bYAxis, bTitle, bGrid;
    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent *);
    void showValues(int, int);
    void doZoom();
    
public slots:
  void mouseMove(const QMouseEvent &e);
  void mousePressed(const QMouseEvent &e);
  void mouseReleased(const QMouseEvent &e);
  void selectMouseCombo(int index);
  void printGraph();
  void saveGraph();
  void toggleLegend(bool);
  void toggleTitle(bool);
  void toggleXLabel(bool);
  void toggleXAxis(bool);
  void toggleYAxis(bool);
  void toggleGrid(bool);
  void doUnzoom();
  void editSeries(long key);
  void updatePlot();
 void clickedStop();
};

#endif
