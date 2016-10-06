//------------------------------------
/**
 * \fn ctor
 * @param context
 * @param bufferSize
 */
ADM_vaEncodingBuffer::ADM_vaEncodingBuffer(ADM_vaEncodingContext *context,int bufferSize)
{
        int xError;
        CHECK_ERROR(vaCreateBuffer(ADM_coreLibVA::display,context->contextId,VAEncCodedBufferType,
                                   bufferSize, 1, NULL, &bufferId));
        if(xError)
        {
            ADM_warning("Cannot create encoding buffer\n");
            bufferId=VA_INVALID;
        }

        
}
/**
 * \fn dtor
 */
ADM_vaEncodingBuffer::~ADM_vaEncodingBuffer()
{
        if(bufferId!=VA_INVALID)
        {
              vaDestroyBuffer(ADM_coreLibVA::display,bufferId);
              bufferId=VA_INVALID;
        }
        
}

/**
 * \fn readBuffers
 * @param bufferId
 * @param maxSize
 * @param to
 * @param sizeOut
 * @return 
 */
bool   ADM_vaEncodingBuffer::readBuffers(int maxSize, uint8_t *to, uint32_t *sizeOut)
{
    int xError;
    CHECK_WORKING(false);
    VACodedBufferSegment *buf_list = NULL;    
    
    *sizeOut=0;
    if(bufferId==VA_INVALID)
    {
        ADM_warning("Using invalid encoding buffer\n");
        return false;
    }
    CHECK_ERROR(vaMapBuffer(ADM_coreLibVA::display,bufferId,(void **)(&buf_list)));
    if(xError)
    {
        ADM_warning("Cannot read buffer\n");
        return false;
    }
    while (buf_list) 
    {
        if(*sizeOut+buf_list->size>maxSize)
        {
            ADM_warning("Overflow\n");
            ADM_assert(0);
        }
        int round=buf_list->size;
        memcpy(to, buf_list->buf, round);
        to+=round;
        *sizeOut+=round;
        buf_list = (VACodedBufferSegment *)buf_list->next;
    }
    CHECK_ERROR(vaUnmapBuffer(ADM_coreLibVA::display,bufferId));
    return true;
}
/**
 * \fn ctor
 */
ADM_vaEncodingContext::ADM_vaEncodingContext()
{
    contextId=VA_INVALID;
    internalSurface[0]=NULL;
    internalSurface[1]=NULL;
    extraData=NULL;
    firstPic=false;
}
/**
 * \fn dtor
 */

ADM_vaEncodingContext::~ADM_vaEncodingContext()
{
    int xError;
    CHECK_WORKING();
    if(contextId!=VA_INVALID)
    {
        CHECK_ERROR(vaDestroyContext(ADM_coreLibVA::display,contextId));
        contextId=VA_INVALID;
    }
    for(int i=0;i<2;i++)
        if(internalSurface[i])
        {
            delete internalSurface[i];
            internalSurface[i]=NULL;
        }
    if(extraData)            
    {
        delete [] extraData;
        extraData=NULL;
    }
}
/**
 * \fn createExtraData
 * @return 
 */
bool        ADM_vaEncodingContext::createExtraData()
{
        int xError;
        CHECK_WORKING(false);
#if 0
        
        VAEncSequenceParameterBufferH264 seq_h264 = {0};
        VABufferID seq_param_buf;
            
            seq_h264.level_idc = 30;
            seq_h264.picture_width_in_mbs = width16/16;
            seq_h264.picture_height_in_mbs = width16/16;
            seq_h264.bits_per_second = 8000000; // bps
            seq_h264.frame_rate = 30; // fps
            seq_h264.initial_qp = 20;
            seq_h264.min_qp = 1;
            seq_h264.basic_unit_size = 0;
            seq_h264.intra_period = 50;
            
            CHECK_ERROR(vaCreateBuffer(ADM_coreLibVA::display, contextId,
                                       VAEncSequenceParameterBufferType,
                                       sizeof(seq_h264),1,&seq_h264,&seq_param_buf));
            if(xError) return false;
            CHECK_ERROR (vaRenderPicture(ADM_coreLibVA::display, contextId, &seq_param_buf, 1));
            if(xError) return false;
            return true;
#endif
            return false;

}
/**
 * \fn encode
 * @param src
 * @param out
 * @param encodingBuffer
 * @return 
 */
