# simple tinypy script that creates a MKV/AC3 file from input
#  a PAL DVB capture

adm=Avidemux()
gui=Gui()
#** Audio **
adm.audioReset()
adm.audioCodec("LavAC3",192)
#** Muxer **
adm.setContainer("MKV")
# Do we need to resize ?
width=adm.getWidth()
height=adm.getHeight()
fps=adm.getFps1000()
aspectW=adm.getPARWidth()
aspectH=adm.getPARHeight()
print('Aspect: ',aspectW,', ',aspectH,'\n')
#
aspect=(aspectW,aspectH)
ar=0
if aspect == (1,1):
        ar=0
elif aspect == (16,15):
        ar=1
elif aspect == (64,45):
        ar=2
else:
        print('Cannot guess aspect ratio')
####################################################
# Create a display menu to select Aspect ratio
# And resize type 576p, 720p, ...
####################################################
listOfWidth=[720,1280]
mnuSourceRatio = DFMenu("Source Aspect Ratio:");
mnuSourceRatio.addItem("1:1")
mnuSourceRatio.addItem("4:3")
mnuSourceRatio.addItem("16:9")
mnuSourceRatio.index=ar

mnuResize = DFMenu("Video to create :");
mnuResize.addItem("720 pixels")
mnuResize.addItem("1280 pixels")

toggleDeint = DFToggle("Deinterlace:");

dlgWizard = DialogFactory("Automkv Source Aspect Ratio "); 
dlgWizard.addControl(mnuSourceRatio);  
dlgWizard.addControl(mnuResize);  
dlgWizard.addControl(toggleDeint);  

res=dlgWizard.show()                                                                                                                        
if res==1: 
        ar=mnuSourceRatio.index
        rsz=listOfWidth[mnuResize.index]
        deinterlace=toggleDeint.value
else:   
        return
#################################################################
#  We have resize width, get resize Height from aspect ratio
#################################################################
if(ar==0) : # 1:1
        newHeight=rsz
elif(ar==1): # 4:3
        newHeight=(rsz*3)/4
elif(ar==2): # 16:9
        newHeight=(rsz*9)/16
# roundup newHeight to closer multiple of 16
newHeight=newHeight>>4
newHeight=newHeight<<4
print("The end ar:",ar,"Width :",rsz," height:",newHeight)
# if deinterlace is set do it with vdpau
if(deinterlace):
        str1="targetWidth="+str(rsz)
        str2="targetHeight="+str(newHeight)
        adm.addVideoFilter("vdpauDeint","resizeToggle=True","deintMode=2",str1,str2)
#else plain resize
else:
        str1="width="+str(rsz)
        str2="height="+str(newHeight)
        adm.addVideoFilter("swscale",str1,str2,"algo=2","sourceAR=1","targetAR=1")
       
# Set video codec 
adm.videoCodec("x264","params=AQ=18","MaxRefFrames=2","MinIdr=10","MaxIdr=150","threads=99","_8x8=True","_8x8P=True","_8x8B=True","_4x4=True" ,"_8x8I=True","_4x4I=True","MaxBFrame=2","profile=30","CABAC=True","Trellis=True")                       

# popup user
popupString="All set for "+str(rsz)+" wide video, x264/AC3/MKV."
if(deinterlace):
        popupString=popupString+"\nDeinterlace is on (vdpau)."
gui.displayInfo("Ok",popupString)
