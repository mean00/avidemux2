/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------

    copyright            : (C) 2011 gruntster/mean
 ***************************************************************************/
#include <math.h>
#include <vector>
#include <QDialog>
#include <QFileDialog>
#include <QTextEdit>
#include <QLineEdit>
using std::vector;
#include "ADM_default.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "../x264_encoder.h"
#include "Q_x264.h"
#include "ADM_paramList.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"

static int pluginVersion=3;

static x264_encoder myCopy; // ugly...
extern bool  x264_encoder_jserialize(const char *file, const x264_encoder *key);
extern bool  x264_encoder_jdeserialize(const char *file, const ADM_paramList *tmpl,x264_encoder *key);
extern "C" 
{
extern const ADM_paramList x264_encoder_param[];
}

typedef struct
{
    uint32_t idcValue;
    const char *idcString;
}idcToken;

static const idcToken listOfIdc[]={
        {(unsigned int)-1,"Auto"},
        {10,"1"},
        {11,"1.1"},
        {12,"1.2"},
        {13,"1.3"},
        {20,"2"},
        {21,"2.1"},
        {22,"2.2"},
        {30,"3"},
        {31,"3.1"},
        {32,"3.2"},
        {40,"4"},
        {41,"4.1"},
        {42,"4.2"},
        {50,"5"},
        {51,"5.1"},

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

// Empty string "" as tuning means no tuning. This is the default.
static const char* listOfTunings[] = { "", "film", "animation", "grain", "stillimage", "psnr", "ssim" };
#define NB_TUNE sizeof(listOfTunings)/sizeof(char*)

static const char* listOfProfiles[] = { "baseline", "main", "high", "high10", "high422", "high444" };
#define NB_PROFILE sizeof(listOfProfiles)/sizeof(char*)

/**
    \fn x264_ui
    \brief hook to enter UI specific dialog
*/
bool x264_ui(x264_encoder *settings)
{
    bool success = false;
    x264Dialog dialog(qtLastRegisteredDialog(), settings);

    qtRegisterDialog(&dialog);

    if (dialog.exec() == QDialog::Accepted)
    {
            dialog.download();
            *settings=myCopy;
            success = true;
    }

    qtUnregisterDialog(&dialog);
    return success;
}
/**

*/  
x264Dialog::x264Dialog(QWidget *parent, void *param) : QDialog(parent)
{
       ui.setupUi(this);
        connect(ui.useAdvancedConfigurationCheckBox, SIGNAL(toggled(bool)), this, SLOT(useAdvancedConfigurationCheckBox_toggled(bool)));
        connect(ui.encodingModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(encodingModeComboBox_currentIndexChanged(int)));
        connect(ui.quantiserSlider, SIGNAL(valueChanged(int)), this, SLOT(quantiserSlider_valueChanged(int)));
        connect(ui.meSlider, SIGNAL(valueChanged(int)), this, SLOT(meSlider_valueChanged(int)));
        connect(ui.quantiserSpinBox, SIGNAL(valueChanged(int)), this, SLOT(quantiserSpinBox_valueChanged(int)));
        connect(ui.meSpinBox, SIGNAL(valueChanged(int)), this, SLOT(meSpinBox_valueChanged(int)));
        connect(ui.targetRateControlSpinBox, SIGNAL(valueChanged(int)), this, SLOT(targetRateControlSpinBox_valueChanged(int)));
        connect(ui.loopFilterCheckBox, SIGNAL(toggled(bool)), this, SLOT(loopFilterCheckBox_toggled(bool)));
        connect(ui.mbTreeCheckBox, SIGNAL(toggled(bool)), this, SLOT(mbTreeCheckBox_toggled(bool)));
        connect(ui.aqVarianceCheckBox, SIGNAL(toggled(bool)), this, SLOT(aqVarianceCheckBox_toggled(bool)));
#if 0
        connect(ui.maxCrfSlider, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSlider_valueChanged(int)));
        connect(ui.maxCrfSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSpinBox_valueChanged(int)));
