#include "ADM_edScriptGenerator.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterApi.h"
#include "ADM_edAudioTrackExternal.h"
#include "audioEncoderApi.h"
#include "GUI_ui.h"
#include "ADM_muxerProto.h"
#include "ADM_coreVideoFilterFunc.h"

extern BVector<ADM_audioEncoder*> ListOfAudioEncoder;
extern BVector<ADM_VideoFilterElement> ADM_VideoFilters;

ADM_ScriptGenerator::ADM_ScriptGenerator(IEditor *editor, IScriptWriter* scriptWriter)
{
	this->_editor = editor;
	this->_scriptWriter = scriptWriter;
}

void ADM_ScriptGenerator::generateScript(std::iostream& stream)
{
    if (!this->_editor->getNbSegment())
	{
        return;
	}

	this->_scriptWriter->connectStream(stream);
	this->_scriptWriter->closeVideo();

	printf("Scripting video streams\n");

	for (uint32_t i = 0; i < this->_editor->getVideoCount(); i++)
    {
        const char *nm = ADM_cleanupPath(this->_editor->getRefVideo(i)->_aviheader->getMyName());

		if (i)
		{
			this->_scriptWriter->appendVideo(nm);
		}
		else
		{
			this->_scriptWriter->loadVideo(nm);
		}

        ADM_dealloc(nm);
    }

	printf("Scripting segments\n");

	this->_scriptWriter->clearSegments();

    for (uint32_t i = 0; i < this->_editor->getNbSegment(); i++)
    {
        _SEGMENT *seg = this->_editor->getSegment(i);

		this->_scriptWriter->addSegment(seg->_reference, seg->_refStartTimeUs, seg->_durationUs);
    }

	// Markers
	printf("Scripting markers\n");
	this->_scriptWriter->setMarkers(this->_editor->getMarkerAPts(), this->_editor->getMarkerBPts());

	// postproc
	printf("Scripting post-processing\n");

	uint32_t pptype, ppstrength;
	bool ppswap;

	this->_editor->getPostProc(&pptype, &ppstrength, &ppswap);

	if (pptype || ppstrength || ppswap)
	{
		this->_scriptWriter->setPostProcessing(pptype, ppstrength, ppswap);
	}

	// Video codec
	printf("Scripting video encoder\n");

	this->_scriptWriter->setVideoEncoder(this->_editor->getCurrentVideoEncoder());

	// Video filters....
	printf("Scripting video filters\n");

    int nbFilter = ADM_vf_getSize();

    for (int i = 0; i < nbFilter; i++)
    {
		ADM_VideoFilterElement *element = &ADM_VideoFilters[i];
		this->_scriptWriter->addVideoFilter(ADM_vf_getPluginFromTag(element->tag), element);
    }

	printf("Scripting audio tracks\n");

	this->_scriptWriter->clearAudioTracks();
	ActiveAudioTracks* activeAudioTracks = this->_editor->getPoolOfActiveAudioTrack();

	// Audio
    for (int i = 0; i < activeAudioTracks->size(); i++)
    {
		EditableAudioTrack* track = this->_editor->getEditableAudioTrackAt(i);

		this->_scriptWriter->addAudioOutput(i, ListOfAudioEncoder[track->encoderIndex], track);

		// Mixer
		CHANNEL_CONF channel = track->audioEncodingConfig.audioFilterGetMixer();

		if (channel != CHANNEL_INVALID)
		{
			this->_scriptWriter->setAudioMixer(i, channel);
		}

		// Resample
        uint32_t x = track->audioEncodingConfig.audioFilterGetResample();

        if (x)
		{
			this->_scriptWriter->setAudioResample(i, track->audioEncodingConfig.audioFilterGetResample());
		}
        this->_scriptWriter->setAudioDrc(i,track->audioEncodingConfig.audioFilterGetDrcMode());
		// Change fps?
		FILMCONV fps = track->audioEncodingConfig.audioFilterGetFrameRate();

		if (fps == FILMCONV_PAL2FILM || fps == FILMCONV_FILM2PAL)
		{
			this->_scriptWriter->stretchAudio(i, fps);
		}

		// --------- Normalize ----------------
        ADM_GAINMode mode;
        uint32_t gain;

        track->audioEncodingConfig.audioFilterGetNormalize(&mode, &gain);

        if (mode != ADM_NO_GAIN)
        {
			this->_scriptWriter->setAudioGain(i, mode, gain);
        }
    }

	// -------- Muxer -----------------------
	printf("Scripting muxer\n");

	this->_scriptWriter->setMuxer(this->_editor->getCurrentMuxer());

	this->_scriptWriter->disconnectStream();
}
