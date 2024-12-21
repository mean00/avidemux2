#
# --  QT6 --
IF(QT6_FOUND)
  find_package(Qt6 REQUIRED COMPONENTS Widgets)
  FUNCTION(ADM_QT_WRAP_UI in)
    SET(INLIST ${ARGN})
    QT6_WRAP_UI(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    MESSAGE(STATUS "QT6 UI:${INLIST} -> ${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_WRAP_CPP in)
    SET(INLIST ${ARGN})
    QT6_WRAP_CPP(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    #SET( plop ${${in}})
    #MESSAGE(STATUS "CPP:${INLIST} -> ${in}=${plop}")
    MESSAGE(STATUS "QT6 CPP:${INLIST} -> ${in}=${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_ADD_RESOURCES in)
    SET(INLIST ${ARGN})
    QT6_ADD_RESOURCES(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
  ENDFUNCTION()
  # -- QT5 --
ELSEIF(QT5_FOUND)
  FUNCTION(ADM_QT_WRAP_UI in)
    SET(INLIST ${ARGN})
    QT5_WRAP_UI(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    MESSAGE(STATUS "QT5 UI:${INLIST} -> ${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_WRAP_CPP in)
    SET(INLIST ${ARGN})
    QT5_WRAP_CPP(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    #SET( plop ${${in}})
    #MESSAGE(STATUS "CPP:${INLIST} -> ${in}=${plop}")
    MESSAGE(STATUS "QT5 CPP:${INLIST} -> ${in}=${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_ADD_RESOURCES in)
    SET(INLIST ${ARGN})
    QT5_ADD_RESOURCES(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
  ENDFUNCTION()

  #--QT4--
ELSEIF(QT4_FOUND)
  #//---------
  FUNCTION(ADM_QT_WRAP_UI in)
    SET(INLIST ${ARGN})
    QT4_WRAP_UI(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    MESSAGE(STATUS "UI:${INLIST} -> ${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_WRAP_CPP in)
    SET(INLIST ${ARGN})
    QT4_WRAP_CPP(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
    #SET( plop ${${in}})
    #MESSAGE(STATUS "CPP:${INLIST} -> ${in}=${plop}")
    MESSAGE(STATUS "QT4 CPP:${INLIST} -> ${in}=${out}")
  ENDFUNCTION()
  #
  #
  #
  FUNCTION(ADM_QT_ADD_RESOURCES in)
    SET(INLIST ${ARGN})
    QT4_ADD_RESOURCES(out ${INLIST})
    SET(${in} ${out} PARENT_SCOPE)
  ENDFUNCTION()
ENDIF()

