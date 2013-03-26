//
// C++ Interface: dataspinner
//
// Description: data spin buffer implementation
//
//
// Author: Tjeerd olde Scheper <tvolde-scheper@brookes.ac.uk>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
//---------------------------------------------------------------------------
#ifndef DATASPINNER_H
#define DATASPINNER_H

#include <qthread.h>
#include <qmutex.h>
#include <queue>
#include <deque>
#include <vector>

#define SPINBUFFER_SIZE 3
#define MAX_DEPTH 8

namespace NS_DataSpinner {

typedef std::deque<double> dblDeque;
typedef std::vector<unsigned int> uintVector;
typedef std::vector<double> dblVector;
typedef std::vector<dblDeque> dblVectorOfDeque;
typedef std::pair<unsigned int, unsigned int> uintRange;

struct TDataBuffer
{
  dblVector dvData;
  dblVector dvDataError;
};

typedef std::vector<TDataBuffer *> TDataBufferVector;

class TSpinBuffer
{
 TDataBufferVector spinner;
 uintVector indeces;
 uintRange range;
 TDataBuffer *curBuffer;
 TDataBuffer *lastBuffer;
 unsigned int nWriteIdx, nReadIdx, nSize, nDepth;
 void resize();
 bool bSpinning;
public:
 TSpinBuffer();
 TSpinBuffer(unsigned int size);
 ~TSpinBuffer();
 void spin();
 void write(dblVector &dv,dblVector &de);
 void read(dblVector &dv,dblVector &de);
 bool active() {return bSpinning;}
};

typedef void * TProducer;
typedef void * TConsumer;

typedef std::map<TProducer, TSpinBuffer *> TProducerMap;
typedef std::map<TConsumer, TSpinBuffer *> TConsumerMap;
typedef std::queue<TSpinBuffer *> TSpinQueue;

class TDataSpinner : QThread
{
  dblVectorOfDeque vdvData;
  dblVectorOfDeque vdvDataError;
  dblDeque dvDataError;
  TProducerMap producers;
  TConsumerMap consumers;
  TSpinQueue queueProduced;
  TSpinQueue queueConsume;
  TSpinBuffer *current;
  bool bSpin, bTerminate, bSpinning, bRunning,bError;
  //void waitSpin();
  //void spin();
  void copyData(TSpinBuffer *p);
  void storeData(TSpinBuffer *p);
  QString qsError;
  bool bKeepError;
  
public:
 TDataSpinner();
 ~TDataSpinner();
 virtual void run(); 
 void startSpinner();
 void stopSpinner();
 void flush();
 void clearData();
 void initData(unsigned int n);
 unsigned int getLyapSize();
 void storeData(TProducer p, double nt, dblVector &dv, dblVector &de);
 void storeLyap(TProducer p, double nt, dblVector &dv);
 void registerProducer(TProducer p);
 void registerConsumer(TConsumer c);
 QString &error();
 bool isOk() { return bError; }
 bool spinUp(TConsumer c, unsigned int n, uintRange &range);
 bool spinUp(TConsumer c, unsigned int n, uintVector &vals);
 void spinDown(TConsumer c);
 bool values(TConsumer c, dblVector &v);
 bool getKeepError() {return bKeepError;};
 void setKeepError(bool);
/* double lastEvolution();
 double firstEvolution();
 unsigned int evolutionSize();
 unsigned int evolutionPos(double d);
 double value(unsigned int idx, unsigned int pos);*/
};

}; // End of namespace
#endif
