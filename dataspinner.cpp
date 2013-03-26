//
// C++ Implementation: dataspinner
//
// Description: data spin buffer implementation
//
//
// Author: Tjeerd olde Scheper <tvolde-scheper@brookes.ac.uk>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
// Data provided by integrator is stored in a spin buffer, afterwards buffer is spun and
// copied by this into global buffer.
#include <qtextedit.h>
#include <float.h>
#include "stdexcept"

#include "dataspinner.h"

using namespace NS_DataSpinner;
using namespace std;

//=== TSpinBuffer ===================================================
TSpinBuffer::TSpinBuffer()
{
 unsigned int i;
 
 for (i=0;i<SPINBUFFER_SIZE;i++)
 	 spinner.push_back(new TDataBuffer);
 nSize=spinner.size();
 nDepth=1;
 nWriteIdx=0;
 nReadIdx=0;
 bSpinning=false;
}

TSpinBuffer::TSpinBuffer(unsigned int size)
{
 unsigned int i;
 
 for (i=0;i<size;i++)
 	 spinner.push_back(new TDataBuffer);
 nSize=spinner.size();
 nDepth=1;
 bSpinning=false;
}

TSpinBuffer::~TSpinBuffer()
{
 unsigned int i;
 
 for (i=0;i<nSize;i++)
 	 delete spinner[i];
 spinner.clear();
 nDepth=1;
}

void TSpinBuffer::resize()
{
 unsigned int i,v;
 
 if (nDepth==MAX_DEPTH)
 	throw std::domain_error("Spinbuffer overflow");
 v=2^nDepth;
 for (i=0;i<v;i++)
 	 spinner.push_back(new TDataBuffer);
 nSize=spinner.size();
 nDepth++;
}

void TSpinBuffer::write(dblVector &dv,dblVector &de)
{
 bSpinning=true;
 spinner[nWriteIdx]->dvData=dv;
 spinner[nWriteIdx]->dvDataError=de;
 nWriteIdx++;
 if ((nWriteIdx-nReadIdx)>nDepth)
 	resize();
}

void TSpinBuffer::read(dblVector &dv,dblVector &de)
{
}

void TSpinBuffer::spin()
{
}
//=== TDataSpinner ==================================================
TDataSpinner::TDataSpinner() : QThread()
{
 bSpin=false;
 bTerminate=false;
 bRunning=false;
 bSpinning=false;
 bError=false;
 bKeepError=false;
}

TDataSpinner::~TDataSpinner()
{
 flush();
}

void  TDataSpinner::run()
{
 bRunning=true;
 try
 {
  while( !bTerminate )
  {
   while( !queueProduced.empty() )
   	{
   	 current=queueProduced.front();
   	 current->spin();
   	 //storeData(current);
   	 queueProduced.pop();
   	}
   while( !queueConsume.empty() )
   	{
   	 current=queueConsume.front();
   	 current->spin();
   	 copyData(current);
   	 queueConsume.pop();
   	}
  }
 }
 catch (std::exception &stdex)
 {
#ifdef _DEBUG
	qDebug("Caught exception in DataSpinner: %s",stdex.what());
#endif
  bError=true;
  qsError=stdex.what();
 }
 bRunning=false;
}

 void TDataSpinner::registerProducer(TProducer p)
 {
  if (!producers.count(p))
  	{
  	 producers.insert(std::pair<TProducer, TSpinBuffer *>(p,new TSpinBuffer()));
  	}
 }
 
 void TDataSpinner::registerConsumer(TConsumer c)
 {
  if (!consumers.count(c))
  	{
  	 consumers.insert(std::pair<TConsumer, TSpinBuffer *>(c,new TSpinBuffer()));
  	}
 }

 void TDataSpinner::startSpinner()
 {
  if (bRunning)
  	return;
  bTerminate=false;
  start();
}

 void TDataSpinner::stopSpinner()
 {
  bTerminate=true;
}

QString &TDataSpinner::error()
{
 return qsError;
}

void TDataSpinner::storeData(TProducer p, double nt, dblVector &dv,dblVector &de)
{
 TSpinBuffer *pb=producers[p];
 pb->write(dv,de);
 queueProduced.push(pb);
}

void TDataSpinner::storeLyap(TProducer p, double nt, dblVector &dv)
{
}

void TDataSpinner::copyData(TSpinBuffer *p)
{
}

void TDataSpinner::initData(unsigned int N)
{
 unsigned int i;
 
 if (N==0)
 	return;
 //Create data store
 for (i=0;i<=N;i++)
 	vdvData.push_back(dblDeque());
 //Create store for estimated integration error
 if (bKeepError)
 {
  for (i=0;i<N;i++)
 	vdvDataError.push_back(dblDeque());
 }
 else
 	dvDataError.assign(N,0.0);
}

void TDataSpinner::clearData()
{
 unsigned int i;
 
 for (i=0;i<vdvData.size();i++)
    vdvData[i].erase(vdvData[i].begin(),vdvData[i].end());
 for (i=0;i<vdvDataError.size();i++)
    vdvDataError[i].erase(vdvDataError[i].begin(),vdvDataError[i].end());
 dvDataError.assign(dvDataError.size(),0.0);
}

void TDataSpinner::flush()
{
 vdvData.erase(vdvData.begin(),vdvData.end());
 vdvDataError.erase(vdvDataError.begin(),vdvDataError.end());
 dvDataError.erase(dvDataError.begin(),dvDataError.end()); 
}

void TDataSpinner::setKeepError(bool b)
{
 unsigned int i,N;
 
 bKeepError=b;
 N=vdvData.size()-1;
 if (bKeepError)
 {
  //clear old data
   for (i=0;i<vdvDataError.size();i++)
   	vdvDataError[i].erase(vdvDataError[i].begin(),vdvDataError[i].end());
   vdvDataError.erase(vdvDataError.begin(),vdvDataError.end());
   
   for (i=0;i<N;i++)
 	vdvDataError.push_back(dblDeque());
}
else
 	dvDataError.assign(N,0.0);
}

bool TDataSpinner::spinUp(TConsumer c, unsigned int n, uintRange &range)
{
 return false;
}

bool TDataSpinner::spinUp(TConsumer c, unsigned int n, uintVector &vals)
{
 return false;
}

void TDataSpinner::spinDown(TConsumer c)
{
}

bool TDataSpinner::values(TConsumer c, dblVector &v)
{
 return false;
}

// double TDataSpinner::lastEvolution()
// {
//  if (vdvData[0].empty())
//  	return DBL_MIN;
// return vdvData[0].back();
// }
// 
// double TDataSpinner::firstEvolution()
// {
//  if (vdvData[0].empty())
//  	return DBL_MIN;
// return vdvData[0].front();
// }
// 
// unsigned int TDataSpinner::evolutionSize()
// {
//  return vdvData[0].size();
// }
// 
// unsigned int TDataSpinner::evolutionPos(double d)
// {
//   return (std::lower_bound(vdvData[0].begin(),vdvData[0].end(),d)-(vdvData[0].begin()));
// }
// 
// double TDataSpinner::value(unsigned int idx, unsigned int pos)
// {
//  double d=DBL_MIN;
//  if ((idx<vdvData.size())&&(pos<vdvData[idx].size()))
//  	d=vdvData[idx][pos];
//  return d;
// }
