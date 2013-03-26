// tglgraph.h

#ifndef TGLGRAPH_H
#define TGLGRAPH_H
#include <qcolor.h>
#include <qfont.h>
#include <qgl.h>
#include <qthread.h>
#include <vector>
#include <deque>

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#include <qwt5/qwt_data.h>
#else
#include <qwt_plot.h>
#include <qwt_data.h>
#include <qwt_scale.h>
#endif

#define GL_SIZESELECTBUF 32
#define PM_SIZE 34
#define MM_SIZE 34*16
#define VP_SIZE 4

typedef enum {emNone, emSelect, emZoom, emRotate} TMouseAction;

typedef std::vector<unsigned int> uintVector;
typedef std::deque<double> dblDequeT;

typedef struct {
QString sName;
QColor Colour;
uintVector uvLists;
} structList;

typedef std::vector<structList> lstVector;

class TGlGraph;

class TGlRender : public QThread
{
        double mScale[3];
	double dZoom[3];
        double MM[MM_SIZE],PM[PM_SIZE];
        int VP[VP_SIZE];
        double winX,winY,winZ;
        double daTransform[4][4];
        double daLast[3];
        double daAxis[3];
        double current_position[3];
        double daZoom[3];
	double daZoomOrigin[3];
	double daOrigin[3];
        float faLight[4];
        double dAngle;
        unsigned int nSelect,nDrop,listIndex;
	double dZoomRadius;
        bool bAxes, bCube, b3dAxes, bAnim, bAxesLabels, bAxesTicks, bZoom;
        bool bLegExpand, bLegend, bSelect, bRotate, bOrigin, bPoint;
	bool bRunning, bAddData, bClearData;
        QColor qclLabels, qclLegendBgnd, qclSelect, qclDarkSelect;
	QColor qclAxesPos,qclAxesNeg, qclColor, qclBckGnd;
	dblDequeT dataX, dataY, dataZ;
        lstVector lvList;
        GLUquadricObj *quad;
        GLuint anSelectBuffer[GL_SIZESELECTBUF];
	QwtAutoScale xAxis, yAxis, zAxis;
//	QwtLinearScaleEngine xAxis, yAxis, zAxis;
	TGlGraph *pwgt;
	void draw3DCube();
	void drawZoomSphere();
	void setupMatrix();
	void setVector3(double *v,double f1,double f2,double f3);
	void setVector4(double *v,double f1,double f2,double f3,double f4);
	void pointToVector(int x, int y, double *v);
	void doTransform();
	void drawLegendIcon();
	void drawSelectLegend();
	void drawLegend();
	void drawAxes();
	void drawAxesLabels();
	void selectAxesLabels();
	void drawLists();
	int doSelect();
	int width,height;
	int xSelect,ySelect;
	int xPoint, yPoint;
	int xOrigin, yOrigin;
	bool bResize;
	double calcScale(double d,const dblDequeT &pd,int s);
	void paint();
	void glColor(QColor c);
	void glColor(QColor c,double d);
	void projectOrigin(double xpos, double ypos);
	void projectPoint(double xpos, double ypos);
	void addData();
	void clearData();
 public:
 	TGlRender(TGlGraph *p);
 	~TGlRender();
 	void run();
	void stop();
	void init();
	void resize(int x,int y);
	void pointToOrigin(int x, int y);
	void calcRotation(int x, int y);
	void setSelected(int x, int y);
	void setOrigin(int x, int y);
	void setPoint(int x, int y);
	void calcZoom();
	unsigned int newSeries(const QString &qs, QColor c);
	void updateSeriesName(const QString &qs,unsigned int nidx);
	void addPoints(unsigned int idx, const double *px, const double *py,const double *pz, unsigned int size);
	void addPoints(unsigned int idx,const dblDequeT &x,const dblDequeT &y,const dblDequeT &z);
	void appendPoints(unsigned int idx,const dblDequeT &x,const dblDequeT &y,const dblDequeT &z,unsigned int xidx, unsigned int yidx, unsigned int zidx);
	void clear();
};

class TGlGraph : public QGLWidget
{
    Q_OBJECT
	
	QFont fontLabels,fontTicks;
	TMouseAction mAction;
	TGlRender *pRender;
	bool bTrack, bZoom, bReady;

public:
  TGlGraph(QWidget *parent, const char* name,QGLWidget *p=0);
  ~TGlGraph();
  unsigned int newSeries(const QString &qs, QColor c);
  void updateSeriesName(const QString &qs,unsigned int nidx);
  void addPoints(unsigned int idx, const double *px, const double *py,const double *pz, unsigned int size);
  void addPoints(unsigned int idx,const dblDequeT &x,const dblDequeT &y,const dblDequeT &z);
  void appendPoints(unsigned int idx,const dblDequeT &x,const dblDequeT &y,const dblDequeT &z,unsigned int xidx, unsigned int yidx, unsigned int zidx);
  void clear();
  void setMouseAction(TMouseAction m);
  void replot();
  void renderTextOrigin(const QString &qs,int type);
  void addVariables(QStringList *psl);
protected:
 void initializeGL();
 void resizeGL( int w, int h );
 void paintGL();
 void mousePressEvent(QMouseEvent*e);
 void mouseMoveEvent(QMouseEvent*e);
 void mouseReleaseEvent(QMouseEvent*e);
 void dropEvent(QDropEvent* event);
 void dragEnterEvent(QDragEnterEvent* event);
 void dragMoveEvent(QDragMoveEvent* event);
};

#endif
