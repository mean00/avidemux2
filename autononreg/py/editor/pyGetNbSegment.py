adm=Avidemux()
editor=Editor()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
i=editor.nbSegments()
print("Nb Segments : "+str(i))

