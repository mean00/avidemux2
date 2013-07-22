
static bool checkMarkers(uint8_t *mark)
{
    if(mark[0]==0x11 &&    mark[800]==0x22 &&    mark[1600]==0x33) return true;
    ADM_info("Markers do not check\n");
    return false;
}
static bool setMarkers(uint8_t *mark)
{
    mark[0]=0x11;
    mark[800]=0x22;
    mark[1600]=0x33;
    return true;
}
static bool resetMarkers(uint8_t *mark)
{
    mark[0]=0x4;
    mark[800]=0x5;
    mark[1600]=0x6;
    return true;
}


/**
 * \fn tryDirectUpload
 * @param title
 * @param admSurface
 * @param image
 * @param image1
 * @return 
 */
static bool tryDirect(const char *title, ADM_vaSurface &admSurface,ADMImage &image1, ADMImage &image2)
{
    setMarkers(image1.GetWritePtr(PLANAR_Y));
    resetMarkers(image2.GetWritePtr(PLANAR_Y));
    if(false==admLibVA::admImageToSurface(&image1,&admSurface))
    {
        ADM_info("Direct upload failed\n");
        return false;
    }        
    if(false==admLibVA::surfaceToAdmImage(&image2,&admSurface))
    {
        ADM_info("Direct download failed\n");
        return false;
    }
    if(!checkMarkers(image2.GetWritePtr(PLANAR_Y)))
    {
         ADM_info("Sanity check failed for direct operation\n");
         return false;
    }
    ADM_info("Direct operation works\n")       ;
    return true;
}


/**
 * \fn tryIndirectUpload
 * @param title
 * @param image
 * @param image1
 * @param image2
 * @return 
 */
static bool tryIndirectUpload(const char *title, ADM_vaSurface &admSurface,VAImage *image, ADMImage &image1)
{
    bool work=false;
    ADM_info("%s indirect upload... \n",title);
    if(!admLibVA::uploadToImage(&image1,image))
    {
        ADM_info("Upload to yv12 image failed \n");
        return false;
    }
    if(!admLibVA::imageToSurface(image,&admSurface))
    {                    
        ADM_info("image to surface failed\n");
        return false;
    }
    return true;
}
/**
 * \fn tryIndirectDownload
 * @param title
 * @param admSurface
 * @param image
 * @param image1
 * @param image2
 * @return 
 */
static bool tryIndirectDownload(const char *title, ADM_vaSurface &admSurface,VAImage *image, ADMImage &image2)
{
    bool work=false;
    ADM_info("%s indirect download... \n",title);
    if(!admLibVA::surfaceToImage(&admSurface,image))
    {
        ADM_info("Surface to image failed\n");
        return false;
    }
    if(!admLibVA::downloadFromImage(&image2,image))            
    {
           ADM_info("download from image failed\n");
           return false;
    }
    if(! checkMarkers(image2.GetReadPtr(PLANAR_Y)))
    {
        ADM_info("sanity check failed\n");
        return false;
    }
    return true;
}

/**
 * 
 * @param title
 * @param admSurface
 * @param image
 * @param image1
 * @return 
 */
static bool tryIndirect(bool isNv12, ADM_vaSurface &admSurface,ADMImage &image1, ADMImage &image2)
{
    bool r=false;
        ADM_info("Trying indirect transfer (%d)...\n",isNv12);
        VAImage          *image=NULL;
        if(isNv12)
                image=admLibVA::allocateNV12Image(640,400);
        else
                image=admLibVA::allocateYV12Image(640,400);
        if(!image)
        {
            ADM_info("Cannot allocate image\n");
            return false;
        }
        setMarkers(image1.GetReadPtr(PLANAR_Y));
        resetMarkers(image2.GetReadPtr(PLANAR_Y));
        if(false==tryIndirectUpload("-",admSurface,image, image1))
        {
            ADM_info(" indirect upload failed\n");
            goto done;
        }
         if(false==tryIndirectDownload("-",admSurface,image,image2))   
        {
            ADM_info(" indirect download failed\n");
            goto done;
        }      
        ADM_info("Works\n");
        r=true;
done:            
        admLibVA::destroyImage(image);
        image=NULL;
        return r;
}