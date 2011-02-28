adm=Avidemux()
gui=Gui()
file="/work/samples/avi/2mn.avi"
goodFq=48000
goodChannels=2
goodEncoding=10*8+5
if(adm.loadVideo(file)==0):
   displayError("Cannot load file")


if(adm.audioFrequency!=goodFq):
   gui.displayError("fq ",str(goodFq)+" vs "+str(adm.audioFrequency))

if(adm.audioChannels!=goodChannels):
   gui.displayError("channels ",str(goodChannels)+" vs "+str(adm.audioChannels))

if(adm.audioEncoding!=goodEncoding):
   gui.displayError("encoding ",str(goodEncoding)+" vs "+str(adm.audioEncoding))

print("All fine...")
