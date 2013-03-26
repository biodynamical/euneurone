// TGlGraph.cpp

#include <qapplication.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qfont.h>
#include <qpainter.h>
#include <stdexcept>
#include <algorithm>

#include "controlobject.h"
#include "tglgraph.h"

#define BLACK_COLORSET

#define GL_XPOSCONTROL -0.85
#define GL_YPOSCONTROL -0.85
#define GL_XMARGLEGEND 0.1
#define GL_XPOSLEGEND 0.9
#define GL_YPOSLEGEND 0.9
#define GL_LEGENDW 0.2
#define GL_LEGENDH 0.1
#define GL_CUBE 0.1
#define GL_AXES_START 0.0
#define GL_AXES_STOP 0.05
#define GL_FONT_DEPTH 0.1
#define GL_LABEL_SCALE 0.2
#define GL_LEGEND_SCALE 0.1

#define ID_SELNONE 0
#define ID_SELX 1
#define ID_SELY 2
#define ID_SELZ 3
#define ID_SELICON 4
#define ID_SELLEGEND 5
#define ID_SELLEGSPHERE 6

#define FONT_LABELS 0
#define FONT_TICKS 1

#define MS_SLEEP 40

extern NS_Control::Control *pControl;

const QString qsX("X");
const QString qsY("Y");
const QString qsZ("Z");
const char *achNoList("Failed creating glList");
//=====================================================
TGlRender::TGlRender(TGlGraph *parent) : QThread()
{
  pwgt=parent;
  width=pwgt->width();
  height=pwgt->height();
  setVector3(mScale,1.0,1.0,1.0);
  setVector3(dZoom,1.0,1.0,1.0);
  setVector3(daAxis,1.0,1.0,1.0);
  setVector3(daOrigin,0.0,0.0,0.0);
  dAngle=-45.0;
  nSelect=0;
  listIndex=0;
  bAxes=true;
  bCube=true;
  b3dAxes=false;
  bLegend=true;
  bSelect=false;
  bLegExpand=false;
  bAxesTicks=true;
  bAxesLabels=true;
  bZoom=false;
  bRotate=false;
  bOrigin=false;
  bPoint=false;
  bAddData=false;
  bClearData=false;
  std::fill(&MM[0],&MM[MM_SIZE-1],0.0);
  std::fill(&PM[0],&PM[PM_SIZE-1],0.0);
  std::fill(&VP[0],&VP[VP_SIZE-1],0);
#ifdef BLACK_COLORSET  
  qclAxesPos=white;
  qclAxesNeg=darkGray;
  qclLabels=lightGray;
  qclLegendBgnd=black;
  qclSelect=yellow;
  qclDarkSelect=darkYellow;
  qclColor=lightGray;
  qclBckGnd=black;
#else
  qclAxesPos=black;
  qclAxesNeg=lightGray;
  qclLabels=darkGray;
  qclLegendBgnd=white;
  qclSelect=blue;
  qclDarkSelect=darkBlue;
  qclColor=darkGray;
  qclBckGnd=white;   
#endif
 //do something with the scales
  xAxis.reset();
  xAxis.adjust(-1.0,1.0);
  yAxis.reset();
  yAxis.adjust(-1.0,1.0);
  zAxis.reset();
  zAxis.adjust(-1.0,1.0);
}

TGlRender::~TGlRender()
{
 pwgt->makeCurrent();
 gluDeleteQuadric(quad);
}

void TGlRender::init()
{
 GLfloat lmodel_ambient[] = {0.4,0.4,0.4,1.0};
 
 pwgt->makeCurrent();
 if (!pwgt->isValid())
 {
 #ifdef _DEBUG
 	qDebug("[TGlRender::init()] Invalid parent widget");
 #endif
 //TODO: Error handling here
 }
 // Setup boundary conditions
 glDrawBuffer(GL_BACK);
 glClearDepth(1.0);
 glClearColor(0.0, 0.0, 0.0, 1.0);
 glLoadIdentity();
 glViewport(0,0,width,height);
 //position viewer
 glMatrixMode(GL_MODELVIEW);
 //inverse the z axis so the positive side is towards the viewer.
 glDepthRange(1,0);
 //setup light source
 faLight[0]=-1.0;
 faLight[1]=1.0;
 faLight[2]=0.999999;
 faLight[3]=0.0;
   
 glPixelStorei(GL_UNPACK_SWAP_BYTES,GL_FALSE);
 glPixelStorei(GL_UNPACK_LSB_FIRST,GL_FALSE);
 glPixelStorei(GL_UNPACK_ROW_LENGTH,width);

 glSelectBuffer(GL_SIZESELECTBUF, anSelectBuffer);

 glShadeModel(GL_SMOOTH);
 glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
 glLightfv(GL_LIGHT0,GL_POSITION,faLight);
 glLightfv(GL_LIGHT0,GL_AMBIENT,lmodel_ambient);
 glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lmodel_ambient);
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_COLOR_MATERIAL);

 glEnable(GL_LIGHTING);
 glEnable(GL_LIGHT0);
 
 pwgt->qglClearColor(qclBckGnd);
 glFinish();
 //create stock quadratic
 quad=gluNewQuadric();

 glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *)daTransform);
 setupMatrix();
}

void TGlRender::stop()
{
 bRunning=false;
}

void TGlRender::resize( int w, int h )
{
  width=w;
  height=h;
  bResize=true;
}

