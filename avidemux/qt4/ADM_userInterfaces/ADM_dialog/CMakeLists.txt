SET(ADM_LIB ADM_dialogQt4)

SET(
        uiFiles
        about.ui
        calculator.ui
        encoding.ui
        license.ui
        jobs.ui
        props.ui  vobsub.ui  audioTracks.ui
        processing.ui
        working.ui)

SET(headers
	Q_about.h  Q_calculator.h
        Q_encoding.h
        Q_audioTracks.h Q_audioTrackClass.h
	Q_license.h  Q_jobs.h  Q_props.h  Q_vobsub.h  Q_working.h  Q_processing.h T_index_pg.h)

ADM_QT_WRAP_UI(${ADM_LIB}_headers ${uiFiles})
ADM_QT_WRAP_CPP(${ADM_LIB}_source ${headers})
ADM_QT_ADD_RESOURCES(${ADM_LIB}_resource  about.qrc)

SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS}
	${${ADM_LIB}_headers}  ${${ADM_LIB}_source}  ${${ADM_LIB}_resource}
        Q_about.cpp
        Q_license.cpp
        Q_jobs.cpp  Q_props.cpp
        Q_working.cpp
        Q_processing.cpp
        T_index_pg.cpp
        alert_qt4.cpp
        DIA_busy.cpp
        DIA_none.cpp
#DIA_xvid4.cpp  OCR_none.cpp
#Q_vobsub.cpp
#Q_calculator.cpp
        Q_encoding.cpp
        Q_audioTracks.cpp
)

ADD_DEFINITIONS(-DADM_SUBVERSION=\"${ADM_SUBVERSION}\" -DADM_VERSION=\"${AVIDEMUX_VERSION}\")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/avidemux/ADM_UIs/ADM_QT4/include")
include_directories("${CMAKE_SOURCE_DIR}/../common/ADM_script2/include")
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
