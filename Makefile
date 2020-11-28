
OS ?= LINUX
TARGET := Enhanced_Cloudscapes
XPVER ?=11


################################
# LINUX
################################

ifeq ($(OS),LINUX)
ifeq ($(XPVER),11)
BUILDDIR	:=	.
else ifeq ($(XPVER), 10)
BUILDDIR	:=	.
endif
SRC_BASE	:=	.
ifeq ($(TARGET), Enhanced_Cloudscapes)
SOURCES = \
	src/simulator_objects.cpp \
	src/dataref_helpers.cpp \
	src/enhanced_cloudscapes.cpp \
	src/opengl_helpers.cpp \
	src/plugin_objects.cpp \
	src/post_processing_program.cpp \
	src/rendering_program.cpp 
	 

LIBS = -lGLEW  
endif

	
INCLUDES = \
	-I$(SRC_BASE)/SDK/CHeaders/XPLM \
	-I$(SRC_BASE)/include \
	-I$(SRC_BASE)/SDK/CHeaders/Widgets \
	-I$(SRC_BASE)/XLua/LuaSDK/include 
	
	
ifeq ($(XPVER),11)
DEFINES = -DXPLM200=1 -DXPLM210=1 -DAPL=0 -DLIN=1 -DXPLM300=1 -DXPLM301=1 -DXPLM302=1 -std=c++11 -DXP11=1 -DXPLM303=1 -DXPLM_DEPRECATED=1
else ifeq ($(XPVER), 10)
DEFINES = -DXPLM200=1 -DXPLM210=1 -DAPL=0 -DIBM=0 -DLIN=1 -std=c++11
endif
############################################################################


VPATH = $(SRC_BASE)
	
CSOURCES	:= $(filter %.c, $(SOURCES))
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))

CDEPS		:= $(patsubst %.c, $(BUILDDIR)/obj32/%.cdep, $(CSOURCES))
CXXDEPS		:= $(patsubst %.cpp, $(BUILDDIR)/obj32/%.cppdep, $(CXXSOURCES))
COBJECTS	:= $(patsubst %.c, $(BUILDDIR)/obj32/%.o, $(CSOURCES))
CXXOBJECTS	:= $(patsubst %.cpp, $(BUILDDIR)/obj32/%.o, $(CXXSOURCES))
ALL_DEPS	:= $(sort $(CDEPS) $(CXXDEPS))
ALL_OBJECTS	:= $(sort $(COBJECTS) $(CXXOBJECTS))

CDEPS64			:= $(patsubst %.c, $(BUILDDIR)/obj64/%.cdep, $(CSOURCES))
CXXDEPS64		:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.cppdep, $(CXXSOURCES))
COBJECTS64		:= $(patsubst %.c, $(BUILDDIR)/obj64/%.o, $(CSOURCES))
CXXOBJECTS64	:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.o, $(CXXSOURCES))
ALL_DEPS64		:= $(sort $(CDEPS64) $(CXXDEPS64))
ALL_OBJECTS64	:= $(sort $(COBJECTS64) $(CXXOBJECTS64))

CFLAGS := $(DEFINES) $(INCLUDES) -fPIC -fvisibility=hidden -Wall 


# Phony directive tells make that these are "virtual" targets, even if a file named "clean" exists.
.PHONY: all clean $(TARGET)
# Secondary tells make that the .o files are to be kept - they are secondary derivatives, not just
# temporary build products.
.SECONDARY: $(ALL_OBJECTS) $(ALL_OBJECTS64) $(ALL_DEPS)



# Target rules - these just induce the right .xpl files.

$(TARGET): $(BUILDDIR)/$(TARGET)/64/lin.xpl

	

$(BUILDDIR)/$(TARGET)/64/lin.xpl: $(ALL_OBJECTS64)
	@echo Linking $@
	mkdir -p $(dir $@)
	gcc -m64 -static-libgcc -shared -O2 -Wl,--version-script=exports.txt -o $@ $(ALL_OBJECTS64) $(LIBS)
	rm -rf $(BUILDDIR)/obj64
	

# Compiler rules

# What does this do?  It creates a dependency file where the affected
# files are BOTH the .o itself and the cdep we will output.  The result
# goes in the cdep.  Thus:
# - if the .c itself is touched, we remake the .o and the cdep, as expected.
# - If any header file listed in the cdep turd is changed, rebuild the .o.

$(BUILDDIR)/obj64/%.o : %.c
	mkdir -p $(dir $@)
	g++ $(CFLAGS) -m64 -c $< -o $@
	g++ $(CFLAGS) -MM -MT $@ -o $(@:.o=.cdep) $<

$(BUILDDIR)/obj64/%.o : %.cpp
	mkdir -p $(dir $@)
	g++ $(CFLAGS) -m64 -c $< -o $@
	g++ $(CFLAGS) -MM -MT $@ -o $(@:.o=.cppdep) $<

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)/$(TARGET)/obj64/
	rm -rf $(BUILDDIR)/$(TARGET)/64/
# Include any dependency turds, but don't error out if they don't exist.
# On the first build, every .c is dirty anyway.  On future builds, if the
# .c changes, it is rebuilt (as is its dep) so who cares if dependencies
# are stale.  If the .c is the same but a header has changed, this 
# declares the header to be changed.  If a primary header includes a 
# utility header and the primary header is changed, the dependency
# needs a rebuild because EVERY header is included.  And if the secondary
# header is changed, the primary header had it before (and is unchanged)
# so that is in the dependency file too.
-include $(ALL_DEPS)
-include $(ALL_DEPS64)
endif

