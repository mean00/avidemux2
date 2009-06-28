/***************************************************************************
  FAC_toggle.cpp
  Handle dialog factory element : Toggle
  (C) 2006 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "T_bitrate.h"
#include "ADM_default.h"
#include "DIA_factory.h"
#include "ADM_dialogFactoryQt4.h"

extern const char *shortkey(const char *);

namespace ADM_Qt4Factory
{
class diaElemBitrate : public diaElemBitrateBase
{
public:
  diaElemBitrate(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip=NULL);
  virtual ~diaElemBitrate() ;
  void setMe(void *dialog, void *opaque,uint32_t line);
  void getMe(void);
  void setMaxQz(uint32_t qz);
  void setMinQz(uint32_t qz);
  int getRequiredLayout(void);
  void updateMe(void);
};

ADM_Qbitrate::ADM_Qbitrate(COMPRES_PARAMS *p,uint32_t minQ, uint32_t mq,QGridLayout *layout,int line)
{
	compress=p;
	combo=new QComboBox();

	_minQ=minQ;
	maxQ=mq;
	int index=0,set=-1;
#define add(x,z,y) if(compress->capabilities & ADM_ENC_CAP_##x) {combo->addItem(QString::fromUtf8(y));\
	if(p->mode==COMPRESS_##z) set=index;\
	index++;}

	add(CBR,CBR,QT_TR_NOOP("Constant Bitrate"));
	add(CQ,CQ,QT_TR_NOOP("Constant Quantiser"));
	add(SAME,SAME,QT_TR_NOOP("Same Quantiser as Input"));
	add(AQ,AQ,QT_TR_NOOP("Constant Rate Factor"));
	add(2PASS,2PASS,QT_TR_NOOP("Two Pass - Video Size"));
	add(2PASS_BR,2PASS_BITRATE,QT_TR_NOOP("Two Pass - Average Bitrate"));

	text1=new QLabel( QString::fromUtf8(QT_TR_NOOP("Encoding mode")));
	text1->setBuddy(combo);

	box=new QSpinBox();

	text2=new QLabel( QString::fromUtf8(QT_TR_NOOP("Bitrate")));
	text2->setBuddy(combo);

	QHBoxLayout *hboxLayout = new QHBoxLayout();
	QHBoxLayout *hboxLayout2 = new QHBoxLayout();
	QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QSpacerItem *spacer2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	hboxLayout->addWidget(combo);
	hboxLayout->addItem(spacer);

	layout->addWidget(text1,line,0);
	layout->addItem(hboxLayout,line,1);

	hboxLayout2->addWidget(box);
	hboxLayout2->addItem(spacer2);

	layout->addWidget(text2,line+1,0);
	layout->addItem(hboxLayout2,line+1,1);

	if(set!=-1) 
	{
		combo->setCurrentIndex(set);
		comboChanged(set);
	}
	connect(combo, SIGNAL(currentIndexChanged(int )), this, SLOT(comboChanged(int )));


}

/**
 * 	\fn 	readPullDown
 * \brief 	Convert the raw read of the combox into the actual compression mode
 */
