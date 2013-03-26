//
// C++ Implementation: graphtype
//
// Description: implementation of derived class dblDequeData which holds data for graphs
//
//
// Author: Tjeerd olde Scheper <tvolde-scheper@brookes.ac.uk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "graphtype.h"
/*
  Constructors
 */
 dblDequeData::dblDequeData()
 {
  px=NULL;
  py=NULL;
  initCache();
 }
 
 dblDequeData::dblDequeData(const dblDequeT &x, const dblDequeT &y)
{
 px=new dblDequeT(x);
 py=new dblDequeT(y);
 initCache();
}

void dblDequeData::initData()
{
 px=new dblDequeT();
 py=new dblDequeT();
 initCache();
}

/*
 dblDequeData&  dblDequeData::operator=(const  dblDequeData &data)
{
    if (this != &data)
    {
      delete py;
      delete px;
      px = new dblDequeT(*(data.px));
      py =new dblDequeT(*(data.py));
      initCache();
    }
    return *this;
}*/

//destructor
dblDequeData::~ dblDequeData()
{
  if (py)
  	{
  	 delete py;
	 py=NULL;
	}
  if (px)
  	{
  	 delete px;
	 py=NULL;
	}
}

size_t  dblDequeData::size() const 
{ 
  if (!px)
    return 0;
  return QMIN(px->size(), py->size()); 
}

double  dblDequeData::x(size_t i) const 
{ 
    return (*px)[(unsigned int)i];
}

double  dblDequeData::y(size_t i) const 
{ 
    return (*py)[(unsigned int)i]; 
}

//This copy member is used by qwt to get a copy of the object
// in our case we do not want to copy the data (it is already copied)
// to be safe, we make a copy of the object but use the same data pointers
QwtData * dblDequeData::copy() const 
{ 
  dblDequeData *p;//=new dblDequeData();
  p=const_cast<dblDequeData *>(this);
  return p; 
}

/*!
  Returns the bounding rectangle of the data. If there is
  no bounding rect, like for empty data the rectangle is invalid:
  QwtDoubleRect::isValid() == FALSE
*/
QwtDoubleRect  dblDequeData::boundingRect() const
{
    return QwtDoubleRect(d_cache.x1(), d_cache.x2(), d_cache.y1(), d_cache.y2());
}

void  dblDequeData::initCache()
{
    const size_t sz = size();

    if ( sz <= 0 )
    {
        d_cache = QwtDoubleRect(1.0, -1.0, 1.0, -1.0); // invalid
        return;
    }

    double minX, maxX, minY, maxY;
    
    dblDequeT::const_iterator xIt = px->begin();
    dblDequeT::const_iterator yIt = py->begin();
    dblDequeT::const_iterator end = px->end();
    minX = maxX = *xIt++;
    minY = maxY = *yIt++;

    while ( xIt < end )
    {
        const double xv = *xIt++;
        if (isfinite(xv))
        {
         if ( xv < minX )
            minX = xv;
         if ( xv > maxX )
            maxX = xv;
       }

        const double yv = *yIt++;
        if (isfinite(yv))
        {
         if ( yv < minY )
            minY = yv;
         if ( yv > maxY )
            maxY = yv;
       }
    }
    
    d_cache.setRect(minX, maxX, minY, maxY);
}

void dblDequeData::clearData()
{
  if (!px)
 	return;
  px->clear();
  py->clear();
  initCache();
}

void dblDequeData::setData(const dblDequeT *pdX, const dblDequeT *pdY)
{
  unsigned int i;
  
  if (!px)
 	return;
  px->clear();
  py->clear();
  for (i=0;i<pdX->size();i++)
	 	px->push_back((*pdX)[i]);
  for (i=0;i<pdY->size();i++)
	 	py->push_back((*pdY)[i]);
  initCache();
}

void dblDequeData::addData(const dblDequeT *pdX, const dblDequeT *pdY)
{
  unsigned int i;
  double minX, maxX, minY, maxY,d;
  size_t sz;
  
  if (!px)
 	return;
 //setup cache sizes
  minX=d_cache.x1();
  maxX=d_cache.x2();
  minY= d_cache.y1();
  maxY=d_cache.y2();
  
  sz=pdX->size();
  for (i=0;i<sz;i++)
  		{
  		 d=(*pdX)[i];
	 	 px->push_back(d);
	 	 if (isfinite(d))
	 	 {
	 	  if (d<minX)
	 	 	minX=d;
	 	  if (d>maxX)
	 	 	maxX=d;
	 	 }
	 	}
  sz=pdY->size();
  for (i=0;i<sz;i++)
  		{
  		 d=(*pdY)[i];
	 	 py->push_back(d);
	 	 if (isfinite(d))
	 	 {
	 	  if (d<minY)
	 	 	minY=d;
	 	  if (d>maxY)
	 	 	maxY=d;
	 	 }
	 	}
  d_cache.setRect(minX, maxX, minY, maxY);
}