void TGlRender::paint()
{
 //clear old data
 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 //push current matrix on matrix stack
 glPushMatrix();
 glLoadIdentity();
 //draw the cube for orientation
 draw3DCube();
 //draw the legend
 drawLegend();
 //draw axes and labels
 drawAxes();
 drawAxesLabels();
 //if zoom selection
 if (bZoom)
 	drawZoomSphere();
 //draw the points
 drawLists();
 glPopMatrix();
 glFinish();
}
//thread main procedure
void TGlRender::run()
{ 
 pwgt->makeCurrent();
 bRunning=true;
 
 while(bRunning)
 {
/*#ifdef _DEBUG
  qDebug("[TGlRender::run] begin");
 #endif */
 //Check if data needs to be cleared
 if (bClearData)
 	clearData();
 //check if data needs to be added
 if (bAddData)
 	addData();
 //if resize required
 if (bResize)
  {
   glViewport(0,0,width,height);
   glPixelStorei(GL_UNPACK_ROW_LENGTH,width);
   bResize=false;
  }
 // if we are rotating, change the transform matrix
 if (bRotate)
  {
   setupMatrix();
   bRotate=false;
  }
 //if we need to calc the origin point
 if (bOrigin)
  {
   projectOrigin(xOrigin,yOrigin);
   bOrigin=false;
  }
 //if we need to calc a relative point
 if (bPoint)
  {
   projectPoint(xPoint,yPoint);
   bPoint=false;
  }
 //if we are selecting, do a selection
 if (bSelect)
 {
  unsigned int sel;
  sel=doSelect();
  if (sel!=nSelect)
  	nSelect=sel;
 }
 //finally, paint the thing
 paint();
 pwgt->swapBuffers();
/* #ifdef _DEBUG
  qDebug("[TGlRender::run]done swap");
 #endif*/
  msleep(MS_SLEEP);
 }
}
//================================================
void TGlRender::glColor(QColor c)
{
  double fr,fg,fb;
  
  fr=(double)c.red()/255.0;
  fg=(double)c.green()/255.0;
  fb=(double)c.blue()/255.0;
  glColor4d(fr,fg,fb,1.0);
}
//---------------------------------------------------------------------------
void TGlRender::glColor(QColor c,double d)
{
  double fr,fg,fb;
  
  fr=(float)c.red()/255.0;
  fg=(float)c.green()/255.0;
  fb=(float)c.blue()/255.0;
  glColor4d(fr,fg,fb,d);
}
//---------------------------------------------------------------------------
void TGlRender::setVector3(double *v,double f1,double f2,double f3)
{
 v[0]=f1;
 v[1]=f2;
 v[2]=f3;
}
//---------------------------------------------------------------------------
void TGlRender::setVector4(double *v,double f1,double f2,double f3,double f4)
{
 v[0]=f1;
 v[1]=f2;
 v[2]=f3;
 v[3]=f4;
}
//---------------------------------------------------------------------------
void TGlRender::setupMatrix()
{
  glFinish();
  glPushMatrix();
  glLoadIdentity();
  glRotated(dAngle, daAxis[0], daAxis[1], daAxis[2]);
  glMultMatrixd((GLdouble *)daTransform);
  glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble *)daTransform);
  glPopMatrix();
}
//---------------------------------------------------------------------------
void TGlRender::doTransform()
{
 glLoadIdentity();
 glMultMatrixd((GLdouble *)daTransform);
 glTranslated(daOrigin[0],daOrigin[1],daOrigin[2]);
 glScaled(mScale[0],mScale[1],mScale[2]);
 glScaled(dZoom[0],dZoom[1],dZoom[2]);
}
//---------------------------------------------------------------------------
void TGlRender::pointToVector(int x, int y, double *v)
{
  double d, a;
  /* project x, y onto a hemi-sphere centered within width, height. */
  v[0] = (2.0 * x - width) / width;
  v[1] = (height - 2.0 * y) / height;
  d = sqrt(v[0] * v[0] + v[1] * v[1]);
  v[2] = cos((M_PI / 2.0) * ((d < 1.0) ? d : 1.0));
  a = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] *= a;
  v[1] *= a;
  v[2] *= a;
}
//---------------------------------------------------------------------------
//public encapsulations
void TGlRender::pointToOrigin(int x, int y)
{
 pointToVector(x,y,daLast);
}
//---------------------------------------------------------------------------
void TGlRender::calcRotation(int x, int y)
{
  GLdouble dx, dy, dz;
  
  //get vector to new  position
  pointToVector(x,y, current_position);
  
  // calculate the angle to rotate by (directly proportional to the length of the mouse movement 
  dx = current_position[0] - daLast[0];
  dy = current_position[1] - daLast[1];
  dz = current_position[2] - daLast[2];
  dAngle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz);

   // calculate the axis of rotation (cross product)
  daAxis[0] = daLast[1] * current_position[2] - daLast[2] * current_position[1];
  daAxis[1] = daLast[2] * current_position[0] - daLast[0] * current_position[2];
  daAxis[2] = daLast[0] * current_position[1] - daLast[1] * current_position[0];

  //reset for next time
  daLast[0] = current_position[0];
  daLast[1] = current_position[1];
  daLast[2] = current_position[2];

  bRotate=true;
}
//---------------------------------------------------------------------------
void TGlRender::projectOrigin(double xpos, double ypos)
{
  glGetDoublev(GL_MODELVIEW_MATRIX,&MM[0]);
  glGetDoublev(GL_PROJECTION_MATRIX,&PM[0]);
  glGetIntegerv(GL_VIEWPORT,&VP[0]);
  gluProject(xpos, ypos, 0.0, MM, PM, VP, &daZoomOrigin[0], &daZoomOrigin[1], &daZoomOrigin[2]);
  gluUnProject(xpos, ypos, daZoomOrigin[2], MM, PM, VP, &daZoomOrigin[0], &daZoomOrigin[1], &daZoomOrigin[2]);
  daZoomOrigin[1]=-daZoomOrigin[1]; //negate y axis
}
//---------------------------------------------------------------------------
void TGlRender::projectPoint(double xpos, double ypos)
{
  glGetDoublev(GL_MODELVIEW_MATRIX,&MM[0]);
  glGetDoublev(GL_PROJECTION_MATRIX,&PM[0]);
  glGetIntegerv(GL_VIEWPORT,&VP[0]);
  gluProject(xpos, ypos, 0.0, MM, PM, VP, &daZoom[0], &daZoom[1], &daZoom[2]);
  gluUnProject(xpos, ypos, daZoom[2], MM, PM, VP, &daZoom[0], &daZoom[1], &daZoom[2]);
  daZoom[1]=-daZoom[1]; //negate y axis
  GLdouble dx=daZoomOrigin[0]-daZoom[0];
  GLdouble dy=daZoomOrigin[1]-daZoom[1];
  GLdouble dz=daZoomOrigin[2]-daZoom[2];
  dZoomRadius=sqrt(dx*dx+dy*dy+dz*dz);
  bZoom=true;
}
//---------------------------------------------------------------------------
void TGlRender::calcZoom()
{
  GLdouble dmin, dmax;
  
  dmin=(daZoomOrigin[0]-daZoom[0])/2.0;
  dmax=(daZoomOrigin[0]-daZoom[0])/2.0;
  dmin=dmin-dZoomRadius;
  dmax=dmax+dZoomRadius;
  dZoom[0]=(dmax-dmin)/dZoom[0];
  xAxis.adjust(dmin,dmax,1);
    
  dmin=(daZoom[1]-daZoomOrigin[1])/2.0;
  dmax=(daZoom[1]-daZoomOrigin[1])/2.0;
  dmin=dmin-dZoomRadius;
  dmax=dmax+dZoomRadius;
  dZoom[1]=(dmax-dmin)/dZoom[1];
  yAxis.adjust(dmin,dmax,1);
    
  dmin=(daZoomOrigin[2]-daZoom[2])/2.0;
  dmax=(daZoomOrigin[2]-daZoom[2])/2.0;
  dmin=dmin-dZoomRadius;
  dmax=dmax+dZoomRadius;
  dZoom[2]=(dmax-dmin)/dZoom[2];
  zAxis.adjust(dmin,dmax,1);
  bZoom=false;
  bPoint=false;
}
//---------------------------------------------------------------------------
void TGlRender::setSelected(int x, int y)
{
 bSelect=true;
 xSelect=x;
 ySelect=y;
}
//---------------------------------------------------------------------------
void TGlRender::setOrigin(int x, int y)
{
 bOrigin=true;
 xOrigin=x;
 yOrigin=y;
}
//---------------------------------------------------------------------------
void TGlRender::setPoint(int x, int y)
{
 bPoint=true;
 xPoint=x;
 yPoint=y;
}
//==================================================
void TGlRender::draw3DCube()
{
 if (bCube)
 {
  glPushMatrix();	//push current matrix
  glLoadIdentity();

  glDisable(GL_BLEND);
  glTranslatef(GL_XPOSCONTROL,GL_YPOSCONTROL,0.0);
  //initial rotation
  glMultMatrixd((GLdouble *)daTransform);
  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
  //draw six faces of a cube
  // NOTE: for unknown reasons this glBegin generates an Invalid Operation error
  // it works ok, but I don't know why.....
  glBegin(GL_QUADS);

  // Red
  glColor(red);
  glNormal3f(0.0,0.0,1.0);
  glVertex3f(GL_CUBE,GL_CUBE,GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,GL_CUBE);
  glVertex3f(-GL_CUBE,-GL_CUBE,GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,GL_CUBE);
   // Green
  glColor(green);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(-GL_CUBE,-GL_CUBE,-GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,-GL_CUBE);
  glVertex3f(GL_CUBE,GL_CUBE,-GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,-GL_CUBE);
   // Blue
  glColor(blue);
  glNormal3f(0.0,1.0,0.0);
  glVertex3f(GL_CUBE,GL_CUBE,GL_CUBE);
  glVertex3f(GL_CUBE,GL_CUBE,-GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,-GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,GL_CUBE);
  // Yellow
  glColor(yellow);
  glNormal3f(0.0,-1.0,0.0);
  glVertex3f(-GL_CUBE,-GL_CUBE,-GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,-GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,GL_CUBE);
  glVertex3f(-GL_CUBE,-GL_CUBE,GL_CUBE);
  // Light blue
  glColor(cyan);
  glNormal3f(1.0,0.0,0.0);
  glVertex3f(GL_CUBE,GL_CUBE,GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,GL_CUBE);
  glVertex3f(GL_CUBE,-GL_CUBE,-GL_CUBE);
  glVertex3f(GL_CUBE,GL_CUBE,-GL_CUBE);
  // Magenta
  glColor(magenta);
  glNormal3f(-1.0,0.0,0.0);
  glVertex3f(-GL_CUBE,-GL_CUBE,-GL_CUBE);
  glVertex3f(-GL_CUBE,-GL_CUBE,GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,GL_CUBE);
  glVertex3f(-GL_CUBE,GL_CUBE,-GL_CUBE);

  // End of quad
  glEnd();
  glPopMatrix();
 }
}
//---------------------------------------------------------------------------
void TGlRender::drawZoomSphere()
{ 
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(daZoom[0],daZoom[1],daZoom[2]);
  glMultMatrixd((GLdouble *)daTransform);
  
  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
  //enable transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glColor(qclSelect,0.1);

  gluQuadricNormals(quad,GLU_SMOOTH);
  gluQuadricDrawStyle(quad,GLU_FILL);
  gluSphere(quad,dZoomRadius,50,50);
  glDisable(GL_BLEND);
  glPopMatrix();
}
//---------------------------------------------------------------------------
void TGlRender::drawLists()
{
 unsigned int i,j;

 for(i=0;i<lvList.size();i++)
 {
  glPushMatrix();
  glLoadIdentity();
  doTransform();
  for (j=0;j<lvList[i].uvLists.size();j++)
    glCallList(lvList[i].uvLists[j]);
  glFlush();
  glPopMatrix();
 }
}
//---------------------------------------------------------------------------
int TGlRender::doSelect()
{
 unsigned int sel,hits,numnames;
 
 GLuint *p;
 sel=nSelect;

 //make sure a different selectbuffer is used...
 glSelectBuffer(GL_SIZESELECTBUF, anSelectBuffer);
 //push current matrix on matrix stack
 glPushMatrix();

 glGetIntegerv(GL_VIEWPORT,&VP[0]);
 glRenderMode(GL_SELECT);
 glInitNames();
 glPushName(ID_SELNONE);

 glMatrixMode(GL_PROJECTION);
 glPushMatrix();

 glLoadIdentity();
 gluPickMatrix(xSelect,VP[3]-ySelect,0.01,0.01,VP);

 glMatrixMode(GL_MODELVIEW);
 //doTransform();
 glMultMatrixd((GLdouble *)daTransform);
 
 drawAxes();
 selectAxesLabels();
 drawSelectLegend();
 
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();

 glMatrixMode(GL_MODELVIEW);

 hits=glRenderMode(GL_RENDER);
 p=anSelectBuffer;

//  found=false;
  sel=0;
  for (unsigned int i=0;i<hits;i++)
      {
       numnames=*p;
       p+=3;
       sel=*p;
       p+=numnames;
      }

 glPopMatrix();
 bSelect=false;
 return sel;
}
//---------------------------------------------------------------------------
void TGlRender::drawLegendIcon()
{
 float w=0.05,h=0.05;

 glPushMatrix();	//push current matrix
 glLoadIdentity();

 glTranslatef(GL_XPOSLEGEND,GL_YPOSLEGEND,0.0);

 if (nSelect==ID_SELICON)
        glColor(qclSelect);
 else
 	glColor(qclLegendBgnd);
 
 if (bSelect)
    glLoadName(ID_SELICON);

 glBegin(GL_QUADS);
 glNormal3f(0.0,0.0,-1.0);
 glVertex3f(w,h,-0.001);
 glVertex3f(-w,h,-0.001);
 glVertex3f(-w,-h,-0.001);
 glVertex3f(w,-h,-0.001);
 glEnd();
  
  if (bSelect)
  {
   glPopMatrix();
   return;
  }
  
  glColor(qclColor);
  glBegin(GL_LINE_LOOP);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(w,h,0);
  glVertex3f(-w,h,0);
  glVertex3f(-w,-h,0);
  glVertex3f(w,-h,0);
  glEnd();
  glBegin(GL_LINE_LOOP);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(0,0,0);
  glVertex3f(-w/2,-h/2,0);
  glVertex3f(w/2,-h/2,0);
  glEnd();

  glPopMatrix();

}
//---------------------------------------------------------------------------
void TGlRender::drawSelectLegend()
{
 if (bLegend)
 {
  unsigned int i,len, size,idx;
  float w,h,uw,uh;
 // if (lvList.size()==0)
 //  return;
  //collapsed?
  if (!bLegExpand)
   {
    drawLegendIcon();
    return;
   }

  //first calc max size:
 /* size=0; idx=0;
  for (i=0;i<lvList.size();i++)
  {
   if (size<lvList[i].sName.length())
   {
    size=lvList[i].sName.length();
    idx=i;
   }
  }
  //width
  uw=gmf['X'].gmfCellIncX;
  w=0;
  for (i=0;i<lvList[idx].sName.length();i++)
   w+=gmf[lvList[idx].sName[i]].gmfCellIncX;
  w+=(2*uw);
  //height
  uh=gmf['W'].gmfBlackBoxY;
  h=(lvList.size()+2)*uh;

  glPushMatrix();	//push current matrix
  glLoadIdentity();

  glTranslatef(GL_XPOSLEGEND,GL_YPOSLEGEND,0.0);
  glColorMaterial(GL_FRONT,GL_AMBIENT);

  glScalef(GL_LEGEND_SCALE,GL_LEGEND_SCALE,GL_LEGEND_SCALE);
  glTranslatef(-w/2+uw,-uh/2,0.0);

  glLoadName(ID_SELLEGEND);

  glBegin(GL_QUADS);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(w/2,h/2,-0.1);
  glVertex3f(-w/2,h/2,-0.1);
  glVertex3f(-w/2,-h/2,-0.1);
  glVertex3f(w/2,-h/2,-0.1);
  glEnd();

  glBegin(GL_LINE_LOOP);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(w/2,h/2,0);
  glVertex3f(-w/2,h/2,0);
  glVertex3f(-w/2,-h/2,0);
  glVertex3f(w/2,-h/2,0);
  glEnd();

  glListBase(GL_3D_FONT_LIST);

  for (i=0;i<lvList.size();i++)
  {
   glLoadName(ID_SELLEGSPHERE+i);
   glPushMatrix();	//push current matrix
   glLoadIdentity();
   glTranslatef(GL_XPOSLEGEND,GL_YPOSLEGEND,0.0);
   glScalef(GL_LEGEND_SCALE,GL_LEGEND_SCALE,GL_LEGEND_SCALE);
  // glTranslatef(-(size/2*uw)-(w/2)+(3.5*uw),-(i*uh*1.5),0);

   glPushMatrix();
 //  glTranslatef(0.2-uw,0.3,0);
   glTranslatef(-w+2*uw,-(i*uh*1.5),0.0);
   gluQuadricDrawStyle(quad,GLU_FILL);
   gluSphere(quad,0.3,8,8);
   glPopMatrix();

   glTranslatef(-w+2.75*uw,-(i*uh*1.5),0.0);
   glTranslatef(0,-uh*0.2,0.0);
   len=lvList[i].sName.length();
   glCallLists (len, GL_UNSIGNED_BYTE, lvList[i].sName.c_str());
   glPopMatrix();
  }
  glPopMatrix();*/
 }
}
//---------------------------------------------------------------------------
void TGlRender::drawLegend()
{
 if (bLegend)
 {
  unsigned int i,len, size,idx;
  float w,h,uw,uh;
  //if (lvList.size()==0)
  // return;
  //collapsed?
  if (!bLegExpand)
   {
    drawLegendIcon();
    return;
   }

  //first calc max size:
/*  size=0; idx=0;
  for (i=0;i<lvList.size();i++)
  {
   if (size<lvList[i].sName.length())
   {
    size=lvList[i].sName.length();
    idx=i;
   }
  }
  //width
  uw=gmf['X'].gmfCellIncX;
  w=0;
  for (i=0;i<lvList[idx].sName.length();i++)
   w+=gmf[lvList[idx].sName[i]].gmfCellIncX;
  w+=(2*uw);
  //height
  uh=gmf['W'].gmfBlackBoxY*1.2;
  h=(lvList.size()+2)*uh;

  glPushMatrix();	//push current matrix
  glLoadIdentity();

  glTranslatef(GL_XPOSLEGEND,GL_YPOSLEGEND,0.0);
  glColorMaterial(GL_FRONT,GL_AMBIENT);

  glScalef(GL_LEGEND_SCALE,GL_LEGEND_SCALE,GL_LEGEND_SCALE);
//  glTranslatef(-w/2+uw,-(lvList.size()*uh)+(0.5*uh),0.0);
//  glTranslatef(-w+2*uw,-(lvList.size()*uh*0.5),0.0);
  glTranslatef(-w+2.75*uw,-(lvList.size()*uh*1.5),0.0);
  glTranslatef(0,-uh*.2,0.0);
  SetColour(clLegendBgnd);

  glBegin(GL_QUADS);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(w/2,h/2,-0.1);
  glVertex3f(-w/2,h/2,-0.1);
  glVertex3f(-w/2,-h/2,-0.1);
  glVertex3f(w/2,-h/2,-0.1);
  glEnd();

  SetColour(clColor);
  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
  glBegin(GL_LINE_LOOP);
  glNormal3f(0.0,0.0,-1.0);
  glVertex3f(w/2,h/2,0);
  glVertex3f(-w/2,h/2,0);
  glVertex3f(-w/2,-h/2,0);
  glVertex3f(w/2,-h/2,0);
  glEnd();

  glListBase(GL_3D_FONT_LIST);

  for (i=0;i<lvList.size();i++)
  {
   glPushMatrix();	//push current matrix
   glLoadIdentity();
   glTranslatef(GL_XPOSLEGEND,GL_YPOSLEGEND,0.0);
   glScalef(GL_LEGEND_SCALE,GL_LEGEND_SCALE,GL_LEGEND_SCALE);
  // glTranslatef(-(size/2*uw)-(w/2)+(3.5*uw),-(i*uh*1.5),0);

   SetColour(lvList[i].Colour);
   glPushMatrix();
 //  glTranslatef(0.2-uw,0.3,0);
  glTranslatef(-w+2*uw,-(i*uh*1.5),0.0);
   gluQuadricDrawStyle(quad,GLU_FILL);
   gluSphere(quad,0.3,8,8);
   glPopMatrix();

  glTranslatef(-w+2.75*uw,-(i*uh*1.5),0.0);
  glTranslatef(0,-uh*0.2,0.0);
   len=lvList[i].sName.length();
   if (nSelect==(ID_SELLEGSPHERE+i))
     SetColour(clSelect);
   else
     SetColour(clColor);
   glCallLists (len, GL_UNSIGNED_BYTE, lvList[i].sName.c_str());
   glPopMatrix();
  }
  glPopMatrix();*/
 }
}
//-------------------------------------------------------------------------------
void TGlRender::drawAxes()
{
 double daPan[3];
 int i,n;
 double d,dstep;
 
 if (bAxes)
 {
  glPushMatrix();	//push current matrix
  if (bSelect)
    glLoadName(ID_SELX);

  glLoadIdentity();
  glMultMatrixd((GLdouble *)daTransform);

  //-- X axis --
  // white
  if (nSelect==ID_SELX)
        glColor(qclSelect);
  else
  	glColor(qclAxesPos);
  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

  if (b3dAxes)
  {
   glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
   gluQuadricDrawStyle(quad,GLU_FILL);
   glGetDoublev(GL_MODELVIEW_MATRIX,&MM[0]);
   glGetDoublev(GL_PROJECTION_MATRIX,&PM[0]);
   glGetIntegerv(GL_VIEWPORT,&VP[0]);
  }

  gluProject(1.0,0.0,0.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   glRotatef(90,0.0,1.0,0.0);
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(1.0,0.0,0.0);
   glEnd();
  }

  if (nSelect==ID_SELX)
        glColor(qclDarkSelect);
  else
	glColor(qclAxesNeg);
  gluProject(-1.0,0.0,0.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   glRotatef(-90,0.0,1.0,0.0);
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(-1.0,0.0,0.0);
   glVertex3f(0.0,0.0,0.0);
   glEnd();
  }
  // Do ticks
  if (bAxesTicks)
  {
   if (nSelect==ID_SELX)
        glColor(qclSelect);
   else
 	glColor(qclAxesNeg);
   n=xAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);   
   for (i=0;i<n; i++)
   {
    d=xAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(-1.0+((double)i*dstep),0.0,0.0);
    gluSphere(quad,0.02,10,10);
    glPopMatrix();
   }
  }

  //-- Y axis --
  if (bSelect)
    glLoadName(ID_SELY);
  
 if (nSelect==ID_SELY)
        glColor(qclSelect);
 else
	glColor(qclAxesPos);
  gluProject(0.0,1.0,0.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   glRotatef(-90,1.0,0.0,0.0);
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(0.0,1.0,0.0);
   glEnd();
  }

  if (nSelect==ID_SELY)
        glColor(qclDarkSelect);
  else
	glColor(qclAxesNeg);
  gluProject(0.0,-1.0,0.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   glRotatef(90,1.0,0.0,0.0);
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(0.0,-1.0,0.0);
   glVertex3f(0.0,0.0,0.0);
   glEnd();
  }
  
  if (bAxesTicks)
  {
   if (nSelect==ID_SELY)
        glColor(qclSelect);
   else
        glColor(qclAxesNeg);
   n=yAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);
   for (i=0;i<n; i++)
   {
    d=yAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(0.0,-1.0+((double)i*dstep),0.0);
    gluSphere(quad,0.02,10,10);
    glPopMatrix();
   }
  }
  //-- Z axis --
 if (bSelect)
    glLoadName(ID_SELZ);

 if (nSelect==ID_SELZ)
        glColor(qclSelect);
 else
	glColor(qclAxesPos);
  gluProject(0.0,0.0,1.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(0.0,0.0,1.0);
   glEnd();
  }

  if (nSelect==ID_SELZ)
        glColor(qclDarkSelect);
  else
	glColor(qclAxesNeg);
  gluProject(0.0,0.0,-1.0,&MM[0],&PM[0],&VP[0],&daPan[0],&daPan[1],&daPan[2]);

  if ((b3dAxes)&&(daPan[2]<=0.5))
  {
   glPushMatrix();
   glRotatef(180,0.0,1.0,0.0);
   gluCylinder(quad,GL_AXES_START,GL_AXES_STOP,1.0,10,10);
   glPopMatrix();
  }
  else
  {
   glBegin(GL_LINES);
   glVertex3f(0.0,0.0,-1.0);
   glVertex3f(0.0,0.0,0.0);
   glEnd();
  }
  
  if (bAxesTicks)
  {
   if (nSelect==ID_SELZ)
        glColor(qclSelect);
   else
        glColor(qclAxesNeg);
   n=zAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);
   for (i=0;i<n; i++)
   {
    d=zAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(0.0,0.0,-1.0+((double)i*dstep));
    gluSphere(quad,0.02,10,10);
    glPopMatrix();
   }
  }

  glPopMatrix();
 }
}
//---------------------------------------------------------------------------
// Note: the qt function renderText is not recognized by gl as a selectable element
// therefore, for selection purposes we simply draw some rect that is used as a placeholder
void TGlRender::selectAxesLabels()
{
 if (bAxes&bSelect)
 { 
  glPushMatrix();
  // x axis labels
  glLoadIdentity();
  glMultMatrixd((GLdouble *)daTransform);
  glTranslatef(1.0,0.0,0.1);
  glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);
   
   glLoadName(ID_SELX);
   glBegin(GL_QUADS);
   glNormal3f(0.0,0.0,-1.0);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(1.0,0.0,0.0);
   glVertex3f(1.0,1.0,0.0);
   glVertex3f(0.0,1.0,0.0);
   glEnd();
   
   glLoadIdentity();
   glMultMatrixd((GLdouble *)daTransform);
   glTranslatef(0.0,1.0,0.1);
   glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);
   glLoadName(ID_SELY);
   glBegin(GL_QUADS);
   glNormal3f(0.0,0.0,-1.0);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(1.0,0.0,0.0);
   glVertex3f(1.0,1.0,0.0);
   glVertex3f(0.0,1.0,0.0);
   glEnd();

   glLoadIdentity();
   glMultMatrixd((GLdouble *)daTransform);
   glTranslatef(0.0,0.0,1.1);
   glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);
   glLoadName(ID_SELZ);
   glBegin(GL_QUADS);
   glNormal3f(0.0,0.0,-1.0);
   glVertex3f(0.0,0.0,0.0);
   glVertex3f(1.0,0.0,0.0);
   glVertex3f(1.0,1.0,0.0);
   glVertex3f(0.0,1.0,0.0);
   glEnd();
   
   glPopMatrix();
 }
}

