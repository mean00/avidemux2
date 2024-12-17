SET(ADM_LIB ADM_muxerGate6)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
ADM_videoCopy.cpp
ADM_videoProcess.cpp
ADM_videoCopyFromAnnexB.cpp
ADM_videoCopyAudRemover.cpp
ADM_videoCopySeiInjector.cpp
)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_editor6_interface ADM_libavutil ADM_libavcodec)
#
