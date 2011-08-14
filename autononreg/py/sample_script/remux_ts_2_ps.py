# convert all ts file in intputFolder ending with extention to PS folder with _conv.ps extension
# ...
#
ext="mpg"
inputFolder="/tmp/ts"
#
def convert(filein):
    fileout=filein+".converted.ps"
    print(filein+"=>"+fileout)
    if(0 == adm.loadVideo(filein)):
        ui.displayError("oops","cannot load "+filein)
        raise
    adm.save(fileout)
    print("Done")
    
#
# Main
#
ui=Gui()
adm=Avidemux()
adm.audioReset()
adm.audioCodec("copy",28152032)
adm.videoCodec("Copy")
adm.setContainer("ffPS","muxingType=3","acceptNonCompliant=False","muxRatekBits=30000","videoRatekBits=25000","bufferSizekBytes=500")
#
list=get_folder_content(inputFolder,ext)
if(list is None):
    raise
for i in list:
        convert(i)
print("Done")
