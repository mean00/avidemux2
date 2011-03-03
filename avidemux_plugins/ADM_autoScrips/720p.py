###############################################
#      Convert to DVD adm tinypy script
#      Mean 2011
###############################################

import ADM_resize
import ADM_image
adm=Avidemux()
finalSizeWidth=1280
finalSizeHeight=[ 720,720]
#
MP2=80
AC3=0x2000
DTS=0x2001
supported=[MP2,AC3,DTS]
############################
# Interlaced ?
############################
mnuDeint = DFMenu("Deint Method:");
mnuDeint.addItem("None")
mnuDeint.addItem("Yadiff")
mnuDeint.addItem("Vdpau")

dlgWizard = DialogFactory("Deinterlacing");
dlgWizard.addControl(mnuDeint);

res=dlgWizard.show()
if res==1:
        deint=mnuDeint.index
else:
        print("cancelled\n");
        exit(1)
#
if(deint==1):
    adm.addVideoFilter("yadif","mode=0","order=1")
    print("Yadiff")
if(deint==2):
    adm.addVideoFilter("vdpauDeint","resizeToggle=False","deintMode=0","targetWidth=512","targetHeight=384")
    print("Vdpau")
# returns aspect ratio
ar=ADM_resize.regularResize("720p",finalSizeWidth,finalSizeHeight)
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
    
