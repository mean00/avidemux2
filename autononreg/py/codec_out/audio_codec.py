#PY  <- Needed to identify#
#--automatically built--
#--Project: /home/fx/allAudio.py

adm=Avidemux()
#** Video **
# 01 videos source 
adm.loadVideo("/work/samples/avi/2mn.avi")
#01 segments
adm.clearSegments()
adm.addSegment(0,0,1281739416)
adm.markerA=0
adm.markerB=1281739416

#** Postproc **
adm.setPostProc(3,3,0)

#** Video Codec conf **
adm.videoCodec("Copy")

#** Filters **

#** Audio **
adm.audioReset()
adm.audioCodec("copy",-1075244360)

#** Muxer **
adm.setContainer("MKV","forceDisplayWidth=False","displayWidth=1280")

allcodec=["LavAAC","LavAC3","LavMP2"]
for i in allcodec:
    print str(i)
    adm.audioCodec(i,128)
    out="/tmp/audio_codec_"+i+".mkv"
    adm.save(out)

#adm.save(output)
#End of script
