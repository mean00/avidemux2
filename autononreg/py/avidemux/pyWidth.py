adm=Avidemux()
gui=Gui()
file="/work/samples/avi/2mn.avi"
goodwidth=512
goodheight=384
goodfcc="DIV3"
if(adm.loadVideo(file)==0):
   displayError("Cannot load file")


if(adm.getWidth()!=goodwidth):
   gui.displayError("BadWidth ",str(goodwidth)+" vs "+str(adm.getWidth()))
if(adm.getHeight()!=goodheight):
   gui.displayError("BadWidth ",str(goodheight)+" vs "+str(adm.getHeight()))
if(adm.getVideoCodec()!=goodfcc):
   gui.displayError("BadFcc ",str(goodfcc)+" vs "+str(adm.getVideoCodec()))


