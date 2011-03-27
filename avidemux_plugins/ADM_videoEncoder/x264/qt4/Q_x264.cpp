/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2011 gruntster/mean
 ***************************************************************************/
#include <math.h>
#include <QtGui/QFileDialog>
#include "ADM_default.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "../x264_encoder.h"
#include "Q_x264.h"

static x264_encoder myCopy; // ugly...

/**
    \fn x264_ui
    \brief hook to enter UI specific dialog
*/
bool x264_ui(x264_encoder *settings)
{
    x264Dialog dialog(NULL,settings);
    if (dialog.exec() == QDialog::Accepted)
    {
            dialog.download();
            memcpy(settings,&myCopy,sizeof(myCopy));
            return true;
    }
    return false;
}
/**

*/  
x264Dialog::x264Dialog(QWidget *parent, void *param)
{
       ui.setupUi(this);
        connect(ui.encodingModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(encodingModeComboBox_currentIndexChanged(int)));
        connect(ui.quantiserSlider, SIGNAL(valueChanged(int)), this, SLOT(quantiserSlider_valueChanged(int)));
        connect(ui.quantiserSpinBox, SIGNAL(valueChanged(int)), this, SLOT(quantiserSpinBox_valueChanged(int)));
        connect(ui.targetRateControlSpinBox, SIGNAL(valueChanged(int)), this, SLOT(targetRateControlSpinBox_valueChanged(int)));
#if 0
        connect(ui.maxCrfSlider, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSlider_valueChanged(int)));
        connect(ui.maxCrfSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxCrfSpinBox_valueChanged(int)));
        connect(ui.mbTreeCheckBox, SIGNAL(toggled(bool)), this, SLOT(mbTreeCheckBox_toggled(bool)));
#endif
       memcpy(&myCopy,param,sizeof(myCopy));