void TGlRender::drawAxesLabels()
{
 int i,n;
 double d,dstep;
 QString qs;
 
 if (bAxes)
 {
  glPushMatrix();

  // x axis labels
  glLoadIdentity();
  glMultMatrixd((GLdouble *)daTransform);

  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);

  glTranslatef(1.0,0.0,0.1);
  glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);

  if (nSelect==ID_SELX)
        glColor(qclSelect);
  else
        glColor(qclColor);
  
  pwgt->renderTextOrigin(qsX,FONT_LABELS);
  
  if (bAxesLabels)
  {
   glLoadIdentity();
   glMultMatrixd((GLdouble *)daTransform);
   n=xAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);
   for (i=0;i<n; i++)
   {
    d=xAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(-1.0+((double)i*dstep),-0.05,0.0);
    qs.setNum(d);
    pwgt->renderTextOrigin(qs,FONT_TICKS);
    glPopMatrix();
   }
  }

  glLoadIdentity();
  glMultMatrixd((GLdouble *)daTransform);
  glTranslatef(0.0,1.0,0.1);
  glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);

  if (nSelect==ID_SELY)
        glColor(qclSelect);
  else
        glColor(qclColor);
  pwgt->renderTextOrigin(qsY,FONT_LABELS);

  if (bAxesLabels)
  {
   glLoadIdentity();
   glMultMatrixd((GLdouble *)daTransform);
   n=yAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);
   for (i=0;i<n; i++)
   {
    d=yAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(0.0,-1.0+((double)i*dstep),-0.05);
    qs.setNum(d);
    pwgt->renderTextOrigin(qs,FONT_TICKS);
    glPopMatrix();
   }
  }
  
  glLoadIdentity();
  glMultMatrixd((GLdouble *)daTransform);
  glTranslatef(0.0,0.0,1.1);
  glScalef(GL_LABEL_SCALE,GL_LABEL_SCALE,GL_LABEL_SCALE);
  
  if (nSelect==ID_SELZ)
        glColor(qclSelect);
  else
        glColor(qclColor);
  pwgt->renderTextOrigin(qsZ, FONT_LABELS);
  
  if (bAxesLabels)
  {
   glLoadIdentity();
   glMultMatrixd((GLdouble *)daTransform);
   n=zAxis.scaleDiv().majCnt();
   dstep=2.0/((double)n-1.0);
   for (i=0;i<n; i++)
   {
    d=zAxis.scaleDiv().majMark(i);
    glPushMatrix();
    //glScaled(mScale[0],mScale[1],mScale[2]); 
    glTranslated(0.0,-0.05,-1.0+((double)i*dstep));
    qs.setNum(d);
    pwgt->renderTextOrigin(qs,FONT_TICKS);
    glPopMatrix();
   }
  }
  glPopMatrix();
 }
}