#endif
       x264_encoder* settings = (x264_encoder*)param;
       myCopy=*settings;

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

        QComboBox *threads=ui.comboBoxThreads;
        threads->clear();
        for(int i=0;i<NB_THREADS;i++)
        {
            const idcToken *t=listOfThreads+i;
            threads->addItem(QString(t->idcString));
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
            const char* _tn=listOfTunings[i];
            // we pass an empty string to the encoder in order to disable tuning,
            // but want to show a descriptive label to the user
            if(_tn=="")
                _tn=QT_TRANSLATE_NOOP("x264","none");
            tunings->addItem(QString(_tn));
        }

        QComboBox* profiles=ui.profileComboBox;
        profiles->clear();
        for(int i=0;i<NB_PROFILE;i++)
        {
            profiles->addItem(QString(listOfProfiles[i]));
        }

        upload();
        ADM_pluginInstallSystem( std::string("x264"),std::string(".json"),pluginVersion);
        updatePresetList();
        int n=ui.configurationComboBox->count();
        ui.configurationComboBox->setCurrentIndex(n-1);
}
/**
    \fn updatePresetList
*/
bool x264Dialog::updatePresetList(void)
{
    QComboBox *combo=ui.configurationComboBox;
    std::string rootPath;
    vector <std::string >  list;
    ADM_pluginGetPath("x264",pluginVersion,rootPath);
    ADM_listFile(rootPath,".json",list);
    int l=list.size();
    combo->clear();
    for( int i=0;i<l;i++)
    {
        combo->addItem(list[i].c_str());
    }
    combo->addItem(QString(QT_TRANSLATE_NOOP("x264","Custom")));
    return true;
}

bool x264Dialog::toogleAdvancedConfiguration(bool advancedEnabled)
{
  ui.useAdvancedConfigurationCheckBox->setChecked(advancedEnabled);
  ui.presetComboBox->setEnabled(!advancedEnabled);
  ui.tuningComboBox->setEnabled(!advancedEnabled);
  ui.profileComboBox->setEnabled(!advancedEnabled);
  ui.fastDecodeCheckBox->setEnabled(!advancedEnabled);
  ui.zeroLatencyCheckBox->setEnabled(!advancedEnabled);
  ui.tabAdvancedRC->setEnabled(advancedEnabled);
  ui.tabMotion->setEnabled(advancedEnabled);
  ui.tabPartition->setEnabled(advancedEnabled);
  ui.tabFrame->setEnabled(advancedEnabled);
  ui.tabAnalysis->setEnabled(advancedEnabled);
  ui.tabQuantiser->setEnabled(advancedEnabled);
  ui.tabAdvanced1->setEnabled(advancedEnabled);
  return true;
}

