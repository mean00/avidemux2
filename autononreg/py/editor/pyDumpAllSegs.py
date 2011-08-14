adm=Avidemux()
editor=Editor()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
editor.dumpAllSegments()
print("Done . ")

