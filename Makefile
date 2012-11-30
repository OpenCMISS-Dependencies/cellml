# Makefile for compiling OpenCMISS(cellml)
#
# Original by David Nickerson.
# Changes:
#	
#----------------------------------------------------------------------------------------------------------------------------------

include Makefile.common

#----------------------------------------------------------------------------------------------------------------------------------

include $(MAKEINC_ROOT)/Makefile_Compilers.inc

CMAKE_OPTIONS = #
CMAKE_OPTIONS += -DCMAKE_PREFIX_PATH="$(CELLML_INSTALL_DIR);$(COMMON_INSTALL_DIR)"
CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX=$(LIBOCCELLML_INSTALL_DIR)

ifeq ($(DEBUG),true)
  CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Debug
else
  CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release
endif

CMAKE_ENV = #
CMAKE_ENV += FC=$(OCE_F90)
CMAKE_ENV += CC=$(OCE_CC)
CMAKE_ENV += CXX=$(OCE_CXX)
CMAKE_ENV += CFLAGS="$(OCE_CFLAGS)"
CMAKE_ENV += CXXFLAGS="$(OCE_CXXFLAGS)"
CMAKE_ENV += FFLAGS="$(OCE_F90FLAGS)"
CMAKE_ENV += LDFLAGS="$(OCE_LDFLAGS)"

#-----------------------------------------------------------------------------
# Perform build operations
#-----------------------------------------------------------------------------

main: preliminaries \
	occellml_main_build

occellml_main_build:
	rm -rf $(LIBOCCELLML_BUILD_DIR) 
	mkdir -p $(LIBOCCELLML_BUILD_DIR) 
	( cd $(LIBOCCELLML_BUILD_DIR) && $(CMAKE_ENV) cmake --debug-output $(CMAKE_OPTIONS) $(CURDIR) > build.log 2>&1 )
	( cd $(LIBOCCELLML_BUILD_DIR) && make >> build.log 2>&1 )
	( cd $(LIBOCCELLML_BUILD_DIR) && make install >> build.log 2>&1 )

#-----------------------------------------------------------------------------
# Make directories 
#-----------------------------------------------------------------------------

preliminaries: $(LIBOCCELLML_BUILD_DIR)

$(LIBOCCELLML_BUILD_DIR):
	mkdir -p $@

#-----------------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Aliases
#-----------------------------------------------------------------------------

include $(MAKEINC_ROOT)/Makefile_Aliases.inc
