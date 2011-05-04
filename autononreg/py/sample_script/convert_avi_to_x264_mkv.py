# check get_folder_content function
# return the list of files with extention ext
# full path is returned !
# i.e. in the example below you will get a list
# /work/samples/avi/foo.avi
# /work/samples/avi/bar.avi
# ...
#
ext="avi"
#
def convert(filein):
    fileout=filein+".mkv"
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
adm.videoCodec("x264","general.params=AQ=20","general.threads=99","general.fast_first_pass=True","level=31","vui.sar_height=1","vui.sar_width=1","MaxRefFrames=2","MinIdr=100","MaxIdr=500","MaxBFrame=2","i_bframe_adaptative=0","i_bframe_bias=0",  \
"i_bframe_pyramid=0","b_deblocking_filter=False","i_deblocking_filter_alphac0=0","i_deblocking_filter_beta=0","cabac=True","interlaced=False","analyze.b_8x8=True","analyze.b_i4x4=False","analyze.b_i8x8=False"\
,"analyze.b_p8x8=False","analyze.b_p16x16=False","analyze.b_b16x16=False","analyze.weighted_pred=0","analyze.weighted_bipred=False","analyze.direct_mv_pred=0","analyze.chroma_offset=0","analyze.me_method=0","analyze.subpel_refine=7","analyze.chroma_me=False","analyze.mixed_references=False","analyze.trellis=1","analyze.fast_pskip=True","analyze.dct_decimate=False","analyze.noise_reduction=0","analyze.psy=True","ratecontrol.rc_method=0","ratecontrol.qp_constant=0","ratecontrol.qp_min=0","ratecontrol.qp_max=0"\
,"ratecontrol.qp_step=0","ratecontrol.bitrate=0","ratecontrol.vbv_max_bitrate=0","ratecontrol.vbv_buffer_size=0","ratecontrol.vbv_buffer_init=0","ratecontrol.ip_factor=0.000000","ratecontrol.pb_factor=0.000000","ratecontrol.aq_mode=0","ratecontrol.aq_strength=0.000000","ratecontrol.mb_tree=False","ratecontrol.lookahead=0")
adm.audioReset()
adm.audioCodec("copy",28152032)
adm.setContainer("MKV","forceDisplayWidth=False","displayWidth=1280")
#
folder=ui.dirSelect("Select avi source folder")
#folder="/work/samples/avi"
if(folder is None):
    ui.displayError("oops","no folder selected")
    raise
list=get_folder_content(folder,ext)
for i in list:
        convert(i)
print("Done")
