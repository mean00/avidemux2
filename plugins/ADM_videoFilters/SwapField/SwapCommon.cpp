
char *AVDMVideoKeepOdd::printConf( void )
{
        static char buf[50];

        sprintf((char *)buf," Keep Odd Fields");
        return buf;
}
char *AVDMVideoKeepEven::printConf( void )
{
        static char buf[50];

        sprintf((char *)buf," Keep Even Fields");
        return buf;
}
//_______________________________________________________________
AVDMVideoKeepOdd::AVDMVideoKeepOdd(
                                                                        AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
        _in=in;
        memcpy(&_info,_in->getInfo(),sizeof(_info));
        _uncompressed=new ADMImage(_info.width,_info.height);
        _info.height>>=1;

}
// ___ destructor_____________
AVDMVideoKeepOdd::~AVDMVideoKeepOdd()
{
        delete _uncompressed;
        _uncompressed=NULL;

}



uint8_t AVDMVideoKeepOdd::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{
//static Image in,out;
                if(frame>= _info.nb_frames) return 0;
                        if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;


                uint32_t w=_info.width;
                uint32_t h=_info.height;

                vidFieldKeepOdd(  w,  h, YPLANE(_uncompressed),YPLANE(data));
                data->copyInfo(_uncompressed);

      return 1;
}

uint8_t AVDMVideoKeepEven::getFrameNumberNoAlloc(uint32_t frame,
                                uint32_t *len,
                                ADMImage *data,
                                uint32_t *flags)
{
//static Image in,out;
                        if(frame>=_info.nb_frames) return 0;


                        // read uncompressed frame
                if(!_in->getFrameNumberNoAlloc(frame, len,_uncompressed,flags)) return 0;

                uint32_t w=_info.width;
                uint32_t h=_info.height;


                data->copyInfo(_uncompressed);
                vidFieldKeepEven(  w,  h, YPLANE(_uncompressed),YPLANE(data));
      return 1;
}