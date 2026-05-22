# WonderBrush-v3 Ownership and Import Hardening Plan

> For Hermes: implement task-by-task using strict Red/Green TDD. Do not change production code without first adding and running a failing focused test.

Goal: harden WonderBrush-v3’s highest-risk ownership/refcount and file import/export paths while preserving Haiku-first semantics.

Architecture: start with a small Linux-friendly test harness for platform-independent C++ units. Use it to drive ownership fixes around `Referenceable`, `Layer`, `NotifyingList`, `CloneContext`, and edit objects. Then add importer/exporter tests for bounded parsing/formatting behavior. The existing Qt/Linux target is useful as an optional compile smoke test if Qt/qmake is installed, but it is not currently available on this NUC.

Tech stack: C++ using existing Haiku-style classes and the incomplete Qt compatibility layer where needed; `g++`/`make` for focused Linux tests; Haiku `jam` remains the final platform validation path.

---

## Testing answer: does the Linux/Qt target help?

Yes, but with limits.

The theoretical Linux/Qt target helps as a compile smoke test and may catch accidental breakage in shared model/render/import code. It does not prove Haiku runtime behavior, and the README explicitly says the Qt port is incomplete/broken.

Current NUC state:

- Available: `g++`, `make`
- Missing: `qmake`, `qmake6`, `qmake-qt5`, `jam`
- No existing test files were found.

Therefore the practical TDD strategy is:

1. Create small focused command-line C++ tests under `tests/`.
2. Compile only the minimal source subset required for each bug.
3. Use the Qt compatibility headers only when unavoidable.
4. Add optional qmake/Qt smoke instructions, but do not depend on them initially.
5. Keep Haiku/jam validation as a later manual or CI step on Haiku.

---

## Phase 0: Test harness and contributor context

### Task 0.1: Add `AGENTS.md`

Objective: document Haiku-first context, strict TDD, and local test strategy.

Files:
- Create: `AGENTS.md`

Steps:
1. Write `AGENTS.md` with project context and Red/Green TDD requirements.
2. Verify it exists: `test -f AGENTS.md`.
3. Commit: `docs: add contributor context for hardening branch`.

Status: done in working tree; commit pending.

### Task 0.2: Add minimal local test harness

Objective: provide a Linux-friendly `make -C tests` entry point before touching production code.

Files:
- Create: `tests/Makefile`
- Create: `tests/support/Test.h`
- Create: `tests/ownership/ReferenceableLifecycleTest.cpp`

RED:
- Add a test executable that asserts `Referenceable` starts with one reference and that `Reference<T>(obj, true)` adopts without incrementing.
- Run: `make -C tests ownership-referenceable`
- Expected initial failure before harness exists: no target/file.

GREEN:
- Add harness and compile the test with:
  - `src/support/Referenceable.cpp`
  - include paths for `src/support` and `src/platform/qt/system/include`
- Run: `make -C tests ownership-referenceable`
- Expected: pass.

REFACTOR:
- Keep harness tiny and dependency-free.

Commit: `test: add focused C++ ownership test harness`

---

## Phase 1: Ownership/refcount fixes

### Task 1.1: Prove `CopyCloneContext` adopts freshly cloned resources

Objective: catch the extra-reference leak in `CopyCloneContext::Clone()`.

Files:
- Test: `tests/ownership/CopyCloneContextTest.cpp`
- Modify: `src/model/CopyCloneContext.cpp`

RED:
- Create a minimal `BaseObject` test double whose destructor increments a counter and whose `Clone()` returns a new object with initial refcount 1.
- Call `CopyCloneContext::Clone(original)` and release the returned `BaseObjectRef`.
- Assert the clone is destroyed when the returned reference goes out of scope.
- Run: `make -C tests ownership-copy-clone-context`
- Expected: FAIL because clone still has an extra reference.

GREEN:
- Change `CopyCloneContext::Clone()` to construct the returned reference with already-owned/adopt semantics, likely `BaseObjectRef(object->Clone(*this), true)`.
- Run focused test; expected PASS.
- Run all local tests: `make -C tests`.

Commit: `fix: adopt cloned resources in CopyCloneContext`

