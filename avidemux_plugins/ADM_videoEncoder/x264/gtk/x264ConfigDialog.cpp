/***************************************************************************
                    x264ConfigDialog.cpp  -  description
                    ------------------------------------

                          GUI for configuring x264

    begin                : Tue Apr 29 2008
    copyright            : (C) 2008 by mean/gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

#ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(x) gettext(x)
#else
#define _(x) x
#endif

#include "ADM_inttype.h"
#include "x264ConfigDialog.h"
#include "gtkSupport.h"

extern "C"
{
#include "x264.h"
}

#define MKAR(x, y, z) {x , y, #x":"#y" ("#z")"}
#define NB_X264_AR sizeof(x264_ar) / sizeof(X264_AR)

#define WID(x) lookup_widget(dialog, #x)

typedef struct X264_AR
{
	uint32_t num, den;
	const char *symbol;
};

const X264_AR x264_ar[] =
{
	MKAR(16,15,1.07),
	MKAR(64,45,1.42),
	MKAR(8,9,0.89),
	MKAR(32,27,1.19)
};

typedef enum 
{
    ACTION_LOAD_DEFAULT = 10,
    ACTION_LOAD_INVALID
} x264_actions;

static int _lastBitrate, _lastVideoSize;

extern "C" int showX264ConfigDialog(vidEncConfigParameters *configParameters, vidEncVideoProperties *properties, vidEncOptions *encodeOptions, x264Options *options)
{
	_lastBitrate = 1500;
	_lastVideoSize = 700;

	GtkWidget *dialog = create_dialog1();

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);

	gtk_window_set_modal(GTK_WINDOW(dialog), 1);
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(configParameters->parent));
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog), WID(buttonResetDaults), ACTION_LOAD_DEFAULT);

#if X264_BUILD >= 58
	gtk_combo_box_append_text(GTK_COMBO_BOX(WID(comboboxMethod)), _("Hadamard Exhaustive Search"));
#endif

#if X264_BUILD >= 65
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("1 - QPel SAD (Fastest)"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("2 - QPel SATD"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("3 - HPel on MB then QPel"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("4 - Always QPel"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("5 - QPel + Bidirectional ME"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("6 - RD on I/P frames (Default)"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("7 - RD on all frames"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("8 - RD refinement on I/P frames"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("9 - RD refinement on all frames"));

	gtk_widget_hide(WID(checkbuttonBidirME));
#else
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("1  - Extremely Low (Fastest)"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("2  - Very Low"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("3  - Low"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("4  - Medium"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("5  - High (Default)"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("6  - Very High"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("6B - Very High (RDO on B-frames)"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("7  - Ultra High"));
	gtk_combo_box_append_text (GTK_COMBO_BOX(WID(comboboxPartitionDecision)), _("7B - Ultra High (RDO on B-frames)"));
#endif

	loadOptions(dialog, options);
	updateMode(dialog, encodeOptions->encodeMode, encodeOptions->encodeModeParameter);
	updateDeblockingFilter(dialog);

	// Fill in A/R
	for(int i = 0; i < NB_X264_AR; i++)
		gtk_combo_box_append_text(GTK_COMBO_BOX(WID(comboboxentry1)),x264_ar[i].symbol); 

	gtk_combo_box_set_active(GTK_COMBO_BOX(WID(comboboxentry1)),0);
	gtk_signal_connect(GTK_OBJECT(WID(comboboxMode)), "changed", GTK_SIGNAL_FUNC(signalReceiver), dialog);
	gtk_signal_connect(GTK_OBJECT(WID(entryTarget)), "changed", GTK_SIGNAL_FUNC(entryTarget_changed), dialog);

	char text[40];

	sprintf(text, "%u:%u", properties->parWidth, properties->parHeight);
	gtk_label_set_text(GTK_LABEL(WID(label40)), text);

	// Connect signal for deblocking filter
	gtk_signal_connect(GTK_OBJECT(WID(checkbuttonDeblockingFilter)), "toggled", GTK_SIGNAL_FUNC(signalReceiver), dialog);

	int reply = 0;

	while (1)
	{
		reply = gtk_dialog_run(GTK_DIALOG(dialog));

		if (reply == ACTION_LOAD_DEFAULT)
		{
			x264Options defaultOptions;

			printf("Resetting to default\n");
			loadOptions(dialog, &defaultOptions);
			updateMode(dialog, DEFAULT_ENCODE_MODE, DEFAULT_ENCODE_MODE_PARAMETER);
		}
		else
			break;
	}

	if (reply == GTK_RESPONSE_OK)
	{
		if (getCurrentEncodeMode(dialog) == ADM_VIDENC_MODE_CQP)
			encodeOptions->encodeModeParameter = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonQuantizer)));
		else
		{
			char *str = gtk_editable_get_chars(GTK_EDITABLE(WID(entryTarget)), 0, -1);
			encodeOptions->encodeModeParameter = atoi(str);
		}

		saveOptions(dialog, options);
	}

	gtk_widget_destroy(dialog);

	return reply == GTK_RESPONSE_OK;  
}

int getCurrentEncodeMode(GtkWidget *dialog)
{
	int modeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxMode)));
	int encodeMode;

	switch (modeIndex)
	{
		case 0:
			encodeMode = ADM_VIDENC_MODE_AQP;
			break;
		case 1:
			encodeMode = ADM_VIDENC_MODE_CQP;
			break;
		case 2:
			encodeMode = ADM_VIDENC_MODE_CBR;
			break;
		case 3:
			encodeMode = ADM_VIDENC_MODE_2PASS_SIZE;
			break;
		case 4:
			encodeMode = ADM_VIDENC_MODE_2PASS_ABR;
			break;
	}

	return encodeMode;
}

void updateMode(GtkWidget *dialog, int encodeMode, int encodeModeParameter)
{
	int modeIndex = 0;
	bool quantiser = false;

	switch (encodeMode)
	{
		case ADM_VIDENC_MODE_AQP:
			modeIndex = 0;
			quantiser = true;
			break;
		case ADM_VIDENC_MODE_CQP:
			modeIndex = 1;
			quantiser = true;
			break;
		case ADM_VIDENC_MODE_CBR:
			modeIndex = 2;
			gtk_label_set_text(GTK_LABEL(WID(labelTarget)),_("Target bitrate (kb/s)"));
			break;
		case ADM_VIDENC_MODE_2PASS_SIZE:
			modeIndex = 3;
			gtk_label_set_text(GTK_LABEL(WID(labelTarget)),_("Target video size (MB)"));
			break;
		case ADM_VIDENC_MODE_2PASS_ABR:
			modeIndex = 4;
			gtk_label_set_text(GTK_LABEL(WID(labelTarget)),_("Average bitrate (kb/s)"));
			break;
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(WID(comboboxMode)), modeIndex);

	if (quantiser)
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonQuantizer)), encodeModeParameter);
	else
	{
		char string[21];

		snprintf(string, 20, "%d", encodeModeParameter);
		gtk_entry_set_text(GTK_ENTRY(WID(entryTarget)), string);
	}

	gtk_widget_set_sensitive(WID(spinbuttonQuantizer), quantiser);
	gtk_widget_set_sensitive(WID(entryTarget), !quantiser);
}

void updateDeblockingFilter(GtkWidget *dialog)
{
	int toggled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonDeblockingFilter)));

	gtk_widget_set_sensitive(WID(table8), toggled);
}

int signalReceiver(GtkObject* object, gpointer user_data)
{
	GtkWidget *dialog = (GtkWidget*)user_data;

	if (object == GTK_OBJECT(WID(comboboxMode)))
	{
		int modeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxMode)));
		int encodeModeParameter;

		switch (modeIndex)
		{
			case 0:
				encodeModeParameter = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonQuantizer)));
				break;
			case 1:
				encodeModeParameter = (int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonQuantizer)));
				break;
			case 2:
				encodeModeParameter = _lastBitrate;
				break;
			case 3:
				encodeModeParameter = _lastVideoSize;
				break;
			case 4:
				encodeModeParameter = _lastBitrate;
				break;
		}

		updateMode(dialog, getCurrentEncodeMode(dialog), encodeModeParameter);
	}
	else if (object == GTK_OBJECT(WID(checkbuttonDeblockingFilter)))
		updateDeblockingFilter(dialog);
}

int entryTarget_changed(GtkObject* object, gpointer user_data)
{
	GtkWidget *dialog = (GtkWidget*)user_data;

	if (gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxMode))) == 3)
		_lastVideoSize = atoi(gtk_entry_get_text(GTK_ENTRY(WID(entryTarget))));
	else
		_lastBitrate = atoi(gtk_entry_get_text(GTK_ENTRY(WID(entryTarget))));
}

void loadOptions(GtkWidget *dialog, x264Options *options)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonfastPSkip)), options->getFastPSkip());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonDCTDecimate)), options->getDctDecimate());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttoninterlaced)), options->getInterlaced());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonBasReference)), options->getBFrameReferences());
#if X264_BUILD < 65
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonBidirME)), options->getBidirectionalMotionEstimation());
#endif
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonAdaptative)), options->getAdaptiveBFrameDecision());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonWeighted)), options->getWeightedPrediction());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonMixedRefs)), options->getMixedReferences());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonCABAC)), options->getCabac());

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonTrellis)), options->getTrellis());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonNoiseReduction)), options->getNoiseReduction());

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonDeblockingFilter)), options->getLoopFilter());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbuttonChromaME)), options->getChromaMotionEstimation());

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonBitrateVariability)), (int)floor(options->getQuantiserCurveCompression() * 100 + .5));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMinQp)), options->getQuantiserMinimum());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxQp)), options->getQuantiserMaximum());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonQpStep)), options->getQuantiserStep());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonSceneCut)), options->getScenecutThreshold());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMinIdr)), options->getGopMinimumSize());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxIdr)), options->getGopMaximumSize());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_max_bitrate)), options->getVbvMaximumBitrate());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_buffer_size)), options->getVbvBufferSize());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_buffer_init)), (int)floor(options->getVbvInitialOccupancy() * 100 + .5));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxRefFrames)), options->getReferenceFrames());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxBFrame)), options->getBFrames());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonBias)), options->getBFrameBias());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonStrength)), options->getLoopFilterAlphaC0());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonThreshold)), options->getLoopFilterBeta());

	char string[11];

	snprintf(string, 10, "%d", options->getSarWidth());
	gtk_entry_set_text(GTK_ENTRY(WID(entryAR_Num)), string);

	snprintf(string, 10, "%d", options->getSarHeight());
	gtk_entry_set_text(GTK_ENTRY(WID(entryAR_Den)), string);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(WID(spinbuttonRange)), options->getMotionVectorSearchRange());

	gtk_combo_box_set_active(GTK_COMBO_BOX(WID(comboboxMethod)), options->getMotionEstimationMethod());
	gtk_combo_box_set_active(GTK_COMBO_BOX(WID(comboboxDirectMode)), options->getDirectPredictionMode());

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8)), options->getDct8x8());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8P)), options->getPartitionP8x8());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8B)), options->getPartitionB8x8());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton4x4)), options->getPartitionP4x4());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8I)), options->getPartitionI8x8());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(checkbutton4x4I)), options->getPartitionI4x4());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WID(radiobuttonAsInputAR)), options->getSarAsInput());

#if X264_BUILD < 65
	int decisionItem;

	if (options->getSubpixelRefinement() < 6)
		decisionItem = options->getSubpixelRefinement() - 1;
	else
		if (options->getSubpixelRefinement() == 6)
			if (options->getBFrameRdo())
				decisionItem = 6;
			else
				decisionItem = 5;
		else
			if (options->getBFrameRdo())
				decisionItem = 8;
			else
				decisionItem = 7;
#else
	int decisionItem = options->getSubpixelRefinement() - 1;
#endif

	gtk_combo_box_set_active(GTK_COMBO_BOX(WID(comboboxPartitionDecision)), decisionItem);
}

void saveOptions(GtkWidget *dialog, x264Options *options)
{
	options->setFastPSkip(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonfastPSkip))));
	options->setDctDecimate(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonDCTDecimate))));
	options->setInterlaced(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttoninterlaced))));
	options->setBFrameReferences(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonBasReference))));
#if X264_BUILD < 65
	options->setBidirectionalMotionEstimation(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonBidirME))));
#endif
	options->setAdaptiveBFrameDecision(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonAdaptative))));
	options->setWeightedPrediction(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonWeighted))));
	options->setMixedReferences(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonMixedRefs))));
	options->setCabac(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonCABAC))));

	options->setTrellis((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonTrellis))));
	options->setNoiseReduction((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonNoiseReduction))));

	options->setLoopFilter(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonDeblockingFilter))));
	options->setChromaMotionEstimation(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbuttonChromaME))));

	options->setQuantiserCurveCompression((float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonBitrateVariability))) / 100);
	options->setQuantiserMinimum((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMinQp))));
	options->setQuantiserMaximum((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxQp))));
	options->setQuantiserStep((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonQpStep))));
	options->setScenecutThreshold((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonSceneCut))));
	options->setGopMinimumSize((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMinIdr))));
	options->setGopMaximumSize((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxIdr))));
	options->setVbvMaximumBitrate((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_max_bitrate))));
	options->setVbvBufferSize((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_buffer_size))));
	options->setVbvInitialOccupancy((float)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonvbv_buffer_init))) / 100);
	options->setReferenceFrames((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxRefFrames))));
	options->setBFrames((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonMaxBFrame))));
	options->setBFrameBias((int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonBias))));
	options->setLoopFilterAlphaC0((int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonStrength))));
	options->setLoopFilterBeta((int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonThreshold))));
	options->setMotionVectorSearchRange((unsigned int)gtk_spin_button_get_value(GTK_SPIN_BUTTON(WID(spinbuttonRange))));

	options->setMotionEstimationMethod(gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxMethod))));
	options->setDirectPredictionMode(gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxDirectMode))));

	options->setDct8x8(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8))));
	options->setPartitionP8x8(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8P))));
	options->setPartitionB8x8(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8B))));
	options->setPartitionP4x4(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton4x4))));
	options->setPartitionI8x8(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton8x8I))));
	options->setPartitionI4x4(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(checkbutton4x4I))));

	/* Extra case for aspect ratio */
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(radiobuttonPredefinedAR))))
	{
		int rank = gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxentry1)));

		options->setSarWidth(x264_ar[rank].num);
		options->setSarHeight(x264_ar[rank].den);
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(radiobuttonCustomAR))))
	{
		options->setSarWidth(atoi(gtk_entry_get_text(GTK_ENTRY(WID(entryAR_Num)))));
		options->setSarHeight(atoi(gtk_entry_get_text(GTK_ENTRY(WID(entryAR_Den)))));
	}
	else
	{
		options->setSarWidth(1);
		options->setSarHeight(1);
	}

	options->setSarAsInput(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(WID(radiobuttonAsInputAR))));

	uint32_t decisionItem = gtk_combo_box_get_active(GTK_COMBO_BOX(WID(comboboxPartitionDecision)));

