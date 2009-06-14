INCLUDE(vf_plugin)


SET(ADM_vf_separateFieldSRCS ADM_vidSeparateField.cpp)
ADD_LIBRARY(ADM_vf_separateField SHARED ${ADM_vf_separateFieldSRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_separateField)
INSTALL_VIDEOFILTER(ADM_vf_separateField)

SET(ADM_vf_mergeFieldSRCS ADM_vidMergeField.cpp)
ADD_LIBRARY(ADM_vf_mergeField SHARED ${ADM_vf_mergeFieldSRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_mergeField)
INSTALL_VIDEOFILTER(ADM_vf_mergeField)

SET(ADM_vf_stackFieldSRCS ADM_vidStackField.cpp)
ADD_LIBRARY(ADM_vf_stackField SHARED ${ADM_vf_stackFieldSRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_stackField)
INSTALL_VIDEOFILTER(ADM_vf_stackField)


SET(ADM_vf_hzStackFieldSRCS ADM_vidHzStackField.cpp)
ADD_LIBRARY(ADM_vf_hzStackField SHARED ${ADM_vf_hzStackFieldSRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_hzStackField)
INSTALL_VIDEOFILTER(ADM_vf_hzStackField)

SET(ADM_vf_unstackFieldSRCS ADM_vidUnstackField.cpp)
ADD_LIBRARY(ADM_vf_unstackField SHARED ${ADM_vf_unstackFieldSRCS})
INIT_VIDEOFILTER_PLUGIN(ADM_vf_unstackField)
INSTALL_VIDEOFILTER(ADM_vf_unstackField)

