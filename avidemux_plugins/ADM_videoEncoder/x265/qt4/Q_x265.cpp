/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 HEVC encoder
                             -------------------
    
    copyright            : (C) 2014 gruntster/mean
 ***************************************************************************/
#include <math.h>
#include <vector>
#include <QFileDialog>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
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

extern bool x265ProbeBitDepth(int depth);

typedef struct
{
    uint32_t idcValue;
    const char *idcString;
}idcToken;

static const idcToken listOfIdc[]={
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

static const idcToken listOfBitDepths[]={
    {8,"8"},
    {10,"10"},
    {12,"12"}
};

#define NB_BITS sizeof(listOfBitDepths)/sizeof(*listOfBitDepths)

static const char* listOfPresets[] = { "ultrafast", "superfast", "veryfast", "faster", "fast", "medium", "slow", "slower", "veryslow", "placebo" };
#define NB_PRESET sizeof(listOfPresets)/sizeof(char*)

static const char* listOfTunings[] = { "psnr", "ssim", "grain", "zerolatency", "fastdecode",
#if X265_BUILD > 173
    "animation"
#endif
};
#define NB_TUNE sizeof(listOfTunings)/sizeof(char*)

static const char* listOfProfiles[] = { "main", "main10", "mainstillpicture" };
#define NB_PROFILE sizeof(listOfProfiles)/sizeof(char*)

static const idcToken listOfSARs[] = {
    { 1, "1:1 (Normal)" },
    { 2, "12:11 (PAL 4:3)" },
    { 4, "16:11 (PAL 16:9)" },
    { 3, "10:11 (NTSC 4:3)" },
    { 5, "40:33 (NTSC 16:9)" },
    { 6, "24:11" },
    { 7, "20:11" },
    { 8, "32:11" },
    { 9, "80:33" },
    { 10, "18:11" },
    { 11, "15:11" },
    { 12, "64:33" },
    { 13, "160:99" },
    { 14, "4:3" },
    { 15, "3:2" },
    { 16, "2:1" },
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
    {12, "SMPTE 432 (Display P3)" },
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
    {18, "arib-std-b67"},
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
    {13, "Chroma Derived CL" },
};

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
x265Dialog::x265Dialog(QWidget *parent, void *param) : QDialog(parent)
{
       ui.setupUi(this);
#if X265_BUILD < 178
        {
            int algos = ui.aqAlgoComboBox->count();
            while(algos-- > 3)
                ui.aqAlgoComboBox->removeItem(algos);
        }
#endif
        connect(ui.useAdvancedConfigurationCheckBox, SIGNAL(toggled(bool)), this, SLOT(useAdvancedConfigurationCheckBox_toggled(bool)));
        connect(ui.encodingModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(encodingModeComboBox_currentIndexChanged(int)));
        connect(ui.quantiserSlider, SIGNAL(valueChanged(int)), this, SLOT(quantiserSlider_valueChanged(int)));
        connect(ui.meSlider, SIGNAL(valueChanged(int)), this, SLOT(meSlider_valueChanged(int)));
        connect(ui.quantiserSpinBox, SIGNAL(valueChanged(int)), this, SLOT(quantiserSpinBox_valueChanged(int)));
        connect(ui.rectInterCheckBox, SIGNAL(toggled(bool)), this, SLOT(rectInterCheckBox_toggled(bool)));
        connect(ui.refFramesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(refFramesSpinBox_valueChanged(int)));
        connect(ui.maxBFramesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxBFramesSpinBox_valueChanged(int)));
        connect(ui.bFrameRefComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(bFrameRefComboBox_currentIndexChanged(int)));
        connect(ui.meSpinBox, SIGNAL(valueChanged(int)), this, SLOT(meSpinBox_valueChanged(int)));
        connect(ui.targetRateControlSpinBox, SIGNAL(valueChanged(int)), this, SLOT(targetRateControlSpinBox_valueChanged(int)));
        connect(ui.rdoqSpinBox, SIGNAL(valueChanged(int)), this, SLOT(rdoqSpinBox_valueChanged(int)));
        connect(ui.cuTreeCheckBox, SIGNAL(toggled(bool)), this, SLOT(cuTreeCheckBox_toggled(bool)));
        connect(ui.aqVarianceCheckBox, SIGNAL(toggled(bool)), this, SLOT(aqVarianceCheckBox_toggled(bool)));
#if 0
        connect(ui.maxCrfSlider, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSlider_valueChanged(int)));
        connect(ui.maxCrfSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSpinBox_valueChanged(int)));
