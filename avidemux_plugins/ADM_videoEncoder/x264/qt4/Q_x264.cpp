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
    {1,"1"},
    {2,"2"},
    {4,"4"}
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

static const idcToken listOfPrimaries[] = {
    {1, "BT.709 (HD)" },
    {4, "BT.470M (old NTSC)" },
    {5, "BT.470BG (PAL)" },
    {6, "SMPTE 170M (NTSC)" }, /* Same as SMTPE 240M */
    {7, "SMPTE 240M" },
    {9, "BT.2020 (UHD)" },
    {8, "Film" },
    {10, "SMPTE 428" },
    {11, "SMPTE 431-2 (DCI-P3)" },
    {12, "SMPTE 432 (Display P3)" }
};

static const idcToken listOfXfers[] = {
    {1,  "BT.709 (HD)" }, /* Same as SMPTE 170M, BT.2020-10, BT.2020-12 */
    {4,  "BT.470M (NTSC)" },
    {5,  "BT.470BG (PAL)" },
    {6,  "SMPTE 170M (NTSC)" },
    {7,  "SMPTE 240M" },
    {14, "BT.2020 10bit (UHD)" },
    {15, "BT.2020 12bit (UHD)" },
    {13, "IEC 61966-2-1 (sRGB)" },
    {11, "IEC 61966-2-4" },
    {16, "SMPTE 2084 (HDR)" },
    {12, "BT.1361e" },
    {17, "SMPTE 428" },
    {8,  "linear" },
    {9,  "log100" },
    {10, "log316" },
    {18, "arib-std-b67" }
};

static const idcToken listOfMatrices[] = { /* Kr      Kb     */
    {1,  "BT.709 (HD)" },                  /* 0.2126  0.0722 */
    {4,  "FCC (old NTSC)" },               /* 0.30    0.11   */
    {5,  "BT.470BG (PAL)" },               /* 0.299   0.114  */
    {6,  "SMPTE 170M (NTSC)" },            /* 0.299   0.114  */
    {7,  "SMPTE 240M" },                   /* 0.212   0.087  */
    {9,  "BT.2020 NCL (UHD)" },            /* 0.2627  0.0593 */
    {10, "BT.2020 CL" },                   /* Color difference uses different values than luma */
    {11, "SMPTE 2085 (HDR)" },
    {14, "ICtCp" },
    {0,  "gbr" },
    {8,  "YCGCO" },
    {12, "Chroma Derived NCL" },
    {13, "Chroma Derived CL" }
};

#define NB_SAR sizeof(predefinedARs)/sizeof(aspectRatio)

