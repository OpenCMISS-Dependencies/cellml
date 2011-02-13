cmake_minimum_required(VERSION 2.8)

PROJECT(liboccellml Fortran CXX )

#ENABLE_TESTING()

#ENABLE_LANGUAGE(Fortran)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${liboccellml_SOURCE_DIR})

FIND_PACKAGE(CellML REQUIRED)
FIND_PACKAGE(CCGS REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED QUIET)

# Probably better to not do this as it probably overrides and external value?
IF( MSVC )
    ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0600 )
ELSE (MSVC )
    # allow dynamic compilation
    ADD_DEFINITIONS(-DDYNAMIC_COMPILE)
    SET (CMAKE_CXX_FLAGS "-Wall -Werror ${LIBXML2_DEFINITIONS}")
ENDIF( MSVC )

INCLUDE_DIRECTORIES(
  ${CELLML_INCLUDE_DIR}
  ${CCGS_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
)

SET(liboccellml_SRCS
  src/CellMLModelDefinitionF.cpp
  src/CellMLModelDefinition.cpp
  src/ccgs_required_functions.cpp
)

# need to handle the fortran code specially?
ADD_LIBRARY(cellml_model_definition STATIC src/cellml_model_definition.f90)
ADD_LIBRARY(occellml STATIC ${liboccellml_SRCS})

# Testing executables
#ADD_EXECUTABLE(test_cellml_model_definition tests/src/test_cellml_model_definition.f90)
#SET_TARGET_PROPERTIES(test_cellml_model_definition PROPERTIES LINKER_LANGUAGE Fortran)
#TARGET_LINK_LIBRARIES(test_cellml_model_definition
#    occellml
#    cellml_model_definition
#    ${CELLML_LIBRARIES}
#    ${CCGS_LIBRARIES}
#    ${LIBXML2_LIBRARIES}
#    -ldl -lm
##    -lstdc++
#)
#ADD_TEST(test_cellml_model_definition test_cellml_model_definition ${CMAKE_SOURCE_DIR}/tests/input/sin_approximations_import-OCD.xml)
#ADD_TEST(check_results diff ${CMAKE_SOURCE_DIR}/tests/input/test_cellml_model_definition_results results)

INSTALL(TARGETS occellml
	ARCHIVE DESTINATION lib
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/cellml_model_definition.mod
	DESTINATION include
)
