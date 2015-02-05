#
#
#
FUNCTION(ADM_QT_WRAP_UI in )
       SET(INLIST ${ARGN})
       if(QT5_FOUND)
         QT5_WRAP_UI(out ${INLIST})
       ELSE(QT5_FOUND)
         QT4_WRAP_UI(out ${INLIST})
       ENDIF(QT5_FOUND)
       SET( ${in} ${out} PARENT_SCOPE)
        MESSAGE(STATUS "UI:${INLIST} -> ${out}")
ENDFUNCTION(ADM_QT_WRAP_UI in)
#
#
#
FUNCTION(ADM_QT_WRAP_CPP in )
       SET(INLIST ${ARGN})
       if(QT5_FOUND)
         QT5_WRAP_CPP(out ${INLIST})
       ELSE(QT5_FOUND)
         QT4_WRAP_CPP(out ${INLIST})
       ENDIF(QT5_FOUND)
       SET(${in} ${out} PARENT_SCOPE)
       SET( plop ${${in}})
       MESSAGE(STATUS "CPP:${INLIST} -> ${in}=${plop}")
       MESSAGE(STATUS "CPP:${INLIST} -> ${in}=${out}")
ENDFUNCTION(ADM_QT_WRAP_CPP in)
#
#
#
FUNCTION(ADM_QT_ADD_RESOURCES in)
       SET(INLIST ${ARGN})
       if(QT5_FOUND)
         QT5_ADD_RESOURCES(out ${INLIST})
       ELSE(QT5_FOUND)
         QT4_ADD_RESOURCES(out ${INLIST})
       ENDIF(QT5_FOUND)
       SET( ${in} ${out} PARENT_SCOPE)
ENDFUNCTION(ADM_QT_ADD_RESOURCES in)

