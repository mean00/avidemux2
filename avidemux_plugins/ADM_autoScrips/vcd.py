###############################################
#      Convert to DVD adm tinypy script
#      Mean 2011
###############################################

import ADM_resize
import ADM_image
finalSizeWidth=352
finalSizeHeight=[ 240,288]
#
MP2=80
supported=[MP2]
# returns aspect ratio
ar=ADM_resize.opticalDiscResize("VCD",finalSizeWidth,finalSizeHeight)
adm=Avidemux()
############################
# Handle audio....
############################
encoding=adm.audioEncoding
fq=adm.audioEncoding
channels=adm.audioChannels
reencode=False
# 1 check frequency
if(fq != 44100):
    adm.audioResample=44100
    reencode=True
if(not(encoding in supported)):
    reencode=True
if(channels!=2):
    adm.audioMixer="STEREO"
    reencode=True
if(True==reencode):
    adm.audioCodec("TwoLame",224)
##################################
#  Video
##################################
ff_ar=0
if(ar==ADM_image.AR_16_9):
    ff_ar=1
#
adm.videoCodec("ffMpeg2","params=CQ=2","lavcSettings=:version=2:MultiThreaded=2:me_method=5:_GMC=0:_4MV=0:_QPEL=0:_TRELLIS_QUANT=1:qmin=2:qmax=31:max_qdiff=3:max_b_frames=2:mpeg_quant=1:is_luma_elim_threshold=1:luma_elim_threshold=4294967294:is_chroma_elim_threshold=1:chroma_elim_threshold=4294967291:lumi_masking=0.050000:is_lumi_masking=1:dark_masking=0.010000:is_dark_masking=1:qcompress=0.500000:qblur=0.500000:"+"minBitrate=0:maxBitrate=9500:user_matrix=1:gop_size=18:interlaced=0:bff=0:widescreen="+str(ff_ar)+":"+"mb_eval=2:vratetol=8000:is_temporal_cplx_masking=0:temporal_cplx_masking=0.000000:is_spatial_cplx_masking=0:spatial_cplx_masking=0.000000:_NORMALIZE_AQP=0:use_xvid_ratecontrol=0:bufferSize=224:override_ratecontrol=0:dummy=0","matrix=0")            
###################################
# Container = Mpeg PS/DVD
###################################
adm.setContainer("ffPS","muxingType=2","acceptNonCompliant=False","muxRatekBits=11000","videoRatekBits=9800","bufferSizekBytes=224")
    
