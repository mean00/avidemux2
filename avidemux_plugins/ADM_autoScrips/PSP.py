###############################################
#      Convert to SVCD adm tinypy script
#      Mean 2011
###############################################

import ADM_imageInfo
import ADM_image
finalSizeWidth=480               # Start with DVD target, we'll adjust later
finalSizeHeight=[272,272,272]
#
#
adm=Avidemux()
##########################
# Compute resize...
##########################
source=ADM_image.image()
dest=ADM_image.image()
desc=""
true_fmt=ADM_imageInfo.get_video_format(desc)
if(true_fmt is None):
    raise
fmt=true_fmt
if(true_fmt==ADM_image.FMT_FILM):
    fmt=ADM_image.FMT_NTSC
source.width=adm.getWidth()
source.height=adm.getHeight()
source.fmt=fmt
dest.fmt=fmt
print("Format : "+str(fmt))
dest.width=16
dest.height=16
#*****************************
# Dialog...
#*****************************
#
mnuResolution = DFMenu("Resolution:");
mnuSourceRatio = DFMenu("Source Aspect Ratio:");
mnuSourceRatio.addItem("1:1")
mnuSourceRatio.addItem("4:3")
mnuSourceRatio.addItem("16:9")

dlgWizard = DialogFactory("auto PSP ("+desc+")");
dlgWizard.addControl(mnuSourceRatio);
res=dlgWizard.show()
if res!=1:
    exit()
source.ar=mnuSourceRatio.index
dest.ar=1
resizer=source.compute_resize(source,dest,finalSizeWidth,finalSizeHeight,ADM_imageInfo.aspectRatio)
if(resizer is None):
    exit()
print("Resize to "+str(resizer.width)+"x"+str(resizer.height))
source.apply_resize(resizer)
############################
# Handle audio....
############################
adm.audioClearTracks()
adm.audioAddTrack(0)
adm.audioSetResample(0,48000)
adm.audioSetMixer(0,"STEREO")
adm.audioCodec(0,"Faac","bitrate=160")
##################################
#  Video
##################################
adm.videoCodec("x264", "general.params=CBR=500", "general.threads=99", "general.fast_first_pass=True", "level=30", "vui.sar_height=1", "vui.sar_width=1",
"MaxRefFrames=2", "MinIdr=0", "MaxIdr=250", "i_scenecut_threshold=40", "intra_refresh=False", "MaxBFrame=2", "i_bframe_adaptive=0",
"i_bframe_bias=0", "i_bframe_pyramid=0", "b_deblocking_filter=False", "i_deblocking_filter_alphac0=0", "i_deblocking_filter_beta=0", "cabac=True",
"interlaced=False", "constrained_intra=False", "tff=True", "fake_interlaced=False", "analyze.b_8x8=False", "analyze.b_i4x4=True", "analyze.b_i8x8=False", "analyze.b_p8x8=True", "analyze.b_p16x16=False",
"analyze.b_b16x16=False", "analyze.weighted_pred=2", "analyze.weighted_bipred=True", "analyze.direct_mv_pred=3",
"analyze.chroma_offset=0", "analyze.me_method=0", "analyze.me_range=16", "analyze.mv_range=-1", "analyze.mv_range_thread=-1", "analyze.subpel_refine=7", "analyze.chroma_me=True", "analyze.mixed_references=True",
"analyze.trellis=1", "analyze.psy_rd=1.000000", "analyze.psy_trellis=0.000000", "analyze.fast_pskip=True", "analyze.dct_decimate=True",
"analyze.noise_reduction=0", "analyze.psy=True", "analyze.intra_luma=11", "analyze.inter_luma=21", "ratecontrol.rc_method=0",
"ratecontrol.qp_constant=0", "ratecontrol.qp_min=0", "ratecontrol.qp_max=32", "ratecontrol.qp_step=4", "ratecontrol.bitrate=0", "ratecontrol.rate_tolerance=1.000000", "ratecontrol.vbv_max_bitrate=0",
"ratecontrol.vbv_buffer_size=0", "ratecontrol.vbv_buffer_init=0", "ratecontrol.ip_factor=1.000000", "ratecontrol.pb_factor=1.000000", "ratecontrol.aq_mode=1",
"ratecontrol.aq_strength=1.000000", "ratecontrol.mb_tree=True", "ratecontrol.lookahead=40")

###################################
# Container = Mpeg PS/DVD
###################################
#
#
adm.setContainer("MP4", "muxerType=1", "useAlternateMp3Tag=True")
    
