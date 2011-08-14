adm=Avidemux()
editor=Editor()
if(0==adm.loadVideo("/work/samples/avi/2mn.avi")):
   throw("cannot load file")
for i in range(0,10):
        editor.printTiming(i)
print("Done . ")

