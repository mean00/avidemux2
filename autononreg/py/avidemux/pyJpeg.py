adm=Avidemux()
gui=Gui()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
#adm.audioCodec("Lame",128)
if(0==adm.saveJpeg("/tmp/foo.jpeg")):
   throw("cannot save jpeg")
gui.displayInfo("Ok","Done")

