/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 HEVC encoder
                             -------------------
    
    copyright            : (C) 2014 gruntster/mean
 ***************************************************************************/
#include <math.h>
#include <vector>
#include <QtGui/QFileDialog>
#include <QtGui/QDialog>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
using std::vector;
#include "ADM_default.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "../x265_settings.h"
#include "Q_x265.h"
#include "ADM_paramList.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"

static int pluginVersion=3;

static x265_settings myCopy; // ugly...
extern bool  x265_settings_jserialize(const char *file, const x265_settings *key);
extern bool  x265_settings_jdeserialize(const char *file, const ADM_paramList *tmpl,x265_settings *key);
extern "C" 
{
extern const ADM_paramList x265_settings_param[];
}

typedef struct
{
    uint32_t idcValue;
    const char *idcString;
}idcToken;

static const idcToken listOfIdc[]={
        {-1,"Auto"},
        {10,"1"},
        {20,"2"},
        {21,"2.1"},
        {30,"3"},
        {31,"3.1"},
        {40,"4"},
        {41,"4.1"},
        {50,"5"},
        {51,"5.1"},
        {52,"5.2"},
        {60,"6"},
        {61,"6.1"},
        {62,"6.2"},
};
#define NB_IDC sizeof(listOfIdc)/sizeof(idcToken)
static const idcToken listOfThreads[]={
        {0,"Auto"},
        {1,"1"},      
        {2,"2"},      
        {4,"4"},
};

#define NB_THREADS sizeof(listOfThreads)/sizeof(idcToken)

typedef struct
{
    uint32_t sarWidth;
    uint32_t sarHeight;
}aspectRatio;

static const aspectRatio predefinedARs[]={
    {16,15},
    {64,45},
    {8,9},
    {32,27},
    
};

#define NB_SAR sizeof(predefinedARs)/sizeof(aspectRatio)

static const char* listOfPresets[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo" };
#define NB_PRESET sizeof(listOfPresets)/sizeof(char*)

static const char* listOfTunings[] = { "psnr", "ssim", "zerolatency", "fastdecode" };
#define NB_TUNE sizeof(listOfTunings)/sizeof(char*)

static const char* listOfProfiles[] = { "main", "main10", "mainstillpicture" };
#define NB_PROFILE sizeof(listOfProfiles)/sizeof(char*)

/**
    \fn x265_ui
    \brief hook to enter UI specific dialog
*/
bool x265_ui(x265_settings *settings)
{
	bool success = false;
    x265Dialog dialog(qtLastRegisteredDialog(), settings);

	qtRegisterDialog(&dialog);

    if (dialog.exec() == QDialog::Accepted)
    {
            dialog.download();
            if(settings->general.preset) ADM_dealloc(settings->general.preset);
            settings->general.preset = NULL;
            if(settings->general.tuning) ADM_dealloc(settings->general.tuning);
            settings->general.tuning = NULL;
            if(settings->general.profile) ADM_dealloc(settings->general.profile);
            settings->general.profile = NULL;
            memcpy(settings,&myCopy,sizeof(myCopy));
            if(myCopy.general.preset) settings->general.preset = ADM_strdup(myCopy.general.preset);
            if(myCopy.general.tuning) settings->general.tuning = ADM_strdup(myCopy.general.tuning);
            if(myCopy.general.profile) settings->general.profile = ADM_strdup(myCopy.general.profile);
            success = true;
    }

	qtUnregisterDialog(&dialog);

    return success;
}
/**

*/  
x265Dialog::x265Dialog(QWidget *parent, void *param) : QDialog(parent)
{
       ui.setupUi(this);
        connect(ui.useAdvancedConfigurationCheckBox, SIGNAL(toggled(bool)), this, SLOT(useAdvancedConfigurationCheckBox_toggled(bool)));
        connect(ui.encodingModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(encodingModeComboBox_currentIndexChanged(int)));
        connect(ui.quantiserSlider, SIGNAL(valueChanged(int)), this, SLOT(quantiserSlider_valueChanged(int)));
        connect(ui.meSlider, SIGNAL(valueChanged(int)), this, SLOT(meSlider_valueChanged(int)));
        connect(ui.quantiserSpinBox, SIGNAL(valueChanged(int)), this, SLOT(quantiserSpinBox_valueChanged(int)));
        connect(ui.meSpinBox, SIGNAL(valueChanged(int)), this, SLOT(meSpinBox_valueChanged(int)));
        connect(ui.targetRateControlSpinBox, SIGNAL(valueChanged(int)), this, SLOT(targetRateControlSpinBox_valueChanged(int)));
        connect(ui.cuTreeCheckBox, SIGNAL(toggled(bool)), this, SLOT(cuTreeCheckBox_toggled(bool)));
        connect(ui.aqVarianceCheckBox, SIGNAL(toggled(bool)), this, SLOT(aqVarianceCheckBox_toggled(bool)));
#if 0
        connect(ui.maxCrfSlider, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSlider_valueChanged(int)));
        connect(ui.maxCrfSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSpinBox_valueChanged(int)));
