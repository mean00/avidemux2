adm=Avidemux()
editor=Editor()
gui=Gui()
if(0==adm.loadVideo("/work/samples/avi/3mn.avi")):
   throw("cannot load file")
lastOk=0
duration=editor.getVideoDuration()
print("================= SCANNING=====================")
for i in range(0,10000000):
        f=editor.nextFrame()
        if (f == 0):
                if(abs(lastOk-duration)<400*1000): # if we get near the end (0.4s near) it probably means ok
                    gui.displayInfo("Done","All ok")
                    return    
                errorString="Error at picture="+str(i)+", \nlast frame ok was at time ="+str(lastOk/1000000)+ "s \n total duration ="+str(duration/1000000)
                gui.displayError("Oops",str(errorString))
                return
        lastOk=editor.getPts(i)
print("================= /SCANNING=====================")
gui.displayInfo("Done","All ok")

