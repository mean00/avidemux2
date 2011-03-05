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

def get_video_format(desc):
    adm=Avidemux()
    ##########################
    # Compute resize...
    ##########################
    source=ADM_image.image()
    fps=adm.getFps1000()
    fmt=source.getFormat(fps)
    print("Fps    : "+str(fps))
    print("Format : "+str(fmt))
    if(fmt==ADM_image.FMT_UNKNOWN):
        return None
    desc="NTSC"
    if(fmt==ADM_image.FMT_FILM):
        desc="FILM"
    if(fmt==ADM_image.FMT_PAL):
        desc="PAL"
    print("Format : "+str(fmt))
    return fmt
