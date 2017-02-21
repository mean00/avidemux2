adm=Avidemux()
editor=Editor()
gui=Gui()
if(0==adm.loadVideo("/work/samples/avi/3mn.avi")):
   throw("cannot load file")
lastOk=0
print("================= SCANNING=====================")
for i in range(0,10):
        f=editor.nextFrame()
        if (f == 0 or i== 17):
                errorString="Error at picture="+str(i)+", last frame ok was at time ="+str(lastOk/1000000)+ "s"
                gui.displayError("Oops",str(errorString))
                return
        lastOk=editor.getDts(i)
print("================= /SCANNING=====================")
gui.displayInfo("Done","All ok")