#endif
       x265_settings* settings = (x265_settings*)param;
       if(myCopy.general.preset) ADM_dealloc(myCopy.general.preset);
       myCopy.general.preset = NULL;
       if(myCopy.general.tuning) ADM_dealloc(myCopy.general.tuning);
       myCopy.general.tuning = NULL;
       if(myCopy.general.profile) ADM_dealloc(myCopy.general.profile);
       myCopy.general.profile = NULL;
       memcpy(&myCopy,settings,sizeof(myCopy));
       if(settings->general.preset) myCopy.general.preset = ADM_strdup(settings->general.preset);
       if(settings->general.tuning) myCopy.general.tuning = ADM_strdup(settings->general.tuning);
       if(settings->general.profile) myCopy.general.profile = ADM_strdup(settings->general.profile);

#define ENCODING(x)  myCopy.general.params.x       
        lastBitrate =   ENCODING(bitrate);
        lastVideoSize = ENCODING(finalsize);

        ui.tabWidget->setCurrentIndex(0);
        connect(ui.deleteButton, SIGNAL(pressed()), this, SLOT(deleteButton_pressed()));
        connect(ui.saveAsButton, SIGNAL(pressed()), this, SLOT(saveAsButton_pressed()));
        connect(ui.configurationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(configurationComboBox_currentIndexChanged(int)));

        // Rebuild idc level list
        QComboBox *idc=ui.idcLevelComboBox;
        idc->clear();
        for(int i=0;i<NB_IDC;i++)
        {
            const idcToken *t=listOfIdc+i;
            idc->addItem(QString(t->idcString));
        }

        QComboBox *poolThreads=ui.comboBoxPoolThreads;
        poolThreads->clear();
        for(int i=0;i<NB_THREADS;i++)
        {
            const idcToken *t=listOfThreads+i;
            poolThreads->addItem(QString(t->idcString));
        }
        
        QComboBox *frameThreads=ui.comboBoxFrameThreads;
        frameThreads->clear();
        for(int i=0;i<NB_THREADS;i++)
        {
            const idcToken *t=listOfThreads+i;
            frameThreads->addItem(QString(t->idcString));
        }
        
        QComboBox* presets=ui.presetComboBox;
        presets->clear();
        for(int i=0;i<NB_PRESET;i++)
        {
            presets->addItem(QString(listOfPresets[i]));
        }

        QComboBox* tunings=ui.tuningComboBox;
        tunings->clear();
        for(int i=0;i<NB_TUNE;i++)
        {
            tunings->addItem(QString(listOfTunings[i]));
        }

        QComboBox* profiles=ui.profileComboBox;
        profiles->clear();
        for(int i=0;i<NB_PROFILE;i++)
        {
            profiles->addItem(QString(listOfProfiles[i]));
        }

        upload();
        ADM_pluginInstallSystem( std::string("x265"),std::string(".json"),pluginVersion);
        updatePresetList();
        int n=ui.configurationComboBox->count();
        ui.configurationComboBox->setCurrentIndex(n-1);
}
/**
    \fn updatePresetList
*/
bool x265Dialog::updatePresetList(void)
{
    QComboBox *combo=ui.configurationComboBox;
    std::string rootPath;
    vector <std::string >  list;
    ADM_pluginGetPath("x265",pluginVersion,rootPath);
    ADM_listFile(rootPath,".json",list);
    int l=list.size();
    combo->clear();
    for( int i=0;i<l;i++)
    {
        combo->addItem(list[i].c_str());
    }
    combo->addItem(QString("Custom"));
    return true;
}

