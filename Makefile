# Project Name
TARGET = PlateauTest

# Sources
CPP_SOURCES = PlateauTest.cpp
CPP_SOURCES += utilities/Utilities.cpp
CPP_SOURCES += dsp/filters/OnePoleFilters.cpp
CPP_SOURCES += dsp/delays/InterpDelay.cpp
CPP_SOURCES += Dattorro.cpp

# Library Locations
LIBDAISY_DIR = ../../libDaisy/
DAISYSP_DIR = ../../DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