### Task 1.2: Prove `Layer::CloneObjects()` releases the creator reference after insertion

Objective: catch leaks when cloned layer children are inserted into the target layer.

Files:
- Test: `tests/ownership/LayerCloneObjectsTest.cpp`
- Modify: `src/model/objects/Layer.cpp`

RED:
- Add a minimal cloneable `Object`/`Layer` fixture if feasible with existing model classes.
- Clone a layer containing one child object.
- Destroy/release the cloned layer.
- Assert the cloned child destructor ran.
- Run: `make -C tests ownership-layer-clone-objects`
- Expected: FAIL due to one extra reference after `target->AddObject(clone)`.

GREEN:
- After successful `target->AddObject(clone)`, release the initial clone reference with `clone->RemoveReference()` or introduce an explicit adopt helper.
- On insertion failure, continue to delete/release correctly.
- Run focused test and all local tests.

Commit: `fix: release creator reference after cloning layer objects`

### Task 1.3: Prove `Document::Clone()` does not leak global resource clones

Objective: catch the same AddObject/initial-reference mismatch for global resources.

Files:
- Test: `tests/ownership/DocumentCloneResourcesTest.cpp`
- Modify: `src/model/document/Document.cpp`

RED:
- Create a document/global-resource fixture with one cloneable resource.
- Clone the document/resource list.
- Release clone references and assert cloned resource destructor ran.
- Run: `make -C tests ownership-document-clone-resources`
- Expected: FAIL from extra reference.

GREEN:
- After `target.AddObject(clone)` succeeds, release the clone creator reference.
- Run focused and all local tests.

Commit: `fix: release creator reference after cloning document resources`

### Task 1.4: Make remove APIs safe against dangling return values

Objective: avoid APIs that return a raw pointer after potentially deleting it.

Files:
- Test: `tests/ownership/RemoveObjectReturnTest.cpp`
- Modify: `src/model/objects/Layer.h`
- Modify: `src/model/objects/Layer.cpp`
- Modify: `src/model/document/NotifyingList.h`
- Modify call sites found by compiler/search.

RED:
- Add tests that remove an object whose only owner is the container.
- Assert removal result does not expose a raw pointer that may be dead. Prefer changing API to `bool RemoveObject(...)` for non-retaining removal.
- Run focused test; expected FAIL or compile failure against old API.

GREEN:
- Change removal API to return `bool` where callers only need success.
- If any caller needs the object, add an explicit retaining function, e.g. `Reference<Object> RemoveObjectRetaining(...)`.
- Run focused and all local tests.

Commit: `fix: avoid dangling pointers from remove APIs`

### Task 1.5: Guard duplicate insertion into `Layer`

Objective: make `Layer::AddObject()` match `NotifyingList::AddObject()` behavior and avoid duplicate raw object entries.

Files:
- Test: `tests/ownership/LayerDuplicateInsertTest.cpp`
- Modify: `src/model/objects/Layer.cpp`

RED:
- Insert the same object into a layer twice.
- Assert second insertion fails and object count remains one.
- Run focused test; expected FAIL because current `Layer::AddObject()` allows duplicates.

GREEN:
- Add `fObjects.HasItem(object)` guard before insertion.
- Run focused and all local tests.

Commit: `fix: reject duplicate objects in layers`

---

## Phase 2: Undo/redo safety

### Task 2.1: Initialize all `MoveObjectsEdit` position slots

Objective: remove undefined behavior from memcmp over uninitialized `PositionInfo` entries.

Files:
- Test: `tests/edits/MoveObjectsEditTest.cpp`
- Modify: `src/edits/MoveObjectsEdit.cpp`

RED:
- Build a nested-layer move scenario where a selected child is ignored because its parent layer is also selected.
- Run under normal test and, if available, with `-fsanitize=address,undefined`.
- Expected: FAIL or sanitizer warning before fix.

GREEN:
- Value-initialize all `PositionInfo` arrays.
- Avoid `memcmp` over structs if padding exists; compare fields explicitly.
- Run focused and all local tests.

Commit: `fix: initialize MoveObjectsEdit positions`

