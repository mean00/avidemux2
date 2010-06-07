adm=Avidemux()
gui=Gui()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
#adm.audioCodec("Lame",128)
if(0==adm.saveBmp("/tmp/foo.bmp")):
   throw("cannot save bmp")
gui.displayInfo("Ok","Done")

