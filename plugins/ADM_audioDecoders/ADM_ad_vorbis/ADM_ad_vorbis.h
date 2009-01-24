

typedef struct 
{

 ogg_sync_state    osync;  // sync and verify incoming physical bitstream
 ogg_stream_state  ostream;  // take physical pages, weld into a logical stream of packets
 ogg_page          opage;  // one Ogg bitstream page. Vorbis packets are inside
 ogg_packet        opacket;  // one raw packet of data for decode
 vorbis_info       vinfo;  // struct that stores all the static vorbis bitstream settings
 vorbis_comment    vcomment;  // struct that stores all the bitstream user comments
 vorbis_dsp_state  vdsp;  // central working state for the packet->PCM decoder
 vorbis_block      vblock;  // local working space for packet->PCM decode
 float		   ampscale;

 }oggVorbis;