void dblDequeData::appendData(const dblDequeT *pdX,unsigned int szX,const dblDequeT *pdY, unsigned int szY)
{
  unsigned int i;
  double minX, maxX, minY, maxY,d;
  size_t sz;
  
  if (!px)
 	return;
 //setup cache sizes
  minX=d_cache.x1();
  maxX=d_cache.x2();
  minY= d_cache.y1();
  maxY=d_cache.y2();
  
  sz=pdX->size();
  for (i=szX;i<sz;i++)
  		{
  		 d=(*pdX)[i];
	 	 px->push_back(d);
	 	 if (isfinite(d))
	 	 {
	 	  if (d<minX)
	 	 	minX=d;
	 	  if (d>maxX)
	 	 	maxX=d;
	 	 }
	 	}
  sz=pdY->size();
  for (i=szY;i<sz;i++)
  		{
  		 d=(*pdY)[i];
	 	 py->push_back(d);
	 	 if (isfinite(d))
	 	 {
	 	  if (d<minY)
	 	 	minY=d;
	 	  if (d>maxY)
	 	 	maxY=d;
	 	 }
	 	}
  d_cache.setRect(minX, maxX, minY, maxY);
}

void dblDequeData::subsetData(const dblDequeT *pdX, const dblDequeT *pdY,unsigned int start, unsigned int stop)
{
  unsigned int i;
  
  if (!px)
 	return;
  px->clear();
  py->clear();
  for (i=start;i<stop;i++)
	 	px->push_back((*pdX)[i]);
  for (i=start;i<stop;i++)
	 	py->push_back((*pdY)[i]);
  initCache();
}
/* ---------------------------------------------------------
	Constructor : graphType
    --------------------------------------------------------- */

graphType::graphType()
{
 nYVarId=0;
 nXVarId=0;
 pPlot=NULL;
 lCurve=0;
 pData=NULL;
 iStart=0;
 iStop=0;
 nSizeX=0;
 nSizeY=0;
 sType=stLine;
 style=QwtSymbol::Ellipse;
 pen=QPen();
 pen.setWidth(1);
 pen.setStyle(Qt::PenStyle(1));
 nSize=4;
}

 graphType & graphType::operator=(const graphType &gt)
 {
  nYVarId=gt.nYVarId;
  nXVarId=gt.nXVarId;
  lCurve=gt.lCurve;
  //pointer copy
  pPlot=gt.pPlot;
  pData=gt.pData;
  iStart=gt.iStart;
  iStop=gt.iStop;
  nSizeX=gt.nSizeX;
  nSizeY=gt.nSizeY;
  sType=gt.sType;
  nSize=gt.nSize;
  pen=gt.pen;
  style=gt.style;
  Title=gt.Title;
  return *this;
 }
 
 void graphType::replot()
 {
  pPlot->replot();
 }

 void graphType::newCurve(unsigned int idx, unsigned int idy)
 {
   nXVarId=idx;
   nYVarId=idy;
   newCurve();
 }
 
 void graphType::newCurve()
 {
   //create curve
   lCurve=pPlot->insertCurve(Title);
   pPlot->setCurvePen(lCurve,pen);
   pPlot->setCurveSymbol(lCurve,QwtSymbol());
   pPlot->setCurveStyle(lCurve,style);
   sType=stLine;
   pPlot->enableLegend(true,lCurve);
   pData=new dblDequeData();
   pData->initData();
   pPlot->setCurveData(lCurve,*pData);
 }
 
 void graphType::deleteCurve()
 {
  pData->clearData();
  pPlot->removeCurve(lCurve);
  //Because Qwt had been given the pointer to the data object,
  // when calling remove curve, it will delete it because it thinks it
  // owns a copy (which would be impractical because then
  // we can't change the data anymore). So *our* pData pointer
  // has become dangling and can be set to NULL.
  pData=NULL;
  lCurve=0;
 }
 
 bool graphType::isPlot(QwtPlot *p)
{
 return (pPlot==p);
}

 bool graphType::isValid(unsigned int max)
 {
   if ((nYVarId>=max)||(nXVarId>=max))
	return false;
   return true;
 }
 
