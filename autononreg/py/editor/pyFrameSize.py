adm=Avidemux()
editor=Editor()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
for i in range(0,10):
        f=editor.getFrameSize(i)
        print("Frame:"+str(i)+", size="+str(f))
print("Done . ")

