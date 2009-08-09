# - Find Redland
# Find the Redland includes and libraries
# This module defines
#  Redland_DEFINITIONS, compiler switches required for using Redland.
#  Redland_LIBRARIES, the libraries needed to use Redland.
#  Redland_VERSION, the version of Redland found
#  Redland_FOUND, If false, do not try to use Redland.
# also defined, but not for general use are
#  Redland_LIBRARY, where to find Redland.

# Find all the config scripts
FIND_PROGRAM(Redland_CONFIG redland-config
        /usr/bin
        /usr/local/bin
)
FIND_PROGRAM(Redland_RAPTOR_CONFIG raptor-config
        /usr/bin
        /usr/local/bin
)
FIND_PROGRAM(Redland_RASQAL_CONFIG rasqal-config
        /usr/bin
        /usr/local/bin
)

IF(Redland_CONFIG AND Redland_RAPTOR_CONFIG AND Redland_RASQAL_CONFIG)
  MESSAGE(STATUS "Found config scripts")
  EXECUTE_PROCESS(COMMAND ${Redland_CONFIG} --cflags OUTPUT_VARIABLE Redland_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND ${Redland_RAPTOR_CONFIG} --cflags OUTPUT_VARIABLE Redland_RAPTOR_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND ${Redland_RASQAL_CONFIG} --cflags OUTPUT_VARIABLE Redland_RASQAL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  SET(Redland_DEFINITIONS
    ${Redland_CFLAGS}
    ${Redland_RAPTOR_CFLAGS}
    ${Redland_RASQAL_CFLAGS}
  )
  EXECUTE_PROCESS(COMMAND ${Redland_CONFIG} --libs OUTPUT_VARIABLE Redland_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND ${Redland_RAPTOR_CONFIG} --libs OUTPUT_VARIABLE Redland_RAPTOR_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND ${Redland_RASQAL_CONFIG} --libs OUTPUT_VARIABLE Redland_RASQAL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
  SET(Redland_LIBRARIES
    ${Redland_LIBS}
    ${Redland_RAPTOR_LIBS}
    ${Redland_RASQAL_LIBS}
  )
  EXECUTE_PROCESS(COMMAND ${Redland_CONFIG} --version OUTPUT_VARIABLE Redland_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF(Redland_CONFIG AND Redland_RAPTOR_CONFIG AND Redland_RASQAL_CONFIG)

# Sytem installed redland 1.0.7 has no cflags?
#IF (Redland_DEFINITIONS AND Redland_LIBRARIES)
IF (Redland_LIBRARIES)
   SET(Redland_FOUND TRUE)
ENDIF (Redland_LIBRARIES)
#ENDIF (Redland_DEFINITIONS AND Redland_LIBRARIES)


IF (Redland_FOUND)
   IF (NOT Redland_FIND_QUIETLY)
      MESSAGE(STATUS "Found Redland (${Redland_VERSION}): ${Redland_LIBRARIES}")
   ENDIF (NOT Redland_FIND_QUIETLY)
ELSE (Redland_FOUND)
   IF (Redland_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Redland")
   ELSE (Redland_FIND_REQUIRED)
      MESSAGE(STATUS "Redland not found")
   ENDIF (Redland_FIND_REQUIRED)
ENDIF (Redland_FOUND)