bool x265Dialog::toogleAdvancedConfiguration(bool advancedEnabled)
{
  ui.useAdvancedConfigurationCheckBox->setChecked(advancedEnabled);
  ui.presetComboBox->setEnabled(!advancedEnabled);
  ui.tuningComboBox->setEnabled(!advancedEnabled);
  ui.profileComboBox->setEnabled(!advancedEnabled);
  ui.tabAdvancedRC->setEnabled(advancedEnabled);
  ui.tabMotion->setEnabled(advancedEnabled);
  ui.tabFrame->setEnabled(advancedEnabled);
  ui.tabAnalysis->setEnabled(advancedEnabled);
  ui.tabQuantiser->setEnabled(advancedEnabled);
}

/**

*/
#define MK_CHECKBOX(x,y) ui.x->setChecked(myCopy.y)
#define MK_UINT(x,y)  ui.x->setValue(myCopy.y)
#define DISABLE(x) ui.x->setEnabled(false);
#define MK_MENU(x,y) ui.x->setCurrentIndex(myCopy.y)
#define MK_RADIOBUTTON(x) ui.x->setChecked(true);
#define MK_COMBOBOX_STR(x,y,list,count) \
  { \
    QComboBox *combobox=ui.x; \
    for(int i=0;i<count;i++) \
    { \
      const char *p=list[i]; \
      if(myCopy.y && !strcmp(myCopy.y, p)) \
      { \
        combobox->setCurrentIndex(i); \
      } \
    } \
  }