static COMPRESSION_MODE readPulldown(COMPRES_PARAMS *copy,int rank)
{
	int index=0;
	COMPRESSION_MODE mode=COMPRESS_MAX;
	
#undef LOOKUP
#define LOOKUP(A,B) \
  if(copy->capabilities & ADM_ENC_CAP_##A) \
  { \
	  if(rank==index) mode=COMPRESS_##B; \
	  index++; \
  } 
  
  LOOKUP(CBR,CBR);
  LOOKUP(CQ,CQ);
  LOOKUP(SAME,SAME);
  LOOKUP(AQ,AQ);
  LOOKUP(2PASS,2PASS);
  LOOKUP(2PASS_BR,2PASS_BITRATE);
  
	ADM_assert(mode!=COMPRESS_MAX);
	return mode;
}

void ADM_Qbitrate::readBack(void)
{
#define Mx(x) compress->mode=x
#define Vx(x) compress->x=box->value();
	COMPRESSION_MODE mode=readPulldown(compress,combo->currentIndex());
  switch(mode)
  {
    case COMPRESS_CBR: Mx(COMPRESS_CBR);Vx(bitrate);break;
    case COMPRESS_CQ: Mx(COMPRESS_CQ);Vx(qz);break;
    case COMPRESS_2PASS: Mx(COMPRESS_2PASS);Vx(finalsize);break;
    case COMPRESS_2PASS_BITRATE: Mx(COMPRESS_2PASS_BITRATE);Vx(avg_bitrate);break;
    case COMPRESS_SAME: Mx(COMPRESS_SAME);break;
    case COMPRESS_AQ: Mx(COMPRESS_AQ);Vx(qz);break;
    default :
          ADM_assert(0);
  }
}
void ADM_Qbitrate::comboChanged(int i)
{
  printf("Changed\n"); 
#define P(x) text2->setText(QString::fromUtf8(x))
#define M(x,y) box->setMinimum  (x);box->setMaximum  (y);
#define S(x)   box->setValue(x);
  COMPRESSION_MODE mode=readPulldown(compress,i);
    switch(mode)
  {
    case COMPRESS_CBR: //CBR
          P(QT_TR_NOOP("Target bitrate (kb/s)"));
          M(0,20000);
          S(compress->bitrate);
          break; 
    case COMPRESS_CQ:// CQ
          P(QT_TR_NOOP("Quantizer"));
          M(_minQ,maxQ);
          S(compress->qz);
          break;
    case COMPRESS_2PASS : // 2pass Filesize
          P(QT_TR_NOOP("Target video size (MB)"));
          M(1,8000);
          S(compress->finalsize);
          break;
    case COMPRESS_2PASS_BITRATE : // 2pass Avg
          P(QT_TR_NOOP("Average bitrate (kb/s)"));
          M(0,20000);
          S(compress->avg_bitrate);
          break;
    case COMPRESS_SAME : // Same Qz as input
          P(QT_TR_NOOP("-"));
          M(0,0);
          break;
    case COMPRESS_AQ : // AQ
          P(QT_TR_NOOP("Quantizer"));
          M(_minQ,maxQ);
          S(compress->qz);
          break;
    default:ADM_assert(0);
  }
}
ADM_Qbitrate::~ADM_Qbitrate()
{
#define DEL(x) if(x) {delete x;x=NULL;}
#if 0 // Automatically deleted
                 DEL(text1)
                 DEL(text2)
                 DEL(box) 
                 DEL(combo) 
#endif
};

//**********************************
diaElemBitrate::diaElemBitrate(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip)
  : diaElemBitrateBase()
{
 
  param=(void *)p;
  memcpy(&copy,p,sizeof(copy));
  paramTitle=NULL;
  this->tip=tip;
  setSize(2);
  minQ=2;
  maxQ=31;
}

void diaElemBitrate::setMinQz(uint32_t qz)
{
  minQ=qz;
}

void diaElemBitrate::setMaxQz(uint32_t qz)
{
  maxQ=qz; 
}

diaElemBitrate::~diaElemBitrate() {};

void diaElemBitrate::setMe(void *dialog, void *opaque,uint32_t line)
{
  QGridLayout *layout=(QGridLayout*) opaque;
  
  ADM_Qbitrate *b=new ADM_Qbitrate((COMPRES_PARAMS *)&copy,minQ,maxQ,layout,line);
  myWidget=(void *)b;
  
}
void diaElemBitrate::getMe(void)
{
  ((ADM_Qbitrate *)myWidget)->readBack();
  memcpy(param,&copy,sizeof(copy));
}

int diaElemBitrate::getRequiredLayout(void) { return FAC_QT_GRIDLAYOUT; }
} // End of namespace
//****************************Hoook*****************

diaElem  *qt4CreateBitrate(COMPRES_PARAMS *p,const char *toggleTitle,const char *tip)
{
	return new  ADM_Qt4Factory::diaElemBitrate(p,toggleTitle,tip);
}
void qt4DestroyBitrate(diaElem *e)
{
	ADM_Qt4Factory::diaElemBitrate *a=(ADM_Qt4Factory::diaElemBitrate *)e;
	delete a;
}
//EOF

