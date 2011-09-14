/***************************************************************************
                             Bitrate calculator
                             ------------------

    begin                : Sat Jun 14 2008
    copyright            : (C) 2008 by gruntster/mean
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

#include "Q_calculator.h"

#include "avi_vars.h"
#include "ADM_videoFilter.h"
#include "ADM_audio/aviaudio.hxx"
#include "audioeng_buildfilters.h"
#include "ADM_video/ADM_vidMisc.h"
#include "ADM_encoder/adm_encConfig.h"
#include "ADM_toolkitQt.h"

calculatorDialog::calculatorDialog(QWidget* parent) : QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBox_currentIndexChanged(int)));
	connect(ui.mediumComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(mediumComboBox_currentIndexChanged(int)));
	connect(ui.customSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBox_valueChanged(int)));
	connect(ui.audioTrack1SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBox_valueChanged(int)));
	connect(ui.audioTrack2SpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBox_valueChanged(int)));

	float duration = 0;
	aviInfo info;
	uint16_t mm, hh, ss, ms;
	unsigned int track1 = 0;

	if (frameStart < frameEnd)
		_videoFrameCount = frameEnd - frameStart;
	else
		_videoFrameCount = frameStart - frameEnd;

	duration = ((float)video_body->getTime(_videoFrameCount) / 1000.);

	if (duration < 0)
		duration = -duration;

	_videoDuration = (uint32_t)ceil(duration);

	video_body->getVideoInfo(&info);
	frame2time(_videoFrameCount, info.fps1000, &hh, &mm, &ss, &ms);

	ui.durationLabel->setText(QString("%1:%2:%3").arg(hh, 2, 10, QLatin1Char('0')).arg(mm, 2, 10, QLatin1Char('0')).arg(ss, 2, 10, QLatin1Char('0')));

	if (audioProcessMode() && currentaudiostream)
	{
		AVDMGenericAudioStream *stream ; //= buildAudioFilter(currentaudiostream, 0);

		if (stream)
			track1 = (stream->getInfo()->byterate * 8) / 1000;

//		deleteAudioFilter(stream);
	}
	else if(currentaudiostream)
		track1 = (currentaudiostream->getInfo()->byterate * 8) / 1000;

	ui.audioTrack1SpinBox->setValue(track1);

	update();
}

calculatorDialog::~calculatorDialog()
{
}

void calculatorDialog::comboBox_currentIndexChanged(int index)
{
	update();
}

void calculatorDialog::mediumComboBox_currentIndexChanged(int value)
{
	ui.customSizeLabel->setEnabled(value == 5);
	ui.customSizeSpinBox->setEnabled(value == 5);
	ui.customSizeMbLabel->setEnabled(value == 5);

	update();
}

void calculatorDialog::spinBox_valueChanged(int value)
{
	update();
}

void calculatorDialog::update(void)
{
	unsigned int track1, track2;
	uint32_t audioSize;
	uint32_t totalSize;
	aviInfo info;

	video_body->getVideoInfo(&info);

	track1 = ui.audioTrack1SpinBox->value();
	track2 = ui.audioTrack2SpinBox->value();

	// kb->Byte
	audioSize = ((((track1 + track2) * 1000) / 8) * _videoDuration) >> 20;

	ui.audioSizeLabel->setText(QString("%1").arg(audioSize));

	// Compute total size
	unsigned int s74, s80, dvd;
	int formatIndex = ui.formatComboBox->currentIndex();

	if (formatIndex == 2)
	{ 
		// Mpeg
		s74 = 730;
		s80 = 790;
		dvd = 4300;
	}
	else
	{
		//AVI or OGM
		s74 = 650;
		s80 = 700;
		dvd = 4300;
	}

	int mediumIndex = ui.mediumComboBox->currentIndex();

	switch(mediumIndex)
	{
		case 0:
			totalSize = s80;
			break;
		case 1:
			totalSize = 2 * s80;
			break;
		case 2:
			totalSize = s74;
			break;
		case 3:
			totalSize= 2 * s74;
			break;
		case 4:
			totalSize = dvd;
			break;
		case 5:
			totalSize = ui.customSizeSpinBox->value();
			break;
	}

	ui.totalSizeLabel->setText(QString("%1").arg(totalSize));

	// Compute muxing overhead size
	unsigned int muxingOverheadSize;
	int numberOfAudioTracks = 0;
	int numberOfChunks;

	switch (formatIndex)
	{ 
		case 0:
			// AVI
			// Muxing overhead is 8 + 32 = 40 bytes per chunk.
			// More or less: numberOfChunks = (x + 1) * _videoFrameCount,
			// where x - the number of audio tracks
			if (track1 != 0)
				numberOfAudioTracks++;

			if (track2 != 0)
				numberOfAudioTracks++;

			numberOfChunks = (numberOfAudioTracks + 1) * _videoFrameCount;
			muxingOverheadSize = (unsigned int)ceil((numberOfChunks * 40) / 1048576.0);
			break;
		case 1:
			// OGM
			// Muxing overhead is 1.1% to 1.2% of (videoSize + audioSize)
			muxingOverheadSize = (unsigned int)ceil(totalSize - totalSize / 1.012);
			break;
		case 2:
			// MPEG
			// Muxing overhead is 1% to 2% of (videoSize + audioSize)
			muxingOverheadSize = (unsigned int)ceil(totalSize - totalSize / 1.02);
			break;
	}

	unsigned int videoSize = 0;

	// and compute
	if (audioSize + muxingOverheadSize >= totalSize)
		videoSize = 0;
	else
		videoSize = totalSize - audioSize - muxingOverheadSize;

	unsigned int picSize;

	// Compute average bps
	float avg;
	float bpp;

	avg = (videoSize * 1024. * 1024.) / _videoDuration;

	// now we have byte /sec, convert to kb per sec
	avg = (avg * 8) / 1000;
	ui.videoBitrateLabel->setText(QString("%1").arg((unsigned int)avg));
	ui.videoSizeLabel->setText(QString("%1").arg(videoSize));

	// Bpp
	bpp = avg * 1000000.;  // kbit->bit + compensate for fps1000

	// Fetch info from filter
	if(videoProcessMode())
		picSize = getPictureSize();
	else
		picSize = info.width * info.height;

	bpp = (bpp / picSize) / info.fps1000;

	ui.bppLabel->setText(QString("%1").arg(bpp, 0, 'g', 3));
}

unsigned int calculatorDialog::getPictureSize(void)
{
	AVDMGenericVideoStream *last = getLastVideoFilter();

	return last->getInfo()->width * last->getInfo()->height;
}

unsigned int calculatorDialog::videoSize(void)
{
	return atoi(ui.videoSizeLabel->text().toUtf8().constData());
}

unsigned int calculatorDialog::videoBitrate(void)
{
	return atoi(ui.videoBitrateLabel->text().toUtf8().constData());
}

void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate)
{
	if (!avifileinfo)
		return;

	calculatorDialog dialog(qtLastRegisteredDialog());
	qtRegisterDialog(&dialog);

	if (dialog.exec() == QDialog::Accepted)
		videoCodecSetFinalSize(dialog.videoSize());

	qtUnregisterDialog(&dialog);

	*sizeInMeg = dialog.videoSize();
	*avgBitrate = dialog.videoBitrate();
}
