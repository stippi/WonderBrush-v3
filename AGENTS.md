# AGENTS.md — WonderBrush-v3 contributor context

## Project context

WonderBrush-v3 is a work-in-progress rewrite of WonderBrush for Haiku. Treat Haiku as the primary platform and API target. Do not "fix" Haiku-specific APIs by replacing them with Linux abstractions unless the change is explicitly scoped to the existing Qt/Linux compatibility layer.

Important architectural pieces:

- `src/model`: document model, resources, objects, layers, properties, snapshots.
- `src/render`: asynchronous snapshot/render pipeline, render buffers, text layout/rendering.
- `src/edits`: undo/redo edits.
- `src/import_export`: document, SVG, bitmap, flat-icon and styled-text import/export.
- `src/platform/qt`: incomplete Linux/Qt compatibility layer for development and experimentation.
- `src/gui/haiku`: Haiku UI implementation.

The current local checkout is on an Ubuntu NUC, so full Haiku runtime validation is not available here. Prefer portable/unit-level tests for code touched on Linux, and document when final validation still needs Haiku.

## Development rules

1. Use Red/Green TDD for production code changes.
   - RED: write a focused failing automated test first.
   - Verify RED by running the test and seeing it fail for the expected reason.
   - GREEN: write the smallest production-code change that makes it pass.
   - Verify GREEN by running the focused test and then the relevant broader test/build command.
   - REFACTOR only after tests are green.

2. Do not change production code without a failing test first, except for pure documentation or build/test harness setup.

3. Keep changes small and commit at logical checkpoints.

4. Preserve Haiku semantics. Linux/Qt tests are a safety net, not the product definition.

5. Be careful with ownership. Many model objects inherit from `Referenceable`, which starts with refcount 1. Container APIs such as `Layer::AddObject()` and `NotifyingList::AddObject()` currently add their own reference. Always make ownership transfer explicit in code and tests.

6. Treat importers as untrusted-input code. Avoid unchecked `sprintf`, unchecked pointer arithmetic, unchecked lengths, and direct target-file overwrite on save/export.

## Local test strategy

There is no existing test suite in this checkout. The Linux/Qt target can help in two ways:

1. If Qt/qmake is installed, use the existing qmake project as a compile smoke test for the Qt port:
   - `qmake wonderbrush2.pro`
   - `make -j2`

2. For TDD on this Ubuntu NUC, add small focused C++ tests that compile with `g++` against the least possible subset of sources. Prefer this for ownership/import/export fixes because it avoids depending on the incomplete Qt GUI port.

Current local limitation: this NUC has `g++` and `make`, but no `qmake`/Qt development tools installed at the time this file was created. So initial tests should be self-contained command-line tests under `tests/` with a tiny Makefile or shell runner.

Suggested local commands after adding tests:

- Focused test: `make -C tests <target>`
- All local tests: `make -C tests`
- Optional Qt smoke test, only if qmake is available: `qmake wonderbrush2.pro && make -j2`

## Current hardening focus

Initial branch focus: ownership/refcount fixes first, then importer/exporter hardening.

High-priority areas from static review:

- `Layer::CloneObjects()` and document/resource cloning: avoid extra references after `AddObject()`.
- `CopyCloneContext`: wrap freshly cloned objects with already-owned reference semantics.
- `Layer::RemoveObject()` and `NotifyingList::RemoveObject()`: avoid returning potentially dangling raw pointers.
- Undo/redo edits: avoid unreferenced parent-layer pointers and partial rollback failures.
- SVG and flat-icon importers: replace unsafe fixed-buffer formatting and unchecked parser movement.
- Exporter: restore atomic save behavior before direct overwrite is trusted.