bool graphType::isCurve(long l)
{
 return (l==lCurve);
}

bool graphType::isSubset(unsigned int start, unsigned int stop)
{
 if ((start<=nSizeY)&&(start<=nSizeX)&&(stop<=nSizeX)&&(stop<=nSizeY)&&(start<=stop))
 	return true;
 return false;
}

void graphType::setPlot(const QwtPlot *p)
{
 pPlot=const_cast<QwtPlot *>(p);
}

 void graphType::setTitle(const QString &qs)
 {
  Title=qs;
 }
 
void graphType::setTitle(const char *p)
{
 Title=p;
}

 void graphType::clearData()
 {
  if(!pData)
    return;
   pData->clearData();
   iStart=0;
   iStop=0;
   nSizeX=0;
   nSizeY=0;
 }
 
  bool graphType::setData(const dblDequeT *pdX,const dblDequeT *pdY)
 {
   if (!pData)
   	return false;
   pData->setData(pdX, pdY);
   nSizeX=pdX->size();
   nSizeY=pdY->size();
   iStart=0;
   iStop=nSizeY;
  return true;
 }
 
 bool graphType::addData(const dblDequeT *pdX,const dblDequeT *pdY)
 {
   if ((nSizeY!=iStop)||(!pData)||(nSizeY==pdY->size()))
   	return false;
   pData->addData(pdX, pdY);
   nSizeX=pdX->size();
   nSizeY=pdY->size();
   iStop=nSizeY;
  return true;
 }
 
 bool graphType::appendData(const dblDequeT *pdX,const dblDequeT *pdY)
 {
   if ((nSizeY!=iStop)||(!pData)||(nSizeY==pdY->size()))
   	return false;
   pData->appendData(pdX, nSizeX,pdY,nSizeY);
   nSizeX=pdX->size();
   nSizeY=pdY->size();
   iStop=nSizeY;
  return true;
 }
 
bool graphType::subsetData(const dblDequeT *pdX,const dblDequeT *pdY,unsigned int start,unsigned int stop)
 {
   pData->subsetData(pdX,pdY,start,stop);
   nSizeX=pdX->size();
   nSizeY=pdY->size();
   iStart=start;
   iStop=stop;
  return true;
 }

void graphType::setStyles()
{
//set the new properties
 switch (sType)
 {
   case stInvisible:
    {//make invisible
     pPlot->setCurveSymbol(lCurve,QwtSymbol()); //no symbol
     pPlot->setCurvePen(lCurve,QPen(Qt::NoPen));
     break;
    }
   case stLine:
    {
     pPlot->setCurveSymbol(lCurve,QwtSymbol()); //no symbol
     if(pen.style()==Qt::NoPen)
     	{
//	 pPlot->setCurvePen(lCurve,QPen());
	 pPlot->setCurveStyle(lCurve,QwtCurve::Dots);
	}
     else
        {
         pPlot->setCurvePen(lCurve,pen);
	 switch(nXVarId)
	 {
	  case IDLYAPUNOV: pPlot->setCurveStyle(lCurve,QwtCurve::Steps); break;
	  default: pPlot->setCurveStyle(lCurve,QwtCurve::Lines); break;
	 }
	}
     break;
    }
   case stSymbols:
    {
     pPlot->setCurvePen(lCurve,QPen(Qt::NoPen));
     pPlot->setCurveSymbol(lCurve,QwtSymbol(style,QBrush(pen.color()),pen,QSize(nSize,nSize)));
     break;
    }
   case stBoth:
    {
     if(pen.style()==Qt::NoPen)
     	{
	 pPlot->setCurvePen(lCurve,QPen());
	 pPlot->setCurveStyle(lCurve,QwtCurve::Dots);
	}
     else
        {
         pPlot->setCurvePen(lCurve,pen);
	 switch(nXVarId)
	 {
	  case IDLYAPUNOV: pPlot->setCurveStyle(lCurve,QwtCurve::Steps); break;
	  default: pPlot->setCurveStyle(lCurve,QwtCurve::Lines); break;
	 }
	}
     pPlot->setCurveSymbol(lCurve,QwtSymbol(style,QBrush(pen.color()),pen,QSize(nSize,nSize)));
     break;
    }
   }
}
