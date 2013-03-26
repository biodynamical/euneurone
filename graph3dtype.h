#ifndef GRAPH3DTYPE_H
#define GRAPH3DTYPE_H

#include <qcolor.h>
#include "tglgraph.h"

//graph structure
typedef struct struct3DGraphT
{
//	UINT xUnit, yUnit;
//	unsigned int nIdx;
	unsigned int nXVarId;
	unsigned int nYVarId;
	unsigned int nZVarId;
	TGlGraph *pPlot;
	unsigned int uCurve;
//        UINT type;
        unsigned int iStart;
	unsigned int iStop;
	unsigned int nSizeX;
	unsigned int nSizeY;
	unsigned int nSizeZ;
	QColor color;
//        Byte MarkStyle;
//        DblVector *pData;
//        bool bAutoScale,bUpdate;
        bool bEnabled;
        QString Title;
	bool isPlot(TGlGraph *p) { return (pPlot==p); }
} struct3DGraph;

typedef std::vector<struct3DGraph> T3DSeriesVector;

#endif
