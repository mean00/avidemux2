#PY  <- Needed to identify#
#--automatically built--
#--Project: /home/fx/earl.py

gui=Gui()
adm=Avidemux()
#** Video **
# 01 videos source 
adm.loadVideo("/work/samples/avi/2mn.avi")
#** Video Codec conf **
adm.videoCodec("x264")
r=adm.videoCodecSetProfile("x264","PSP")
if(not r):
  gui.displayInfo("Oops","Cannot load PSP profile")
r=adm.videoCodecSetProfile("x264","proutprout")
if(r):
  gui.displayInfo("Oops","Loaded a non existing profile ok")
   
#End of script
