CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -g

ROOT := ..
PLATFORM := $(ROOT)/src/platform/qt/system
COMMON_INCLUDES := \
	-I$(ROOT)/tests/support \
	-I$(ROOT)/tests/stubs \
	-I$(ROOT)/src/support \
	-I$(ROOT)/src/agg/include \
	-I$(PLATFORM)/include \
	-I$(PLATFORM)

SUPPORT_SOURCES := \
	$(ROOT)/src/support/Referenceable.cpp

.PHONY: all ownership-referenceable ownership-copy-clone-context ownership-layer-clone-objects clean

all: ownership-referenceable ownership-copy-clone-context ownership-layer-clone-objects

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

ownership-layer-clone-objects: build
	$(CXX) $(CXXFLAGS) -Wno-unused-parameter -include $(ROOT)/tests/stubs/TestCompat.h $(COMMON_INCLUDES) \
		-I$(ROOT)/src/model \
		-I$(ROOT)/src/model/objects \
		-I$(ROOT)/src/model/fills \
		-I$(ROOT)/src/model/property \
		-I$(ROOT)/src/model/property/specific_properties \
		-I$(ROOT)/src/model/rendering \
		-I$(ROOT)/src/model/snapshots \
		$(ROOT)/tests/ownership/LayerCloneObjectsTest.cpp \
		$(ROOT)/src/model/CloneContext.cpp \
		$(ROOT)/src/model/objects/Object.cpp \
		$(ROOT)/src/model/objects/Layer.cpp \
		$(ROOT)/tests/support/TransformableStubs.cpp \
		$(ROOT)/tests/support/LayerDependencyStubs.cpp \
		$(SUPPORT_SOURCES) \
		-o build/ownership/layer_clone_objects
	./build/ownership/layer_clone_objects

clean:
	rm -rf build