### Task 2.2: Add rollback on partial add failure

Objective: make multi-object add/move edit operations transactional.

Files:
- Test: `tests/edits/AddObjectRollbackTest.cpp`
- Modify: `src/edits/AddObjectsEdit.h`
- Modify: `src/edits/MoveObjectsEdit.cpp`

RED:
- Use a test layer or injection point that fails an insertion after the first object succeeds.
- Assert the original object tree is restored.
- Expected: FAIL due to partial mutation.

GREEN:
- Track successful insertions and roll them back on failure.
- Run focused and all local tests.

Commit: `fix: roll back partial edit insertions`

---

## Phase 3: Import/export hardening

### Task 3.1: Harden SVG error formatting

Objective: replace unsafe `sprintf`/`vsprintf` in SVG import error paths.

Files:
- Test: `tests/import_export/SVGErrorFormattingTest.cpp`
- Modify: `src/import_export/svg/SVGImporter.cpp`
- Modify: `src/import_export/svg/SVGParser.cpp`
- Modify: `src/import_export/svg/SVGException.h`

RED:
- Create long path/error-string inputs.
- Assert formatting is bounded and null-terminated.
- Expected: FAIL or sanitizer failure with existing code.

GREEN:
- Use `snprintf`/`vsnprintf` or dynamic string storage.
- Run focused and all local tests.

Commit: `fix: bound SVG importer error formatting`

### Task 3.2: Harden SVG `url(...)` parsing

Objective: stop out-of-bounds reads for malformed URL references.

Files:
- Test: `tests/import_export/SVGUrlParsingTest.cpp`
- Modify: `src/import_export/svg/SVGParser.cpp`
- Possibly modify: `src/import_export/svg/DocumentBuilder.*`

RED:
- Test malformed inputs: `url(foo`, `url(nohash)`, `url(#unterminated`, empty string.
- Expected: FAIL or sanitizer warning.

GREEN:
- Parse within string bounds; return false/error on malformed URLs.
- Replace fixed 64-byte URL buffers in `DocumentBuilder` with dynamic strings or bounded copy.
- Run focused and all local tests.

Commit: `fix: validate SVG URL references`

### Task 3.3: Harden flat-icon buffer movement

Objective: make `LittleEndianBuffer` safe for zero-length reads, skip bounds, and realloc failure.

Files:
- Test: `tests/icon/LittleEndianBufferTest.cpp`
- Modify: `src/icon/flat_icon/LittleEndianBuffer.cpp`
- Modify: `src/icon/flat_icon/LittleEndianBuffer.h` if needed

RED:
- Test `Read(..., 0)`, `Skip()` past end, and write-growth failure if injectable.
- Expected: FAIL for unchecked skip/zero read behavior.

GREEN:
- Add bounds checks.
- Use temp pointer for `realloc`.
- Return status rather than silently moving beyond buffer.
- Run focused and all local tests.

Commit: `fix: validate flat icon buffer bounds`

### Task 3.4: Restore atomic export semantics

Objective: avoid direct destructive overwrite during export.

Files:
- Test: `tests/import_export/ExporterAtomicSaveTest.cpp`
- Modify: `src/import_export/Exporter.cpp`

RED:
- Simulate an export failure after opening/writing temp output.
- Assert original target file content remains unchanged.
- Expected: FAIL because current code opens target with `B_ERASE_FILE`.

GREEN:
- Write to temp file in same directory, then rename/replace only after successful export.
- Run focused and all local tests.

Commit: `fix: save exports atomically`

---

## Verification checkpoints

After each task:

1. Focused test fails before production change.
2. Focused test passes after production change.
3. `make -C tests` passes.
4. `git status --short` contains only intended files before commit.
5. Commit with conventional message.

Optional broader checks when tools exist:

- Linux/Qt smoke: `qmake wonderbrush2.pro && make -j2`
- Haiku smoke: `jam -q -j2`
- Haiku debug: `DEBUG=1 jam -q -j2`

---

## First implementation slice

Start with Phase 0.2, then Task 1.1. Do not attempt the larger Layer/Document API changes until the test harness proves it can compile focused ownership tests locally.