static const char* listOfPresets[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo" };
#define NB_PRESET sizeof(listOfPresets)/sizeof(char*)

static const char* listOfTunings[] = { "film", "animation", "grain", "stillimage", "psnr", "ssim" };
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
    \brief Set combo box items to be elements in container with data.

    Elements of container should be idcToken.  The string will be the item name
    and value will be added as item data.

    One could design function that used a lambda to convert from element type
    into label and value, so that any element type can be used, but that is
    C++14.

    If initial is non-NULL, it will be added first and the data used will be
    initdata;

    As a somewhat ugly hack for Qt4, the initial string will go through
    fromUtf8() in case it has been translated to non-ASCII.  As none of the
    items in the containers used are translated, they don't need this.

    \param [in] box ComboBox to set items in
    \param [in] items Container (array) of idcToken to use.
    \param [in] initial An option first item to add.
    \param [in] initdata The data to be assigned with optional first item, default is empty.
*/
template <typename ContainerT>
static void fillComboBoxData(QComboBox *box, const ContainerT& items, const char* initial = NULL, const QVariant& initdata = QVariant())
{
    box->clear();
    if (initial)
        box->addItem(QString::fromUtf8(initial), initdata);
    for (const idcToken *t = std::begin(items); t != std::end(items); t++)
        box->addItem(t->idcString, QVariant(t->idcValue));
}

/**
    \brief Set combo box items to be elements in container.

    Elements should be const char*.  In C++11 we could use anything convertable
    to a QString by changing "const char* const *" to "auto".

    The data assigned to each item will be the index in items whence it came.

    See fillComboBoxData().
*/
template <typename ContainerT>
static void fillComboBox(QComboBox *box, const ContainerT& items, const char* initial = NULL)
{
    box->clear();
    if (initial)
        box->addItem(QString::fromUtf8(initial), -1);
    int i = 0;
    for (const char * const *t = std::begin(items); t != std::end(items); t++, i++)
        box->addItem(*t, i);
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
        connect(ui.trellisCheckBox, SIGNAL(toggled(bool)), this, SLOT(trellisCheckBox_toggled(bool)));
        connect(ui.trellisComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(trellisComboBox_currentIndexChanged(int)));
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

        const char *automatic = QT_TRANSLATE_NOOP("x264","Auto");
        const char *none = QT_TRANSLATE_NOOP("x264","none");
        const char *unknown = QT_TRANSLATE_NOOP("x264","Unknown");

        // Rebuild idc level list
        fillComboBoxData(ui.idcLevelComboBox, listOfIdc, automatic, -1);

        fillComboBoxData(ui.comboBoxThreads, listOfThreads, automatic, 0U);

        fillComboBox(ui.presetComboBox, listOfPresets);
        fillComboBox(ui.tuningComboBox, listOfTunings, none);
        fillComboBox(ui.profileComboBox, listOfProfiles);

        fillComboBoxData(ui.colourPrimariesComboBox, listOfPrimaries, unknown, 2);
        fillComboBoxData(ui.transferCharacteristicsComboBox, listOfXfers, unknown, 2);
        fillComboBoxData(ui.colourMatrixComboBox, listOfMatrices, unknown, 2);

        upload();
        ADM_pluginInstallSystem( std::string("x264"),std::string("json"),pluginVersion);
        updatePresetList();
#if defined(__APPLE__) && QT_VERSION == QT_VERSION_CHECK(5,13,0)
        // On macOS with Qt 5.13.0, the width of the view is too small, resulting in entries being truncated due to wrapping.
        int width=0;
        for(int i=0;i<ui.encodingModeComboBox->count();i++)
        {
            QString text=ui.encodingModeComboBox->itemText(i);
            QFontMetrics fm=ui.encodingModeComboBox->fontMetrics();
            int w=fm.boundingRect(text).width();
            if(w>width) width=w;
        }
        width+=ui.encodingModeComboBox->minimumSizeHint().width();
        ui.encodingModeComboBox->view()->setMinimumWidth(width);
#endif
        adjustSize();
}
/**
    \fn updatePresetList
*/
bool x264Dialog::updatePresetList(const char *match)
{
    QComboBox *combo=ui.configurationComboBox;
    std::string rootPath;
    vector <std::string >  list;
    ADM_pluginGetPath("x264",pluginVersion,rootPath);
    ADM_listFile(rootPath,"json",list);
    int l=list.size();
    int idx=l;
    std::string current;
    if(match)
        current=std::string(match);
    combo->clear();
    for( int i=0;i<l;i++)
    {
        if(match && list[i]==current)
            idx=i;
        combo->addItem(list[i].c_str());
    }
    combo->addItem(QString(QT_TRANSLATE_NOOP("x264","Custom")));
    combo->setCurrentIndex(idx);
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
#define DISABLE(x) ui.x->setEnabled(false)
#define MK_MENU(x,y) ui.x->setCurrentIndex(myCopy.y)
#define MK_RADIOBUTTON(x) ui.x->setChecked(true)
#define MK_COMBOBOX_STR(x,y) \
{ \
    const int idx = ui.x->findText(myCopy.y.c_str()); \
    ui.x->setCurrentIndex(idx < 0 ? 0 : idx); \
}
#define MK_COMBOBOX_DATA(x,y) ui.x->setCurrentIndex(ui.x->findData(myCopy.y))

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
          MK_COMBOBOX_STR(presetComboBox, general.preset)
          MK_COMBOBOX_STR(profileComboBox, general.profile)
          MK_COMBOBOX_STR(tuningComboBox, general.tuning)

          // udate idc
          MK_COMBOBOX_DATA(idcLevelComboBox, level);

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

        MK_COMBOBOX_DATA(colourPrimariesComboBox, vui.colorprim);
        MK_COMBOBOX_DATA(transferCharacteristicsComboBox, vui.transfer);
        MK_COMBOBOX_DATA(colourMatrixComboBox, vui.colmatrix);

        MK_CHECKBOX(fullRangeSamplesCheckBox, vui.fullrange);

        DISABLE(spsiComboBox);
        DISABLE(openGopCheckBox);
        DISABLE(groupBoxQuantMatrix);
        //DISABLE(tabAdvanced1);
        DISABLE(tabAdvanced2);
        //DISABLE(tabOutput2);
        DISABLE(overscanComboBox);
        DISABLE(videoFormatComboBox);
        DISABLE(hrdComboBox);
        DISABLE(chromaSampleSpinBox);
        DISABLE(maxCrfCheckBox);
        DISABLE(sarAsInputRadioButton);
        DISABLE(groupBoxQuantCurveCompress);
        DISABLE(accessUnitCheckBox);
        DISABLE(groupBoxZones);
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
#undef MK_COMBOBOX_DATA
#define MK_CHECKBOX(x,y)    myCopy.y=ui.x->isChecked()
#define MK_UINT(x,y)        myCopy.y=ui.x->value()
#define MK_MENU(x,y)        myCopy.y=ui.x->currentIndex()
#define MK_RADIOBUTTON(x,y)   myCopy.y=ui.x->setChecked(true);
#define MK_COMBOBOX_STR(x,y,list,count,none) \
{ \
    const QComboBox* combo=ui.x; \
    const int idx = combo->itemData(combo->currentIndex()).toInt(); \
    ADM_assert(idx < 0 || idx<count); \
    myCopy.y = idx < 0 ? none : list[idx]; \
}
#define MK_COMBOBOX_DATA(x,y) \
{ \
    const QComboBox* combo=ui.x; \
    myCopy.y = combo->itemData(combo->currentIndex()).toInt(); \
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

          MK_COMBOBOX_STR(presetComboBox, general.preset, listOfPresets, NB_PRESET, "");
          MK_COMBOBOX_STR(profileComboBox, general.profile, listOfProfiles, NB_PROFILE, "");
          MK_COMBOBOX_STR(tuningComboBox, general.tuning, listOfTunings, NB_TUNE, "none");

          MK_COMBOBOX_DATA(idcLevelComboBox, level);

          switch(ui.encodingModeComboBox->currentIndex())
          {
            case 0: ENCODING(mode)=COMPRESS_CBR; ENCODING(bitrate)=ui.targetRateControlSpinBox->value();break;
            case 1: ENCODING(mode)=COMPRESS_CQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 2: ENCODING(mode)=COMPRESS_AQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 3: ENCODING(mode)=COMPRESS_2PASS;ENCODING(finalsize)=ui.targetRateControlSpinBox->value();;break;
            case 4: ENCODING(mode)=COMPRESS_2PASS_BITRATE;ENCODING(avg_bitrate)=ui.targetRateControlSpinBox->value();;break;
          }
          // update thread count
          MK_COMBOBOX_DATA(comboBoxThreads, general.threads);

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

        MK_CHECKBOX(fullRangeSamplesCheckBox, vui.fullrange);

        MK_COMBOBOX_DATA(colourPrimariesComboBox, vui.colorprim);
        MK_COMBOBOX_DATA(transferCharacteristicsComboBox, vui.transfer);
        MK_COMBOBOX_DATA(colourMatrixComboBox, vui.colmatrix);

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
    if(value>9)
    {
        ui.aqVarianceCheckBox->setChecked(true);
        ui.trellisCheckBox->setChecked(true);
        ui.trellisComboBox->setCurrentIndex(1);
    }
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
                {
                        ui.mbTreeCheckBox->setChecked(false);
                        if(ui.meSpinBox->value()>9)
                                ui.meSpinBox->setValue(9);
                }else
                        ui.aqVarianceCheckBox->setChecked(true);
        }
}

void x264Dialog::trellisCheckBox_toggled(bool checked)
{
    if(!checked && ui.meSpinBox->value()>9)
    {
        ui.meSpinBox->setValue(9);
    }
}

void x264Dialog::trellisComboBox_currentIndexChanged(int index)
{
    if(index<1 && ui.meSpinBox->value()>9)
    {
        ui.meSpinBox->setValue(9);
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
static char *getProfileName(QDialog *parent)
{
  QDialog dialog(parent);
  dialog.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("x264","Save Profile")));
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
//  dialog.setModal(true);
//  dialog.setWindowModality(Qt::ApplicationModal);
  dialog.setLayout(vboxLayout);

  if(dialog.exec()!=QDialog::Accepted)
  {
        ADM_info("Cancelled");
        return NULL;
  }
  std::string st = std::string(text->text().toUtf8().constData());
  return ADM_strdup(st.c_str());
}
/**
        \fn saveAsButton_pressed
        \brief Save the current settings as preset
*/
void x264Dialog::saveAsButton_pressed(void)
{
  // 1-ask name
  char *out=getProfileName(this);
  if(!out) return;
  ADM_info("Using %s\n",out);
  download();
  std::string rootPath;
  ADM_pluginGetPath("x264",pluginVersion,rootPath);
  std::string name=std::string(out);
  std::string fullpath=rootPath+std::string(ADM_SEPARATOR)+name+std::string(".json");

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
  updatePresetList(name.c_str());
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
  if(true==GUI_Confirmation_HIG(QT_TRANSLATE_NOOP("x264","Delete"),QT_TRANSLATE_NOOP("x264","Delete preset"),msg.toUtf8().constData()))
  {
    std::string rootPath;
    ADM_pluginGetPath("x264",pluginVersion,rootPath);
    QString text=QString("/")+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    if(!ADM_eraseFile(text.toUtf8().constData()))
        ADM_warning("Could not delete %s\n",text.toUtf8().constData());
  }
  updatePresetList();
}

