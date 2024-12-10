include(admFFmpegVersion)
SET(ADM_LIB ADM_dialogQt4)

SET(
        uiFiles
        about.ui
        encoding.ui
        license.ui
        jobs.ui
        props.ui
        audioTracks.ui
        processing.ui
        working.ui
        scriptShortcutConfig.ui)

SET(headers
        Q_about.h
        Q_encoding.h
        Q_audioTracks.h
        Q_audioTrackClass.h
        Q_license.h
        Q_jobs.h
        Q_props.h
        Q_working.h
        Q_processing.h
        Q_scriptShortcutConfig.h
)

ADM_QT_WRAP_UI(${ADM_LIB}_headers ${uiFiles})
ADM_QT_WRAP_CPP(${ADM_LIB}_source ${headers})
ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  about.qrc)

ADD_LIBRARY(${ADM_LIB} STATIC )

TARGET_SOURCES(${ADM_LIB} PRIVATE ${${ADM_LIB}_headers}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource})
TARGET_SOURCES(${ADM_LIB} PRIVATE
        Q_about.cpp
        Q_license.cpp
        Q_jobs.cpp
        Q_props.cpp
        Q_working.cpp
        Q_processing.cpp
        Q_scriptShortcutConfig.cpp
        alert_qt4.cpp
        DIA_none.cpp
        Q_encoding.cpp
        Q_audioTracks.cpp
)

STRING(TIMESTAMP CURRENT_YEAR "%Y")
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE ADM_FFMPEG_VERSION=\"${FFMPEG_VERSION}\")
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE COPYRIGHT_YEAR=${CURRENT_YEAR})
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE ADM_VERSION=\"${AVIDEMUX_VERSION}\")
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/avidemux/ADM_UIs/ADM_QT4/include")
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PRIVATE "${CMAKE_SOURCE_DIR}/../common/ADM_script2/include")
TARGET_LINK_LIBRARIES(${ADM_LIB} PRIVATE  ADM_commonUI6 ADM_audioFilter6_interface ADM_editor6_interface)
