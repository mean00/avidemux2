
SET(ADM_xvba_SRCS
ADM_coreXvba.cpp
fglrx.cpp
)	
#*************************************************
#*************************************************
ADM_ADD_SHARED_LIBRARY(ADM_coreXvba6 ${ADM_xvba_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ADM_coreXvba/include)
TARGET_LINK_LIBRARIES(ADM_coreXvba6 ADM_core6 ADM_coreUI6 X11 Xext)
ADM_INSTALL_LIB(ADM_coreXvba6)
