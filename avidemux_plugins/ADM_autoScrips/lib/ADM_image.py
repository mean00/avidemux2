# Describe an image
#  mean (c) 2011
#
FMT_NTSC=0
FMT_PAL=1
FMT_FILM=2
FMT_UNKNOWN=-3

AR_1_1=0
AR_4_3=1
AR_16_9=2
class image:
    def __init__(self):
        self.ar=0 	# See the AR_xxx above
        self.fmt=1      # NTSC/PAL/FILM
        self.width=0
        self.height=0
    def getFormat(self,fps1000):
        if(fps1000 >24800 and fps1000 < 25200):
                return FMT_PAL
        if(fps1000 >23700 and fps1000 < 24200):
                return FMT_FILM
        if(fps1000 >29700 and fps1000 < 30200):
                return FMT_NTSC
        return FMT_UNKNOWN
    def internal_resize(self, source,dest,aspectRatio,useHeightAsReference):
        sar=source.ar
        dar=dest.ar
        fmt=source.fmt
        sr_mul=aspectRatio[fmt][sar]
        dst_mul=aspectRatio[fmt][dar]
        #print("source mul="+str(sr_mul))
        #print("dest   mul="+str(dst_mul))
        ar=source.width/(source.height/(sr_mul*dst_mul))
        if(False==useHeightAsReference):
            dest.height=dest.width/ar
        else:
            dest.width=dest.height*ar
        # Round up to 16
        dest.width=(dest.width+7)&0xfffff0
        dest.height=(dest.height+7)&0xfffff0
    def video_resize(self,source,dest,wantedWidth,wantedHeight,aspectRatio):
        dest.width=wantedWidth
        dest.height=16
        dest.fmt=source.fmt
        self.internal_resize(source,dest,aspectRatio,False) # Try width=720 first
        limit=wantedHeight[source.fmt]
        if(dest.height>limit):
            dest.height=limit # Force height, compute width
            self.internal_resize(source,dest,aspectRatio,True)
        # Now add black borders
        leftRight=(wantedWidth-dest.width)/2
        topDown=(wantedHeight[source.fmt]-dest.height)/2
        print("x ="+str(dest.width)+", y="+str(dest.height))
        print("top=down="+str(topDown)+" left=right="+str(leftRight))
        # Add the filters 
       	adm=Avidemux()  
        adm.addVideoFilter("swscale","width="+str(dest.width),"height="+str(dest.height),"algo=1","sourceAR="+str(source.ar),"targetAR="+str(dest.ar))
        adm.addVideoFilter("addBorder","left="+str(leftRight),"right="+str(leftRight),"top="+str(topDown),"bottom="+str(topDown))

  

  
