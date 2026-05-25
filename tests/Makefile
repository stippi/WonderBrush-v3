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

.PHONY: all build ownership-referenceable ownership-copy-clone-context ownership-layer-clone-objects ownership-document-clone-resources edits-move-objects import-export-svg-error-formatting clean

all: ownership-referenceable ownership-copy-clone-context ownership-layer-clone-objects ownership-document-clone-resources edits-move-objects import-export-svg-error-formatting

build:
	mkdir -p build/ownership build/edits build/import_export

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
	$(CXX) $(CXXFLAGS) -Wno-unused-parameter -Wno-multichar -include $(ROOT)/tests/stubs/TestCompat.h $(COMMON_INCLUDES) \
		-I$(ROOT)/src/model \
		-I$(ROOT)/src/model/document \
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

ownership-document-clone-resources: build
	$(CXX) $(CXXFLAGS) -Wno-unused-parameter -Wno-multichar -DWB_DOCUMENT_MINIMAL_TEST -include $(ROOT)/tests/stubs/TestCompat.h $(COMMON_INCLUDES) \
		-I$(ROOT)/src/model \
		-I$(ROOT)/src/model/document \
		-I$(ROOT)/src/savers \
		-I$(ROOT)/src/model/objects \
		-I$(ROOT)/src/model/fills \
		-I$(ROOT)/src/model/property \
		-I$(ROOT)/src/model/property/specific_properties \
		-I$(ROOT)/src/model/rendering \
		-I$(ROOT)/src/model/snapshots \
		-I$(ROOT)/src/edits/base \
		$(ROOT)/tests/ownership/DocumentCloneResourcesTest.cpp \
		$(ROOT)/src/model/CloneContext.cpp \
		$(ROOT)/src/model/document/Document.cpp \
		$(ROOT)/src/model/objects/Object.cpp \
		$(ROOT)/src/model/objects/Layer.cpp \
		$(ROOT)/tests/support/TransformableStubs.cpp \
		$(ROOT)/tests/support/LayerDependencyStubs.cpp \
		$(SUPPORT_SOURCES) \
		-o build/ownership/document_clone_resources
	./build/ownership/document_clone_resources

edits-move-objects: build
	$(CXX) $(CXXFLAGS) -Wno-unused-parameter -Wno-multichar -include $(ROOT)/tests/stubs/TestCompat.h $(COMMON_INCLUDES) \
		-I$(ROOT)/src/model \
		-I$(ROOT)/src/model/document \
		-I$(ROOT)/src/model/objects \
		-I$(ROOT)/src/model/fills \
		-I$(ROOT)/src/model/property \
		-I$(ROOT)/src/model/property/specific_properties \
		-I$(ROOT)/src/model/rendering \
		-I$(ROOT)/src/model/snapshots \
		-I$(ROOT)/src/edits \
		-I$(ROOT)/src/edits/base \
		$(ROOT)/tests/edits/MoveObjectsEditTest.cpp \
		$(ROOT)/src/model/CloneContext.cpp \
		$(ROOT)/src/model/objects/Object.cpp \
		$(ROOT)/src/model/objects/Layer.cpp \
		$(ROOT)/src/edits/MoveObjectsEdit.cpp \
		$(ROOT)/tests/support/TransformableStubs.cpp \
		$(ROOT)/tests/support/LayerDependencyStubs.cpp \
		$(SUPPORT_SOURCES) \
		-o build/edits/move_objects
	./build/edits/move_objects

import-export-svg-error-formatting: build
	$(CXX) $(CXXFLAGS) -fsanitize=address $(COMMON_INCLUDES) \
		-I$(ROOT)/src/import_export/svg \
		$(ROOT)/tests/import_export/SVGErrorFormattingTest.cpp \
		-o build/import_export/svg_error_formatting
	./build/import_export/svg_error_formatting

clean:
	rm -rf build
