CC       ?= gcc
CXX      ?= g++
PKGS     ?=
BUILD_DIR ?= build
TARGET   ?= $(notdir $(CURDIR))

C_SOURCES   := $(wildcard *.c src/*.c)
CXX_SOURCES := $(wildcard *.cc *.cpp *.cxx src/*.cc src/*.cpp src/*.cxx)
SOURCES     := $(C_SOURCES) $(CXX_SOURCES)
OBJECTS     := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(basename $(SOURCES))))
DEPS        := $(OBJECTS:.o=.d)
LEGACY_OBJECTS := $(addsuffix .o,$(basename $(SOURCES)))
LEGACY_DEPS    := $(LEGACY_OBJECTS:.o=.d)
BINARY      := $(BUILD_DIR)/$(TARGET)

PKG_CFLAGS := $(shell pkg-config --cflags $(PKGS) 2>/dev/null)
PKG_LIBS   := $(shell pkg-config --libs $(PKGS) 2>/dev/null)
LINKER     := $(if $(CXX_SOURCES),$(CXX),$(CC))

CPPFLAGS += -Iinclude $(PKG_CFLAGS)
CFLAGS   += -m64 -MMD -MP
CXXFLAGS += -m64 -MMD -MP
LDFLAGS  += -m64
LDLIBS   += $(PKG_LIBS)

.PHONY: all clean
all: $(BINARY)

$(BINARY): $(OBJECTS)
	@mkdir -p $(@D)
	$(LINKER) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cc
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.cxx
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(LEGACY_OBJECTS) $(LEGACY_DEPS)

-include $(DEPS)
