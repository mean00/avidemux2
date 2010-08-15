adm=Avidemux()
gui=Gui()
editor=Editor()
file="/work/samples/avi/2mn.avi"
if(adm.loadVideo(file)==0):
   displayError("Cannot load file")
if(adm.loadVideo(file)==0):
   displayError("Cannot load file")
editor.dumpAllSegments()

