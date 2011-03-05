###############################################
#      Convert to DVD adm tinypy script
#      Mean 2011
###############################################

import ADM_imageInfo
import ADM_image
adm=Avidemux()
finalSizeWidth=1280
finalSizeHeight=[ 720,720]
#
source=ADM_image.image()
dest=ADM_image.image()
desc=""
true_fmt=ADM_imageInfo.get_video_format(desc)
if(true_fmt is None):
    raise
fmt=true_fmt
source.width=adm.getWidth()
source.height=adm.getHeight()
source.fmt=fmt
dest.fmt=fmt
dest.width=16
dest.height=16
#
MP2=80
AC3=0x2000
DTS=0x2001
supported=[MP2,AC3,DTS]
#
print("Format : "+str(fmt))
############################
# Interlaced/ AR 
############################
mnuDeint = DFMenu("Deint Method:");
mnuDeint.addItem("None")
mnuDeint.addItem("Yadiff")
mnuDeint.addItem("Vdpau")
mnuSourceRatio = DFMenu("Source Aspect Ratio:");
mnuSourceRatio.addItem("1:1")
mnuSourceRatio.addItem("4:3")
mnuSourceRatio.addItem("16:9")

dlgWizard = DialogFactory("auto 720p ("+desc+")");
dlgWizard.addControl(mnuSourceRatio);
dlgWizard.addControl(mnuDeint);
res=dlgWizard.show()
if res!=1:
    exit()
source.ar=mnuSourceRatio.index
dest.ar=ADM_image.AR_1_1
deint=mnuDeint.index
#
if(deint==1):
    adm.addVideoFilter("yadif","mode=0","order=1")
    print("Yadiff")
if(deint==2):
    adm.addVideoFilter("vdpauDeint","resizeToggle=False","deintMode=0","targetWidth=512","targetHeight=384")
    print("Vdpau")
#  Resize
resizer=source.compute_resize(source,dest,1280,[720,720,720],ADM_imageInfo.aspectRatio)
if(resizer is None):
    exit()
print("Resize to "+str(resizer.width)+"x"+str(resizer.height))
# No need to add black border
resizer.leftright=0
resizer.topbottom=0
source.apply_resize(resizer)

############################
# Handle audio....
############################
encoding=adm.audioEncoding
fq=adm.audioEncoding
channels=adm.audioChannels
reencode=False
##################################
#  Video
##################################

#
#
adm.videoCodec("x264","params=AQ=20","MaxRefFrames=2","MinIdr=10","MaxIdr=150","threads=99","_8x8=True","_8x8P=True","_8x8B=True","_4x4=True","_8x8I=True","_4x4I=True","MaxBFrame=2","profile=30","CABAC=True","Trellis=True")
###################################
# Container = Mpeg PS/DVD
###################################
adm.setContainer("MKV","forceDisplayWidth=False","displayWidth=1280")
    
