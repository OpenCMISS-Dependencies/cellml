cmake_minimum_required(VERSION 2.6)

PROJECT(liboccellml)

ENABLE_LANGUAGE(Fortran)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${liboccellml_SOURCE_DIR})

FIND_PACKAGE(CellML REQUIRED)
FIND_PACKAGE(CCGS REQUIRED)
FIND_PACKAGE(Redland REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED QUIET)

# Probably better to not do this as it probably overrides and external value?
SET (CMAKE_CXX_FLAGS "-Wall -Werror ${LIBXML2_DEFINITIONS} ${Redland_DEFINITIONS}")

INCLUDE_DIRECTORIES(
  ${CELLML_INCLUDE_DIR}
  ${CCGS_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
)

SET(liboccellml_SRCS
  src/CellMLModelDefinitionF.cpp
  src/cellml_model_definition.f90
  src/CellMLModelDefinition.cpp
  src/RDFGraph.cpp
  src/SimulationDescription.cpp
  src/ccgs_required_functions.cpp
)

ADD_LIBRARY(occellml STATIC ${liboccellml_SRCS})