//========================================
double TGlRender::calcScale(double d,const dblDequeT &pd,int axisscale)
{
 double dmin,dmax;
 double ds,dh,dl,dz;
 //calc the scaling
 dmin=*std::min_element(pd.begin(),pd.end());
 dmax=*std::max_element(pd.begin(),pd.end());
 if (dmin==dmax)
  return d;
 switch(axisscale)
 {
  default: ds=1.0/(dmax-dmin); break;
  case ID_SELX: 
  	{
	 xAxis.adjust(dmin,dmax);
	 dh=xAxis.scaleDiv().hBound();
	 dl=xAxis.scaleDiv().lBound();
	 dz=dl-dh;
	 if (dz!=0.0)
	  ds=2.0/dz;
	 else
	  ds=0.0;
	 daOrigin[0]=ds;
	 dz=dh-dl;
	 if (dz!=0.0)
	  ds=1.0/dz;
	 else
	  ds=1.0;
	 break;
	} 
  case ID_SELY:
  	{
	 yAxis.adjust(dmin,dmax);
	 dh=yAxis.scaleDiv().hBound();
	 dl=yAxis.scaleDiv().lBound();
	 dz=dl-dh;
	 if (dz!=0.0)
	  ds=2.0/dz;
	 else
	  ds=0.0;
	 daOrigin[1]=ds;
	 dz=dh-dl;
	 if (dz!=0.0)
	  ds=1.0/dz;
	 else
	  ds=1.0;
	 break;
	} 
  case ID_SELZ:
  	{
	 zAxis.adjust(dmin,dmax);
	 dh=zAxis.scaleDiv().hBound();
	 dl=zAxis.scaleDiv().lBound();
	 dz=dl-dh;
	 if (dz!=0.0)
	  ds=2.0/dz;
	 else
	  ds=0.0;
	 daOrigin[2]=ds;
	 dz=dh-dl;
	 if (dz!=0.0)
	  ds=1.0/dz;
	 else
	  ds=1.0;
	 break;
	} 
 }
#ifdef _DEBUG
	qDebug("[TGlRender::calcScale] dmin=%f, dmax=%f, scale=%f, new=%f",dmin,dmax,d,ds);
#endif
 if (ds<d)
  return ds;
 return d;
}
//---------------------------------------------------------------------------
unsigned int TGlRender::newSeries(const QString &qs, QColor c)
{
 structList sl;

 sl.sName=qs;
 sl.Colour=c;
 lvList.push_back(sl);
 return (lvList.size()-1);
}
//---------------------------------------------------------------------------
void TGlRender::updateSeriesName(const QString &qs,unsigned int nidx)
{
 if (nidx>=lvList.size())
  return;
 lvList[nidx].sName=qs;
}
//---------------------------------------------------------------------------
void TGlRender::addData()
{
 unsigned int lst,i,size;
 double x,y,z;

 bAddData=false;
 //add points to display list by creating a new one 
 lst=glGenLists(1);
 if (lst==0)
  {
   qDebug("Error creating list: glGenLists");
   return;
  }
 lvList[listIndex].uvLists.push_back(lst);
#ifdef _DEBUG
 qDebug("[TGlRender::addData] begin list=%d (%d)",lst,listIndex);
#endif

 mScale[0]=calcScale(mScale[0],dataX,ID_SELX);
 mScale[1]=calcScale(mScale[1],dataY,ID_SELY);
 mScale[2]=calcScale(mScale[2],dataZ,ID_SELZ);
  
 glNewList(lst,GL_COMPILE);

 glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
 glColor(lvList[listIndex].Colour,1.0);

 glEnable(GL_DEPTH_TEST);

 //enable transparency
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

 //smoothing and size
 //glEnable(GL_POINT_SMOOTH);
 //glPointSize(4.0);
 glEnable(GL_LINE_SMOOTH);
 //glLineStipple(1,52428);
// glEnable(GL_LINE_STIPPLE);
// glLineWidth(2.0);

  glBegin(GL_LINE_STRIP);

 size=dataX.size();
// glBegin(GL_POINTS);
 for (i=0;i<size;i++)
  {
   x=dataX[i];
   y=dataY[i];
   z=dataZ[i];
   glVertex3d(x,y,z);
  }
 glEnd();

 //glDisable(GL_LINE_STIPPLE);
 glDisable(GL_BLEND);
 glDisable(GL_DEPTH_TEST);
 glEndList();
 //clear data lists
 dataX.clear();
 dataY.clear();
 dataZ.clear();
#ifdef _DEBUG
 qDebug("[TGlRender::addData] done %d",size);
#endif
 }
