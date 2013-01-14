adm=Avidemux()
gui=Gui()
file="/work/samples/avi/2mn.avi"
file2="/work/samples/mkv/multitrack.mkv"

if(adm.loadVideo(file)==0):
   displayError("Cannot load file")
if(adm.audioTracksCount()!=1):
   gui.displayError("bad # of tracks ",str(1)+" vs "+str(adm.audioTracksCount()))

if(adm.loadVideo(file2)==0):
   displayError("Cannot load file")
if(adm.audioTracksCount()!=2):
   gui.displayError("bad # of tracks ",str(2)+" vs "+str(adm.audioTracksCount()))


print("All fine...")
