cmake_minimum_required(VERSION 3.1)
PROJECT(cellml-api VERSION 1.0 LANGUAGES Fortran CXX)

option(BUILD_TESTS "${PROJECT_NAME} - Build tests" ON)
option(PACKAGE_CONFIG_DIR "Directory for package config files (relative to CMAKE_INSTALL_PREFIX)" "lib/cmake")

# This is our own package
FIND_PACKAGE(LIBCELLML REQUIRED) # gives ccgs
FIND_PACKAGE(LibXml2 REQUIRED)

# Use common include directory
SET(LIBCELLML_INCLUDE_DEST include/cellml)

# Probably better to not do this as it probably overrides and external value?
IF( MSVC )
    ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0600 )
ELSE()
    # allow dynamic compilation
    ADD_DEFINITIONS(-DDYNAMIC_COMPILE)
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
ENDIF()

SET(cellml-api-src
  src/CellMLModelDefinitionF.cpp
  src/CellMLModelDefinition.cpp
  src/ccgs_required_functions.cpp
)

# need to handle the fortran code specially?
ADD_LIBRARY(cellml_model_definition src/cellml_model_definition.f90)
ADD_LIBRARY(cellml_api ${cellml-api-src})
target_link_libraries(cellml_api PUBLIC ccgs xml2)

# Testing executables
if(BUILD_TESTS)
    ENABLE_TESTING()
    ADD_EXECUTABLE(test_cellml_model_definition tests/src/test_cellml_model_definition.f90)
    SET_TARGET_PROPERTIES(test_cellml_model_definition PROPERTIES LINKER_LANGUAGE Fortran)
    TARGET_LINK_LIBRARIES(test_cellml_model_definition
        cellml_api
        cellml_model_definition
        )
    ADD_TEST(test_cellml_model_definition test_cellml_model_definition ${CMAKE_SOURCE_DIR}/tests/input/sin_approximations_import-OCD.xml)
    ADD_TEST(check_results diff ${CMAKE_SOURCE_DIR}/tests/input/test_cellml_model_definition_results results)
endif()

# Install part
install(TARGETS cellml_model_definition EXPORT cellml-config
    DESTINATION lib
    INCLUDES DESTINATION ${LIBCELLML_INCLUDE_DEST})
install(FILES ${CMAKE_BINARY_DIR}/cellml_model_definition.mod
	DESTINATION ${LIBCELLML_INCLUDE_DEST}
)
install(TARGETS cellml_api EXPORT cellml-config
    DESTINATION lib
    INCLUDES DESTINATION include)
install(EXPORT cellml-config DESTINATION ${PACKAGE_CONFIG_DIR})   
include(CMakePackageConfigHelpers)
WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/cellml-config-version.cmake
    COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-version.cmake DESTINATION ${PACKAGE_CONFIG_DIR})