#endif
       x265_settings* settings = (x265_settings*)param;
       myCopy=*settings;

#define ENCODING(x)  myCopy.general.params.x       
        lastBitrate =   ENCODING(bitrate);
        lastVideoSize = ENCODING(finalsize);

        ui.tabWidget->setCurrentIndex(0);
        connect(ui.deleteButton, SIGNAL(pressed()), this, SLOT(deleteButton_pressed()));
        connect(ui.saveAsButton, SIGNAL(pressed()), this, SLOT(saveAsButton_pressed()));
        connect(ui.configurationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(configurationComboBox_currentIndexChanged(int)));

        const char *automatic=QT_TRANSLATE_NOOP("x265","Auto");
        const char *none=QT_TRANSLATE_NOOP("x265","none");
        const char *dflt=QT_TRANSLATE_NOOP("x265","Default");
        const char *unknown=QT_TRANSLATE_NOOP("x265","Unknown");

        // Rebuild idc level list
        fillComboBoxData(ui.idcLevelComboBox, listOfIdc, automatic, -1);

        fillComboBoxData(ui.comboBoxPoolThreads, listOfThreads, automatic, 0U);

        fillComboBoxData(ui.comboBoxFrameThreads, listOfThreads, automatic, 0U);

        fillComboBox(ui.presetComboBox, listOfPresets);

        fillComboBox(ui.tuningComboBox, listOfTunings, none);

        fillComboBox(ui.profileComboBox, listOfProfiles);
        if (!x265ProbeBitDepth(10))
            ui.profileComboBox->removeItem(ui.profileComboBox->findText("main10"));

        QComboBox *depths=ui.comboBoxBitDepth;
        depths->clear();
        depths->addItem(QString::fromUtf8(dflt), QVariant(0U));
        for(int i=0;i<NB_BITS;i++)
        {
            const idcToken *t = listOfBitDepths + i;
            if(x265ProbeBitDepth(t->idcValue))
                depths->addItem(QString(t->idcString), QVariant(t->idcValue));
        }

        fillComboBoxData(ui.sarPredefinedComboBox, listOfSARs);
        fillComboBoxData(ui.colourPrimariesComboBox, listOfPrimaries, unknown, 2);
        fillComboBoxData(ui.transferCharacteristicsComboBox, listOfXfers, unknown, 2);
        fillComboBoxData(ui.colourMatrixComboBox, listOfMatrices, unknown, 2);

        upload();

        rdoqSpinBox_valueChanged(ui.rdoqSpinBox->value());
        rectInterCheckBox_toggled(ui.rectInterCheckBox->isChecked());

        ADM_pluginInstallSystem( std::string("x265"),std::string("json"),pluginVersion);
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
bool x265Dialog::updatePresetList(const char *match)
{
    QComboBox *combo=ui.configurationComboBox;
    std::string rootPath;
    vector <std::string >  list;
    ADM_pluginGetPath("x265",pluginVersion,rootPath);
    ADM_listFile(rootPath,"json",list);
    int l=list.size();
    int idx=l;
    combo->clear();
    for( int i=0;i<l;i++)
    {
        if(match && list[i]==match)
            idx=i;
        combo->addItem(list[i].c_str());
    }
    combo->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Custom")));
    combo->setCurrentIndex(idx);
    return true;
}

bool x265Dialog::toogleAdvancedConfiguration(bool advancedEnabled)
{
  ui.useAdvancedConfigurationCheckBox->setChecked(advancedEnabled);
  ui.presetComboBox->setEnabled(!advancedEnabled);
  ui.tuningComboBox->setEnabled(!advancedEnabled);
  ui.profileComboBox->setEnabled(!advancedEnabled);
  ui.comboBoxBitDepth->setEnabled(advancedEnabled);
  ui.tabAdvancedRC->setEnabled(advancedEnabled);
  ui.tabMotion->setEnabled(advancedEnabled);
  ui.tabFrame->setEnabled(advancedEnabled);
  ui.tabAnalysis->setEnabled(advancedEnabled);
  ui.tabQuantiser->setEnabled(advancedEnabled);
  return true;
}

