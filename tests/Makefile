CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -g

ROOT := ..
PLATFORM := $(ROOT)/src/platform/qt/system
COMMON_INCLUDES := \
	-I$(ROOT)/tests/support \
	-I$(ROOT)/tests/stubs \
	-I$(ROOT)/src/support \
	-I$(PLATFORM)/include \
	-I$(PLATFORM)

SUPPORT_SOURCES := \
	$(ROOT)/src/support/Referenceable.cpp

.PHONY: all ownership-referenceable ownership-copy-clone-context clean

all: ownership-referenceable ownership-copy-clone-context

build:
	mkdir -p build/ownership

ownership-referenceable: build
	$(CXX) $(CXXFLAGS) $(COMMON_INCLUDES) \
		$(ROOT)/tests/ownership/ReferenceableLifecycleTest.cpp \
		$(SUPPORT_SOURCES) \
		-o build/ownership/referenceable_lifecycle
	./build/ownership/referenceable_lifecycle

ownership-copy-clone-context: build
	$(CXX) $(CXXFLAGS) $(COMMON_INCLUDES) \
		-I$(ROOT)/src/model \
		-I$(ROOT)/src/model/property \
		$(ROOT)/tests/ownership/CopyCloneContextTest.cpp \
		$(ROOT)/src/model/CloneContext.cpp \
		$(ROOT)/src/model/CopyCloneContext.cpp \
		$(SUPPORT_SOURCES) \
		-o build/ownership/copy_clone_context
	./build/ownership/copy_clone_context

clean:
	rm -rf build
