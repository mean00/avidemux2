#include "ADM_assert.h"
#include "ADM_edScriptGenerator.h"
#include "ADM_videoFilters.h"
#include "ADM_videoFilterApi.h"
#include "ADM_edAudioTrackExternal.h"
#include "audioEncoderApi.h"
#include "GUI_ui.h"
#include "ADM_muxerProto.h"
#include "ADM_coreVideoFilterFunc.h"
/**
 * 
 * @param editor
 * @param scriptWriter
 */
ADM_ScriptGenerator::ADM_ScriptGenerator(IEditor *editor, IScriptWriter* scriptWriter)
{
	this->_editor = editor;
	this->_scriptWriter = scriptWriter;
}
/**
 * 
 * @param stream
 * @param type
 */
void ADM_ScriptGenerator::generateScript(std::iostream& stream,const GeneratorType &type)
{
    if (!this->_editor->getNbSegment() && type==GENERATE_ALL)
	{
        return;
	}
    init(stream);	
	this->_scriptWriter->closeVideo();
    if(type==GENERATE_ALL)
    {
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
        
        // HDR config
        printf("Scripting HDR tone mapping config\n");
        uint32_t toneMappingMethod,gamutMethod;
        float saturationAdjust, boostAdjust;
        bool adaptiveRGB;
        if(this->_editor->getHDRConfig(&toneMappingMethod, &saturationAdjust, &boostAdjust, &adaptiveRGB, &gamutMethod))
            this->_scriptWriter->setHDRConfig(toneMappingMethod, saturationAdjust, boostAdjust, adaptiveRGB, gamutMethod);
        else
            printf("No HDR tone mapping config available, skipping.\n");

        // Video codec
    }
    printf("Scripting video encoder\n");
    this->_scriptWriter->setVideoEncoder(this->_editor->getCurrentVideoEncoder());
    if(type==GENERATE_ALL)
    {
        // Video filters....        
        printf("Scripting video filters\n");
        saveVideoFilters();
  

        printf("Scripting audio tracks\n");

        this->_scriptWriter->clearAudioTracks();
        ActiveAudioTracks* activeAudioTracks = this->_editor->getPoolOfActiveAudioTrack();
        // Add external audio tracks to pool if needed
        // and set language 
        PoolOfAudioTracks *pool= this->_editor->getPoolOfAudioTrack();
        for (int i = 0; i < pool->size(); i++)
        {

            if(pool->at(i)->getTrackType()==ADM_EDAUDIO_EXTERNAL)
            {
                 std::string name=pool->at(i)->castToExternal()->getMyName();
                 this->_scriptWriter->addExternalAudioTrack(i,name.c_str());
            }
            const std::string lang=pool->at(i)->getLanguage();            
            this->_scriptWriter->setAudioPoolLanguage(i, lang.c_str());
        }

        // Active Audio now
        for (int i = 0; i < activeAudioTracks->size(); i++)
        {
            EditableAudioTrack* track = this->_editor->getEditableAudioTrackAt(i);
            this->_scriptWriter->addAudioOutput(i,ListOfAudioEncoder[track->encoderIndex],track);
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
            bool drcActive;
            int drcNormalize;
            float drcNFloor, drcAttTime, drcDecTime, drcRatio, drcThresDB;
            track->audioEncodingConfig.audioFilterGetDrcConfig(&drcActive, &drcNormalize, &drcNFloor, &drcAttTime, &drcDecTime, &drcRatio, &drcThresDB);
            this->_scriptWriter->setAudioDrc(i, drcActive, drcNormalize, drcNFloor, drcAttTime, drcDecTime, drcRatio, drcThresDB);
            
            bool eqActive;
            float eqLo, eqMd, eqHi, eqLmcut, eqMhcut;
            track->audioEncodingConfig.audioFilterGetEqConfig(&eqActive, &eqLo, &eqMd, &eqHi, &eqLmcut, &eqMhcut);
            this->_scriptWriter->setAudioEq(i, eqActive, eqLo, eqMd, eqHi, eqLmcut, eqMhcut);

            float fadeIn, fadeOut;
            bool fadeVideoFilterBridge;
            track->audioEncodingConfig.audioFilterGetFadeConfig(&fadeIn, &fadeOut, &fadeVideoFilterBridge);
            this->_scriptWriter->setAudioFade(i, fadeIn, fadeOut, fadeVideoFilterBridge);

            float chf[9];
            track->audioEncodingConfig.audioFilterGetChannelGains(chf+0, chf+1, chf+2, chf+3, chf+4, chf+5, chf+6, chf+7, chf+8);
            this->_scriptWriter->setAudioChannelGains(i, chf[0], chf[1], chf[2], chf[3], chf[4], chf[5], chf[6], chf[7], chf[8]);

            int chint[9];
            track->audioEncodingConfig.audioFilterGetChannelDelays(chint+0, chint+1, chint+2, chint+3, chint+4, chint+5, chint+6, chint+7, chint+8);
            this->_scriptWriter->setAudioChannelDelays(i, chint[0], chint[1], chint[2], chint[3], chint[4], chint[5], chint[6], chint[7], chint[8]);

            bool remapActive;
            track->audioEncodingConfig.audioFilterGetChannelRemap(&remapActive, chint+0, chint+1, chint+2, chint+3, chint+4, chint+5, chint+6, chint+7, chint+8);
            this->_scriptWriter->setAudioChannelRemap(i, remapActive, chint[0], chint[1], chint[2], chint[3], chint[4], chint[5], chint[6], chint[7], chint[8]);
            
            bool shiftEnabled=false;
            int32_t shiftValue=0;
            track->audioEncodingConfig.audioFilterGetShift(&shiftEnabled,&shiftValue);
            this->_scriptWriter->setAudioShift(i,shiftEnabled,shiftValue);
                    // Change fps?
                    FILMCONV fps = track->audioEncodingConfig.audioFilterGetFrameRate();

                    if (fps == FILMCONV_PAL2FILM || fps == FILMCONV_FILM2PAL)
                    {
                            this->_scriptWriter->stretchAudio(i, fps);
                    }
                    if (fps == FILMCONV_CUSTOM)
                    {
                        double tempo, pitch;
                        if (!track->audioEncodingConfig.audioFilterGetCustomFrameRate(&tempo, &pitch))
                        {
                            tempo = pitch = 1.0;
                        }
                        this->_scriptWriter->stretchAudioCustom(i, tempo, pitch);
                    }

                    // --------- Normalize ----------------
            ADM_GAINMode mode;
            int32_t gain, maxlevel;

            track->audioEncodingConfig.audioFilterGetNormalize(&mode, &gain, &maxlevel);

            if (mode != ADM_NO_GAIN)
            {
                            this->_scriptWriter->setAudioGain(i, mode, gain, maxlevel);
            }
        }
    }
	// -------- Muxer -----------------------
	printf("Scripting muxer\n");
	this->_scriptWriter->setMuxer(this->_editor->getCurrentMuxer());
	end();
}
/**
 * \fn saveVideoFilters
 * @param stream
 */
bool ADM_ScriptGenerator::saveVideoFilters()
{
    int nbFilter = ADM_vf_getSize();

    for (int i = 0; i < nbFilter; i++)
    {
		ADM_VideoFilterElement *element = &ADM_VideoFilters[i];
		if(!element->enabled) continue;
		this->_scriptWriter->addVideoFilter(ADM_vf_getPluginFromTag(element->tag), element);
    }
    return true;
}
/**
 * \fn init
 * @param stream
 */
bool ADM_ScriptGenerator::init(std::iostream& stream)
{
  this->_scriptWriter->connectStream(stream);
  return true;
}
/**
 * \fn end
 * @return 
 */
bool ADM_ScriptGenerator::end()
{
        this->_scriptWriter->disconnectStream();
        return true;
}