//---------------------------------------------------------------------------
void TGlRender::clearData()
{
 unsigned int i,j;

 bClearData=false;
 for (i=0;i<lvList.size();i++)
 {
  for (j=0;j<lvList[i].uvLists.size();j++)
  {
   glDeleteLists(lvList[i].uvLists[j],1);
  }
  lvList[i].uvLists.erase(lvList[i].uvLists.begin(),lvList[i].uvLists.end());
 }
}
//---------------------------------------------------------------------------
void TGlRender::addPoints(unsigned int idx,const  double *px,const  double *py,const  double *pz,unsigned int size)
{
 unsigned int i;

 if (idx>=lvList.size())
 	return;
 if (size==0)
 	return;
 listIndex=idx;
 for (i=0;i<size;i++)
  {
   dataX.push_back(px[i]);
   dataY.push_back(py[i]);
   dataZ.push_back(pz[i]);
  }
  bAddData=true;
}
//---------------------------------------------------------------------------
void TGlRender::appendPoints(unsigned int idx,const dblDequeT &px,const dblDequeT &py,const dblDequeT &pz,
					unsigned int xidx,unsigned int yidx, unsigned int zidx)
{
 unsigned int xsize,ysize,zsize;

 xsize=px.size();
 ysize=py.size();
 zsize=pz.size();
 if ((xsize==0)||(ysize==0)||(zsize==0))
 	return;
 if ((idx>=lvList.size())||(xidx>=xsize)||(yidx>=ysize)||(zidx>=zsize))
 	return;
  
 try
 {
  
  bool bDone=false;
  listIndex=idx;
  while(!bDone)
  {
   dataX.push_back(px[xidx]);
   dataY.push_back(py[yidx]);
   dataZ.push_back(pz[zidx]);
   xidx++;
   yidx++;
   zidx++;
   if ((xidx>=xsize)||(yidx>=ysize)||(zidx>=zsize))
    bDone=true;
  }
  bAddData=true;
}
catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
     }
}
//---------------------------------------------------------------------------
void TGlRender::addPoints(unsigned int idx,const dblDequeT &px,const dblDequeT &py,const dblDequeT &pz)
{
 unsigned int i,xsize,ysize,zsize;

 if (idx>=lvList.size())
  return;
 xsize=px.size();
 ysize=py.size();
 zsize=pz.size();
 if ((xsize==0)||(ysize==0)||(zsize==0))
 	return;
 //add points to display list by creating a new one
 try
 {
  listIndex=idx;
  dataX.push_back(px[0]);
   dataY.push_back(py[0]);
   dataZ.push_back(pz[0]);
   for (i=1;(i<xsize)&&(i<ysize)&&(i<zsize);i++)
     {
     dataX.push_back(px[i]);
     dataY.push_back(py[i]);
     dataZ.push_back(pz[i]);
     }
  bAddData=true;
}
catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
     }
}
//---------------------------------------------------------------------------
void TGlRender::clear()
{
 bClearData=true;
 listIndex=0;
}
//==================================================
// The widget that handles the window and events
//=====================================================
TGlGraph::TGlGraph(QWidget *parent, const char* name,QGLWidget *pshare) : QGLWidget(parent, name,pshare)
{
 //create thread
 pRender=new TGlRender(this);
 mAction=emRotate;
 bTrack=false;
 bZoom=false;
 bReady=false;
 //Note: apparently the GLX can not use TTF fonts in gl environment (!),
 //so we'll get Qt to use only bitmap fonts
 fontLabels= parent->font();
 fontTicks= parent->font();
#if QT_VERSION<=0x030101
 fontLabels.setStyleStrategy(QFont::PreferBitmap);
 fontTicks.setStyleStrategy(QFont::PreferBitmap);
#endif
 fontLabels.setBold(true);
 fontLabels.setPointSize(14);
 fontTicks.setBold(false);
 fontTicks.setPointSize(10);
 setAutoBufferSwap(false);
 setMouseTracking(true);
 setAcceptDrops(true);
}

