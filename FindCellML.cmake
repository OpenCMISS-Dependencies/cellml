# - Find CellML
# Find the CellML API includes and library
# This module defines
#  CELLML_INCLUDE_DIR, where to find CellML API header files.
#  CELLML_LIBRARIES, the libraries needed to use the CellML API.
#  CELLML_FOUND, If false, do not try to use the CellML API.
# also defined, but not for general use are
#  CELLML_LIBRARY, where to find the CellML library.

FIND_PATH(CELLML_INCLUDE_DIR IfaceCellML_APISPEC.hxx 
        /usr/include/
        /usr/local/include/
)

FIND_LIBRARY(CELLML_LIBRARY cellml
        /usr/lib 
        /usr/local/lib
) 

IF (CELLML_INCLUDE_DIR AND CELLML_LIBRARY)
   SET(CELLML_LIBRARIES ${CELLML_LIBRARY})
   SET(CELLML_FOUND TRUE)
ENDIF (CELLML_INCLUDE_DIR AND CELLML_LIBRARY)


IF (CELLML_FOUND)
   IF (NOT CellML_FIND_QUIETLY)
      MESSAGE(STATUS "Found CellML: ${CELLML_LIBRARIES}")
   ENDIF (NOT CellML_FIND_QUIETLY)
ELSE (CELLML_FOUND)
   IF (CellML_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CellML")
   ELSE (CellML_FIND_REQUIRED)
      MESSAGE(STATUS "CellML API not found")
   ENDIF (CellML_FIND_REQUIRED)
ENDIF (CELLML_FOUND)
