

#include "ADM_default.h"
#include "ADM_muxer.h"
#include "ADM_dummy.h"
bool muxerDummy::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
                printf("[DummyMuxer] Opening %s\n",file);
                vStream=s;
                return true;
}

bool muxerDummy::save(void) 
{
    ADM_info("[dummy] Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t   *audioBuffer;
    uint8_t   *videoBuffer;
    uint32_t  len,flags;
    uint64_t  pts,dts;
    uint32_t  written=0;
    ADMBitstream in(bufSize);
    audioBuffer=new uint8_t[10*4*8*1024];
    videoBuffer=new uint8_t[bufSize];
    in.data=videoBuffer;
    ADM_info("[dummy]avg fps=%u\n",vStream->getAvgFps1000());

    initUI("Saving dummy");
    encoding->setContainer("dummy");

    while(1)
    {
        if(false==vStream->getPacket(&in)) goto abt;
        encoding->pushVideoFrame(in.len,in.in_quantizer,in.dts);
        if(updateUI()==false)
        {  
            goto abt;
        }
        written++;
    }
abt:
    closeUI();
    delete [] videoBuffer;
    videoBuffer=NULL;
    delete [] audioBuffer;
    audioBuffer=NULL;
    ADM_info("[dummy] Wrote %d frames, nb audio streams %d\n",written,nbAStreams);
    return true;
}
bool muxerDummy::close(void) 
{
        printf("[dummyMuxer] close\n");
        return true;

}

