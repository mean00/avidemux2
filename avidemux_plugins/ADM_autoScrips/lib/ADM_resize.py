import ADM_image
# Recompute dest image source/dest 
# Taking aspect ratio into account
# Ported from js mean (c) 2011
#
aspectRatio=[
	(1.,0.888888,1.19), # NTSC 1:1 4:3 16:9
        (1.,1.066667,1.43),  # PAL  1:1 4:3 16:9
	(1.,0.888888,1.19), # FILM 1:1 4:3 16:9
]
fps_predef=[ 29970, 25000, 23976]
#
#
def opticalDiscResize(title,finalSizeWidth,finalSizeHeight):
    adm=Avidemux() 
    source=ADM_image.image()
    fps=adm.getFps1000()
    fmt=source.getFormat(fps)
    true_fmt=fmt
    print("Fps    : "+str(fps))
    print("Format : "+str(fmt))
    if(fmt==ADM_image.FMT_UNKNOWN):
        exit()
    #*****************************
    #
    #*****************************
    desc="NTSC"
    if(fmt==ADM_image.FMT_FILM):
        fmt=ADM_image.FMT_NTSC
        desc="FILM"
    if(fmt==ADM_image.FMT_PAL):
       desc="PAL"
    source.fmt=fmt
    #
    mnuResolution = DFMenu("Resolution:");
    mnuSourceRatio = DFMenu("Source Aspect Ratio:");
    mnuSourceRatio.addItem("1:1")
    mnuSourceRatio.addItem("4:3")
    mnuSourceRatio.addItem("16:9")
    mnuDestRatio = DFMenu("Destination Aspect Ratio:");
    mnuDestRatio.addItem("4:3")
    mnuDestRatio.addItem("16:9")
    
    dlgWizard = DialogFactory("auto "+str(title)+" DVD "+desc);
    dlgWizard.addControl(mnuSourceRatio);
    dlgWizard.addControl(mnuDestRatio);
    res=dlgWizard.show()
    if res!=1:
        exit()

    # Compute resize etc...


    source.ar=mnuSourceRatio.index
    source.width=adm.getWidth()
    source.height=adm.getHeight()
    dest=ADM_image.image()
    dest.ar=mnuDestRatio.index+1
    dest.width=finalSizeWidth
    dest.height=16
    dest.fmt=source.fmt
    source.video_resize(source,dest,finalSizeWidth,finalSizeHeight,aspectRatio)
    # Force fps...
    adm.addVideoFilter("resampleFps","mode=0","newFpsDen=1000","newFpsNum="+str(fps_predef[true_fmt]))
    # Done
     
