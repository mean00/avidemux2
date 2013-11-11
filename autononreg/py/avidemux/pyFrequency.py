adm=Avidemux()
gui=Gui()
file="/work/samples/avi/2mn.avi"
goodFq=48000
goodChannels=2
goodEncoding=10*8+5
goodBitrate=128
if(adm.loadVideo(file)==0):
   displayError("Cannot load file")


if(adm.audioFrequency(0)!=goodFq):
   gui.displayError("fq ",str(goodFq)+" vs "+str(adm.audioFrequency))

if(adm.audioBitrate(0)!=goodBitrate):
   gui.displayError("Bitrate ",str(goodBitrate)+" vs "+str(adm.audioBitrate(0)))

if(adm.audioChannels(0)!=goodChannels):
   gui.displayError("channels ",str(goodChannels)+" vs "+str(adm.audioChannels(0)))

if(adm.audioEncoding(0)!=goodEncoding):
   gui.displayError("encoding ",str(goodEncoding)+" vs "+str(adm.audioEncoding(0)))

print("All fine...")