TGlGraph::~TGlGraph()
{
 if(pRender->running())
 {
  pRender->stop();
  pRender->wait();
 }
 delete pRender;
}

void TGlGraph::replot()
{
//    doGL();
// updateGL();
}

void TGlGraph::initializeGL()
{
 if (!bReady)
 {
  //check if context has been created
 #ifdef _DEBUG
  qDebug("[TGlGraph::initizializeGL] hasopengl=%d, valid=%d",QGLFormat::hasOpenGL(),context()->isValid());
 #endif
 if ((!QGLFormat::hasOpenGL ())||(!context()->isValid()))
  	return;
  doneCurrent();
  pRender->init();
  bReady=true;
 #ifdef _DEBUG
  qDebug("[TGlGraph::initizializeGL] done");
 #endif
 }
}

void TGlGraph::resizeGL(int w,int h)
{
 if (bReady)
 	pRender->resize(w,h);
}

void TGlGraph::paintGL()
{ //paint asked, so run thread
 if (bReady)
 {
  #ifdef _DEBUG
  qDebug("[TGlGraph::painGL]begin");
 #endif
 if (!pRender->running())
  	{
	 doneCurrent();
 	 pRender->start();
	}
 }
  #ifdef _DEBUG
  qDebug("[TGlGraph::paintGL]done");
 #endif
}

