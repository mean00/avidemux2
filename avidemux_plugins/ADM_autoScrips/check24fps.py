###############################################
# Check if a video is rather NTSC or film
###############################################

adm=Avidemux()
editor=Editor()
gui=Gui()
#
duration=editor.getVideoDuration()
frame=0
lastDts=-1
count=0
dts=-1
#
ntsc=0
film=0
pal=0
other=0
maxfailure=40 # if we have more than failure to get DTS, stop
while True:
    dts=editor.getDts(frame)
    frame=frame+1
    count=count+1
    if(dts!=-1):
        if(lastDts==-1):
            lastDts=dts
            count=0
            continue
        deltaTime=dts-lastDts
        deltaTime/=count
        lastDts=dts
        count=0
        if(deltaTime>41000 and deltaTime<42000):
            film=film+1
        elif(deltaTime>33000 and deltaTime<34000):
            ntsc=ntsc+1
        elif(deltaTime>39000 and deltaTime<41000):
            pal=pal+1
        else:
            other=other+1
    else: # Dts=-1, if we are more than at 90% of the movie it is probably the end...
        if(count>(maxfailure)):
            break
# end while
total=ntsc+film+other+pal
if(total==0):
    gui.displayInfo("Not enough info to check...")      
    exit(0)
    
pal=round(0.4+(100*pal)/total)
ntsc=round(0.4+(100*ntsc)/total)
film=round((100*film)/total)
other=round(0.4+(100*other)/total)
gui.displayInfo("Framerate check","Pal ="+str(pal)+"%\nFilm="+str(film)+"%\n Ntsc="+str(ntsc)+"%\nOther="+str(other)+"%")      
#

    
