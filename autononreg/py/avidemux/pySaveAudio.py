adm=Avidemux()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
if(0==adm.saveAudio("/tmp/audio.mp3")):
   throw("cannot save audio")


