SET(ADM_update "ADM_update${QT_LIBRARY_EXTENSION}6")
#ADD_DEFINITIONS( ${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(../private_inc)
INCLUDE_DIRECTORIES(${QT_HEADERS_DIR})

ADM_QT_WRAP_CPP(${ADM_update}_source  ADM_update.cpp ../private_inc/ADM_updateImpl.h )


ADD_LIBRARY( ${ADM_update} STATIC ADM_update.cpp ${${ADM_update}_source})