void TGlGraph::renderTextOrigin(const QString &qs,int type)
{
  switch(type)
 {
  default:
  case FONT_LABELS:
  {
   renderText(0.0,0.0,0.0,qs,fontLabels);
   break;
  }
  case FONT_TICKS:
  {
   renderText(0.0,0.0,0.0,qs,fontTicks);
   break;
  }
 }
}

void TGlGraph::mousePressEvent(QMouseEvent*e)
{
 if (!bReady)
	return;
 switch(mAction)
 {
  default: break;
  case emRotate:
  {
   bTrack=true;
   pRender->pointToOrigin(e->x(),e->y());
   setCursor(Qt::BlankCursor);
   break;
  }
  case emZoom:
  {
   bTrack=true;
   bZoom=true;
   setCursor(Qt::BlankCursor);
   pRender->setOrigin(e->x(),e->y());
   break;
  }
 }
}
//---------------------------------------------------------------------------
void TGlGraph::mouseMoveEvent(QMouseEvent*e)
{
 if (!bReady)
	return;
 if (bTrack)
 {
  switch(mAction)
  {
   default: break;
   case emRotate:
   {
    pRender->calcRotation(e->x(), e->y());
    break;
   }
   case emZoom:
   {
    pRender->setPoint(e->x(),e->y());
    break;
   }
  }
 }
 else
 {
   pRender->setSelected(e->x(),e->y());
  }
}
//---------------------------------------------------------------------------
void TGlGraph::mouseReleaseEvent(QMouseEvent*)
{
 if (!bReady)
	return;
 if (bTrack)
   unsetCursor();
 bTrack=false;
 if (bZoom)
   {//calc the boundaries of the zoom area
    bZoom=false;
    pRender->calcZoom();
   }
}
//---------------------------------------------------------------------------
void TGlGraph::dragEnterEvent(QDragEnterEvent* event)
{
 bool b;
 
 b=NS_Control::TDragObject::canDecode(event);
 if (b)
 {
  QWidget *p=(QWidget *)parent();
  p->setActiveWindow();
  QTimer::singleShot(500,p,SLOT(raise()));
 }
 if (!bReady)
 	b=false;
 event->accept(b);    
}
//---------------------------------------------------------------------------
void TGlGraph::dragMoveEvent(QDragMoveEvent* event)
{
 if (!bReady)
	return;
 if (!NS_Control::TDragObject::canDecode(event))
 	{
 	 event->ignore();
	 return;
 	}
 QPoint qp=event->pos();
 pRender->setSelected(qp.x(),qp.y());
 event->accept();
//      pTimer->Enabled=true;
#ifdef _DEBUG
 qDebug("[TGlGraph::dragMoveEvent]");
#endif
}
//---------------------------------------------------------------------------
void TGlGraph::dropEvent(QDropEvent* event)
{
 unsigned int nId;
 
 try
 {
  if (!bReady)
	return;
  if (NS_Control::TDragObject::decode(event, &nId))
  {
#ifdef _DEBUG
 qDebug("[TGlGraph::dropEvent] decode id=%d",nId);
#endif
 	;//pControl->AddDataStore(this, nId);
  }
 }
  catch (std::exception &stdex)
     {
      pControl->Error(stdex.what());
     }

}
//========================================
void TGlGraph::setMouseAction(TMouseAction m)
{
 mAction=m;
}

