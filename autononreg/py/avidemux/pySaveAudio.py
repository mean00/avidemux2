adm=Avidemux()
gui=Gui()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
#adm.audioCodec("Lame",128)
if(0==adm.saveAudio("/tmp/audio.mp3")):
   throw("cannot save audio")
gui.displayInfo("Ok","Done")