#if X264_BUILD < 65
	if (decisionItem < 6)
	{
		options->setSubpixelRefinement(decisionItem + 1);
		options->setBFrameRdo(false);
	}
	else if (decisionItem == 6)
	{
		options->setSubpixelRefinement(6);
		options->setBFrameRdo(true);
	}
	else
	{
		options->setSubpixelRefinement(7);
		options->setBFrameRdo(decisionItem == 8);
	}
#else
	options->setSubpixelRefinement(decisionItem + 1);
#endif
}

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *notebook1;
  GtkWidget *alignment14;
  GtkWidget *tableBitrate;
  GtkWidget *labelEncodingMode;
  GtkWidget *labelQuantizer;
  GtkWidget *labelTarget;
  GtkObject *spinbuttonQuantizer_adj;
  GtkWidget *spinbuttonQuantizer;
  GtkWidget *entryTarget;
  GtkWidget *comboboxMode;
  GtkWidget *labelPageBitrate;
  GtkWidget *alignment15;
  GtkWidget *vbox5;
  GtkWidget *frame6;
  GtkWidget *alignment6;
  GtkWidget *vbox6;
  GtkWidget *table7;
  GtkWidget *labelPartitionDecision;
  GtkWidget *labelMethod;
  GtkWidget *comboboxPartitionDecision;
  GtkWidget *comboboxMethod;
  GtkWidget *table11;
  GtkWidget *labelMaxRefFrames;
  GtkWidget *labelRange;
  GtkObject *spinbuttonRange_adj;
  GtkWidget *spinbuttonRange;
  GtkObject *spinbuttonMaxRefFrames_adj;
  GtkWidget *spinbuttonMaxRefFrames;
  GtkWidget *checkbuttonChromaME;
  GtkWidget *checkbuttonMixedRefs;
  GtkWidget *hbox8;
  GtkWidget *checkbuttonfastPSkip;
  GtkWidget *checkbuttonDCTDecimate;
  GtkWidget *checkbuttoninterlaced;
  GtkWidget *label24;
  GtkWidget *frameSampleAR;
  GtkWidget *alignment12;
  GtkWidget *table35;
  GtkWidget *hbox12;
  GtkWidget *entryAR_Num;
  GtkWidget *label53;
  GtkWidget *entryAR_Den;
  GtkWidget *radiobuttonCustomAR;
  GSList *radiobuttonCustomAR_group = NULL;
  GtkWidget *radiobuttonPredefinedAR;
  GtkWidget *radiobuttonAsInputAR;
  GtkWidget *label40;
  GtkWidget *comboboxentry1;
  GtkWidget *label39;
  GtkWidget *frame7;
  GtkWidget *alignment7;
  GtkWidget *vbox7;
  GtkWidget *alignment21;
  GtkWidget *hbox2;
  GtkWidget *checkbuttonCABAC;
  GtkWidget *hbox7;
  GtkWidget *labelTrellis;
  GtkObject *spinbuttonTrellis_adj;
  GtkWidget *spinbuttonTrellis;
  GtkWidget *hbox11;
  GtkWidget *labelNoiseReduction;
  GtkObject *spinbuttonNoiseReduction_adj;
  GtkWidget *spinbuttonNoiseReduction;
  GtkWidget *hbox3;
  GtkWidget *checkbuttonDeblockingFilter;
  GtkWidget *table8;
  GtkWidget *labelThreshold;
  GtkWidget *labelStrength;
  GtkObject *spinbuttonThreshold_adj;
  GtkWidget *spinbuttonThreshold;
  GtkObject *spinbuttonStrength_adj;
  GtkWidget *spinbuttonStrength;
  GtkWidget *label25;
  GtkWidget *labelPageMotion_More;
  GtkWidget *alignment16;
  GtkWidget *vbox2;
  GtkWidget *frame4;
  GtkWidget *alignment4;
  GtkWidget *vbox3;
  GtkWidget *checkbutton8x8;
  GtkWidget *checkbutton8x8P;
  GtkWidget *checkbutton8x8B;
  GtkWidget *checkbutton4x4;
  GtkWidget *checkbutton8x8I;
  GtkWidget *checkbutton4x4I;
  GtkWidget *label19;
  GtkWidget *frame5;
  GtkWidget *alignment5;
  GtkWidget *vbox4;
  GtkWidget *table5;
  GtkWidget *labelBias;
  GtkWidget *labelMaxConsecutive;
  GtkObject *spinbuttonBias_adj;
  GtkWidget *spinbuttonBias;
  GtkObject *spinbuttonMaxBFrame_adj;
  GtkWidget *spinbuttonMaxBFrame;
  GtkWidget *alignment20;
  GtkWidget *table6;
  GtkWidget *checkbuttonBidirME;
  GtkWidget *checkbuttonWeighted;
  GtkWidget *checkbuttonAdaptative;
  GtkWidget *checkbuttonBasReference;
  GtkWidget *hbox1;
  GtkWidget *labelBFrameDirectMode;
  GtkWidget *comboboxDirectMode;
  GtkWidget *label20;
  GtkWidget *labelPagePartion_Frame;
  GtkWidget *alignment17;
  GtkWidget *vbox1;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *table2;
  GtkObject *spinbuttonKeyframeBoost_adj;
  GtkWidget *spinbuttonKeyframeBoost;
  GtkObject *spinbuttonBframeReduction_adj;
  GtkWidget *spinbuttonBframeReduction;
  GtkObject *spinbuttonBitrateVariability_adj;
  GtkWidget *spinbuttonBitrateVariability;
  GtkWidget *labelKeyframeboost;
  GtkWidget *label11;
  GtkWidget *label12;
  GtkWidget *label7;
  GtkWidget *frame2;
  GtkWidget *alignment2;
  GtkWidget *table3;
  GtkWidget *label13;
  GtkWidget *label14;
  GtkWidget *label15;
  GtkObject *spinbuttonMinQp_adj;
  GtkWidget *spinbuttonMinQp;
  GtkObject *spinbuttonMaxQp_adj;
  GtkWidget *spinbuttonMaxQp;
  GtkObject *spinbuttonQpStep_adj;
  GtkWidget *spinbuttonQpStep;
  GtkWidget *label8;
  GtkWidget *frame3;
  GtkWidget *alignment3;
  GtkWidget *table4;
  GtkObject *spinbuttonSceneCut_adj;
  GtkWidget *spinbuttonSceneCut;
  GtkWidget *label16;
  GtkWidget *label17;
  GtkObject *spinbuttonMinIdr_adj;
  GtkWidget *spinbuttonMinIdr;
  GtkObject *spinbuttonMaxIdr_adj;
  GtkWidget *spinbuttonMaxIdr;
  GtkWidget *label18;
  GtkWidget *labelMoreEncodingSettings;
  GtkWidget *frameVideoBufferVerifier;
  GtkWidget *alignment11;
  GtkWidget *table12;
  GtkWidget *labelVBVBufferSize;
  GtkWidget *labelVBVInitialbuffer;
  GtkWidget *labelVBVMaximumBitrate;
  GtkObject *spinbuttonvbv_max_bitrate_adj;
  GtkWidget *spinbuttonvbv_max_bitrate;
  GtkObject *spinbuttonvbv_buffer_size_adj;
  GtkWidget *spinbuttonvbv_buffer_size;
  GtkObject *spinbuttonvbv_buffer_init_adj;
  GtkWidget *spinbuttonvbv_buffer_init;
  GtkWidget *labelVideoBufferVerifier;
  GtkWidget *labelPageRateControl;
  GtkWidget *alignment18;
  GtkWidget *vbox8;
  GtkWidget *frame10;
  GtkWidget *alignment10;
  GtkWidget *table15;
  GtkWidget *labelBitrateVariance;
  GtkObject *spinbuttonBitrateVariance_adj;
  GtkWidget *spinbuttonBitrateVariance;
  GtkObject *spinbuttonQuantizerCompression_adj;
  GtkWidget *spinbuttonQuantizerCompression;
  GtkObject *spinbuttonTempBlurFrame_adj;
  GtkWidget *spinbuttonTempBlurFrame;
  GtkObject *spinbuttonTempBlurQuant_adj;
  GtkWidget *spinbuttonTempBlurQuant;
  GtkWidget *labelQuantizerCompression;
  GtkWidget *labelTempBlurEstFrame;
  GtkWidget *labelTempBlueQuant;
  GtkWidget *labelMisc;
  GtkWidget *frame11;
  GtkWidget *alignment13;
  GtkWidget *table16;
  GtkObject *spinbutton8_adj;
  GtkWidget *spinbutton8;
  GtkObject *spinbutton9_adj;
  GtkWidget *spinbutton9;
  GtkObject *spinbuttonChromaQPOffset_adj;
  GtkWidget *spinbuttonChromaQPOffset;
  GtkWidget *labelFactorbetweenIandP;
  GtkWidget *labelFactorbetweenPandB;
  GtkWidget *labelChromaQPOffset;
  GtkWidget *labelQuantizers;
  GtkWidget *frame9;
  GtkWidget *alignment9;
  GtkWidget *vbox9;
  GtkWidget *radiobuttonFlatmatrix;
  GSList *radiobuttonFlatmatrix_group = NULL;
  GtkWidget *radiobuttonJVTmatrix;
  GtkWidget *hbox13;
  GtkWidget *radiobuttonCustommatrix;
  GtkWidget *hbox14;
  GtkWidget *filechooserbuttonOpenCQMFile;
  GtkWidget *vseparator1;
  GtkWidget *buttonEditCustomMatrix;
  GtkWidget *labelFrameQuantizationMatrices;
  GtkWidget *labelMore;
  GtkWidget *dialog_action_area1;
  GtkWidget *buttonResetDaults;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), _("x264 Configuration"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), notebook1, TRUE, TRUE, 0);

  alignment14 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment14);
  gtk_container_add (GTK_CONTAINER (notebook1), alignment14);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment14), 7, 7, 7, 7);

  tableBitrate = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (tableBitrate);
  gtk_container_add (GTK_CONTAINER (alignment14), tableBitrate);
  gtk_table_set_row_spacings (GTK_TABLE (tableBitrate), 3);
  gtk_table_set_col_spacings (GTK_TABLE (tableBitrate), 10);

  labelEncodingMode = gtk_label_new (_("Encoding Mode"));
  gtk_widget_show (labelEncodingMode);
  gtk_table_attach (GTK_TABLE (tableBitrate), labelEncodingMode, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelEncodingMode), 0, 0.5);

  labelQuantizer = gtk_label_new (_("Quantizer"));
  gtk_widget_show (labelQuantizer);
  gtk_table_attach (GTK_TABLE (tableBitrate), labelQuantizer, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelQuantizer), 0, 0.5);

  labelTarget = gtk_label_new (_("Target Size"));
  gtk_widget_show (labelTarget);
  gtk_table_attach (GTK_TABLE (tableBitrate), labelTarget, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelTarget), 0, 0.5);

  spinbuttonQuantizer_adj = gtk_adjustment_new (4, 0, 51, 1, 10, 0);
  spinbuttonQuantizer = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonQuantizer_adj), 1, 0);
  gtk_widget_show (spinbuttonQuantizer);
  gtk_table_attach (GTK_TABLE (tableBitrate), spinbuttonQuantizer, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonQuantizer, _("Constant Quantizer - Each frame will have the same compression. Low numbers equal higher quality while high numbers equal lower quality."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonQuantizer), TRUE);

  entryTarget = gtk_entry_new ();
  gtk_widget_show (entryTarget);
  gtk_table_attach (GTK_TABLE (tableBitrate), entryTarget, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, entryTarget, _("Target bitrate or file size"), NULL);
  gtk_entry_set_width_chars (GTK_ENTRY (entryTarget), 4);

  comboboxMode = gtk_combo_box_new_text ();
  gtk_widget_show (comboboxMode);
  gtk_table_attach (GTK_TABLE (tableBitrate), comboboxMode, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMode), _("Single Pass - Quality Quantizer (Average)"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMode), _("Single Pass - Quality Quantizer (Constant)"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMode), _("Single Pass - Bitrate (Average)"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMode), _("Two Pass - Video Size"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMode), _("Two Pass - Average Bitrate"));

  labelPageBitrate = gtk_label_new (_("Bitrate"));
  gtk_widget_show (labelPageBitrate);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), labelPageBitrate);
  gtk_misc_set_alignment (GTK_MISC (labelPageBitrate), 0, 0.5);

  alignment15 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment15);
  gtk_container_add (GTK_CONTAINER (notebook1), alignment15);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment15), 7, 7, 7, 7);

  vbox5 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox5);
  gtk_container_add (GTK_CONTAINER (alignment15), vbox5);

  frame6 = gtk_frame_new (NULL);
  gtk_widget_show (frame6);
  gtk_box_pack_start (GTK_BOX (vbox5), frame6, FALSE, TRUE, 0);

  alignment6 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment6);
  gtk_container_add (GTK_CONTAINER (frame6), alignment6);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment6), 7, 4, 7, 7);

  vbox6 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox6);
  gtk_container_add (GTK_CONTAINER (alignment6), vbox6);

  table7 = gtk_table_new (2, 3, FALSE);
  gtk_widget_show (table7);
  gtk_box_pack_start (GTK_BOX (vbox6), table7, FALSE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table7), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table7), 10);

  labelPartitionDecision = gtk_label_new (_("Partition Decision"));
  gtk_widget_show (labelPartitionDecision);
  gtk_table_attach (GTK_TABLE (table7), labelPartitionDecision, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelPartitionDecision), 0, 0.5);

  labelMethod = gtk_label_new (_("Method"));
  gtk_widget_show (labelMethod);
  gtk_table_attach (GTK_TABLE (table7), labelMethod, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelMethod), 0, 0.5);

  comboboxPartitionDecision = gtk_combo_box_new_text ();
  gtk_widget_show (comboboxPartitionDecision);
  gtk_table_attach (GTK_TABLE (table7), comboboxPartitionDecision, 1, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  comboboxMethod = gtk_combo_box_new_text ();
  gtk_widget_show (comboboxMethod);
  gtk_table_attach (GTK_TABLE (table7), comboboxMethod, 1, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMethod), _("Diamond Search"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMethod), _("Hexagonal Search"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMethod), _("Uneven Multi-hexagon"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxMethod), _("Exhaustive Search"));

  table11 = gtk_table_new (2, 3, FALSE);
  gtk_widget_show (table11);
  gtk_box_pack_start (GTK_BOX (vbox6), table11, TRUE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table11), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table11), 10);

  labelMaxRefFrames = gtk_label_new (_("Max. Ref. Frames"));
  gtk_widget_show (labelMaxRefFrames);
  gtk_table_attach (GTK_TABLE (table11), labelMaxRefFrames, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelMaxRefFrames), 0, 0.5);

  labelRange = gtk_label_new (_("Range"));
  gtk_widget_show (labelRange);
  gtk_table_attach (GTK_TABLE (table11), labelRange, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelRange), 0, 0.5);

  spinbuttonRange_adj = gtk_adjustment_new (17, 0, 64, 1, 10, 0);
  spinbuttonRange = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonRange_adj), 1, 0);
  gtk_widget_show (spinbuttonRange);
  gtk_table_attach (GTK_TABLE (table11), spinbuttonRange, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonRange, _("Define how many pixels are analysed for motion estimation. The higher the range the more accurate the analysis but the slower the encoding time."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonRange), TRUE);

  spinbuttonMaxRefFrames_adj = gtk_adjustment_new (1, 1, 16, 1, 10, 0);
  spinbuttonMaxRefFrames = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMaxRefFrames_adj), 1, 0);
  gtk_widget_show (spinbuttonMaxRefFrames);
  gtk_table_attach (GTK_TABLE (table11), spinbuttonMaxRefFrames, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonMaxRefFrames, _("Set how many previous frames can be referenced by a P/B-frame. Numbers above 5 do not seem to improve quality greatly. Numbers 3 to 5 are recommended."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMaxRefFrames), TRUE);

  checkbuttonChromaME = gtk_check_button_new_with_mnemonic (_("Chroma ME"));
  gtk_widget_show (checkbuttonChromaME);
  gtk_table_attach (GTK_TABLE (table11), checkbuttonChromaME, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 30, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonChromaME, _("Use color information for detecting motion to improve quality"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonChromaME), TRUE);

  checkbuttonMixedRefs = gtk_check_button_new_with_mnemonic (_("Mixed Refs"));
  gtk_widget_show (checkbuttonMixedRefs);
  gtk_table_attach (GTK_TABLE (table11), checkbuttonMixedRefs, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 30, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonMixedRefs, _("Calculate referencing individually based on each partition"), NULL);

  hbox8 = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox6), hbox8, TRUE, TRUE, 0);

  checkbuttonfastPSkip = gtk_check_button_new_with_mnemonic (_("Fast P-Skip"));
  gtk_widget_show (checkbuttonfastPSkip);
  gtk_box_pack_start (GTK_BOX (hbox8), checkbuttonfastPSkip, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonfastPSkip, _("Early skip detection on P-frames to speedup encoding. Quality will be slightly reduced."), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonfastPSkip), TRUE);

  checkbuttonDCTDecimate = gtk_check_button_new_with_mnemonic (_("DCT Decimate"));
  gtk_widget_show (checkbuttonDCTDecimate);
  gtk_box_pack_start (GTK_BOX (hbox8), checkbuttonDCTDecimate, FALSE, FALSE, 0);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonDCTDecimate), TRUE);

  checkbuttoninterlaced = gtk_check_button_new_with_mnemonic (_("Interlaced"));
  gtk_widget_show (checkbuttoninterlaced);
  gtk_box_pack_start (GTK_BOX (hbox8), checkbuttoninterlaced, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttoninterlaced, _("Input video is interlaced"), NULL);

  label24 = gtk_label_new (_("<b>Motion Estimation</b>"));
  gtk_widget_show (label24);
  gtk_frame_set_label_widget (GTK_FRAME (frame6), label24);
  gtk_label_set_use_markup (GTK_LABEL (label24), TRUE);

  frameSampleAR = gtk_frame_new (NULL);
  gtk_widget_show (frameSampleAR);
  gtk_box_pack_start (GTK_BOX (vbox5), frameSampleAR, FALSE, TRUE, 0);

  alignment12 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment12);
  gtk_container_add (GTK_CONTAINER (frameSampleAR), alignment12);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment12), 7, 7, 7, 7);

  table35 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table35);
  gtk_container_add (GTK_CONTAINER (alignment12), table35);
  gtk_table_set_row_spacings (GTK_TABLE (table35), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table35), 10);

  hbox12 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox12);
  gtk_table_attach (GTK_TABLE (table35), hbox12, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  entryAR_Num = gtk_entry_new ();
  gtk_widget_show (entryAR_Num);
  gtk_box_pack_start (GTK_BOX (hbox12), entryAR_Num, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, entryAR_Num, _("Enforce the size of a decoded pixel decoded to a certain value. Set this to 1:1 for non-anamorphic video."), NULL);
  gtk_entry_set_text (GTK_ENTRY (entryAR_Num), _("1"));
  gtk_entry_set_width_chars (GTK_ENTRY (entryAR_Num), 4);

  label53 = gtk_label_new (_(":"));
  gtk_widget_show (label53);
  gtk_box_pack_start (GTK_BOX (hbox12), label53, FALSE, FALSE, 3);

  entryAR_Den = gtk_entry_new ();
  gtk_widget_show (entryAR_Den);
  gtk_box_pack_start (GTK_BOX (hbox12), entryAR_Den, TRUE, TRUE, 0);
  gtk_tooltips_set_tip (tooltips, entryAR_Den, _("Enforce the size of a decoded pixel decoded to a certain value. Set this to 1:1 for non-anamorphic video."), NULL);
  gtk_entry_set_text (GTK_ENTRY (entryAR_Den), _("1"));
  gtk_entry_set_width_chars (GTK_ENTRY (entryAR_Den), 4);

  radiobuttonCustomAR = gtk_radio_button_new_with_mnemonic (NULL, _("Custom"));
  gtk_widget_show (radiobuttonCustomAR);
  gtk_table_attach (GTK_TABLE (table35), radiobuttonCustomAR, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, radiobuttonCustomAR, _("Set a custom aspect ratio. The default 1:1 is recommended for most video."), NULL);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonCustomAR), radiobuttonCustomAR_group);
  radiobuttonCustomAR_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonCustomAR));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonCustomAR), TRUE);

  radiobuttonPredefinedAR = gtk_radio_button_new_with_mnemonic (NULL, _("Predefined Aspect Ratio"));
  gtk_widget_show (radiobuttonPredefinedAR);
  gtk_table_attach (GTK_TABLE (table35), radiobuttonPredefinedAR, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, radiobuttonPredefinedAR, _("Set a common predefined aspect ratio"), NULL);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonPredefinedAR), radiobuttonCustomAR_group);
  radiobuttonCustomAR_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonPredefinedAR));

  radiobuttonAsInputAR = gtk_radio_button_new_with_mnemonic (NULL, _("As Input"));
  gtk_widget_show (radiobuttonAsInputAR);
  gtk_table_attach (GTK_TABLE (table35), radiobuttonAsInputAR, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonAsInputAR), radiobuttonCustomAR_group);
  radiobuttonCustomAR_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonAsInputAR));

  label40 = gtk_label_new ("1:1 (1:1)");
  gtk_widget_show (label40);
  gtk_table_attach (GTK_TABLE (table35), label40, 1, 2, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label40), 0, 0.5);

  comboboxentry1 = gtk_combo_box_new_text ();
  gtk_widget_show (comboboxentry1);
  gtk_table_attach (GTK_TABLE (table35), comboboxentry1, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  label39 = gtk_label_new (_("<b>Sample Aspect Ratio</b>"));
  gtk_widget_show (label39);
  gtk_frame_set_label_widget (GTK_FRAME (frameSampleAR), label39);
  gtk_label_set_use_markup (GTK_LABEL (label39), TRUE);

  frame7 = gtk_frame_new (NULL);
  gtk_widget_show (frame7);
  gtk_box_pack_start (GTK_BOX (vbox5), frame7, FALSE, TRUE, 0);

  alignment7 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment7);
  gtk_container_add (GTK_CONTAINER (frame7), alignment7);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment7), 7, 7, 7, 7);

  vbox7 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox7);
  gtk_container_add (GTK_CONTAINER (alignment7), vbox7);

  alignment21 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment21);
  gtk_box_pack_start (GTK_BOX (vbox7), alignment21, TRUE, TRUE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment21), 0, 3, 0, 0);

  hbox2 = gtk_hbox_new (FALSE, 17);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment21), hbox2);

  checkbuttonCABAC = gtk_check_button_new_with_mnemonic (_("CABAC"));
  gtk_widget_show (checkbuttonCABAC);
  gtk_box_pack_start (GTK_BOX (hbox2), checkbuttonCABAC, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonCABAC, _("Lossless compression technique that reduces bitrate by approximately 10%. Increases encoding and decoding time."), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonCABAC), TRUE);

  hbox7 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (hbox2), hbox7, FALSE, FALSE, 0);

  labelTrellis = gtk_label_new (_("Trellis"));
  gtk_widget_show (labelTrellis);
  gtk_box_pack_start (GTK_BOX (hbox7), labelTrellis, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (labelTrellis), 0, 0.5);

  spinbuttonTrellis_adj = gtk_adjustment_new (0, 0, 2, 1, 10, 0);
  spinbuttonTrellis = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonTrellis_adj), 1, 0);
  gtk_widget_show (spinbuttonTrellis);
  gtk_box_pack_start (GTK_BOX (hbox7), spinbuttonTrellis, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonTrellis, _("Use Trellis Rate Distortion Optimisation to find optimal encoding for each block. Level 0 equals disabled, level 1 equals normal and level 2 equals high."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonTrellis), TRUE);

  hbox11 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox11);
  gtk_box_pack_start (GTK_BOX (hbox2), hbox11, FALSE, FALSE, 2);

  labelNoiseReduction = gtk_label_new (_("Noise Reduction"));
  gtk_widget_show (labelNoiseReduction);
  gtk_box_pack_start (GTK_BOX (hbox11), labelNoiseReduction, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (labelNoiseReduction), 0, 0.5);

  spinbuttonNoiseReduction_adj = gtk_adjustment_new (0, 0, 1000000000, 1, 10, 0);
  spinbuttonNoiseReduction = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonNoiseReduction_adj), 1, 0);
  gtk_widget_show (spinbuttonNoiseReduction);
  gtk_box_pack_start (GTK_BOX (hbox11), spinbuttonNoiseReduction, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonNoiseReduction, _("Set the amount of noise reduction"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonNoiseReduction), TRUE);

  hbox3 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox7), hbox3, TRUE, TRUE, 0);

  checkbuttonDeblockingFilter = gtk_check_button_new_with_mnemonic (_("Deblocking Filter"));
  gtk_widget_show (checkbuttonDeblockingFilter);
  gtk_box_pack_start (GTK_BOX (hbox3), checkbuttonDeblockingFilter, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonDeblockingFilter, _("Enable in-loop deblocking to filter the video"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonDeblockingFilter), TRUE);

  table8 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table8);
  gtk_box_pack_start (GTK_BOX (hbox3), table8, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table8), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table8), 10);

  labelThreshold = gtk_label_new (_("Threshold"));
  gtk_widget_show (labelThreshold);
  gtk_table_attach (GTK_TABLE (table8), labelThreshold, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (labelThreshold), TRUE);
  gtk_misc_set_alignment (GTK_MISC (labelThreshold), 0, 0.5);

  labelStrength = gtk_label_new (_("Strength"));
  gtk_widget_show (labelStrength);
  gtk_table_attach (GTK_TABLE (table8), labelStrength, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_use_markup (GTK_LABEL (labelStrength), TRUE);
  gtk_misc_set_alignment (GTK_MISC (labelStrength), 0, 0.5);

  spinbuttonThreshold_adj = gtk_adjustment_new (0, -6, 6, 1, 10, 0);
  spinbuttonThreshold = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonThreshold_adj), 1, 0);
  gtk_widget_show (spinbuttonThreshold);
  gtk_table_attach (GTK_TABLE (table8), spinbuttonThreshold, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonThreshold, _("Threshold for block detection. Positive values will detect more blocks, negative values will detect less."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonThreshold), TRUE);

  spinbuttonStrength_adj = gtk_adjustment_new (0, -6, 6, 1, 10, 0);
  spinbuttonStrength = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonStrength_adj), 1, 0);
  gtk_widget_show (spinbuttonStrength);
  gtk_table_attach (GTK_TABLE (table8), spinbuttonStrength, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonStrength, _("Positive values will soften the video, negative values will sharpen"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonStrength), TRUE);

  label25 = gtk_label_new (_("<b>Misc. Options</b>"));
  gtk_widget_show (label25);
  gtk_frame_set_label_widget (GTK_FRAME (frame7), label25);
  gtk_label_set_use_markup (GTK_LABEL (label25), TRUE);

  labelPageMotion_More = gtk_label_new (_("Motion & Misc"));
  gtk_widget_show (labelPageMotion_More);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), labelPageMotion_More);

  alignment16 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment16);
  gtk_container_add (GTK_CONTAINER (notebook1), alignment16);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment16), 7, 7, 7, 7);

  vbox2 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (alignment16), vbox2);

  frame4 = gtk_frame_new (NULL);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (vbox2), frame4, FALSE, TRUE, 0);

  alignment4 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment4);
  gtk_container_add (GTK_CONTAINER (frame4), alignment4);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment4), 7, 7, 7, 7);

  vbox3 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox3);
  gtk_container_add (GTK_CONTAINER (alignment4), vbox3);

  checkbutton8x8 = gtk_check_button_new_with_mnemonic (_("8x8 Transform"));
  gtk_widget_show (checkbutton8x8);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton8x8, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton8x8, _("General block breakdown transformation"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton8x8), TRUE);

  checkbutton8x8P = gtk_check_button_new_with_mnemonic (_("8x8, 8x16 and 16x8 P-frame search"));
  gtk_widget_show (checkbutton8x8P);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton8x8P, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton8x8P, _("Improve the P-frame quality"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton8x8P), TRUE);

  checkbutton8x8B = gtk_check_button_new_with_mnemonic (_("8x8, 8x16 and 16x8 B-frame search"));
  gtk_widget_show (checkbutton8x8B);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton8x8B, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton8x8B, _("Improve the B-frame quality"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton8x8B), TRUE);

  checkbutton4x4 = gtk_check_button_new_with_mnemonic (_("4x4, 4x8 and 8x4 P-frame search"));
  gtk_widget_show (checkbutton4x4);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton4x4, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton4x4, _("Further improve the P-frame quality"), NULL);

  checkbutton8x8I = gtk_check_button_new_with_mnemonic (_("8x8 Intra search"));
  gtk_widget_show (checkbutton8x8I);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton8x8I, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton8x8I, _("Enable DCT Intra block search to improve quality"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton8x8I), TRUE);

  checkbutton4x4I = gtk_check_button_new_with_mnemonic (_("4x4 Intra search"));
  gtk_widget_show (checkbutton4x4I);
  gtk_box_pack_start (GTK_BOX (vbox3), checkbutton4x4I, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, checkbutton4x4I, _("Further enable DCT Intra block search to improve quality"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton4x4I), TRUE);

  label19 = gtk_label_new (_("<b>Partition Macroblocks</b>"));
  gtk_widget_show (label19);
  gtk_frame_set_label_widget (GTK_FRAME (frame4), label19);
  gtk_label_set_use_markup (GTK_LABEL (label19), TRUE);

  frame5 = gtk_frame_new (NULL);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (vbox2), frame5, FALSE, TRUE, 0);

  alignment5 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment5);
  gtk_container_add (GTK_CONTAINER (frame5), alignment5);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment5), 7, 7, 7, 7);

  vbox4 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox4);
  gtk_container_add (GTK_CONTAINER (alignment5), vbox4);

  table5 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table5);
  gtk_box_pack_start (GTK_BOX (vbox4), table5, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table5), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table5), 10);

  labelBias = gtk_label_new (_("Bias"));
  gtk_widget_show (labelBias);
  gtk_table_attach (GTK_TABLE (table5), labelBias, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelBias), 0, 0.5);

  labelMaxConsecutive = gtk_label_new (_("Max Consecutive"));
  gtk_widget_show (labelMaxConsecutive);
  gtk_table_attach (GTK_TABLE (table5), labelMaxConsecutive, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelMaxConsecutive), 0, 0.5);

  spinbuttonBias_adj = gtk_adjustment_new (0, -100, 100, 1, 10, 0);
  spinbuttonBias = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBias_adj), 1, 0);
  gtk_widget_show (spinbuttonBias);
  gtk_table_attach (GTK_TABLE (table5), spinbuttonBias, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonBias, _("Increase / decrease probability for how often B-frames are used. It will not violate the maximum consecutive frame limit."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonBias), TRUE);

  spinbuttonMaxBFrame_adj = gtk_adjustment_new (3, 0, 16, 1, 10, 0);
  spinbuttonMaxBFrame = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMaxBFrame_adj), 1, 0);
  gtk_widget_show (spinbuttonMaxBFrame);
  gtk_table_attach (GTK_TABLE (table5), spinbuttonMaxBFrame, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonMaxBFrame, _("Set the maximum number of consecutive B-frames. This defines how many duplicate frames can be dropped. Numbers 2 to 5 are recommended. This greatly improves the use of bitrate and quality."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMaxBFrame), TRUE);

  alignment20 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment20);
  gtk_box_pack_start (GTK_BOX (vbox4), alignment20, TRUE, TRUE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment20), 3, 3, 0, 0);

  table6 = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (table6);
  gtk_container_add (GTK_CONTAINER (alignment20), table6);
  gtk_table_set_row_spacings (GTK_TABLE (table6), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table6), 10);

  checkbuttonBidirME = gtk_check_button_new_with_mnemonic (_("Bidirectional ME"));
  gtk_widget_show (checkbuttonBidirME);
  gtk_table_attach (GTK_TABLE (table6), checkbuttonBidirME, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonBidirME, _("Optimise both motion vectors in B-frames. This will improve quality but take more time for encoding."), NULL);

  checkbuttonWeighted = gtk_check_button_new_with_mnemonic (_("Weighted Biprediction"));
  gtk_widget_show (checkbuttonWeighted);
  gtk_table_attach (GTK_TABLE (table6), checkbuttonWeighted, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonWeighted, _("Enables weighting of B-frames to help fades"), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonWeighted), TRUE);

  checkbuttonAdaptative = gtk_check_button_new_with_mnemonic (_("Adaptative DCT"));
  gtk_widget_show (checkbuttonAdaptative);
  gtk_table_attach (GTK_TABLE (table6), checkbuttonAdaptative, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonAdaptative, _("Use fewer B-frames if needed. This is always recommended. If not enabled, the codec will always use the maximum number of consecutive B-frames."), NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbuttonAdaptative), TRUE);

  checkbuttonBasReference = gtk_check_button_new_with_mnemonic (_("Use as Reference"));
  gtk_widget_show (checkbuttonBasReference);
  gtk_table_attach (GTK_TABLE (table6), checkbuttonBasReference, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, checkbuttonBasReference, _("Allow B-frames to make references non-linearly to another B-frame (instead of creating a duplicate copy)"), NULL);

  hbox1 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox4), hbox1, TRUE, TRUE, 0);

  labelBFrameDirectMode = gtk_label_new (_("B-Frame Direct Mode"));
  gtk_widget_show (labelBFrameDirectMode);
  gtk_box_pack_start (GTK_BOX (hbox1), labelBFrameDirectMode, FALSE, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (labelBFrameDirectMode), 0, 0.5);

  comboboxDirectMode = gtk_combo_box_new_text ();
  gtk_widget_show (comboboxDirectMode);
  gtk_box_pack_start (GTK_BOX (hbox1), comboboxDirectMode, FALSE, TRUE, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxDirectMode), _("None"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxDirectMode), _("Spatial"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxDirectMode), _("Temporal"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (comboboxDirectMode), _("Auto"));

  label20 = gtk_label_new (_("<b>B-Frames</b>"));
  gtk_widget_show (label20);
  gtk_frame_set_label_widget (GTK_FRAME (frame5), label20);
  gtk_label_set_use_markup (GTK_LABEL (label20), TRUE);

  labelPagePartion_Frame = gtk_label_new (_("Partitions & Frames"));
  gtk_widget_show (labelPagePartion_Frame);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), labelPagePartion_Frame);

  alignment17 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment17);
  gtk_container_add (GTK_CONTAINER (notebook1), alignment17);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment17), 7, 7, 7, 7);

  vbox1 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (alignment17), vbox1);

  frame1 = gtk_frame_new (NULL);
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (vbox1), frame1, FALSE, TRUE, 1);

  alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment1);
  gtk_container_add (GTK_CONTAINER (frame1), alignment1);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 7, 7, 7, 7);

  table2 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table2);
  gtk_container_add (GTK_CONTAINER (alignment1), table2);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 10);

  spinbuttonKeyframeBoost_adj = gtk_adjustment_new (40, 0, 100, 1, 10, 0);
  spinbuttonKeyframeBoost = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonKeyframeBoost_adj), 1, 0);
  gtk_widget_show (spinbuttonKeyframeBoost);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonKeyframeBoost, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_size_request (spinbuttonKeyframeBoost, 115, -1);
  gtk_tooltips_set_tip (tooltips, spinbuttonKeyframeBoost, _("Set how much \"bitrate bonus\" a keyframe can get"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonKeyframeBoost), TRUE);

  spinbuttonBframeReduction_adj = gtk_adjustment_new (30, 0, 100, 1, 10, 0);
  spinbuttonBframeReduction = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBframeReduction_adj), 1, 0);
  gtk_widget_show (spinbuttonBframeReduction);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonBframeReduction, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonBframeReduction, _("Set how much bitrate is deducted from a B-frame as compared to the previous P-frame"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonBframeReduction), TRUE);

  spinbuttonBitrateVariability_adj = gtk_adjustment_new (60, 0, 100, 1, 10, 0);
  spinbuttonBitrateVariability = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBitrateVariability_adj), 1, 0);
  gtk_widget_show (spinbuttonBitrateVariability);
  gtk_table_attach (GTK_TABLE (table2), spinbuttonBitrateVariability, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonBitrateVariability, _("Define how much the bitrate can fluctuate over the entire video. 0% results in a constant bitrate stream, while 100% results in a pure quality based bitrate stream."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonBitrateVariability), TRUE);

  labelKeyframeboost = gtk_label_new (_("Keyframe Boost (%)"));
  gtk_widget_show (labelKeyframeboost);
  gtk_table_attach (GTK_TABLE (table2), labelKeyframeboost, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelKeyframeboost), 0, 0.5);

  label11 = gtk_label_new (_("B-frame Reduction (%)"));
  gtk_widget_show (label11);
  gtk_table_attach (GTK_TABLE (table2), label11, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

  label12 = gtk_label_new (_("Bitrate Variability (%)"));
  gtk_widget_show (label12);
  gtk_table_attach (GTK_TABLE (table2), label12, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label12), 0, 0.5);

  label7 = gtk_label_new (_("<b>Bitrate</b>"));
  gtk_widget_show (label7);
  gtk_frame_set_label_widget (GTK_FRAME (frame1), label7);
  gtk_label_set_use_markup (GTK_LABEL (label7), TRUE);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, FALSE, TRUE, 1);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame2), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 7, 7, 7, 7);

  table3 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table3);
  gtk_container_add (GTK_CONTAINER (alignment2), table3);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 20);

  label13 = gtk_label_new (_("Min QP"));
  gtk_widget_show (label13);
  gtk_table_attach (GTK_TABLE (table3), label13, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);

  label14 = gtk_label_new (_("Max QP"));
  gtk_widget_show (label14);
  gtk_table_attach (GTK_TABLE (table3), label14, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);

  label15 = gtk_label_new (_("Max QP Step"));
  gtk_widget_show (label15);
  gtk_table_attach (GTK_TABLE (table3), label15, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);

  spinbuttonMinQp_adj = gtk_adjustment_new (10, 10, 51, 1, 10, 0);
  spinbuttonMinQp = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMinQp_adj), 1, 0);
  gtk_widget_show (spinbuttonMinQp);
  gtk_table_attach (GTK_TABLE (table3), spinbuttonMinQp, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_size_request (spinbuttonMinQp, 54, -1);
  gtk_tooltips_set_tip (tooltips, spinbuttonMinQp, _("Enforce a minimum quantizer level"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMinQp), TRUE);

  spinbuttonMaxQp_adj = gtk_adjustment_new (51, 10, 51, 1, 10, 0);
  spinbuttonMaxQp = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMaxQp_adj), 1, 0);
  gtk_widget_show (spinbuttonMaxQp);
  gtk_table_attach (GTK_TABLE (table3), spinbuttonMaxQp, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonMaxQp, _("Enforce a maximum quantizer level"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMaxQp), TRUE);

  spinbuttonQpStep_adj = gtk_adjustment_new (4, 0, 10, 1, 10, 0);
  spinbuttonQpStep = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonQpStep_adj), 1, 0);
  gtk_widget_show (spinbuttonQpStep);
  gtk_table_attach (GTK_TABLE (table3), spinbuttonQpStep, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonQpStep, _("Define how much the quantizer rate can change between two consecutive frames"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonQpStep), TRUE);

  label8 = gtk_label_new (_("<b>Quantization Limits</b>"));
  gtk_widget_show (label8);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label8);
  gtk_label_set_use_markup (GTK_LABEL (label8), TRUE);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, FALSE, TRUE, 1);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame3), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 7, 7, 7, 7);

  table4 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table4);
  gtk_container_add (GTK_CONTAINER (alignment3), table4);
  gtk_table_set_row_spacings (GTK_TABLE (table4), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table4), 10);

  spinbuttonSceneCut_adj = gtk_adjustment_new (40, 0, 100, 1, 10, 0);
  spinbuttonSceneCut = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonSceneCut_adj), 1, 0);
  gtk_widget_show (spinbuttonSceneCut);
  gtk_table_attach (GTK_TABLE (table4), spinbuttonSceneCut, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_set_size_request (spinbuttonSceneCut, 115, -1);
  gtk_tooltips_set_tip (tooltips, spinbuttonSceneCut, _("Increase / decrease sensitivity for detecting scene changes. Improves I-frame usage and quality."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonSceneCut), TRUE);

  label16 = gtk_label_new (_("Scene Cut Threshold"));
  gtk_widget_show (label16);
  gtk_table_attach (GTK_TABLE (table4), label16, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  label17 = gtk_label_new (_("Min IDR Frame Interval"));
  gtk_widget_show (label17);
  gtk_table_attach (GTK_TABLE (table4), label17, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);

  spinbuttonMinIdr_adj = gtk_adjustment_new (25, 0, 100, 1, 10, 0);
  spinbuttonMinIdr = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMinIdr_adj), 1, 0);
  gtk_widget_show (spinbuttonMinIdr);
  gtk_table_attach (GTK_TABLE (table4), spinbuttonMinIdr, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonMinIdr, _("Set minimum frame interval between IDR frames. Defines the minimum amount a frame can be reused and referenced by other frames before a new IDR frame is established."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMinIdr), TRUE);

  spinbuttonMaxIdr_adj = gtk_adjustment_new (250, 1, 1000, 1, 10, 0);
  spinbuttonMaxIdr = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonMaxIdr_adj), 1, 0);
  gtk_widget_show (spinbuttonMaxIdr);
  gtk_table_attach (GTK_TABLE (table4), spinbuttonMaxIdr, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonMaxIdr, _("Set maximum frame interval between IDR frames. Defines the maximum amount a frame can be reused and referenced by other frames before a new IDR frame is established."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonMaxIdr), TRUE);

  label18 = gtk_label_new (_("Max IDR Frame Interval"));
  gtk_widget_show (label18);
  gtk_table_attach (GTK_TABLE (table4), label18, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label18), 0, 0.5);

  labelMoreEncodingSettings = gtk_label_new (_("<b>More Rate Settings</b>"));
  gtk_widget_show (labelMoreEncodingSettings);
  gtk_frame_set_label_widget (GTK_FRAME (frame3), labelMoreEncodingSettings);
  gtk_label_set_use_markup (GTK_LABEL (labelMoreEncodingSettings), TRUE);

  frameVideoBufferVerifier = gtk_frame_new (NULL);
  gtk_widget_show (frameVideoBufferVerifier);
  gtk_box_pack_start (GTK_BOX (vbox1), frameVideoBufferVerifier, FALSE, TRUE, 0);

  alignment11 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment11);
  gtk_container_add (GTK_CONTAINER (frameVideoBufferVerifier), alignment11);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment11), 7, 7, 7, 7);

  table12 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table12);
  gtk_container_add (GTK_CONTAINER (alignment11), table12);
  gtk_table_set_row_spacings (GTK_TABLE (table12), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table12), 10);

  labelVBVBufferSize = gtk_label_new (_("VBV Buffer Size"));
  gtk_widget_show (labelVBVBufferSize);
  gtk_table_attach (GTK_TABLE (table12), labelVBVBufferSize, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelVBVBufferSize), 0, 0.5);

  labelVBVInitialbuffer = gtk_label_new (_("Initial VBV Buffer (%)"));
  gtk_widget_show (labelVBVInitialbuffer);
  gtk_table_attach (GTK_TABLE (table12), labelVBVInitialbuffer, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelVBVInitialbuffer), 0, 0.5);

  labelVBVMaximumBitrate = gtk_label_new (_("Maximum Local Bitrate"));
  gtk_widget_show (labelVBVMaximumBitrate);
  gtk_table_attach (GTK_TABLE (table12), labelVBVMaximumBitrate, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelVBVMaximumBitrate), 0, 0.5);

  spinbuttonvbv_max_bitrate_adj = gtk_adjustment_new (0, 0, 99999, 1, 10, 0);
  spinbuttonvbv_max_bitrate = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonvbv_max_bitrate_adj), 1, 0);
  gtk_widget_show (spinbuttonvbv_max_bitrate);
  gtk_table_attach (GTK_TABLE (table12), spinbuttonvbv_max_bitrate, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonvbv_max_bitrate), TRUE);

  spinbuttonvbv_buffer_size_adj = gtk_adjustment_new (0, 0, 99999, 1, 10, 0);
  spinbuttonvbv_buffer_size = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonvbv_buffer_size_adj), 1, 0);
  gtk_widget_show (spinbuttonvbv_buffer_size);
  gtk_table_attach (GTK_TABLE (table12), spinbuttonvbv_buffer_size, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonvbv_buffer_size), TRUE);

  spinbuttonvbv_buffer_init_adj = gtk_adjustment_new (90, 0, 100, 1, 10, 0);
  spinbuttonvbv_buffer_init = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonvbv_buffer_init_adj), 1, 0);
  gtk_widget_show (spinbuttonvbv_buffer_init);
  gtk_table_attach (GTK_TABLE (table12), spinbuttonvbv_buffer_init, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonvbv_buffer_init), TRUE);

  labelVideoBufferVerifier = gtk_label_new (_("<b>Video Buffer Verifier</b>"));
  gtk_widget_show (labelVideoBufferVerifier);
  gtk_frame_set_label_widget (GTK_FRAME (frameVideoBufferVerifier), labelVideoBufferVerifier);
  gtk_label_set_use_markup (GTK_LABEL (labelVideoBufferVerifier), TRUE);

  labelPageRateControl = gtk_label_new (_("Rate Control"));
  gtk_widget_show (labelPageRateControl);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), labelPageRateControl);

  alignment18 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment18);
  gtk_container_add (GTK_CONTAINER (notebook1), alignment18);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment18), 7, 7, 7, 7);

  vbox8 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox8);
  gtk_container_add (GTK_CONTAINER (alignment18), vbox8);

  frame10 = gtk_frame_new (NULL);
  gtk_widget_show (frame10);
  gtk_box_pack_start (GTK_BOX (vbox8), frame10, FALSE, TRUE, 0);

  alignment10 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment10);
  gtk_container_add (GTK_CONTAINER (frame10), alignment10);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment10), 7, 7, 7, 7);

  table15 = gtk_table_new (4, 2, FALSE);
  gtk_widget_show (table15);
  gtk_container_add (GTK_CONTAINER (alignment10), table15);
  gtk_table_set_row_spacings (GTK_TABLE (table15), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table15), 10);

  labelBitrateVariance = gtk_label_new (_("Bitrate Variance"));
  gtk_widget_show (labelBitrateVariance);
  gtk_table_attach (GTK_TABLE (table15), labelBitrateVariance, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelBitrateVariance), 0, 0.5);

  spinbuttonBitrateVariance_adj = gtk_adjustment_new (1, 0, 1, 0.10000000149, 10, 0);
  spinbuttonBitrateVariance = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonBitrateVariance_adj), 0.10000000149, 1);
  gtk_widget_show (spinbuttonBitrateVariance);
  gtk_table_attach (GTK_TABLE (table15), spinbuttonBitrateVariance, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonBitrateVariance, _("Allowed variance of average bitrate. Lower values mean less variance. Higher values mean more variance."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonBitrateVariance), TRUE);

  spinbuttonQuantizerCompression_adj = gtk_adjustment_new (0.600000023842, 0, 1, 0.10000000149, 10, 0);
  spinbuttonQuantizerCompression = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonQuantizerCompression_adj), 0.10000000149, 1);
  gtk_widget_show (spinbuttonQuantizerCompression);
  gtk_table_attach (GTK_TABLE (table15), spinbuttonQuantizerCompression, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonQuantizerCompression, _("Allowed variance of average quantizer or quality. Lower values mean less variance. Higher values mean more variance. Note that 0 means constant quality while 1 means constant fluctuation. Recommended 0.6."), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonQuantizerCompression), TRUE);

  spinbuttonTempBlurFrame_adj = gtk_adjustment_new (20, 0, 999, 1, 10, 0);
  spinbuttonTempBlurFrame = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonTempBlurFrame_adj), 1, 0);
  gtk_widget_show (spinbuttonTempBlurFrame);
  gtk_table_attach (GTK_TABLE (table15), spinbuttonTempBlurFrame, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonTempBlurFrame, _("Reduced fluctuations in Quantizer (before curve compression)"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonTempBlurFrame), TRUE);

  spinbuttonTempBlurQuant_adj = gtk_adjustment_new (0.5, 0, 1, 0.5, 10, 0);
  spinbuttonTempBlurQuant = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonTempBlurQuant_adj), 0.5, 1);
  gtk_widget_show (spinbuttonTempBlurQuant);
  gtk_table_attach (GTK_TABLE (table15), spinbuttonTempBlurQuant, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonTempBlurQuant), TRUE);

  labelQuantizerCompression = gtk_label_new (_("Quantizer Compression"));
  gtk_widget_show (labelQuantizerCompression);
  gtk_table_attach (GTK_TABLE (table15), labelQuantizerCompression, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelQuantizerCompression), 0, 0.5);

  labelTempBlurEstFrame = gtk_label_new (_("Temp. Blur of Est. Frame Complexity"));
  gtk_widget_show (labelTempBlurEstFrame);
  gtk_table_attach (GTK_TABLE (table15), labelTempBlurEstFrame, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelTempBlurEstFrame), 0, 0.5);

  labelTempBlueQuant = gtk_label_new (_("Temp. Blur of Quant. after CC"));
  gtk_widget_show (labelTempBlueQuant);
  gtk_table_attach (GTK_TABLE (table15), labelTempBlueQuant, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelTempBlueQuant), 0, 0.5);

  labelMisc = gtk_label_new (_("<b>Misc</b>"));
  gtk_widget_show (labelMisc);
  gtk_frame_set_label_widget (GTK_FRAME (frame10), labelMisc);
  gtk_label_set_use_markup (GTK_LABEL (labelMisc), TRUE);

  frame11 = gtk_frame_new (NULL);
  gtk_widget_show (frame11);
  gtk_box_pack_start (GTK_BOX (vbox8), frame11, FALSE, TRUE, 0);

  alignment13 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment13);
  gtk_container_add (GTK_CONTAINER (frame11), alignment13);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment13), 7, 7, 7, 7);

  table16 = gtk_table_new (3, 2, FALSE);
  gtk_widget_show (table16);
  gtk_container_add (GTK_CONTAINER (alignment13), table16);
  gtk_table_set_row_spacings (GTK_TABLE (table16), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table16), 10);

  spinbutton8_adj = gtk_adjustment_new (1.39999997616, 1, 10, 0.10000000149, 10, 0);
  spinbutton8 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton8_adj), 0.10000000149, 1);
  gtk_widget_show (spinbutton8);
  gtk_table_attach (GTK_TABLE (table16), spinbutton8, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbutton8, _("Quantization factors used between I and P-frames"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton8), TRUE);

  spinbutton9_adj = gtk_adjustment_new (1.29999995232, 1, 10, 0.10000000149, 10, 0);
  spinbutton9 = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton9_adj), 0.10000000149, 1);
  gtk_widget_show (spinbutton9);
  gtk_table_attach (GTK_TABLE (table16), spinbutton9, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbutton9, _("Quantization used between P and B-frames"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton9), TRUE);

  spinbuttonChromaQPOffset_adj = gtk_adjustment_new (0, -12, 12, 1, 10, 0);
  spinbuttonChromaQPOffset = gtk_spin_button_new (GTK_ADJUSTMENT (spinbuttonChromaQPOffset_adj), 1, 0);
  gtk_widget_show (spinbuttonChromaQPOffset);
  gtk_table_attach (GTK_TABLE (table16), spinbuttonChromaQPOffset, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_tooltips_set_tip (tooltips, spinbuttonChromaQPOffset, _("Quantization difference between chroma (color) and luma (brightness)"), NULL);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbuttonChromaQPOffset), TRUE);

  labelFactorbetweenIandP = gtk_label_new (_("Factor between I and P-frame Quants"));
  gtk_widget_show (labelFactorbetweenIandP);
  gtk_table_attach (GTK_TABLE (table16), labelFactorbetweenIandP, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelFactorbetweenIandP), 0, 0.5);

  labelFactorbetweenPandB = gtk_label_new (_("Factor between P and B-frame Quants"));
  gtk_widget_show (labelFactorbetweenPandB);
  gtk_table_attach (GTK_TABLE (table16), labelFactorbetweenPandB, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelFactorbetweenPandB), 0, 0.5);

  labelChromaQPOffset = gtk_label_new (_("Chroma QP Offset"));
  gtk_widget_show (labelChromaQPOffset);
  gtk_table_attach (GTK_TABLE (table16), labelChromaQPOffset, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (labelChromaQPOffset), 0, 0.5);

  labelQuantizers = gtk_label_new (_("<b>Quantizers</b>"));
  gtk_widget_show (labelQuantizers);
  gtk_frame_set_label_widget (GTK_FRAME (frame11), labelQuantizers);
  gtk_label_set_use_markup (GTK_LABEL (labelQuantizers), TRUE);

  frame9 = gtk_frame_new (NULL);
  gtk_widget_show (frame9);
  gtk_box_pack_start (GTK_BOX (vbox8), frame9, FALSE, TRUE, 0);

  alignment9 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment9);
  gtk_container_add (GTK_CONTAINER (frame9), alignment9);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment9), 7, 7, 7, 7);

  vbox9 = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox9);
  gtk_container_add (GTK_CONTAINER (alignment9), vbox9);

  radiobuttonFlatmatrix = gtk_radio_button_new_with_mnemonic (NULL, _("Flat Matrix"));
  gtk_widget_show (radiobuttonFlatmatrix);
  gtk_box_pack_start (GTK_BOX (vbox9), radiobuttonFlatmatrix, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonFlatmatrix), radiobuttonFlatmatrix_group);
  radiobuttonFlatmatrix_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonFlatmatrix));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobuttonFlatmatrix), TRUE);

  radiobuttonJVTmatrix = gtk_radio_button_new_with_mnemonic (NULL, _("JVT Matrix"));
  gtk_widget_show (radiobuttonJVTmatrix);
  gtk_box_pack_start (GTK_BOX (vbox9), radiobuttonJVTmatrix, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonJVTmatrix), radiobuttonFlatmatrix_group);
  radiobuttonFlatmatrix_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonJVTmatrix));

  hbox13 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox13);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox13, FALSE, FALSE, 0);

  radiobuttonCustommatrix = gtk_radio_button_new_with_mnemonic (NULL, _("Custom Matrix"));
  gtk_widget_show (radiobuttonCustommatrix);
  gtk_box_pack_start (GTK_BOX (hbox13), radiobuttonCustommatrix, TRUE, TRUE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobuttonCustommatrix), radiobuttonFlatmatrix_group);
  radiobuttonFlatmatrix_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobuttonCustommatrix));

  hbox14 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox14);
  gtk_box_pack_start (GTK_BOX (hbox13), hbox14, TRUE, TRUE, 0);

  filechooserbuttonOpenCQMFile = gtk_file_chooser_button_new (_("Open CQM file"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_show (filechooserbuttonOpenCQMFile);
  gtk_box_pack_start (GTK_BOX (hbox14), filechooserbuttonOpenCQMFile, TRUE, TRUE, 0);

  vseparator1 = gtk_vseparator_new ();
  gtk_widget_show (vseparator1);
  gtk_box_pack_start (GTK_BOX (hbox14), vseparator1, TRUE, TRUE, 0);

  buttonEditCustomMatrix = gtk_button_new_with_mnemonic (_("Edit Custom Matrix"));
  gtk_widget_show (buttonEditCustomMatrix);
  gtk_box_pack_start (GTK_BOX (hbox14), buttonEditCustomMatrix, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, buttonEditCustomMatrix, _("Edit loaded custom quantization matrix file"), NULL);

  labelFrameQuantizationMatrices = gtk_label_new (_("<b>Quantization Matrices</b>"));
  gtk_widget_show (labelFrameQuantizationMatrices);
  gtk_frame_set_label_widget (GTK_FRAME (frame9), labelFrameQuantizationMatrices);
  gtk_label_set_use_markup (GTK_LABEL (labelFrameQuantizationMatrices), TRUE);

  labelMore = gtk_label_new (_("More"));
  gtk_widget_show (labelMore);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), labelMore);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  buttonResetDaults = gtk_button_new_with_mnemonic (_("_Defaults"));
  gtk_widget_show (buttonResetDaults);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), buttonResetDaults, 0);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment14, "alignment14");
  GLADE_HOOKUP_OBJECT (dialog1, tableBitrate, "tableBitrate");
  GLADE_HOOKUP_OBJECT (dialog1, labelEncodingMode, "labelEncodingMode");
  GLADE_HOOKUP_OBJECT (dialog1, labelQuantizer, "labelQuantizer");
  GLADE_HOOKUP_OBJECT (dialog1, labelTarget, "labelTarget");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonQuantizer, "spinbuttonQuantizer");
  GLADE_HOOKUP_OBJECT (dialog1, entryTarget, "entryTarget");
  GLADE_HOOKUP_OBJECT (dialog1, comboboxMode, "comboboxMode");
  GLADE_HOOKUP_OBJECT (dialog1, labelPageBitrate, "labelPageBitrate");
  GLADE_HOOKUP_OBJECT (dialog1, alignment15, "alignment15");
  GLADE_HOOKUP_OBJECT (dialog1, vbox5, "vbox5");
  GLADE_HOOKUP_OBJECT (dialog1, frame6, "frame6");
  GLADE_HOOKUP_OBJECT (dialog1, alignment6, "alignment6");
  GLADE_HOOKUP_OBJECT (dialog1, vbox6, "vbox6");
  GLADE_HOOKUP_OBJECT (dialog1, table7, "table7");
  GLADE_HOOKUP_OBJECT (dialog1, labelPartitionDecision, "labelPartitionDecision");
  GLADE_HOOKUP_OBJECT (dialog1, labelMethod, "labelMethod");
  GLADE_HOOKUP_OBJECT (dialog1, comboboxPartitionDecision, "comboboxPartitionDecision");
  GLADE_HOOKUP_OBJECT (dialog1, comboboxMethod, "comboboxMethod");
  GLADE_HOOKUP_OBJECT (dialog1, table11, "table11");
  GLADE_HOOKUP_OBJECT (dialog1, labelMaxRefFrames, "labelMaxRefFrames");
  GLADE_HOOKUP_OBJECT (dialog1, labelRange, "labelRange");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonRange, "spinbuttonRange");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMaxRefFrames, "spinbuttonMaxRefFrames");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonChromaME, "checkbuttonChromaME");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonMixedRefs, "checkbuttonMixedRefs");
  GLADE_HOOKUP_OBJECT (dialog1, hbox8, "hbox8");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonfastPSkip, "checkbuttonfastPSkip");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonDCTDecimate, "checkbuttonDCTDecimate");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttoninterlaced, "checkbuttoninterlaced");
  GLADE_HOOKUP_OBJECT (dialog1, label24, "label24");
  GLADE_HOOKUP_OBJECT (dialog1, frameSampleAR, "frameSampleAR");
  GLADE_HOOKUP_OBJECT (dialog1, alignment12, "alignment12");
  GLADE_HOOKUP_OBJECT (dialog1, table35, "table35");
  GLADE_HOOKUP_OBJECT (dialog1, hbox12, "hbox12");
  GLADE_HOOKUP_OBJECT (dialog1, entryAR_Num, "entryAR_Num");
  GLADE_HOOKUP_OBJECT (dialog1, label53, "label53");
  GLADE_HOOKUP_OBJECT (dialog1, entryAR_Den, "entryAR_Den");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonCustomAR, "radiobuttonCustomAR");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonPredefinedAR, "radiobuttonPredefinedAR");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonAsInputAR, "radiobuttonAsInputAR");
  GLADE_HOOKUP_OBJECT (dialog1, label40, "label40");
  GLADE_HOOKUP_OBJECT (dialog1, comboboxentry1, "comboboxentry1");
  GLADE_HOOKUP_OBJECT (dialog1, label39, "label39");
  GLADE_HOOKUP_OBJECT (dialog1, frame7, "frame7");
  GLADE_HOOKUP_OBJECT (dialog1, alignment7, "alignment7");
  GLADE_HOOKUP_OBJECT (dialog1, vbox7, "vbox7");
  GLADE_HOOKUP_OBJECT (dialog1, alignment21, "alignment21");
  GLADE_HOOKUP_OBJECT (dialog1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonCABAC, "checkbuttonCABAC");
  GLADE_HOOKUP_OBJECT (dialog1, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (dialog1, labelTrellis, "labelTrellis");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonTrellis, "spinbuttonTrellis");
  GLADE_HOOKUP_OBJECT (dialog1, hbox11, "hbox11");
  GLADE_HOOKUP_OBJECT (dialog1, labelNoiseReduction, "labelNoiseReduction");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonNoiseReduction, "spinbuttonNoiseReduction");
  GLADE_HOOKUP_OBJECT (dialog1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonDeblockingFilter, "checkbuttonDeblockingFilter");
  GLADE_HOOKUP_OBJECT (dialog1, table8, "table8");
  GLADE_HOOKUP_OBJECT (dialog1, labelThreshold, "labelThreshold");
  GLADE_HOOKUP_OBJECT (dialog1, labelStrength, "labelStrength");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonThreshold, "spinbuttonThreshold");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonStrength, "spinbuttonStrength");
  GLADE_HOOKUP_OBJECT (dialog1, label25, "label25");
  GLADE_HOOKUP_OBJECT (dialog1, labelPageMotion_More, "labelPageMotion_More");
  GLADE_HOOKUP_OBJECT (dialog1, alignment16, "alignment16");
  GLADE_HOOKUP_OBJECT (dialog1, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (dialog1, frame4, "frame4");
  GLADE_HOOKUP_OBJECT (dialog1, alignment4, "alignment4");
  GLADE_HOOKUP_OBJECT (dialog1, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton8x8, "checkbutton8x8");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton8x8P, "checkbutton8x8P");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton8x8B, "checkbutton8x8B");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton4x4, "checkbutton4x4");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton8x8I, "checkbutton8x8I");
  GLADE_HOOKUP_OBJECT (dialog1, checkbutton4x4I, "checkbutton4x4I");
  GLADE_HOOKUP_OBJECT (dialog1, label19, "label19");
  GLADE_HOOKUP_OBJECT (dialog1, frame5, "frame5");
  GLADE_HOOKUP_OBJECT (dialog1, alignment5, "alignment5");
  GLADE_HOOKUP_OBJECT (dialog1, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (dialog1, table5, "table5");
  GLADE_HOOKUP_OBJECT (dialog1, labelBias, "labelBias");
  GLADE_HOOKUP_OBJECT (dialog1, labelMaxConsecutive, "labelMaxConsecutive");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonBias, "spinbuttonBias");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMaxBFrame, "spinbuttonMaxBFrame");
  GLADE_HOOKUP_OBJECT (dialog1, alignment20, "alignment20");
  GLADE_HOOKUP_OBJECT (dialog1, table6, "table6");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonBidirME, "checkbuttonBidirME");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonWeighted, "checkbuttonWeighted");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonAdaptative, "checkbuttonAdaptative");
  GLADE_HOOKUP_OBJECT (dialog1, checkbuttonBasReference, "checkbuttonBasReference");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, labelBFrameDirectMode, "labelBFrameDirectMode");
  GLADE_HOOKUP_OBJECT (dialog1, comboboxDirectMode, "comboboxDirectMode");
  GLADE_HOOKUP_OBJECT (dialog1, label20, "label20");
  GLADE_HOOKUP_OBJECT (dialog1, labelPagePartion_Frame, "labelPagePartion_Frame");
  GLADE_HOOKUP_OBJECT (dialog1, alignment17, "alignment17");
  GLADE_HOOKUP_OBJECT (dialog1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, frame1, "frame1");
  GLADE_HOOKUP_OBJECT (dialog1, alignment1, "alignment1");
  GLADE_HOOKUP_OBJECT (dialog1, table2, "table2");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonKeyframeBoost, "spinbuttonKeyframeBoost");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonBframeReduction, "spinbuttonBframeReduction");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonBitrateVariability, "spinbuttonBitrateVariability");
  GLADE_HOOKUP_OBJECT (dialog1, labelKeyframeboost, "labelKeyframeboost");
  GLADE_HOOKUP_OBJECT (dialog1, label11, "label11");
  GLADE_HOOKUP_OBJECT (dialog1, label12, "label12");
  GLADE_HOOKUP_OBJECT (dialog1, label7, "label7");
  GLADE_HOOKUP_OBJECT (dialog1, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (dialog1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (dialog1, table3, "table3");
  GLADE_HOOKUP_OBJECT (dialog1, label13, "label13");
  GLADE_HOOKUP_OBJECT (dialog1, label14, "label14");
  GLADE_HOOKUP_OBJECT (dialog1, label15, "label15");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMinQp, "spinbuttonMinQp");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMaxQp, "spinbuttonMaxQp");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonQpStep, "spinbuttonQpStep");
  GLADE_HOOKUP_OBJECT (dialog1, label8, "label8");
  GLADE_HOOKUP_OBJECT (dialog1, frame3, "frame3");
  GLADE_HOOKUP_OBJECT (dialog1, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (dialog1, table4, "table4");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonSceneCut, "spinbuttonSceneCut");
  GLADE_HOOKUP_OBJECT (dialog1, label16, "label16");
  GLADE_HOOKUP_OBJECT (dialog1, label17, "label17");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMinIdr, "spinbuttonMinIdr");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonMaxIdr, "spinbuttonMaxIdr");
  GLADE_HOOKUP_OBJECT (dialog1, label18, "label18");
  GLADE_HOOKUP_OBJECT (dialog1, labelMoreEncodingSettings, "labelMoreEncodingSettings");
  GLADE_HOOKUP_OBJECT (dialog1, frameVideoBufferVerifier, "frameVideoBufferVerifier");
  GLADE_HOOKUP_OBJECT (dialog1, alignment11, "alignment11");
  GLADE_HOOKUP_OBJECT (dialog1, table12, "table12");
  GLADE_HOOKUP_OBJECT (dialog1, labelVBVBufferSize, "labelVBVBufferSize");
  GLADE_HOOKUP_OBJECT (dialog1, labelVBVInitialbuffer, "labelVBVInitialbuffer");
  GLADE_HOOKUP_OBJECT (dialog1, labelVBVMaximumBitrate, "labelVBVMaximumBitrate");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonvbv_max_bitrate, "spinbuttonvbv_max_bitrate");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonvbv_buffer_size, "spinbuttonvbv_buffer_size");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonvbv_buffer_init, "spinbuttonvbv_buffer_init");
  GLADE_HOOKUP_OBJECT (dialog1, labelVideoBufferVerifier, "labelVideoBufferVerifier");
  GLADE_HOOKUP_OBJECT (dialog1, labelPageRateControl, "labelPageRateControl");
  GLADE_HOOKUP_OBJECT (dialog1, alignment18, "alignment18");
  GLADE_HOOKUP_OBJECT (dialog1, vbox8, "vbox8");
  GLADE_HOOKUP_OBJECT (dialog1, frame10, "frame10");
  GLADE_HOOKUP_OBJECT (dialog1, alignment10, "alignment10");
  GLADE_HOOKUP_OBJECT (dialog1, table15, "table15");
  GLADE_HOOKUP_OBJECT (dialog1, labelBitrateVariance, "labelBitrateVariance");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonBitrateVariance, "spinbuttonBitrateVariance");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonQuantizerCompression, "spinbuttonQuantizerCompression");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonTempBlurFrame, "spinbuttonTempBlurFrame");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonTempBlurQuant, "spinbuttonTempBlurQuant");
  GLADE_HOOKUP_OBJECT (dialog1, labelQuantizerCompression, "labelQuantizerCompression");
  GLADE_HOOKUP_OBJECT (dialog1, labelTempBlurEstFrame, "labelTempBlurEstFrame");
  GLADE_HOOKUP_OBJECT (dialog1, labelTempBlueQuant, "labelTempBlueQuant");
  GLADE_HOOKUP_OBJECT (dialog1, labelMisc, "labelMisc");
  GLADE_HOOKUP_OBJECT (dialog1, frame11, "frame11");
  GLADE_HOOKUP_OBJECT (dialog1, alignment13, "alignment13");
  GLADE_HOOKUP_OBJECT (dialog1, table16, "table16");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton8, "spinbutton8");
  GLADE_HOOKUP_OBJECT (dialog1, spinbutton9, "spinbutton9");
  GLADE_HOOKUP_OBJECT (dialog1, spinbuttonChromaQPOffset, "spinbuttonChromaQPOffset");
  GLADE_HOOKUP_OBJECT (dialog1, labelFactorbetweenIandP, "labelFactorbetweenIandP");
  GLADE_HOOKUP_OBJECT (dialog1, labelFactorbetweenPandB, "labelFactorbetweenPandB");
  GLADE_HOOKUP_OBJECT (dialog1, labelChromaQPOffset, "labelChromaQPOffset");
  GLADE_HOOKUP_OBJECT (dialog1, labelQuantizers, "labelQuantizers");
  GLADE_HOOKUP_OBJECT (dialog1, frame9, "frame9");
  GLADE_HOOKUP_OBJECT (dialog1, alignment9, "alignment9");
  GLADE_HOOKUP_OBJECT (dialog1, vbox9, "vbox9");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonFlatmatrix, "radiobuttonFlatmatrix");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonJVTmatrix, "radiobuttonJVTmatrix");
  GLADE_HOOKUP_OBJECT (dialog1, hbox13, "hbox13");
  GLADE_HOOKUP_OBJECT (dialog1, radiobuttonCustommatrix, "radiobuttonCustommatrix");
  GLADE_HOOKUP_OBJECT (dialog1, hbox14, "hbox14");
  GLADE_HOOKUP_OBJECT (dialog1, filechooserbuttonOpenCQMFile, "filechooserbuttonOpenCQMFile");
  GLADE_HOOKUP_OBJECT (dialog1, vseparator1, "vseparator1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonEditCustomMatrix, "buttonEditCustomMatrix");
  GLADE_HOOKUP_OBJECT (dialog1, labelFrameQuantizationMatrices, "labelFrameQuantizationMatrices");
  GLADE_HOOKUP_OBJECT (dialog1, labelMore, "labelMore");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonResetDaults, "buttonResetDaults");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, tooltips, "tooltips");

  gtk_widget_grab_default (okbutton1);
  return dialog1;
}