#define ENCODING(x)  myCopy.general.params.x       
        lastBitrate =   ENCODING(bitrate);
        lastVideoSize = ENCODING(finalsize);

        ui.tabWidget->setCurrentIndex(0);
        upload();

        
        
}
/**

*/
#define MK_CHECKBOX(x,y) ui.x->setChecked(myCopy.y)
#define MK_UINT(x,y)  ui.x->setValue(myCopy.y)
#define DISABLE(x) ui.x->setEnabled(false);
#define MK_MENU(x,y) ui.x->setCurrentIndex(myCopy.y)
bool x264Dialog::upload(void)
{
          
          MK_CHECKBOX(fastFirstPassCheckBox,analyze.fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,analyze.weighted_bipred);
          MK_CHECKBOX(dct8x8CheckBox,analyze.b_8x8);
          MK_CHECKBOX(i4x4CheckBox,analyze.b_i4x4);
          MK_CHECKBOX(i8x8CheckBox,analyze.b_i8x8);
          MK_CHECKBOX(p4x4CheckBox,analyze.b_p8x8);
          MK_CHECKBOX(p8x8CheckBox,analyze.b_p16x16);
          MK_CHECKBOX(b8x8CheckBox,analyze.b_b16x16);

          MK_CHECKBOX(cabacCheckBox,cabac);
    
          MK_CHECKBOX(mixedRefsCheckBox,analyze.mixed_references);
          MK_CHECKBOX(chromaMotionEstCheckBox,analyze.chroma_me);
          MK_CHECKBOX(dctDecimateCheckBox,analyze.dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(maxBFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(meSpinBox,analyze.subpel_refine);

          MK_UINT(lookaheadSpinBox,ratecontrol.lookahead);
          MK_CHECKBOX(mbTreeCheckBox,ratecontrol.mb_tree);
          

          MK_MENU(meMethodComboBox,analyze.me_method);
          MK_MENU(weightedPPredictComboBox,analyze.weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);

          MK_MENU(predictModeComboBox,analyze.direct_mv_pred);

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
                            encodingModeComboBox_currentIndexChanged(1);
                            ui.quantiserSpinBox->setValue(ENCODING(qz));
                            break;

            default: ADM_assert(0);break;
        }


          DISABLE(loopFilterCheckBox);
          DISABLE(openGopCheckBox);
          DISABLE(interlacedCheckBox);
          DISABLE(intraRefreshCheckBox);
          DISABLE(noiseReductionSpinBox);
          DISABLE(mvRangeSpinBox);
          DISABLE(mvLengthSpinBox);
          DISABLE(minThreadBufferSpinBox);
          DISABLE(constrainedIntraCheckBox);
          DISABLE(IFrameThresholdSpinBox);
          DISABLE(intraLumaSpinBox);
          DISABLE(interLumaSpinBox);
          DISABLE(groupBox_14);
          DISABLE(tab_7);
          DISABLE(tab_6);
          DISABLE(tab_9);
          DISABLE(tab);
          DISABLE(maxCrfCheckBox);
          DISABLE(psychoRdoSpinBox);
          return true;
}
#undef MK_CHECKBOX
#undef MK_UINT
#undef MK_MENU
#define MK_CHECKBOX(x,y)    myCopy.y=ui.x->isChecked()
#define MK_UINT(x,y)        myCopy.y=ui.x->value()
#define MK_MENU(x,y)        myCopy.y=ui.x->currentIndex()
bool x264Dialog::download(void)
{
          MK_CHECKBOX(fastFirstPassCheckBox,analyze.fast_pskip);
          MK_CHECKBOX(weightedPredictCheckBox,analyze.weighted_bipred);
          MK_CHECKBOX(dct8x8CheckBox,analyze.b_8x8);
          MK_CHECKBOX(i4x4CheckBox,analyze.b_i4x4);
          MK_CHECKBOX(i8x8CheckBox,analyze.b_i8x8);
          MK_CHECKBOX(p4x4CheckBox,analyze.b_p8x8);
          MK_CHECKBOX(p8x8CheckBox,analyze.b_p16x16);
          MK_CHECKBOX(b8x8CheckBox,analyze.b_b16x16);

          MK_CHECKBOX(cabacCheckBox,cabac);
    
          MK_CHECKBOX(mixedRefsCheckBox,analyze.mixed_references);
          MK_CHECKBOX(chromaMotionEstCheckBox,analyze.chroma_me);
          MK_CHECKBOX(dctDecimateCheckBox,analyze.dct_decimate);

          MK_UINT(maxBFramesSpinBox,MaxBFrame);
          MK_UINT(maxBFramesSpinBox,MaxRefFrames);
          MK_UINT(minGopSizeSpinBox,MinIdr);
          MK_UINT(maxGopSizeSpinBox,MaxIdr);
          MK_UINT(meSpinBox,analyze.subpel_refine);
          MK_UINT(BFrameBiasSpinBox,i_bframe_bias);

          MK_MENU(meMethodComboBox,analyze.me_method);
          MK_MENU(weightedPPredictComboBox,analyze.weighted_pred);
          MK_MENU(bFrameRefComboBox,i_bframe_pyramid);

          MK_UINT(lookaheadSpinBox,ratecontrol.lookahead);
          MK_CHECKBOX(mbTreeCheckBox,ratecontrol.mb_tree);

          MK_MENU(predictModeComboBox,analyze.direct_mv_pred);

          switch(ui.encodingModeComboBox->currentIndex())
          {
            case 0: ENCODING(mode)=COMPRESS_CBR; ENCODING(bitrate)=ui.targetRateControlSpinBox->value();break;
            case 1: ENCODING(mode)=COMPRESS_CQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 2: ENCODING(mode)=COMPRESS_AQ;ENCODING(qz)=ui.quantiserSpinBox->value();break;
            case 3: ADM_assert(0);
            case 4: ADM_assert(0);
          }
          return true;
}

// General tab
void x264Dialog::encodingModeComboBox_currentIndexChanged(int index)
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

	if (!enableMaxCrf)
		ui.maxCrfCheckBox->setChecked(false);

	ui.maxCrfCheckBox->setEnabled(enableMaxCrf);
}

void x264Dialog::quantiserSlider_valueChanged(int value)
{
	ui.quantiserSpinBox->setValue(value);
}

void x264Dialog::quantiserSpinBox_valueChanged(int value)
{
	ui.quantiserSlider->setValue(value);
}

void x264Dialog::targetRateControlSpinBox_valueChanged(int value)
{
	if (ui.encodingModeComboBox->currentIndex() == 3)	// Video Size - 2 pass
		lastVideoSize = value;
	else
		lastBitrate = value;
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

void x264Dialog::mbTreeCheckBox_toggled(bool checked)
{
	if (!disableGenericSlots && checked && !ui.aqVarianceCheckBox->isChecked())
	{
		if (GUI_Question(tr("Macroblock-Tree optimisation requires Variance Adaptive Quantisation to be enabled.  Variance Adaptive Quantisation will automatically be enabled.\n\nDo you wish to continue?").toUtf8().constData()))
			ui.aqVarianceCheckBox->setChecked(true);
		else
			ui.mbTreeCheckBox->setChecked(false);
	}
}
#endif