//========================================
void TGlGraph::addVariables(QStringList *psl)
{ 
 unsigned int i,id;
 struct3DGraph sg;
 
 if (!bReady)
	return;
 sg.nXVarId=IDINVALID;
 sg.nYVarId=IDINVALID;
 sg.nZVarId=IDINVALID;
 sg.pPlot=this;
 for (i=0;i<psl->size();i++)
 {
  id=pControl->GetVarIdx(*psl->at(i));
  if (id!=IDINVALID)
  {
    if (!pControl->GetVarStruct3D(id,sg,(i%3)))
   	return;
  }  
  if ((sg.nXVarId!=IDINVALID)&&(sg.nYVarId!=IDINVALID)&&(sg.nZVarId!=IDINVALID))
  {
   sg.uCurve=pRender->newSeries(sg.Title,sg.color);
   pControl->add3DSeries(sg);
   sg.nXVarId=IDINVALID;
   sg.nYVarId=IDINVALID;
   sg.nZVarId=IDINVALID;
  }
 }
}

/*void TGlGraph::addVariable(unsigned int n, struct3DGraph &sg, int axis)
{ 
 if (!pControl->GetVarStruct3D(n,sg,axis))
   return;
  
  sg.pPlot=this;
   //create curve
  sg.uCurve=pRender->newSeries(sg.qs,sg.color);
}*/

unsigned int TGlGraph::newSeries(const QString &qs, QColor c)
{
  if (!bReady)
	return 0;
  return (pRender->newSeries(qs,c));
}
//---------------------------------------------------------------------------
void TGlGraph::updateSeriesName(const QString &qs,unsigned int nidx)
{
 if (!bReady)
	return;
 pRender->updateSeriesName(qs,nidx);
}
//---------------------------------------------------------------------------
void TGlGraph::addPoints(unsigned int idx,const  double *px,const  double *py,const  double *pz,unsigned int size)
{
 if (!bReady)
	return;
  pRender->addPoints(idx,px,py,pz,size);
}
//---------------------------------------------------------------------------
void TGlGraph::appendPoints(unsigned int idx,const dblDequeT &px,const dblDequeT &py,const dblDequeT &pz,
					unsigned int xidx,unsigned int yidx, unsigned int zidx)
{
 if (!bReady)
	return;
  pRender->appendPoints(idx,px,py,pz,xidx,yidx,zidx);
}
//---------------------------------------------------------------------------
void TGlGraph::addPoints(unsigned int idx,const dblDequeT &px,const dblDequeT &py,const dblDequeT &pz)
{
 if (!bReady)
	return;
  pRender->addPoints(idx,px,py,pz);
}
//---------------------------------------------------------------------------
void TGlGraph::clear()
{
 if (bReady)
 	pRender->clear();
}
//---------------------------------------------------------------------------
