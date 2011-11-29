# Makefile for compiling OpenCMISS(cellml)
#
# Original by David Nickerson.
# Changes:
#	
#----------------------------------------------------------------------------------------------------------------------------------

include Makefile.common

#----------------------------------------------------------------------------------------------------------------------------------

CMAKE_OPTIONS =
CMAKE_OPTIONS += -DCMAKE_INCLUDE_PATH=$(CDA_DIR)/include
#CMAKE_OPTIONS += -DCMAKE_INCLUDE_PATH=$(COMMON_DIR)/lib
CMAKE_OPTIONS += -DCMAKE_LIBRARY_PATH=$(CDA_DIR)/lib
#CMAKE_OPTIONS += -DCMAKE_LIBRARY_PATH=$(COMMON_DIR)/lib
CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX=$(LIBOCCELLML_INSTALL_DIR)

ifeq ($(DEBUG),true)
  CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Debug
else
  CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release
endif

CMAKE_ENV =
ifeq ($(OPERATING_SYSTEM),linux)
  # need to make sure that CMake uses the compilers that we expect
  ifeq ($(COMPILER),gnu)
    CMAKE_ENV += FC=gfortran
    CMAKE_ENV += CC=gcc
    CMAKE_ENV += CXX=g++
    ifeq ($(ABI),32)
      CMAKE_ENV += CFLAGS="-m32"
      CMAKE_ENV += CXXFLAGS="-m32"
      CMAKE_ENV += FFLAGS="-m32"
      CMAKE_ENV += LDFLAGS="-m32"
    else
      CMAKE_ENV += CFLAGS="-m64"
      CMAKE_ENV += CXXFLAGS="-m64"
      CMAKE_ENV += FFLAGS="-m64"
      CMAKE_ENV += LDFLAGS="-m64"
    endif
  else
    ifeq ($(COMPILER),ibm)
      CMAKE_ENV += FC=xlf90_r
      CMAKE_ENV += CC=xlc_r
      CMAKE_ENV += CXX=xlC_r
      ifeq ($(ABI),32)
        CMAKE_ENV += CFLAGS="-q32"
        CMAKE_ENV += CXXFLAGS="-q32"
        CMAKE_ENV += FFLAGS="-q32"
        CMAKE_ENV += LDFLAGS="-q32"
      else
        CMAKE_ENV += CFLAGS="-q64"
        CMAKE_ENV += CXXFLAGS="-q64"
        CMAKE_ENV += FFLAGS="-q64"
        CMAKE_ENV += LDFLAGS="-q64"
      endif
    else
      ifeq ($(COMPILER),intel)
        CMAKE_ENV += FC=ifort
        CMAKE_ENV += CC=icc
        CMAKE_ENV += CXX=icpc
        ifeq ($(ABI),32)
          CMAKE_ENV += CFLAGS="-m32"
          CMAKE_ENV += CXXFLAGS="-m32"
          CMAKE_ENV += FFLAGS="-m32"
          CMAKE_ENV += LDFLAGS="-m32"
        else
          CMAKE_ENV += CFLAGS="-m64"
          CMAKE_ENV += CXXFLAGS="-m64"
          CMAKE_ENV += FFLAGS="-m64"
          CMAKE_ENV += LDFLAGS="-m64"
        endif
       else
        $(error invalid COMPILER specification)
      endif
    endif
  endif
  occellml_build = occellml_build_linux
endif

main: preliminaries \
	$(occellml_build)

occellml_build_linux:
	( cd $(LIBOCCELLML_BUILD_DIR) && $(CMAKE_ENV) cmake --debug-output $(CMAKE_OPTIONS) $(CURDIR) > build.log 2>&1 )
	( cd $(LIBOCCELLML_BUILD_DIR) && make >> build.log 2>&1 )
	( cd $(LIBOCCELLML_BUILD_DIR) && make install >> build.log 2>&1 )

preliminaries: $(LIBOCCELLML_BUILD_DIR)

$(LIBOCCELLML_BUILD_DIR):
	mkdir -p $@

debug opt debug64 opt64:
	$(MAKE) --no-print-directory DEBUG=$(DEBUG) ABI=$(ABI)

debug debug64: DEBUG=true
opt opt64: DEBUG=false
ifneq (,$(filter $(MACHNAME),ia64 x86_64))# ia64 or x86_64
   debug opt: ABI=64
else
   debug opt: ABI=32
endif
debug64 opt64: ABI=64

all: debug opt
all64: debug64 opt64
