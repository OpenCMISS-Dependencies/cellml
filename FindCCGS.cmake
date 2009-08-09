# - Find CCGS
# Find the CellML API C Code Generation Service includes and library
# This module defines
#  CCGS_INCLUDE_DIR, where to find CCGS header files.
#  CCGS_LIBRARIES, the libraries needed to use the CCGS.
#  CCGS_FOUND, If false, do not try to use the CCGS.
# also defined, but not for general use are
#  CCGS_LIBRARY, where to find the CCGS library.

FIND_PATH(CCGS_INCLUDE_DIR IfaceCCGS.hxx 
        /usr/include/
        /usr/local/include/
)

FIND_LIBRARY(CCGS_LIBRARY ccgs
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(ANNO_TOOLS_LIBRARY annotools
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(CEVAS_LIBRARY cevas
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(CUSES_LIBRARY cuses
        /usr/lib 
        /usr/local/lib
)
FIND_LIBRARY(MALAES_LIBRARY malaes
        /usr/lib 
        /usr/local/lib
)

IF (CCGS_INCLUDE_DIR AND CCGS_LIBRARY AND ANNO_TOOLS_LIBRARY AND CEVAS_LIBRARY AND CUSES_LIBRARY AND MALAES_LIBRARY)
   SET(CCGS_LIBRARIES ${CCGS_LIBRARY} ${ANNO_TOOLS_LIBRARY} ${CEVAS_LIBRARY} ${CUSES_LIBRARY} ${MALAES_LIBRARY})
   SET(CCGS_FOUND TRUE)
ENDIF (CCGS_INCLUDE_DIR AND CCGS_LIBRARY AND ANNO_TOOLS_LIBRARY AND CEVAS_LIBRARY AND CUSES_LIBRARY AND MALAES_LIBRARY)


IF (CCGS_FOUND)
   IF (NOT CCGS_FIND_QUIETLY)
      MESSAGE(STATUS "Found CCGS: ${CCGS_LIBRARIES}")
   ENDIF (NOT CCGS_FIND_QUIETLY)
ELSE (CCGS_FOUND)
   IF (CCGS_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find CCGS")
   ELSE (CCGS_FIND_REQUIRED)
      MESSAGE(STATUS "CCGS not found")
   ENDIF (CCGS_FIND_REQUIRED)
ENDIF (CCGS_FOUND)
