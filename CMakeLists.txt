cmake_minimum_required(VERSION 3.1)
PROJECT(cellml-api VERSION 1.0 LANGUAGES Fortran CXX)

option(BUILD_TESTS "${PROJECT_NAME} - Build tests" ON)
option(PACKAGE_CONFIG_DIR "Directory for package config files (relative to CMAKE_INSTALL_PREFIX)" "lib/cmake")

# This is our own package
find_package(LIBCELLML ${LIBCELLML_VERSION} REQUIRED) # gives ccgs
find_package(LibXml2 ${LIBXML2_VERSION} REQUIRED)

# Use common include directory
set(LIBCELLML_INCLUDE_DEST include/cellml)

# Probably better to not do this as it probably overrides and external value?
if( MSVC )
    add_definitions( -D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0600 )    
elseif(NOT MINGW)
    add_definitions(-DDYNAMIC_COMPILE)
endif()
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wall -Werror" W_FLAGS)
if (W_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

SET(cellml-api-src
  src/CellMLModelDefinitionF.cpp
  src/CellMLModelDefinition.cpp
  src/ccgs_required_functions.cpp
)

# need to handle the fortran code specially?

add_library(cellml_api ${cellml-api-src})
target_link_libraries(cellml_api PUBLIC ccgs xml2)
add_library(cellml_model_definition src/cellml_model_definition.f90)
target_link_libraries(cellml_model_definition LINK_INTERFACE_LIBRARIES cellml_api)

# Testing executables
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_cellml_model_definition tests/src/test_cellml_model_definition.f90)
    set_target_properties(test_cellml_model_definition PROPERTIES LINKER_LANGUAGE Fortran)
    target_link_libraries(test_cellml_model_definition
        cellml_api
        cellml_model_definition
        )
    add_test(test_cellml_model_definition test_cellml_model_definition ${CMAKE_SOURCE_DIR}/tests/input/sin_approximations_import-OCD.xml)
    find_program(DIFF_EXEC diff)
    if (DIFF_EXEC)
        add_test(check_results ${DIFF_EXEC} ${CMAKE_SOURCE_DIR}/tests/input/test_cellml_model_definition_results results)
    endif()
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
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake 
    "include(CMakeFindDependencyMacro)\r\n"
	"set(${PROJECT_NAME}_IMPORT_PREFIX \${_IMPORT_PREFIX})\r\n"
    "find_dependency(LIBCELLML ${LIBCELLML_VERSION})\r\n"
    "find_dependency(LibXml2 ${LIBXML2_VERSION})\r\n"
    "set(_IMPORT_PREFIX \${${PROJECT_NAME}_IMPORT_PREFIX})"
)
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-version.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake
    DESTINATION ${PACKAGE_CONFIG_DIR})