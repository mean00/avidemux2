ADD_SUBDIRECTORY(Source)
SET(ADM_coreJSON_SRCS 
        ADM_json.cpp
)
include_directories(../include Source)
ADM_ADD_SHARED_LIBRARY(ADM_coreJSON6 ${ADM_coreJSON_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreJSON6 ADM_json ADM_core6)

ADM_INSTALL_LIB(ADM_coreJSON6)
