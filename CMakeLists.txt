cmake_minimum_required(VERSION 3.1)
PROJECT(cellml-api VERSION 1.1 LANGUAGES CXX Fortran)

option(BUILD_TESTS "${PROJECT_NAME} - Build tests" ON)
set(PACKAGE_CONFIG_DIR "lib/cmake" CACHE STRING "Directory for package config files (relative to CMAKE_INSTALL_PREFIX)")
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fortran)

# Is this the best place for this?
# FIXME: Need to get this to work with DYNAMIC_COMPILE
option(CELLML_USE_CSIM "${PROJECT_NAME} - Use CSim to handle CellML models." ON)

# This is our own package
if(CELLML_USE_CSIM)
    find_package(CSim ${CSIM_VERSION} CONFIG REQUIRED)
else()
    find_package(LIBCELLML ${LIBCELLML_VERSION} REQUIRED) # gives ccgs
    find_package(LibXml2 ${LIBXML2_VERSION} REQUIRED)
endif()

# Use common include directory
set(LIBCELLML_INCLUDE_DEST include)

# Probably better to not do this as it probably overrides and external value?
if( MSVC )
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0600)
elseif(NOT MINGW)
    add_definitions(-DDYNAMIC_COMPILE)
endif()
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wall" W_FLAGS)
if (W_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

SET(cellml-api-src
  src/CellMLModelDefinitionF.cpp
  src/CellMLModelDefinition.cpp
  src/ccgs_required_functions.cpp
)

add_library(cellml_api ${cellml-api-src})
if (CELLML_USE_CSIM)
    target_compile_definitions(cellml_api PUBLIC $<BUILD_INTERFACE:CELLML_USE_CSIM>)
    target_link_libraries(cellml_api PUBLIC csim)
else()
    target_link_libraries(cellml_api PUBLIC ccgs xml2)
endif()
add_library(cellml_model_definition src/cellml_model_definition.f90)
target_link_libraries(cellml_model_definition INTERFACE cellml_api)

# Testing executables
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_cellml_model_definition tests/src/test_cellml_model_definition.f90)
    # Intel compiler 2017.4.196 requires us to explicitly state the linker language, otherwise it uses the CXX linker
    # which then has an undefined symbol 'main' error.
    set_target_properties(test_cellml_model_definition PROPERTIES LINKER_LANGUAGE Fortran)
    target_link_libraries(test_cellml_model_definition
        cellml_api
        cellml_model_definition
        )
    # for now, add a simple test that just checks we can load a model (with some imports)
    add_test(test_cellml_model_definition test_cellml_model_definition ${CMAKE_SOURCE_DIR}/tests/input/noble-1962/experiments/free-running.xml)
    #find_program(DIFF_EXEC diff)
    #if (DIFF_EXEC)
    #    add_test(check_results ${DIFF_EXEC} ${CMAKE_SOURCE_DIR}/tests/input/test_cellml_model_definition_results results)
    #endif()
endif()

# Install part
install(TARGETS cellml_model_definition 
    EXPORT cellml-config
    DESTINATION lib
    INCLUDES DESTINATION ${LIBCELLML_INCLUDE_DEST})
install(DIRECTORY ${CMAKE_Fortran_MODULE_DIRECTORY}/
    DESTINATION ${LIBCELLML_INCLUDE_DEST}
    FILES_MATCHING PATTERN "cellml_model_definition.mod"
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
	"set(${PROJECT_NAME}_IMPORT_PREFIX \${_IMPORT_PREFIX})\r\n")
if (CELLML_USE_CSIM)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake	
        "set(CSIM_FIND_SYSTEM ${CSIM_FIND_SYSTEM})\r\nfind_dependency(CSim ${CSIM_VERSION})\r\n"
    )
else()
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake	
        "set(LIBCELLML_FIND_SYSTEM ${LIBCELLML_FIND_SYSTEM})\r\nfind_dependency(LIBCELLML ${LIBCELLML_VERSION})\r\n"
        "set(LIBXML2_FIND_SYSTEM ${LIBXML2_FIND_SYSTEM})\r\nfind_dependency(LibXml2 ${LIBXML2_VERSION})\r\n"
    )
endif()
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake
    "set(_IMPORT_PREFIX \${${PROJECT_NAME}_IMPORT_PREFIX})"
)
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-version.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cellml-config-dependencies.cmake
    DESTINATION ${PACKAGE_CONFIG_DIR})
