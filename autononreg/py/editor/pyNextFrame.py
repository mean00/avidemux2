adm=Avidemux()
editor=Editor()
if(0==adm.loadVideo("/work/samples/avi/3mn.avi")):
   throw("cannot load file")
lastOk=0
print("================= SCANNING=====================")
for i in range(0,10):
        f=editor.nextFrame()
        if (f == 0 or i== 7):
                errorString="Error at picture="+str(i)+" at time ="+str(lastOk)
                print errorString
                #+str(i)+", last ok was "+str(lastOk)
                #throw(errorString)
        #lastOk=editor.getDts()
print("================= /SCANNING=====================")
print("Done  "+str(lastOk))