/**

*/
#define MK_CHECKBOX(x,y) ui.x->setChecked(myCopy.y)
#define MK_UINT(x,y)  ui.x->setValue(myCopy.y)
#define DISABLE(x) ui.x->setEnabled(false);
#define MK_MENU(x,y) ui.x->setCurrentIndex(myCopy.y)
#define MK_RADIOBUTTON(x) ui.x->setChecked(true);
#define MK_COMBOBOX_STR(x,y,list,count) updateComboBox(ui.x,myCopy.y,count,list)
/**

*/
static void updateComboBox(QComboBox *combo,const std::string &mine,int count,const char **list)
{
    for(int i=0;i<count;i++) 
    { 
      const char *p=list[i]; 
      if(mine.size() && !strcmp(mine.c_str(), p)) 
      { 
        combo->setCurrentIndex(i); 
      } 
    } 
}
bool x264Dialog::upload(void)
{
          toogleAdvancedConfiguration(myCopy.useAdvancedConfiguration);
          MK_CHECKBOX(fastDecodeCheckBox,general.fast_decode);
          MK_CHECKBOX(zeroLatencyCheckBox,general.zero_latency);
          MK_CHECKBOX(fastFirstPassCheckBox,general.fast_first_pass);
          MK_CHECKBOX(fastPSkipCheckBox,analyze.fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,analyze.weighted_bipred);
          MK_CHECKBOX(dct8x8CheckBox,analyze.b_8x8);
          MK_CHECKBOX(i4x4CheckBox,analyze.b_i4x4);
          MK_CHECKBOX(i8x8CheckBox,analyze.b_i8x8);
          MK_CHECKBOX(p4x4CheckBox,analyze.b_p8x8);
          MK_CHECKBOX(p8x8CheckBox,analyze.b_p16x16);
          MK_CHECKBOX(b8x8CheckBox,analyze.b_b16x16);
          MK_CHECKBOX(trellisCheckBox,analyze.trellis);
          MK_UINT(psychoRdoSpinBox,analyze.psy_rd);
          MK_UINT(psychoTrellisSpinBox,analyze.psy_trellis);
          MK_UINT(noiseReductionSpinBox,analyze.noise_reduction);
          MK_UINT(intraLumaSpinBox,analyze.intra_luma);
          MK_UINT(interLumaSpinBox,analyze.inter_luma);
          MK_UINT(vbvMaxBitrateSpinBox,ratecontrol.vbv_max_bitrate);
          MK_UINT(vbvBufferSizeSpinBox,ratecontrol.vbv_buffer_size);
          MK_UINT(vbvBufferOccupancySpinBox,ratecontrol.vbv_buffer_init);


          if(myCopy.analyze.trellis)
          {
                ui.trellisComboBox->setCurrentIndex(myCopy.analyze.trellis-1);
          }

          MK_CHECKBOX(cabacCheckBox,cabac);
          if (myCopy.interlaced || myCopy.fake_interlaced) {
                  ui.interlacedCheckBox->setChecked(true);
          } else {
                  ui.interlacedCheckBox->setChecked(false);
          }
          if (myCopy.fake_interlaced) {
                  ui.interlacedComboBox->setCurrentIndex(2);
          } else {
                  if (myCopy.tff) {
                          ui.interlacedComboBox->setCurrentIndex(1);
                  } else {
                          ui.interlacedComboBox->setCurrentIndex(0);
                  }
          }

          MK_CHECKBOX(mixedRefsCheckBox,analyze.mixed_references);
          MK_CHECKBOX(chromaMotionEstCheckBox,analyze.chroma_me);
          MK_CHECKBOX(dctDecimateCheckBox,analyze.dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(IFrameThresholdSpinBox,i_scenecut_threshold);
          MK_CHECKBOX(intraRefreshCheckBox,intra_refresh);
          MK_UINT(meSpinBox,analyze.subpel_refine);

          MK_UINT(quantiserMinSpinBox,ratecontrol.qp_min);
          MK_UINT(quantiserMaxSpinBox,ratecontrol.qp_max);
          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
          MK_UINT(avgBitrateToleranceSpinBox,ratecontrol.rate_tolerance*100.0);
          MK_UINT(quantiserIpRatioSpinBox,ratecontrol.ip_factor);
          MK_UINT(quantiserPbRatioSpinBox,ratecontrol.pb_factor);
          MK_UINT(chromaLumaOffsetSpinBox,analyze.chroma_offset);
          uint32_t aq_mode = myCopy.ratecontrol.aq_mode;
          if (aq_mode > 0)
          {
                ui.aqVarianceCheckBox->setChecked(true);
                ui.aqAlgoComboBox->setCurrentIndex(aq_mode-1);
                MK_UINT(aqStrengthSpinBox,ratecontrol.aq_strength);
          }

          MK_UINT(lookaheadSpinBox,ratecontrol.lookahead);
          MK_CHECKBOX(mbTreeCheckBox,ratecontrol.mb_tree);

          MK_CHECKBOX(loopFilterCheckBox,b_deblocking_filter);
          MK_UINT(alphaC0SpinBox,i_deblocking_filter_alphac0);
          MK_UINT(betaSpinBox,i_deblocking_filter_beta);

          MK_MENU(meMethodComboBox,analyze.me_method);
          MK_MENU(weightedPPredictComboBox,analyze.weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);
          MK_MENU(adaptiveBFrameComboBox,i_bframe_adaptive);
          MK_CHECKBOX(constrainedIntraCheckBox,constrained_intra);

          MK_MENU(predictModeComboBox,analyze.direct_mv_pred);
          MK_UINT(mvRangeSpinBox,analyze.me_range);

          int32_t mv_range = myCopy.analyze.mv_range;
          if(mv_range >= 0)
          {
              ui.mvLengthCheckBox->setChecked(true);
              MK_UINT(mvLengthSpinBox,analyze.mv_range);
          }

          int32_t mv_range_thread = myCopy.analyze.mv_range_thread;

          if(mv_range_thread >= 0)
          {
              ui.minThreadBufferCheckBox->setChecked(true);
              MK_UINT(minThreadBufferSpinBox,analyze.mv_range_thread);
          }

          // preset
          MK_COMBOBOX_STR(presetComboBox, general.preset.c_str(), listOfPresets, NB_PRESET);
          MK_COMBOBOX_STR(profileComboBox, general.profile.c_str(), listOfProfiles, NB_PROFILE);
          MK_COMBOBOX_STR(tuningComboBox, general.tuning.c_str(), listOfTunings, NB_TUNE);

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
          QComboBox *threads=ui.comboBoxThreads;
          for(int i=0;i<NB_THREADS;i++)
          {
                const idcToken *t=listOfThreads+i;
                if(myCopy.general.threads==t->idcValue)
                {
                        threads->setCurrentIndex(i);
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

        DISABLE(spsiComboBox);
        DISABLE(openGopCheckBox);
        DISABLE(groupBox_14); // quant matrix
        //DISABLE(tabAdvanced1);
        DISABLE(tabAdvanced2);
        DISABLE(tabOutput2);
        DISABLE(maxCrfCheckBox);
        DISABLE(sarAsInputRadioButton);
        DISABLE(groupBox_3);
        DISABLE(accessUnitCheckBox);
        
        //
        MK_CHECKBOX(blueRayCompat,general.blueray_compatibility);
        MK_CHECKBOX(fakeInterlaced,general.fake_interlaced);
        
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
#define MK_COMBOBOX_STR(x,y,list,count) x_readComboBox(ui.x,myCopy.y,count,list)

static void x_readComboBox(QComboBox *combo, std::string &inout,int count,const char **list)
{
    int idx=combo->currentIndex(); 
    ADM_assert(idx<count);    
    inout = std::string(list[idx]); 
}




bool x264Dialog::download(void)
{
          MK_CHECKBOX(useAdvancedConfigurationCheckBox,useAdvancedConfiguration);
          MK_CHECKBOX(fastDecodeCheckBox,general.fast_decode);
          MK_CHECKBOX(zeroLatencyCheckBox,general.zero_latency);
          MK_CHECKBOX(fastFirstPassCheckBox,general.fast_first_pass);
          MK_CHECKBOX(fastPSkipCheckBox,analyze.fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,analyze.weighted_bipred);
          MK_CHECKBOX(dct8x8CheckBox,analyze.b_8x8);
          MK_CHECKBOX(i4x4CheckBox,analyze.b_i4x4);
          MK_CHECKBOX(i8x8CheckBox,analyze.b_i8x8);
          MK_CHECKBOX(p4x4CheckBox,analyze.b_p8x8);
          MK_CHECKBOX(p8x8CheckBox,analyze.b_p16x16);
          MK_CHECKBOX(b8x8CheckBox,analyze.b_b16x16);

          MK_CHECKBOX(cabacCheckBox,cabac);
          if (ui.interlacedCheckBox->isChecked()) {
                  myCopy.interlaced = (ui.interlacedComboBox->currentIndex() < 2);
                  myCopy.fake_interlaced = (ui.interlacedComboBox->currentIndex() == 2);
                  myCopy.tff = (ui.interlacedComboBox->currentIndex() == 1);
          } else {
                  myCopy.interlaced = false;
                  myCopy.fake_interlaced = false;
                  myCopy.tff = (ui.interlacedComboBox->currentIndex() == 1);
          }

          MK_CHECKBOX(mixedRefsCheckBox,analyze.mixed_references);
          MK_CHECKBOX(chromaMotionEstCheckBox,analyze.chroma_me);
          MK_CHECKBOX(dctDecimateCheckBox,analyze.dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(IFrameThresholdSpinBox,i_scenecut_threshold);
          MK_CHECKBOX(intraRefreshCheckBox,intra_refresh);
          MK_UINT(meSpinBox,analyze.subpel_refine);
          MK_UINT(BFrameBiasSpinBox,i_bframe_bias);
          MK_UINT(vbvMaxBitrateSpinBox,ratecontrol.vbv_max_bitrate);
          MK_UINT(vbvBufferSizeSpinBox,ratecontrol.vbv_buffer_size);
          MK_UINT(vbvBufferOccupancySpinBox,ratecontrol.vbv_buffer_init);


          MK_MENU(meMethodComboBox,analyze.me_method);
          MK_MENU(weightedPPredictComboBox,analyze.weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);
          MK_MENU(adaptiveBFrameComboBox,i_bframe_adaptive);
          MK_CHECKBOX(constrainedIntraCheckBox,constrained_intra);

          MK_UINT(quantiserMinSpinBox,ratecontrol.qp_min);
          MK_UINT(quantiserMaxSpinBox,ratecontrol.qp_max);
          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
          MK_UINT(avgBitrateToleranceSpinBox, ratecontrol.rate_tolerance);
          myCopy.ratecontrol.rate_tolerance /= 100.0;
          MK_UINT(quantiserIpRatioSpinBox,ratecontrol.ip_factor);
          MK_UINT(quantiserPbRatioSpinBox,ratecontrol.pb_factor);
          MK_UINT(chromaLumaOffsetSpinBox,analyze.chroma_offset);
          int a=ui.aqAlgoComboBox->currentIndex();
          if(!ui.aqVarianceCheckBox->isChecked())
          {
                myCopy.ratecontrol.aq_mode=0;
          }else
          {
                myCopy.ratecontrol.aq_mode=a+1;
                MK_UINT(aqStrengthSpinBox,ratecontrol.aq_strength);
          }

          MK_UINT(lookaheadSpinBox,ratecontrol.lookahead);
          MK_CHECKBOX(mbTreeCheckBox,ratecontrol.mb_tree);

          MK_CHECKBOX(loopFilterCheckBox,b_deblocking_filter);
          MK_UINT(alphaC0SpinBox,i_deblocking_filter_alphac0);
          MK_UINT(betaSpinBox,i_deblocking_filter_beta);

          MK_MENU(predictModeComboBox,analyze.direct_mv_pred);
          MK_UINT(mvRangeSpinBox,analyze.me_range);

          if(ui.mvLengthCheckBox->isChecked())
              MK_UINT(mvLengthSpinBox,analyze.mv_range);
          else
              myCopy.analyze.mv_range=-1;

          if(ui.minThreadBufferCheckBox->isChecked())
              MK_UINT(minThreadBufferSpinBox,analyze.mv_range_thread);
          else
              myCopy.analyze.mv_range_thread=-1;

          MK_UINT(psychoRdoSpinBox,analyze.psy_rd);
          MK_UINT(psychoTrellisSpinBox,analyze.psy_trellis);
          MK_UINT(noiseReductionSpinBox,analyze.noise_reduction);
          MK_UINT(intraLumaSpinBox,analyze.intra_luma);
          MK_UINT(interLumaSpinBox,analyze.inter_luma);

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
          QComboBox *threads=ui.comboBoxThreads;
          int threadIndex=threads->currentIndex();
          myCopy.general.threads=listOfThreads[threadIndex].idcValue;


          int t=ui.trellisComboBox->currentIndex();
          if(!ui.trellisCheckBox->isChecked())
          {
                myCopy.analyze.trellis=0;
          }else
                myCopy.analyze.trellis=t+1;

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
        MK_CHECKBOX(blueRayCompat,general.blueray_compatibility);
        MK_CHECKBOX(fakeInterlaced,general.fake_interlaced);
        return true;
}

// General tab
void x264Dialog::encodingModeComboBox_currentIndexChanged(int index)
{
        bool enableQp = false, enableMaxCrf = false;

        switch (index)
        {
                case 0:
                        ui.targetRateControlLabel1->setText(QT_TRANSLATE_NOOP("x264","Target Bitrate:"));
                        ui.targetRateControlLabel2->setText(QT_TRANSLATE_NOOP("x264","kbit/s"));
                        ui.targetRateControlSpinBox->setValue(lastBitrate);
                        break;
                case 1: // Constant Quality - 1 pass
                        ui.quantiserLabel2->setText(QT_TRANSLATE_NOOP("x264","Quantiser:"));
                        enableQp = true;
                        break;
                case 2: // Average Quantiser - 1 pass
                        ui.quantiserLabel2->setText(QT_TRANSLATE_NOOP("x264","Quality:"));
                        enableQp = true;
                        enableMaxCrf = true;
                        break;
                case 3: // Video Size - 2 pass
                        ui.targetRateControlLabel1->setText(QT_TRANSLATE_NOOP("x264","Target Video Size:"));
                        ui.targetRateControlLabel2->setText(QT_TRANSLATE_NOOP("x264","MB"));
                        ui.targetRateControlSpinBox->setValue(lastVideoSize);
                        break;
                case 4: // Average Bitrate - 2 pass
                        ui.targetRateControlLabel1->setText(QT_TRANSLATE_NOOP("x264","Average Bitrate:"));
                        ui.targetRateControlLabel2->setText(QT_TRANSLATE_NOOP("x264","kbit/s"));
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

void x264Dialog::useAdvancedConfigurationCheckBox_toggled(bool checked)
{
  toogleAdvancedConfiguration(checked);
}

void x264Dialog::quantiserSlider_valueChanged(int value)
{
        ui.quantiserSpinBox->setValue(value);
}

void x264Dialog::meSlider_valueChanged(int value)
{
        ui.meSpinBox->setValue(value);
}
void x264Dialog::quantiserSpinBox_valueChanged(int value)
{
        ui.quantiserSlider->setValue(value);
}
void x264Dialog::meSpinBox_valueChanged(int value)
{
        ui.meSlider->setValue(value);
}

void x264Dialog::targetRateControlSpinBox_valueChanged(int value)
{
        if (ui.encodingModeComboBox->currentIndex() == 3)	// Video Size - 2 pass
                lastVideoSize = value;
        else
                lastBitrate = value;
}

void x264Dialog::loopFilterCheckBox_toggled(bool checked)
{
        if (!checked)
        {
                ui.alphaC0SpinBox->setValue(0);
                ui.betaSpinBox->setValue(0);
        }
}

void x264Dialog::mbTreeCheckBox_toggled(bool checked)
{
        if (checked && !ui.aqVarianceCheckBox->isChecked())
        {
                QString st=QT_TRANSLATE_NOOP("x264","Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.\n\nDo you wish to continue?");
                if (GUI_Question(st.toUtf8().constData()))
                        ui.aqVarianceCheckBox->setChecked(true);
                else
                        ui.mbTreeCheckBox->setChecked(false);
        }
}

void x264Dialog::aqVarianceCheckBox_toggled(bool checked)
{
        if (!checked && ui.mbTreeCheckBox->isChecked())
        {
                 QString st=QT_TRANSLATE_NOOP("x264","Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.\n\nDo you wish to continue?");
                if (GUI_Question(st.toUtf8().constData()))
                        ui.mbTreeCheckBox->setChecked(false);
                else
                        ui.aqVarianceCheckBox->setChecked(true);
        }
}
#if 0
void x264Dialog::maxCrfSlider_valueChanged(int value)
{
        ui.maxCrfSpinBox->setValue(value);
}

void x264Dialog::maxCrfSpinBox_valueChanged(int value)
{
        ui.maxCrfSlider->setValue(value);
}
#endif

/**
    \fn configurationComboBox_currentIndexChanged
*/
void x264Dialog::configurationComboBox_currentIndexChanged(int index)
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
    ADM_pluginGetPath("x264",pluginVersion,rootPath);
    QString text=QString(ADM_SEPARATOR)+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    char *t=ADM_strdup(text.toUtf8().constData());
    ADM_info("Loading preset %s\n",t);
    if(false==x264_encoder_jdeserialize(t,x264_encoder_param,&myCopy))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x264","Error"),QT_TRANSLATE_NOOP("x264","Cannot load preset"));
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
  dialog.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("x264","Save Profile")));
  QDialogButtonBox *buttonBox = new QDialogButtonBox();  
  QVBoxLayout *vboxLayout = new QVBoxLayout();
  buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

  QLineEdit *text=new QLineEdit;
//  text->setAcceptRichText(false);

  text->setText(QT_TRANSLATE_NOOP("x264","my profile"));
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
void x264Dialog::saveAsButton_pressed(void)
{
  // 1-ask name
  char *out=getProfileName();
  if(!out) return;
  ADM_info("Using %s\n",out);
  download();
  std::string rootPath;
  ADM_pluginGetPath("x264",pluginVersion,rootPath);
  std::string fullpath=rootPath+std::string(ADM_SEPARATOR)+out+std::string(".json");

  if(ADM_fileExist(fullpath.c_str()))
  {
        if(false==GUI_Confirmation_HIG(QT_TRANSLATE_NOOP("x264","Overwrite"),QT_TRANSLATE_NOOP("x264","Replace the following preset ?:"),out))
        {
            ADM_dealloc(out);
            return;
        }
  }
  ADM_dealloc(out);
  if(false==x264_encoder_jserialize(fullpath.c_str(),&myCopy))
  {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x264","Error"),QT_TRANSLATE_NOOP("x264","Cannot save preset"));
        ADM_error("Cannot write to %s\n",out);
  }
  updatePresetList();
}
/**

*/
void x264Dialog::deleteButton_pressed(void)
{ 
    int n=ui.configurationComboBox->currentIndex();
    int m=ui.configurationComboBox->count();
    if(n==m-1) // custom
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x264","Error"),QT_TRANSLATE_NOOP("x264","Cannot delete custom profile"));
        return;
    }
  QString preset=ui.configurationComboBox->itemText(n);
  QString msg=QString(QT_TRANSLATE_NOOP("x264","Do you really want to delete the "))+preset+
            QString(QT_TRANSLATE_NOOP("x264"," profile ?.\nIf it is a system profile it will be recreated next time."));
  if(true==GUI_Confirmation_HIG(QT_TRANSLATE_NOOP("x264","Delete preset"),QT_TRANSLATE_NOOP("x264","Delete"),msg.toUtf8().constData()))
  {
    std::string rootPath;
    ADM_pluginGetPath("x264",pluginVersion,rootPath);
    QString text=QString("/")+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    unlink(text.toUtf8().constData());
  }
  updatePresetList();
}


