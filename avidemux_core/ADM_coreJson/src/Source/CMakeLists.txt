SET(ADM_LIB ADM_json)

SET(${ADM_LIB}_SRCS 
internalJSONNode.cpp
JSON_Base64.cpp
JSONChildren.cpp
JSONDebug.cpp
JSONIterators.cpp
JSONMemory.cpp
JSONNode.cpp
JSONNode_Mutex.cpp
JSONStream.cpp
JSONValidator.cpp
JSONWorker.cpp
JSONWriter.cpp
libjson.cpp
)
ADD_DEFINITIONS(-DNDEBUG)
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
IF (UNIX)
	ADD_TARGET_CFLAGS(${ADM_LIB} -fPIC)
ENDIF (UNIX)
# EOF
