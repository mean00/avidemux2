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
#
#
#
class resizer(object):
    def __init(self):
        self.width=0
        self.height=0
        self.topbottom=0
        self.leftright=0
        self.ar=ADM_image.AR_1_1
#
#
#
#
class image:
    def __init__(self):
        self.ar=0 	# See the AR_xxx above
        self.fmt=1      # NTSC/PAL/FILM
        self.width=0
        self.height=0
    def getFormat(self,fps1000):
        if((fps1000 >24800 and fps1000 < 25200) or (fps1000 >24800*2 and fps1000<25200*2)):
                return FMT_PAL
        if((fps1000 >23700 and fps1000 < 24200) or (fps1000 >23700*2 and fps1000<24200*2)):
                return FMT_FILM
        if((fps1000 >29700 and fps1000 < 30200) or (fps1000 >29700*2 and fps1000<30200*2)):
                return FMT_NTSC
        return FMT_UNKNOWN
    #
    #
    #
    def internal_resize(self, source,dest,aspectRatio,useHeightAsReference):
        sar=source.ar
        dar=dest.ar
        fmt=source.fmt
        sr_mul=aspectRatio[fmt][sar]
        dst_mul=aspectRatio[fmt][dar]
        #for f in (0,1,2):
           #for j in (0,1,2):
               #print("Ratio "+str(f)+" "+str(j)+"="+str(aspectRatio[f][j]))
        #print("fmt ="+str(fmt))
        #print("dar ="+str(dar))
        #print("source mul="+str(sr_mul))
        #print("dest   mul="+str(dst_mul))
        ar=source.width/((source.height*dst_mul)/(sr_mul))
        #print("ar   ="+str(ar))
        if(False==useHeightAsReference):
            dest.height=dest.width/ar
            #print("dest.height   ="+str(dest.height))
        else:
            dest.width=dest.height*ar
            #print("dest.width   ="+str(dest.width))
        # Round up to 16
        dest.width=(dest.width+7)&0xfffff0
        dest.height=(dest.height+7)&0xfffff0
    #
    # Return a resizer
    #
    def compute_resize(self,source,dest,wantedWidth,wantedHeight,aspectRatio):
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
        rsize=resizer()
        rsize.topbottom=topDown
        rsize.leftright=leftRight
        rsize.width=dest.width
        rsize.height=dest.height
        rsize.ar=dest.ar
        return rsize
     #
     # Apply resize
     # 
    def apply_resize(self,resize):
         adm=Avidemux()
         if(self.width!=resize.width or self.height!=resize.height): 
             adm.addVideoFilter("swscale","width="+str(resize.width),"height="+str(resize.height),"algo=1","sourceAR="+str(self.ar),"targetAR="+str(resize.ar))
         if(resize.topbottom!=0 or resize.leftright!=0):
             l=str(resize.leftright)
             t=str(resize.topbottom)
             adm.addVideoFilter("addBorder","left="+l,"right="+l,"top="+t,"bottom="+t)
#
#
#
  

  
