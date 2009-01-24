

#include "ADM_default.h"
#include "ADM_muxer.h"
#include "ADM_dummy.h"
bool muxerDummy::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
                printf("[DummyMuxer] Opening %s\n",file);
                return true;
}

bool muxerDummy::save(void) 
{
        printf("[dummyMuxer] Save\n");
        return true;
}
bool muxerDummy::close(void) 
{
        printf("[dummyMuxer] close\n");
        return true;

}