/**

*/
#define MK_CHECKBOX(x,y) ui.x->setChecked(myCopy.y)
#define MK_UINT(x,y)  ui.x->setValue(myCopy.y)
#define DISABLE(x) ui.x->setEnabled(false);
#define MK_MENU(x,y) ui.x->setCurrentIndex(myCopy.y)
#define MK_RADIOBUTTON(x) ui.x->setChecked(true);
#define MK_COMBOBOX_STR(x,y) \
    { \
        const int idx = ui.x->findText(myCopy.y.c_str()); \
        ui.x->setCurrentIndex(idx < 0 ? 0 : idx); \
    }
#define MK_COMBOBOX_DATA(x,y) ui.x->setCurrentIndex(ui.x->findData(myCopy.y))

bool x265Dialog::upload(void)
{
          toogleAdvancedConfiguration(myCopy.useAdvancedConfiguration);
          MK_CHECKBOX(fastPSkipCheckBox,fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,weighted_bipred);
          MK_CHECKBOX(rectInterCheckBox,rect_inter);
          MK_CHECKBOX(AMPInterCheckBox,amp_inter);
          MK_CHECKBOX(limitInterModesCheckBox,limit_modes);
          MK_UINT(rdoSpinBox,rd_level);
          MK_UINT(psychoRdoSpinBox,psy_rd);
          MK_UINT(rdoqSpinBox,rdoq_level);
          MK_UINT(psychoRdoqSpinBox,psy_rdoq); /* double, not uint, but setValue() is the same */

          if (myCopy.interlaced_mode > 0) {
        	  ui.interlacedCheckBox->setChecked(true);
                  ui.interlacedComboBox->setCurrentIndex(myCopy.interlaced_mode - 1);
          } else {
        	  ui.interlacedCheckBox->setChecked(false);
          }
    
          MK_CHECKBOX(dctDecimateCheckBox,dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          MK_CHECKBOX(limitRefDepthCheckBox,limit_refs & X265_REF_LIMIT_DEPTH);
          MK_CHECKBOX(limitRefCUCheckBox,limit_refs & X265_REF_LIMIT_CU);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(IFrameThresholdSpinBox,i_scenecut_threshold);
          MK_UINT(meSpinBox,subpel_refine);

          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
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
          MK_CHECKBOX(openGopCheckBox,b_open_gop);

          MK_MENU(meMethodComboBox,me_method);
          MK_MENU(weightedPPredictComboBox,weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);
          MK_MENU(adaptiveBFrameComboBox,i_bframe_adaptive);
          MK_CHECKBOX(constrainedIntraCheckBox,constrained_intra);
          MK_CHECKBOX(bIntraCheckBox,b_intra);

          MK_UINT(mvRangeSpinBox,me_range);

          // preset
          MK_COMBOBOX_STR(presetComboBox, general.preset);
          MK_COMBOBOX_STR(profileComboBox, general.profile);
          MK_COMBOBOX_STR(tuningComboBox, general.tuning);

        // udate idc
        MK_COMBOBOX_DATA(idcLevelComboBox, level);

        // update threads
#if X265_BUILD >= 47
        DISABLE(comboBoxPoolThreads);
#else
        MK_COMBOBOX_DATA(comboBoxPoolThreads, general.poolThreads);
#endif
          
        /* both 0 and 99 mean auto? */
        const unsigned int framethreads = myCopy.general.frameThreads == 99 ? 0 : myCopy.general.frameThreads;
        ui.comboBoxFrameThreads->setCurrentIndex(ui.comboBoxFrameThreads->findData(framethreads));

        // update bit depth
        QComboBox *bdc=ui.comboBoxBitDepth;
        int i = bdc->findData(myCopy.general.output_bit_depth);
        if(i == -1) /* Not found.  Maybe saved project and depth is not supported anymore?  Use default. */
        {
            ADM_warning("X265 output bit depth %u not supported, using default\n", myCopy.general.output_bit_depth);
            i = 0;
        }
        bdc->setCurrentIndex(i);

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

    if (myCopy.vui.sar_idc == 0)
    {
        MK_RADIOBUTTON(sarUnspecifiedRadioButton);
    } else if (myCopy.vui.sar_idc == X265_EXTENDED_SAR)
    {
        MK_RADIOBUTTON(sarCustomRadioButton);
        MK_UINT(sarCustomSpinBox1,vui.sar_width);
        MK_UINT(sarCustomSpinBox2,vui.sar_height);
    } else
    {
        MK_RADIOBUTTON(sarPredefinedRadioButton);
        MK_COMBOBOX_DATA(sarPredefinedComboBox,vui.sar_idc);
    }

    MK_UINT(noiseReductionIntraSpinBox,noise_reduction_intra);
    MK_UINT(noiseReductionInterSpinBox,noise_reduction_inter);

    MK_CHECKBOX(strongIntraSmoothingCheckBox,strong_intra_smoothing);

    MK_CHECKBOX(strictCbrCheckBox,ratecontrol.strict_cbr);

    /* VUI */
    MK_COMBOBOX_DATA(colourPrimariesComboBox,vui.color_primaries);
    MK_COMBOBOX_DATA(transferCharacteristicsComboBox,vui.transfer_characteristics);
    MK_COMBOBOX_DATA(colourMatrixComboBox,vui.matrix_coeffs);
    DISABLE(tabOutput); /* These aren't implemented */
    DISABLE(videoFormatComboBox);
    DISABLE(sarAsInputRadioButton);

          DISABLE(spsiComboBox);
          DISABLE(groupBox_14); // quant matrix
          DISABLE(tabAdvanced1);
          DISABLE(tabAdvanced2);
          DISABLE(maxCrfCheckBox);
          DISABLE(groupBox_3);
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

bool x265Dialog::download(void)
{
          MK_CHECKBOX(useAdvancedConfigurationCheckBox,useAdvancedConfiguration);
          MK_CHECKBOX(fastPSkipCheckBox,fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,weighted_bipred);
          MK_CHECKBOX(rectInterCheckBox,rect_inter);
          MK_CHECKBOX(AMPInterCheckBox,amp_inter);
          MK_CHECKBOX(limitInterModesCheckBox,limit_modes);

          if (ui.interlacedCheckBox->isChecked()) {
                  myCopy.interlaced_mode = ui.interlacedComboBox->currentIndex() + 1;
          } else {
        	  myCopy.interlaced_mode = 0;
          }
    
          MK_CHECKBOX(dctDecimateCheckBox,dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(refFramesSpinBox,MaxRefFrames);
          myCopy.limit_refs = (ui.limitRefDepthCheckBox->isChecked() ? X265_REF_LIMIT_DEPTH : 0) |
                              (ui.limitRefCUCheckBox->isChecked() ? X265_REF_LIMIT_CU : 0);
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
          MK_CHECKBOX(bIntraCheckBox,b_intra);

          MK_UINT(quantiserMaxStepSpinBox,ratecontrol.qp_step);
          
          MK_CHECKBOX(strictCbrCheckBox,ratecontrol.strict_cbr);
          
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
          MK_CHECKBOX(openGopCheckBox,b_open_gop);

          MK_UINT(mvRangeSpinBox,me_range);

          MK_UINT(rdoSpinBox,rd_level);
          MK_UINT(psychoRdoSpinBox,psy_rd);
          MK_UINT(rdoqSpinBox,rdoq_level);
          MK_UINT(psychoRdoqSpinBox,psy_rdoq); /* double, not uint, but value() is the same */
          
          MK_UINT(noiseReductionIntraSpinBox,noise_reduction_intra);
          MK_UINT(noiseReductionInterSpinBox,noise_reduction_inter);

          MK_CHECKBOX(strongIntraSmoothingCheckBox,strong_intra_smoothing);

          MK_COMBOBOX_STR(presetComboBox, general.preset, listOfPresets, NB_PRESET, "");
          MK_COMBOBOX_STR(profileComboBox, general.profile, listOfProfiles, NB_PROFILE, "");
          MK_COMBOBOX_STR(tuningComboBox, general.tuning, listOfTunings, NB_TUNE, "none");

          MK_COMBOBOX_DATA(idcLevelComboBox, level);
          MK_COMBOBOX_DATA(comboBoxBitDepth, general.output_bit_depth);

          switch(ui.encodingModeComboBox->currentIndex())
          {
            case 0: ENCODING(mode)=COMPRESS_CBR; ENCODING(bitrate)=ui.targetRateControlSpinBox->value();break;
            case 1: ENCODING(mode)=COMPRESS_CQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 2: ENCODING(mode)=COMPRESS_AQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 3: ENCODING(mode)=COMPRESS_2PASS;ENCODING(finalsize)=ui.targetRateControlSpinBox->value();;break;
            case 4: ENCODING(mode)=COMPRESS_2PASS_BITRATE;ENCODING(avg_bitrate)=ui.targetRateControlSpinBox->value();;break;
          }
          
#if X265_BUILD < 47
          MK_COMBOBOX_DATA(comboBoxPoolThreads, general.poolThreads);
#endif
          MK_COMBOBOX_DATA(comboBoxFrameThreads, general.frameThreads);

          if(ui.sarUnspecifiedRadioButton->isChecked())
          {
              myCopy.vui.sar_idc = 0;
          }else if(ui.sarCustomRadioButton->isChecked())
          {
              myCopy.vui.sar_idc = X265_EXTENDED_SAR;
              MK_UINT(sarCustomSpinBox1,vui.sar_width);
              MK_UINT(sarCustomSpinBox2,vui.sar_height);
          }else
          {
              MK_COMBOBOX_DATA(sarPredefinedComboBox,vui.sar_idc);
          }

          MK_COMBOBOX_DATA(colourPrimariesComboBox, vui.color_primaries);
          MK_COMBOBOX_DATA(transferCharacteristicsComboBox, vui.transfer_characteristics);
          MK_COMBOBOX_DATA(colourMatrixComboBox, vui.matrix_coeffs);

          return true;
}

// General tab
void x265Dialog::encodingModeComboBox_currentIndexChanged(int index)
{
	bool enableQp = false, enableMaxCrf = false, enableStrictCbr = false;

	switch (index)
	{
		case 0:
			ui.targetRateControlLabel1->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Target Bitrate:")));
			ui.targetRateControlLabel2->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","kbit/s")));
			ui.targetRateControlSpinBox->setValue(lastBitrate);
                        enableStrictCbr = true;
			break;
		case 1: // Constant Quality - 1 pass
			ui.quantiserLabel2->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Quantiser:")));
			enableQp = true;
			break;
		case 2: // Average Quantiser - 1 pass
			ui.quantiserLabel2->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Quality:")));
			enableQp = true;
			enableMaxCrf = true;
			break;
		case 3: // Video Size - 2 pass
			ui.targetRateControlLabel1->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Target Video Size:")));
			ui.targetRateControlLabel2->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","MB")));
			ui.targetRateControlSpinBox->setValue(lastVideoSize);
			break;
		case 4: // Average Bitrate - 2 pass
			ui.targetRateControlLabel1->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Average Bitrate:")));
			ui.targetRateControlLabel2->setText(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","kbit/s")));
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
        
#if X265_BUILD >= 41
        ui.strictCbrCheckBox->setEnabled(enableStrictCbr);
#endif

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

void x265Dialog::rectInterCheckBox_toggled(bool checked)
{
    ui.AMPInterCheckBox->setEnabled(checked);
    ui.limitInterModesCheckBox->setEnabled(checked);
    if(!checked)
    {
        ui.AMPInterCheckBox->setChecked(false);
        ui.limitInterModesCheckBox->setChecked(false);
    }
}

void x265Dialog::refFramesSpinBox_valueChanged(int value)
{
    // Limiting reference frames doesn't have an effect when they are already limited to 1
    const bool enable = value > 1;
    ui.limitRefDepthCheckBox->setEnabled(enable);
    ui.limitRefCUCheckBox->setEnabled(enable);
}

void x265Dialog::maxBFramesSpinBox_valueChanged(int value)
{
    // HEVC specification allows a maximum of 8 total reference frames only if B frames are disabled.
    // Enforce the limit to ensure compliance.
    if(!value)
    {
        ui.refFramesSpinBox->setMaximum(8);
    }else // with B frames enabled, the maximum decreases to 7
    {
        if(ui.bFrameRefComboBox->currentIndex()>0)
        { // max ref frames decreases further to 6 if B pyramid is enabled as well
            ui.refFramesSpinBox->setMaximum(6);
        }else
        {
            ui.refFramesSpinBox->setMaximum(7);
        }
    }
}

void x265Dialog::bFrameRefComboBox_currentIndexChanged(int index)
{
     if(ui.maxBFramesSpinBox->value()) // if B frames are enabled
     {
         if(index>0) // and B pyramid is enabled too, reduce max ref frames accordingly
         {
             ui.refFramesSpinBox->setMaximum(6);
         }else
         {
             ui.refFramesSpinBox->setMaximum(7);
         }
     }else
     {
         ui.refFramesSpinBox->setMaximum(8);
     }
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

void x265Dialog::rdoqSpinBox_valueChanged(int value)
{
	const bool enable = value >= 1;

	ui.lblPsyRDOQ->setEnabled(enable);
	ui.psychoRdoqSpinBox->setEnabled(enable);
}

void x265Dialog::cuTreeCheckBox_toggled(bool checked)
{
	if (checked && !ui.aqVarianceCheckBox->isChecked())
	{
                QString st=QT_TRANSLATE_NOOP("x265","Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.\n\nDo you wish to continue?");
		if (GUI_Question(st.toUtf8().constData()))
			ui.aqVarianceCheckBox->setChecked(true);
		else
			ui.cuTreeCheckBox->setChecked(false);
	}
}

void x265Dialog::aqVarianceCheckBox_toggled(bool checked)
{
	if (!checked && ui.cuTreeCheckBox->isChecked())
	{
                QString st=QT_TRANSLATE_NOOP("x265","Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Macroblock-Tree optimisation will automatically be disabled.\n\nDo you wish to continue?");
		if (GUI_Question(st.toUtf8().constData()))
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
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x265","Error"),QT_TRANSLATE_NOOP("x265","Cannot load preset"));
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
  dialog.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("x265","Save Profile")));
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
//  dialog.setModal(true);
//  dialog.setWindowModality(Qt::ApplicationModal);

  if(dialog.exec()!=QDialog::Accepted)
  {
        ADM_info("Cancelled");
        return NULL;
  }
  std::string st = text->text().toUtf8().constData();
  return ADM_strdup(st.c_str());
}
/**
        \fn saveAsButton_pressed
        \brief Save the current settings as preset
*/
void x265Dialog::saveAsButton_pressed(void)
{
  // 1-ask name
  char *out=getProfileName(this);
  if(!out) return;
  ADM_info("Using %s\n",out);
  download();
  std::string rootPath;
  ADM_pluginGetPath("x265",pluginVersion,rootPath);
  std::string name=out;
  ADM_dealloc(out);
  std::string fullpath=rootPath+std::string(ADM_SEPARATOR)+name+std::string(".json");

  if(ADM_fileExist(fullpath.c_str()))
  {
        if(false==GUI_Confirmation_HIG(QT_TRANSLATE_NOOP("x265","Overwrite"),QT_TRANSLATE_NOOP("x265","Replace the following preset ?:"),name.c_str()))
            return;
  }
  if(false==x265_settings_jserialize(fullpath.c_str(),&myCopy))
  {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x265","Error"),QT_TRANSLATE_NOOP("x265","Cannot save preset"));
        ADM_error("Cannot write to \"%s\"\n",fullpath.c_str());
  }else
  {
        updatePresetList(name.c_str());
  }
}
/**
    \fn deleteButton_pressed
    \brief Delete json file for the currently selected profile
*/
void x265Dialog::deleteButton_pressed(void)
{ 
    int n=ui.configurationComboBox->currentIndex();
    int m=ui.configurationComboBox->count();
    if(n==m-1) // custom
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("x265","Error"),QT_TRANSLATE_NOOP("x265","Cannot delete custom profile"));
        return;
    }
  QString preset=ui.configurationComboBox->itemText(n);
  QString msg=QString(QT_TRANSLATE_NOOP("x265","Do you really want to delete the "))+preset+
            QString(QT_TRANSLATE_NOOP("x265"," profile ?.\nIf it is a system profile it will be recreated next time."));
  if(true==GUI_Confirmation_HIG(QT_TRANSLATE_NOOP("x265","Delete"),QT_TRANSLATE_NOOP("x265","Delete preset"),msg.toUtf8().constData()))
  {
    std::string rootPath;
    ADM_pluginGetPath("x265",pluginVersion,rootPath);
    QString text=QString("/")+ui.configurationComboBox->itemText(n);
    text=QString(rootPath.c_str())+text+QString(".json");
    if(!ADM_eraseFile(text.toUtf8().constData()))
        ADM_warning("Could not delete %s\n",text.toUtf8().constData());
    else
        updatePresetList();
  }
}

