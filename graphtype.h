#ifndef GRAPHTYPE_H
#define GRAPHTYPE_H

#include <qpen.h>
#include <vector>
#include <deque>

#ifdef QWT5
#include <qwt5/qwt_plot.h>
#include <qwt5/qwt_data.h>
#include <qwt5/qwt_symbol.h>
#else
#include <qwt_plot.h>
#include <qwt_data.h>
#include <qwt_symbol.h>
#endif

#define IDINVALID 0xFFFFFFFF
#define IDNORMAL IDINVALID-1
#define IDLYAPUNOV IDNORMAL-1
#define IDSTATS IDLYAPUNOV-1
#define IDERRORS IDSTATS-1
#define IDRECURRENCE IDERRORS-1
#define IDMAXLYAP IDRECURRENCE-1
#define IDPERIOD IDMAXLYAP-1
#define IDPOWER IDPERIOD-1
#define IDPOINCARE1D IDPOWER-1
#define IDPOINCARE2D IDPOINCARE1D-1
#define IDISI IDPOINCARE2D-1
#define IDSPIKE IDISI-1

typedef std::deque<double> dblDequeT;

typedef enum {ctiNone, ctiNormal, ctiLyapunov, ctiStats, ctiErrors, ctiRecur, ctiMaxLyap, ctiPeriod, ctiPower, ctiPoincare1d, ctiPoincare2d, ctiIsi, ctiSpike} TCalcTypeItem;

typedef enum { stInvisible, stLine, stSymbols, stBoth} seriesType;

//data store for graph
class dblDequeData: public QwtData
{
public:
    dblDequeData();
    ~dblDequeData();
    dblDequeData(const dblDequeT &x, const dblDequeT &y);
   // dblDequeData &operator=(const dblDequeData &);
    virtual QwtData *copy() const;

    virtual size_t size() const;
    virtual double x(size_t i) const;
    virtual double y(size_t i) const;

    virtual QwtDoubleRect boundingRect() const;

    void initData();
    void clearData();
    void setData(const dblDequeT *,const dblDequeT *);
    void addData(const dblDequeT *,const dblDequeT *);
    void appendData(const dblDequeT *, unsigned int ,const dblDequeT *,unsigned int);
    void subsetData(const dblDequeT *, const dblDequeT *,unsigned int, unsigned int);
private:
    void initCache();
    dblDequeT *px;
    dblDequeT *py;
    QwtDoubleRect d_cache;
};

//graph structure
class graphType
{
	unsigned int nYVarId;
	unsigned int nXVarId;
	QwtPlot *pPlot;
	long lCurve;
	dblDequeData *pData;
        unsigned int iStart;
	unsigned int iStop;
	unsigned int nSizeX;
	unsigned int nSizeY;
	QPen pen;
	QwtSymbol::Style style;
	seriesType sType;
	int nSize;
        QString Title;
public:
	 graphType();
 	graphType &operator=(const graphType &);
 	void replot();
	unsigned int idX() {return nXVarId; };
	unsigned int idY() {return nYVarId; };
	void setIdX(unsigned int id) { nXVarId=id; };
	void setIdY(unsigned int id) { nYVarId=id; };
	bool isValid(unsigned int);
	bool isPlot(QwtPlot *);
	bool isCurve(long);
	bool isSubset(unsigned int start, unsigned int stop);
	long getCurve() { return lCurve;};
	QwtPlot *getPlot() {return pPlot;};
	void setPlot(const QwtPlot *);
	void setTitle(const QString &);
	void setTitle(const char *);
	QString &getTitle() {return Title;};
	void setPen(QPen &p) {pen=p;};
	QPen &getPen() {return pen;};
	void setStyle(QwtSymbol::Style s) {style=s;};
	void setBounds(unsigned int start, unsigned int stop) {iStart=start; iStop=stop;};
	unsigned int getStart() { return iStart;};
	unsigned int getStop() { return iStop;};
	unsigned int getSizeX() { return nSizeX;};
	unsigned int getSizeY() { return nSizeY;};
	int getSymbolSize() {return nSize;};
	void setSymbolSize(int s) { nSize=s;};
	int getLineWidth() { return pen.width();}
	void setLineWidth(int w) { pen.setWidth(w);}
	seriesType getType() {return sType;};
	void setType(seriesType t) { sType=t;};
	QwtSymbol::Style getStyle() {return style;};
	
	void setStyles();
	void newCurve(unsigned int idx, unsigned int idy);
	void newCurve();
	void deleteCurve();
	void clearData();
	bool setData(const dblDequeT *,const dblDequeT *);
	bool addData(const dblDequeT *,const dblDequeT *);
	bool appendData(const dblDequeT *,const dblDequeT *);
	bool subsetData(const dblDequeT *,const dblDequeT *,unsigned int,unsigned int);
};

typedef std::vector<graphType> TSeriesVector;
typedef std::vector<graphType *> TPSeriesVector;

#endif
