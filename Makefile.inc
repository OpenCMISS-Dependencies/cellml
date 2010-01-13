# Makefile.inc to ease the inclusion of OpenCMISS(cellml) in OpenCMISS(cm) build process


## FIXME: why doesn't include Makefile.common load the local Makefile.common???

MAKEFLAGS = --no-builtin-rules --warn-undefined-variables

#----------------------------------------------------------------------------------------------------------------------------------


ifndef OPENCMISS_ROOT
  OPENCMISS_ROOT := ..
  MAKEINC_ROOT := $(CURDIR)/../../../../../opencmiss/cm/utils
else
  MAKEINC_ROOT := ${OPENCMISS_ROOT}/cm/utils
endif

ifndef OPENCMISSEXTRAS_ROOT
  OPENCMISSEXTRAS_ROOT := ${OPENCMISS_ROOT}/../opencmissextras
  CDA_ROOT := $(CURDIR)/../../../..
else
  CDA_ROOT := ${OPENCMISSEXTRAS_ROOT}
endif

include $(MAKEINC_ROOT)/Makefile.inc

CDA_DIR = $(CDA_ROOT)/cellml/$(LIB_ARCH_DIR)$(DEBUG_SUFFIX)$(PROF_SUFFIX)/$(COMPILER)

LIBOCCELLML_DIR = $(CURDIR)/$(LIB_ARCH_DIR)$(DEBUG_SUFFIX)$(PROF_SUFFIX)/$(COMPILER)

CELLML_LIBRARIES = -loccellml -lcellml -lccgs -lannotools -lcevas -lcuses -lmalaes -lrdf -lraptor -lrasqal -lxml2 -ldl -lstdc++

CELLML_LIB_PATH = -L$(LIBOCCELLML_DIR) -L$(CDA_DIR)/lib

CELLML_INCLUDE_PATH = -I$(LIBOCCELLML_DIR)