bool x265Dialog::upload(void)
{
          toogleAdvancedConfiguration(myCopy.useAdvancedConfiguration);
          MK_CHECKBOX(fastPSkipCheckBox,fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,weighted_bipred);
          MK_CHECKBOX(trellisCheckBox,trellis);
          MK_UINT(psychoRdoSpinBox,psy_rd);
          if(myCopy.trellis)
          {
                ui.trellisComboBox->setCurrentIndex(myCopy.trellis-1);
          }

          if (myCopy.interlaced_mode > 0) {
        	  ui.interlacedCheckBox->setChecked(true);
                  ui.interlacedComboBox->setCurrentIndex(myCopy.interlaced_mode - 1);
          } else {
        	  ui.interlacedCheckBox->setChecked(false);
          }
    
          MK_CHECKBOX(dctDecimateCheckBox,dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(IFrameThresholdSpinBox,i_scenecut_threshold);
          MK_UINT(meSpinBox,subpel_refine);

          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
          MK_UINT(avgBitrateToleranceSpinBox,ratecontrol.rate_tolerance*100.0);
          MK_UINT(quantiserIpRatioSpinBox,ratecontrol.ip_factor);
          MK_UINT(quantiserPbRatioSpinBox,ratecontrol.pb_factor);
          MK_UINT(cbChromaLumaOffsetSpinBox,cb_chroma_offset);
          MK_UINT(crChromaLumaOffsetSpinBox,cr_chroma_offset);
          uint32_t aq_mode = myCopy.ratecontrol.aq_mode;
          if (aq_mode > 0)
          {
                ui.aqVarianceCheckBox->setChecked(true);
                ui.aqAlgoComboBox->setCurrentIndex(aq_mode-1);
                MK_UINT(aqStrengthSpinBox,ratecontrol.aq_strength);
          }

          MK_UINT(lookaheadSpinBox,lookahead);
          MK_CHECKBOX(cuTreeCheckBox,ratecontrol.cu_tree);
          
          MK_CHECKBOX(loopFilterCheckBox,b_deblocking_filter);

          MK_MENU(meMethodComboBox,me_method);
          MK_MENU(weightedPPredictComboBox,weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);
          MK_MENU(adaptiveBFrameComboBox,i_bframe_adaptive);
          MK_CHECKBOX(constrainedIntraCheckBox,constrained_intra);

          MK_UINT(mvRangeSpinBox,me_range);

          // preset
          MK_COMBOBOX_STR(presetComboBox, general.preset, listOfPresets, NB_PRESET);
          MK_COMBOBOX_STR(profileComboBox, general.profile, listOfProfiles, NB_PROFILE);
          MK_COMBOBOX_STR(tuningComboBox, general.tuning, listOfTunings, NB_TUNE);

          // udate idc
          QComboBox *idc=ui.idcLevelComboBox;
          for(int i=0;i<NB_IDC;i++)
          {
                const idcToken *t=listOfIdc+i;
                if(myCopy.level==t->idcValue)
                {
                        idc->setCurrentIndex(i);
                        break;
                }
          }
        // update threads
          QComboBox *poolThreads=ui.comboBoxPoolThreads;
          for(int i=0;i<NB_THREADS;i++)
          {
                const idcToken *t=listOfThreads+i;
                if(myCopy.general.poolThreads==t->idcValue)
                {
                        poolThreads->setCurrentIndex(i);
                        break;
                }
          }
          
          QComboBox *frameThreads=ui.comboBoxFrameThreads;
          for(int i=0;i<NB_THREADS;i++)
          {
                const idcToken *t=listOfThreads+i;
                if(myCopy.general.frameThreads==t->idcValue)
                {
                        frameThreads->setCurrentIndex(i);
                        break;
                }
          }
        switch(ENCODING(mode))
        {
            case COMPRESS_AQ: // CRF
                            ui.encodingModeComboBox->setCurrentIndex(2);
                            ui.quantiserSpinBox->setValue(ENCODING(qz));
                            break;
            case COMPRESS_CBR:
                            ui.encodingModeComboBox->setCurrentIndex(0);
                            ui.targetRateControlSpinBox->setValue(ENCODING(bitrate));
                            break;
            case COMPRESS_2PASS:
                            ui.encodingModeComboBox->setCurrentIndex(3);
                            ui.targetRateControlSpinBox->setValue(ENCODING(finalsize));
                            break;
            case COMPRESS_SAME:
                            ADM_assert(0);
                            break;
            case COMPRESS_2PASS_BITRATE:
                            ui.encodingModeComboBox->setCurrentIndex(4);
                            ui.targetRateControlSpinBox->setValue(ENCODING(avg_bitrate));
                            break;

            case COMPRESS_CQ:
                            ui.encodingModeComboBox->setCurrentIndex(1);
                            //encodingModeComboBox_currentIndexChanged(1);
                            ui.quantiserSpinBox->setValue(ENCODING(qz));
                            break;

            default: ADM_assert(0);break;
        }

        bool predefined = false;

        for (int i= 0;i<NB_SAR;i++)
	{
                if (myCopy.vui.sar_width == predefinedARs[i].sarWidth && myCopy.vui.sar_height == predefinedARs[i].sarHeight)
                {
                     MK_RADIOBUTTON(sarPredefinedRadioButton);
                     ui.sarPredefinedComboBox->setCurrentIndex(i);
                     predefined = true;
                     break;
                }
	}

	if (!predefined)
	{
                MK_RADIOBUTTON(sarCustomRadioButton);
                MK_UINT(sarCustomSpinBox1,vui.sar_width);
                MK_UINT(sarCustomSpinBox2,vui.sar_height);
	}

#if X265_BUILD >= 40
    DISABLE(noiseReductionSpinBox);
    MK_UINT(noiseReductionIntraSpinBox,noise_reduction_intra);
    MK_UINT(noiseReductionInterSpinBox,noise_reduction_inter);
#else
    DISABLE(noiseReductionIntraSpinBox);
    DISABLE(noiseReductionInterSpinBox);
    MK_UINT(noiseReductionSpinBox,noise_reduction);
#endif
        
	      DISABLE(spsiComboBox);
	      DISABLE(openGopCheckBox);
          DISABLE(groupBox_14); // quant matrix
          DISABLE(tabAdvanced1);
          DISABLE(tabAdvanced2);
          DISABLE(tabOutput2);
          DISABLE(maxCrfCheckBox);
          DISABLE(sarAsInputRadioButton);
          DISABLE(groupBox_3);
          DISABLE(accessUnitCheckBox);
          return true;
}
#undef MK_CHECKBOX
#undef MK_UINT
#undef MK_MENU
#undef MK_RADIOBUTTON
#undef MK_COMBOBOX_STR
#define MK_CHECKBOX(x,y)    myCopy.y=ui.x->isChecked()
#define MK_UINT(x,y)        myCopy.y=ui.x->value()
#define MK_MENU(x,y)        myCopy.y=ui.x->currentIndex()
#define MK_RADIOBUTTON(x,y)   myCopy.y=ui.x->setChecked(true);
#define MK_COMBOBOX_STR(x,y,list,count) \
  { \
    QComboBox* combo=ui.x; \
    int idx=combo->currentIndex(); \
    ADM_assert(idx<count); \
    if(myCopy.y) ADM_dealloc(myCopy.y); \
    myCopy.y = ADM_strdup(list[idx]); \
  }

bool x265Dialog::download(void)
{
          MK_CHECKBOX(useAdvancedConfigurationCheckBox,useAdvancedConfiguration);
          MK_CHECKBOX(fastPSkipCheckBox,fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,weighted_bipred);

          if (ui.interlacedCheckBox->isChecked()) {
                  myCopy.interlaced_mode = ui.interlacedComboBox->currentIndex() + 1;
          } else {
        	  myCopy.interlaced_mode = 0;
          }
    
          MK_CHECKBOX(dctDecimateCheckBox,dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(IFrameThresholdSpinBox,i_scenecut_threshold);
          MK_UINT(meSpinBox,subpel_refine);
          MK_UINT(BFrameBiasSpinBox,i_bframe_bias);

          MK_MENU(meMethodComboBox,me_method);
          MK_MENU(weightedPPredictComboBox,weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);
          MK_MENU(adaptiveBFrameComboBox,i_bframe_adaptive);
          MK_CHECKBOX(constrainedIntraCheckBox,constrained_intra);

          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
          MK_UINT(avgBitrateToleranceSpinBox, ratecontrol.rate_tolerance);
          myCopy.ratecontrol.rate_tolerance /= 100.0;
          MK_UINT(quantiserIpRatioSpinBox,ratecontrol.ip_factor);
          MK_UINT(quantiserPbRatioSpinBox,ratecontrol.pb_factor);
          MK_UINT(cbChromaLumaOffsetSpinBox,cb_chroma_offset);
          MK_UINT(crChromaLumaOffsetSpinBox,cr_chroma_offset);
          int a=ui.aqAlgoComboBox->currentIndex();
          if(!ui.aqVarianceCheckBox->isChecked())
          {
                myCopy.ratecontrol.aq_mode=0;
          }else
          {
                myCopy.ratecontrol.aq_mode=a+1;
                MK_UINT(aqStrengthSpinBox,ratecontrol.aq_strength);
          }
          
          MK_UINT(lookaheadSpinBox,lookahead);
          MK_CHECKBOX(cuTreeCheckBox,ratecontrol.cu_tree);

          MK_CHECKBOX(loopFilterCheckBox,b_deblocking_filter);

          MK_UINT(mvRangeSpinBox,me_range);

          MK_UINT(psychoRdoSpinBox,psy_rd);
          
#if X265_BUILD >= 40
          MK_UINT(noiseReductionIntraSpinBox,noise_reduction_intra);
          MK_UINT(noiseReductionInterSpinBox,noise_reduction_inter);
#else
          MK_UINT(noiseReductionSpinBox,noise_reduction);
#endif
          
          MK_COMBOBOX_STR(presetComboBox, general.preset, listOfPresets, NB_PRESET);
          MK_COMBOBOX_STR(profileComboBox, general.profile, listOfProfiles, NB_PROFILE);
          MK_COMBOBOX_STR(tuningComboBox, general.tuning, listOfTunings, NB_TUNE);

          QComboBox *idc=ui.idcLevelComboBox;
          int dex=idc->currentIndex();
          ADM_assert(dex<NB_IDC);
          myCopy.level=listOfIdc[dex].idcValue;

          switch(ui.encodingModeComboBox->currentIndex())
          {
            case 0: ENCODING(mode)=COMPRESS_CBR; ENCODING(bitrate)=ui.targetRateControlSpinBox->value();break;
            case 1: ENCODING(mode)=COMPRESS_CQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 2: ENCODING(mode)=COMPRESS_AQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 3: ENCODING(mode)=COMPRESS_2PASS;ENCODING(finalsize)=ui.targetRateControlSpinBox->value();;break;
            case 4: ENCODING(mode)=COMPRESS_2PASS_BITRATE;ENCODING(avg_bitrate)=ui.targetRateControlSpinBox->value();;break;
          }
          // update thread count
          QComboBox *poolThreads=ui.comboBoxPoolThreads;
          int poolThreadIndex=poolThreads->currentIndex();
          myCopy.general.poolThreads=listOfThreads[poolThreadIndex].idcValue;
          
          QComboBox *frameThreads=ui.comboBoxFrameThreads;
          int frameThreadIndex=frameThreads->currentIndex();
          myCopy.general.frameThreads=listOfThreads[frameThreadIndex].idcValue;
          
          int t=ui.trellisComboBox->currentIndex();
          if(!ui.trellisCheckBox->isChecked())
          {
                myCopy.trellis=0;
          }else
                myCopy.trellis=t+1;

          if(ui.sarPredefinedRadioButton->isChecked())
          {
                const aspectRatio *r=predefinedARs+ui.sarPredefinedComboBox->currentIndex();
                myCopy.vui.sar_width=r->sarWidth;
                myCopy.vui.sar_height=r->sarHeight;
          }else
          {
                MK_UINT(sarCustomSpinBox1,vui.sar_width);
                MK_UINT(sarCustomSpinBox2,vui.sar_height);
          }

          return true;
}

// General tab
void x265Dialog::encodingModeComboBox_currentIndexChanged(int index)
{
	bool enableQp = false, enableMaxCrf = false;

	switch (index)
	{
		case 0:
			ui.targetRateControlLabel1->setText(tr("Target Bitrate:"));
			ui.targetRateControlLabel2->setText(tr("kbit/s"));
			ui.targetRateControlSpinBox->setValue(lastBitrate);
			break;
		case 1: // Constant Quality - 1 pass
			ui.quantiserLabel2->setText(tr("Quantiser:"));
			enableQp = true;
			break;
		case 2: // Average Quantiser - 1 pass
			ui.quantiserLabel2->setText(tr("Quality:"));
			enableQp = true;
			enableMaxCrf = true;
			break;
		case 3: // Video Size - 2 pass
			ui.targetRateControlLabel1->setText(tr("Target Video Size:"));
			ui.targetRateControlLabel2->setText(tr("MB"));
			ui.targetRateControlSpinBox->setValue(lastVideoSize);
			break;
		case 4: // Average Bitrate - 2 pass
			ui.targetRateControlLabel1->setText(tr("Average Bitrate:"));
			ui.targetRateControlLabel2->setText(tr("kbit/s"));
			ui.targetRateControlSpinBox->setValue(lastBitrate);
			break;
	}

	ui.quantiserLabel1->setEnabled(enableQp);
	ui.quantiserLabel2->setEnabled(enableQp);
	ui.quantiserLabel3->setEnabled(enableQp);
	ui.quantiserSlider->setEnabled(enableQp);
	ui.quantiserSpinBox->setEnabled(enableQp);

	ui.targetRateControlLabel1->setEnabled(!enableQp);
	ui.targetRateControlLabel2->setEnabled(!enableQp);
	ui.targetRateControlSpinBox->setEnabled(!enableQp);

#if 0
	if (!enableMaxCrf)
		ui.maxCrfCheckBox->setChecked(false);

	ui.maxCrfCheckBox->setEnabled(enableMaxCrf);
#endif
}

void x265Dialog::useAdvancedConfigurationCheckBox_toggled(bool checked)
{
  toogleAdvancedConfiguration(checked);
}

void x265Dialog::quantiserSlider_valueChanged(int value)
{
	ui.quantiserSpinBox->setValue(value);
}

void x265Dialog::meSlider_valueChanged(int value)
{
	ui.meSpinBox->setValue(value);
}
void x265Dialog::quantiserSpinBox_valueChanged(int value)
{
	ui.quantiserSlider->setValue(value);
}
void x265Dialog::meSpinBox_valueChanged(int value)
{
	ui.meSlider->setValue(value);
}

void x265Dialog::targetRateControlSpinBox_valueChanged(int value)
{
	if (ui.encodingModeComboBox->currentIndex() == 3)	// Video Size - 2 pass
		lastVideoSize = value;
	else
		lastBitrate = value;
}

void x265Dialog::cuTreeCheckBox_toggled(bool checked)
{
	if (checked && !ui.aqVarianceCheckBox->isChecked())
	{
		if (GUI_Question(tr("Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.\n\nDo you wish to continue?").toUtf8().constData()))
			ui.aqVarianceCheckBox->setChecked(true);
		else
			ui.cuTreeCheckBox->setChecked(false);
	}
}

void x265Dialog::aqVarianceCheckBox_toggled(bool checked)
{
	if (!checked && ui.cuTreeCheckBox->isChecked())
	{
		if (GUI_Question(tr("Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.\n\nDo you wish to continue?").toUtf8().constData()))
			ui.cuTreeCheckBox->setChecked(false);
		else
			ui.aqVarianceCheckBox->setChecked(true);
	}
}
#if 0
void x265Dialog::maxCrfSlider_valueChanged(int value)
{
	ui.maxCrfSpinBox->setValue(value);
}

void x265Dialog::maxCrfSpinBox_valueChanged(int value)
{
	ui.maxCrfSlider->setValue(value);
}
#endif

/**
    \fn configurationComboBox_currentIndexChanged
*/
void x265Dialog::configurationComboBox_currentIndexChanged(int index)
{
    int n=ui.configurationComboBox->currentIndex();
    int m=ui.configurationComboBox->count();
    if(n==m-1) // custom
    {
        ui.deleteButton->setEnabled(false);
        return;
    }
    ui.deleteButton->setEnabled(true);
    // get text
    std::string rootPath;
    ADM_pluginGetPath("x265",pluginVersion,rootPath);
    QString text=QString(ADM_SEPARATOR)+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    char *t=ADM_strdup(text.toUtf8().constData());
    ADM_info("Loading preset %s\n",t);
    if(false==x265_settings_jdeserialize(t,x265_settings_param,&myCopy))
    {
        GUI_Error_HIG("Error","Cannot load preset");
        ADM_error("Cannot read from %s\n",t);
    }else       
    {
        upload();
    }
    ADM_dezalloc(t);
}
/**
    \fn getProfileName  
    \brief Popup a dialog that asks the user the preset name
*/
static char *getProfileName(void)
{
  QDialog dialog;
  dialog.setWindowTitle(QString::fromUtf8("Save Profile"));
  QDialogButtonBox *buttonBox = new QDialogButtonBox();  
  QVBoxLayout *vboxLayout = new QVBoxLayout();
  buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

  QLineEdit *text=new QLineEdit;
//  text->setAcceptRichText(false);
  
  text->setText("my profile");
  text->selectAll();

  vboxLayout->addWidget(text);
  vboxLayout->addWidget(buttonBox);

  dialog.setLayout(vboxLayout);

  if(dialog.exec()!=QDialog::Accepted)
  {
        ADM_info("Canceled");
        return NULL;
  }
  QString fileName=text->text();
  const char *out=fileName.toUtf8().constData();
  return ADM_strdup(out);
}
/**
        \fn saveAsButton_pressed
        \brief Save the current settings as preset
*/
void x265Dialog::saveAsButton_pressed(void)
{
  // 1-ask name
  char *out=getProfileName();
  if(!out) return;
  ADM_info("Using %s\n",out);
  download();
  std::string rootPath;
  ADM_pluginGetPath("x265",pluginVersion,rootPath);
  std::string fullpath=rootPath+std::string(ADM_SEPARATOR)+out+std::string(".json");

  if(ADM_fileExist(fullpath.c_str()))
  {
        if(false==GUI_Confirmation_HIG("Overwrite","Replace the following preset ?:",out))
        {
            ADM_dealloc(out);
            return;
        }
  }
  ADM_dealloc(out);
  if(false==x265_settings_jserialize(fullpath.c_str(),&myCopy))
  {
        GUI_Error_HIG("Error","Cannot save preset");
        ADM_error("Cannot write to %s\n",out);
  }
  updatePresetList();
}
/**

*/
void x265Dialog::deleteButton_pressed(void)
{ 
    int n=ui.configurationComboBox->currentIndex();
    int m=ui.configurationComboBox->count();
    if(n==m-1) // custom
    {
        GUI_Error_HIG("Error","Cannot delete custom profile");
        return;
    }
  QString preset=ui.configurationComboBox->itemText(n);
  QString msg=QString("Do you really want to delete the ")+preset+
            QString(" profile ?.\nIf it is a system profile it will be recreated next time.");
  if(true==GUI_Confirmation_HIG("Delete preset","Delete",msg.toUtf8().constData()))
  {
    std::string rootPath;
    ADM_pluginGetPath("x265",pluginVersion,rootPath);
    QString text=QString("/")+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    unlink(text.toUtf8().constData());
  }
  updatePresetList();
}