bool        ADM_vaEncodingContext::encode(ADM_vaSurface *src, ADMBitstream *out,ADM_vaEncodingBuffer *encodingBuffer)
{
#if 0
        int xError;

        CHECK_WORKING(false);

        CHECK_ERROR(vaBeginPicture(ADM_coreLibVA::display, contextId, src->surface));
        if(xError) return false;

        if(firstPic)
        {
            firstPic=false;
            if(!createExtraData())
            {
                ADM_warning("Cannot create SPS\n");
                return false;
            }
        }
        VAEncPictureParameterBufferH264 pic_h264;
        VABufferID                      pic_param_buf;
        VAEncSliceParameterBuffer       slice_h264;
        VABufferID                      slice_param_buf;
#if 0
        pic_h264.reference_picture = internalSurface[toggle]->surface;
        pic_h264.reconstructed_picture= internalSurface[1^toggle]->surface;
        pic_h264.coded_buf = encodingBuffer->bufferId;

        pic_h264.picture_width = width16;
        pic_h264.picture_height = height16;
        pic_h264.last_picture = 0; // FIXME
#endif                
        CHECK_ERROR(vaCreateBuffer(ADM_coreLibVA::display, contextId,VAEncPictureParameterBufferType,
                                   sizeof(pic_h264),1,&pic_h264,&pic_param_buf));
        if(xError) return false;
 

        CHECK_ERROR(vaRenderPicture(ADM_coreLibVA::display, contextId, &pic_param_buf, 1));
        if(xError) return false;
        
        slice_h264.start_row_number = 0;
        slice_h264.slice_height = height16/16; /* Measured by MB */
        slice_h264.slice_flags.bits.is_intra = 1;
        slice_h264.slice_flags.bits.disable_deblocking_filter_idc = 0;
        CHECK_ERROR(vaCreateBuffer(ADM_coreLibVA::display, contextId,VAEncSliceParameterBufferType,
                                   sizeof(slice_h264),1,&slice_h264,&slice_param_buf));
        if(xError) return false;
        
        CHECK_ERROR(vaRenderPicture(ADM_coreLibVA::display, contextId, &slice_param_buf, 1));
        if(xError) return false;
        
        CHECK_ERROR(vaEndPicture(ADM_coreLibVA::display, contextId));
        if(xError) return false;

        CHECK_ERROR(vaSyncSurface(ADM_coreLibVA::display, src->surface));
        if(xError) return false;
        VASurfaceStatus surface_status ;
        CHECK_ERROR(vaQuerySurfaceStatus(ADM_coreLibVA::display, src->surface,&surface_status));
        if(xError) return false;
        
        //frame_skipped = (surface_status & VASurfaceSkipped);

        //save_coded_buf(coded_buf[codedbuf_idx], i, frame_skipped);
        bool r=encodingBuffer->readBuffers(out->bufferSize,out->data,&(out->len));
        if(!r)
        {
            ADM_warning("Cannot read buffer\n");
            return false;
        }
        out->dts=ADM_NO_PTS;
        out->pts=ADM_NO_PTS;
        out->flags=AVI_KEY_FRAME;
#endif
        return true;   
}


/**
 * \fn init
 * @param width
 * @param height
 * @param surfaceCount
 * @param surfaces
 * @return 
 */
 bool        ADM_vaEncodingContext::init(int width, int height, int surfaceCount, ADM_vaSurface **surfaces)
 {
    int xError;
    CHECK_WORKING(false);
    if(false==ADM_coreLibVAEnc::encoders::vaH264.enabled)
    {
        ADM_warning("H264 encoding not supported\n");
        return false;
    }
     width16=(width+15)&~15;
     height16=(height+15)&~15;
     
     internalSurface[0]=new ADM_vaSurface(width16,height16);
     internalSurface[1]=new ADM_vaSurface(width16,height16);
     if(!internalSurface[0] || !internalSurface[1])
     {
         ADM_warning("Cannot allocate internal surface\n");
         return false;
     }
     
     VASurfaceID *s=new VASurfaceID[surfaceCount+2];
     for(int i=0;i<surfaceCount;i++)
         s[i]=surfaces[i]->surface;
     s[surfaceCount]=internalSurface[0]->surface;
     s[surfaceCount+1]=internalSurface[1]->surface;
     CHECK_ERROR(vaCreateContext(ADM_coreLibVA::display,  ADM_coreLibVAEnc::encoders::vaH264.configId,
                                width16, height16,
                                VA_PROGRESSIVE,
                                 s,surfaceCount+2,&contextId));
     delete [] s;
     if(xError)
     {
         ADM_warning("Cannot create encoding context\n");
         return false;
     }
     aprintf("Context created ok\n");
     return true;
 }
