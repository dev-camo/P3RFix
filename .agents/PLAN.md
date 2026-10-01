# Replace the dumped SDK with a focused Unreal integration layer for P3RFix 1.3.0

This ExecPlan is a living document. Keep `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` current as implementation proceeds. A contributor must be able to resume the work using this file and the repository alone. There is no repository `PLANS.md` or project-local `AGENTS.md` at the time of writing; the execution and validation requirements are included here.

Execution is complete on branch `refactor/minimal-unreal-integration`; this file records preparation, implementation, automated verification, and human live-game acceptance. The user has requested removal of this file after this completed record is committed, followed by a branch push and a pull request for review. The target release is **1.3.0**, following the existing **1.2.5** release. Preparing this plan does not publish a release or claim that implementation checks have passed.

## Purpose / Big Picture

P3RFix should keep enabling the game's developer console and increasing the resolution of menu render textures while carrying only the game integration code those features need. At the preparation baseline the repository contained a generated description of much of the game, although the fix accesses only a few engine objects, fields, and functions. A smaller, project-owned interface will let maintainers understand these dependencies, review game compatibility changes, and build the fix without compiling thousands of unused engine wrappers.

The observable result is a Windows x64 `P3RFix.asi` reporting version `1.3.0` that preserves the existing console, render texture, and unrelated fix behavior. A player can enable the console, press the configured key, and use it; at higher resolutions, menu character models continue using the scaled render textures. A maintainer can trace every game memory access used by these two features to a small private directory, build both release packages, and demonstrate that the full SDK is absent from the current source tree and compiler inputs. Record source reduction and build measurements; do not promise a particular binary size or build speed improvement without measuring it.

## Progress

- [x] (2026-10-01) Confirmed that the starting working tree was clean and that `main` pointed to `e119244314f345dada02914a01a504a169bc0c16`.
- [x] (2026-10-01 05:04Z) Created and selected `refactor/minimal-unreal-integration`.
- [x] (2026-10-01 05:05Z) Inspected the build, SDK include dependencies, console implementation, render texture hooks, support code, package script, and Windows workflows.
- [x] (2026-10-01 05:05Z) Recorded the baseline: 950 SDK files, 58,516,425 bytes; 20 reachable SDK files, 13,330,488 bytes; three SDK implementation files compiled.
- [x] (2026-10-01 05:15Z) Wrote and checked the implementation and acceptance specification for release `1.3.0` in `.agents/PLAN.md`.
- [x] (2026-10-01 05:32Z) Milestone 1: Baseline and independent pruned MSVC release builds passed; retained exactly 20 reachable SDK files. Live comparison was handed to the user; final acceptance is recorded below.
- [x] (2026-10-01 05:34Z) Milestone 2: Feature callers isolated behind the public interface; temporary SDK adapter clean MSVC build and public-header-only translation unit passed. Final healthy-path acceptance is recorded below from the user's live test and supplied log.
- [x] (2026-10-01 05:35Z) Milestone 3: Minimal prototype MSVC x64 debug and release tests both passed 60/60 named checks; compiler inputs are only the fixture source and private runtime. Prototype was verified before the production switch; final tests now always call the real public implementation.
- [x] (2026-10-01 05:42Z) Milestone 4 implementation and structural verification: Production uses the minimal runtime; all SDK/obsolete support removed; both MSVC configurations build and pass 69/69 public checks; inventory and actual compiler dependency audits pass.
- [x] (2026-10-01 06:30Z) Milestone 4 live behavior acceptance: User reported the live game test looked good and authorized completion; supplied log verifies successful name reporting, console spawn and scaled render targets without integration errors. Broader observations are user-attested, not invented log facts.
- [x] (2026-10-01 05:51Z) Milestone 5 independent implementation/verification: Version defaults/examples/CI/notices updated; both 1.3.0 packages built and inspected with MSVC 2022; source/default/embedded versions verified; full diff and YAML syntax checks pass.
- [x] (2026-10-01 05:54Z) Pushed implementation commits through `5aa4bc5` to `origin/refactor/minimal-unreal-integration`; local and remote SHA matched, with a clean working tree. No main merge, release tag, or publication was attempted.
- [x] (2026-10-01 06:30Z) Milestone 5 live acceptance and completion date: Accepted the user's live-test pass and audited `/tmp/P3RFix.log`; recorded executable identity from the matching local Steam installation; dated the 1.3.0 changelog 2026-10-01. Individual scenario details and unspecified variants remain explicitly unattributed rather than fabricated.

The working checkout is Linux. Windows validation uses the isolated VM directory `C:\Users\camo\Projects\P3RFix-sdk-refactor` (baseline, pruned, then adapter/final snapshots), with Visual Studio 2022 Build Tools at `C:\BuildTools2022`, cl 14.44.35207, bundled CMake 3.31.6-msvc6, portable xmake 3.1.1, PowerShell 7.5.4, and Python 3.13.7. The VM exposes four Ryzen 7950X3D virtual processors and runs Windows 11 IoT Enterprise LTSC 10.0.26100. Baseline, pruned, adapter, and final debug/release builds passed; both final test configurations pass 69/69 checks. No game was run by the agent. The user performed the live game test, reported success, and supplied the log audited below.

## Surprises & Discoveries

The VM also has Visual Studio 2026. The unchanged packaging script's reconfiguration initially selected it despite previous project selection of VS 2022. Recovery used an isolated `XMAKE_GLOBALDIR`, `xmake g -y --vs=2022`, and a fresh `xmake f -c` configuration to remove the cached 2026 choice. Final package compiler/version assertions prove VS 2022 (cl 14.44.35207) before and after packaging. The initial 2026 output is diagnostic only and was replaced by the verified 2022 artifacts. Source/build configuration and dependencies were not changed to solve this environment issue.

During final review, the prototype's failed metadata name conversions were skipped and could return a misleading `Pending` engine lookup. The final runtime tracks conversion failures per lookup: absent target metadata after failed conversion reports `LookupFailed` with a concrete diagnostic, and failed InputSettings metadata conversion leaves the console `Enabled` with `NameUnavailable`. Tests now assert those statuses/diagnostics and prove unrelated failed names do not hide a healthy match. The final suite has 69 checks; both MSVC debug and release runs pass.

The inherited private UTF helper requires four dependent-type `typename` additions, removal of three redundant `typename` tokens, direct `<climits>`, `<iterator>`, and `<utility>` includes, and a matched diagnostic push to compile with strict Clang. These are portability corrections, not a Unicode rewrite. Its attributed implementation remains private. Supplemental Clang checks use `-fshort-wchar` only to exercise fixtures; the Windows builds establish the supported ABI.

`create_release.ps1` removes `build/package-staging/` on success. Final metadata evidence must therefore come from the Reloaded-II ZIP or a separate extraction, rather than the plan's original staged-file command. The packaging cleanup behavior stays unchanged.

Execution environment (2026-10-01 05:27Z): the user supplied an MSVC VM at `ssh camo@192.168.122.141` and authorized installing or modifying required tools there; Windows sudo is enabled and the account is an administrator without a password requirement. The SSH connection succeeds. Use an isolated checkout for measurements. The user prohibits the agent from running the game: complete independent implementation/build/package checks, then hand the baseline and final artifacts and scenarios to the user for testing. The live acceptance boundary was subsequently satisfied by the user's pass report and supplied log, as recorded in Outcomes & Retrospective.

The large dump is mostly outside the build, but the parts inside the build are still substantial. Following quoted project-local includes from `src/dllmain.cpp` and the three SDK source files reaches 20 SDK files totaling 13,330,488 bytes. The whole `src/SDK/` directory has 950 files totaling 58,516,425 bytes. Removing unreachable files therefore removes 930 files and 45,185,937 bytes, approximately 77% of the dump. These figures exclude the top-level generated support headers.

    Baseline xmake.lua compiled:
        src/SDK/Engine_functions.cpp
        src/SDK/CoreUObject_functions.cpp
        src/SDK/Basic.cpp

    Engine_functions.cpp: 97,290 lines, 3,924,301 bytes
    Engine_classes.hpp:                 2,685,418 bytes
    Engine_parameters.hpp:              4,731,166 bytes
    Engine_structs.hpp:                 1,570,540 bytes

`src/SDK.hpp`, the umbrella header for the entire dump, is not included by the build. Deleting it reduces repository clutter but will not itself improve compilation. The largest compiled wrapper is needed only for `UEngine::GetEngine()` and `UGameplayStatics::SpawnObject()` at the project's direct call sites; their supporting object and name helpers explain the remaining dependencies.

`UpdateOffsets()` scans for a `ProcessEvent` address and stores it in `SDK::Offsets::ProcessEvent`, but `UObject::ProcessEvent()` actually reads virtual table entry `SDK::Offsets::ProcessEventIdx`, currently `0x44`. A virtual table is the array of function pointers an object uses for virtual calls. The scanned function address does not control the current dispatch. Preserve the existing table dispatch during this refactor and remove the unused scan when the legacy implementation is retired.

The console's generated helpers assume some lookups always succeed. `GetDefaultObjImpl()` dereferences `StaticClass()` without checking it, and the generated `SpawnObject()` wrapper dereferences its function lookup. Offset scan failures also leave default relative offsets of zero, which can cause the SDK to interpret the executable's base address as object data or callable code. The new integration must report unavailable dependencies before those dereferences or calls, without preventing independent fixes from running.

Name conversion has a memory ownership constraint. `FName::GetRawString()` passes a thread-local `FAllocatedString(1024)` to the game's `AppendString` function and then clears its element count. The inherited container comments explicitly describe this buffer as suitable only for a function that does not reallocate it. Preserve and verify that assumption in the prototype; do not silently replace this with a game-allocated string that the fix frees using the C++ allocator.

The SDK packs `UTextureRenderTarget2D` with byte packing while explicitly aligning it to 16 bytes. Flattening that representation must preserve the observed field offsets; ordinary C++ inheritance is unnecessary for reading three fields. Compile-time layout checks verify the fix's representation, not whether a future game update still uses that representation. In-game evidence is required separately.

## Decision Log

Decision: Close live acceptance on the user's pass report and audited log, preserve exactly observed game identity, and remove the completed plan after recording it in Git history. Rationale: the user explicitly accepted the live test and requested completion/removal/PR; unspecified scenario details and game variants must remain unclaimed rather than being fabricated or requiring redundant confirmation. Date/Author: 2026-10-01, user/Codex.

Decision: Distinguish failed metadata name conversion from metadata that has not yet appeared. Rationale: the plan requires reporting scratch-buffer contract failures, and waiting twenty seconds with a missing-engine message would conceal the actual failed boundary. Only an unsuccessful lookup with encountered conversion errors is terminal; a valid match remains usable. Date/Author: 2026-10-01, Codex.

Decision: During milestone 3 compile the private console runtime tests without the temporary SDK-backed `Integration.cpp`; enable public/render tests when production switches in milestone 4. Rationale: the prototype must have zero SDK compiler dependencies; final render tests must call the shipped implementation without a duplicate test backend. Date/Author: 2026-10-01, Codex.

Decision: Initially keep the 1.3.0 changelog entry marked Unreleased while human game acceptance is outstanding; date it on the user's final acceptance. Rationale: a dated completion or release entry would imply evidence the agent is prohibited from collecting; use the actual completion date after user verification. Date/Author: 2026-10-01, Codex.

Decision: Follow the user's later execution instructions for VM setup, frequent commits, branch push, and human game testing. Rationale: those instructions supersede the preparation plan's statement that no push is included, and prohibit agent-run game checks. Do not tag, publish a release, or merge to main; push the implementation branch when ready for review. Date/Author: 2026-10-01, user/Codex.

Decision: Deliver option 3, a focused integration layer, through file pruning and minimal extraction first. Rationale: this gives reviewable intermediate states while eliminating both unused files and the enormous compiled wrapper. Moving the intact dump into another folder would not satisfy the objective. Date/Author: 2026-10-01, Codex, following the user's approved direction.

Decision: Target release `1.3.0`. Rationale: the last release in this repository is `1.2.5`, and the user specified the next version for this work. Update version-bearing build defaults, examples, release notes, and package evidence together in the final milestone. Date/Author: 2026-10-01, user/Codex.

Decision: Expose only feature operations and ordinary C++ value types in `src/unreal/Integration.hpp`. Rationale: callers should not depend on generated engine classes, reflection internals, container templates, or padding. Game-specific layouts stay private under `src/unreal/detail/`. Date/Author: 2026-10-01, Codex.

Decision: Preserve the current console construction route through the reflected `GameplayStatics.SpawnObject` function and virtual table entry `0x44`. Reflection means finding a named class or function through metadata stored by the game. Rationale: this route already works in the inherited implementation; replacing it with a newly scanned native constructor or a different dispatch mechanism would introduce a separate compatibility change. Remove the unused direct `ProcessEvent` scan rather than inventing a dependency on it. Date/Author: 2026-10-01, Codex.

Decision: Use absolute discovered addresses and explicit initialization for console runtime dependencies. Rationale: a null discovery must remain null rather than turning into the executable's base address through addition to a zero relative offset. Render target field access needs neither the object registry nor name conversion and must remain usable independently. Date/Author: 2026-10-01, Codex.

Decision: Extract only necessary container operations and retain the existing UTF conversion support privately if needed. Rationale: Unicode conversion and engine buffer ownership should not become unrelated rewrites. A small attributed helper is acceptable; the full generated SDK and general-purpose container collection are not. Date/Author: 2026-10-01, Codex.

Decision: Keep the current render scaling formula, hook signatures, retry window, configuration format, loading model, and unrelated fixes. Rationale: those behaviors are the comparison baseline. Fixing the suspicious repeated `iCurrentResX` condition, redesigning threading, or adding general game-version discovery would expand this refactor. Necessary missing-dependency guards are included because extraction exposes existing unsafe calls. Date/Author: 2026-10-01, Codex.

Decision: Use a small native test executable with synthetic object memory and fake game callbacks, plus real Windows and in-game checks. Rationale: binary layout, reflected parameter passing, and selective failure handling are meaningful correctness boundaries that compilation alone cannot prove. No new test framework or general SDK generator is required. Date/Author: 2026-10-01, Codex.

## Outcomes & Retrospective

Preparation was recorded in `6d3b9ac`. Milestone 1, committed as `73a6813`, removes 930 SDK files totaling 45,185,937 bytes plus the unused umbrella header, leaving 20 generated files totaling 13,330,488 bytes. Inventory before and after reports zero unresolved includes, and the retained contents are unchanged. The baseline and independently pruned legacy backend both compiled and linked successfully on the same MSVC VM, retaining the three original SDK source inputs and an ASI size of 1,196,544 bytes. Warm-Zydis clean fix builds took 7.0251014 seconds baseline and 7.1562145 seconds pruned; the initial cold baseline took 14.9384503 seconds. This demonstrates checkout reduction without claiming a speed or binary-size improvement. Full logs and binaries are ignored under `build/sdk-refactor/baseline-windows` and `pruned-windows`.

Milestone 2, committed as `594cc1a`, preserves the retained SDK in a guarded temporary adapter while feature code uses only `Integration.hpp`, passes absolute discovery addresses, and continues unrelated fixes on unavailable runtime. The independent adapter clean MSVC release build passed in 7.5326694 seconds with warm Zydis and produced 1,199,616 bytes. A standalone public-header-only translation unit compiled with MSVC and no SDK includes. Evidence is under `build/sdk-refactor/adapter-windows`; SDK/UC references are absent from `dllmain.cpp`. The source inventory also passed six temporary fault checks for absent required inputs, legacy paths/references, excessive budget, and unresolved includes.

Milestone 3, committed as `2975e4e`, adds five private runtime/support files totaling 75,443 bytes and the nondefault standalone fixture target. MSVC debug and release runs each observed `60/60 checks passed`, with no SDK, hooking, logging, Zydis, or game dependency. Independent byte fixtures verify guarded/pending lookup, registry holes and chunk transitions, exact spawn receiver/parameters/flags, flag restoration after a thrown fake callback, cache resets and repeated success, and Unicode/string contract handling. The extracted healthy semantics were reviewed against the retained original symbols. Logs are under `build/sdk-refactor/prototype-windows`. MSVC dependency-cache audits for baseline and adapter contain exactly the same 20 retained SDK headers/sources and zero deleted paths; evidence is `build/sdk-refactor/compiler-dependency-audit.log`.

Milestone 4, committed as `d747e51`, switches production and final tests to the same minimal implementation. All 950 SDK files, the umbrella, and four obsolete top-level support headers are absent from the current tree. Inventory reports seven integration files totaling 79,466 bytes (below 262,144), zero active legacy references, and PASS. MSVC debug and release public suites each pass 69/69 checks, including independent render/capture byte-write sentinels and conversion diagnostic regressions. Both clean DLL builds pass; debug produces 2,008,064 bytes in 5.8213837 seconds and release produces 1,200,128 bytes in 5.6545912 seconds with warm Zydis. Compared with the same-machine warm baseline, release is 3,584 bytes larger (about 0.30%); source/compiler reduction did not reduce binary size. Each final DLL compiler audit finds 11 production sources including Integration.cpp and Runtime.cpp, with zero SDK source/header or obsolete support dependencies. Evidence is under `build/sdk-refactor/final-windows`, `final-compiler-dependency-audit.log`, and `final-compiler-deps/`. Attribution comments and license copies remain; notices describe extracted private code. Live acceptance and the actual startup-version log are now recorded below. Version defaults and package checks are recorded below.


Milestone 5 independent verification is committed as `5aa4bc5`. Both version defaults are 1.3.0, examples and the changelog are updated, and CI checks minimal inventory plus debug/release tests before packaging while preserving source/version checks and tool pins. Workflow/issue YAML syntax passed; GitHub-hosted execution itself was not run. The unchanged package script ran successfully with MSVC 2022 using the isolated environment selection. An additional fresh xmake configuration without version/VS command overrides resolved defaults to fix_version=1.3.0 and vs=2022. Binary inspection finds the 1.3.0 literal and no 1.2.5 literal; the supplied live log separately confirms v1.3.0 loaded. Both archives match the final ASI (1,200,128 bytes), root configuration/notices and all license copies byte for byte; only standalone contains the verified pinned loader, and only Reloaded-II contains ModConfig.json with ModVersion=1.3.0. Archives contain no SDK, tests, tools, or plan. Artifact hash and size evidence follows; logs are under `build/sdk-refactor/package-windows`. A second local zipfile inspection also passed and `git diff --check e119244` passed.

Live-game acceptance was received on 2026-10-01: the user reported that everything looked good in the live test, supplied `/tmp/P3RFix.log`, and explicitly requested completion, plan removal, commit/push, and a PR. The log contains 88 informational entries from 2026-10-01 00:20:27 through 00:20:30, with no warning/error/critical entries. It directly verifies `P3RFix v1.3.0 loaded`, object/name discoveries, console construction with `Tilde` reporting, current resolution 5120x1440, and render target resizing from 1920x1080 to 2560x1440 at multiplier 1.3333334 after initial multiplier-1 targets. The successful key conversion satisfies the checked name-buffer contract for the observed callbacks. Config has console/render enabled, user multiplier 1, and screen percentage 100. Intro, aspect/HUD, and menu FPS hooks are logged after console creation.

Broader live behavior is accepted on the user's report. The log does not independently demonstrate actual console opening/query output, the alternate key or each settings/toggle/clamp/borderless scenario, menu image quality, capture dimensions, or baseline comparisons. No per-scenario command/result was invented. The base-content/Episode Aigis variant and renderer API were not supplied, so support is limited to the user's tested installation and no additional variants/platforms are claimed. This distinction is retained for the reviewer; it is not an outstanding request for the user to repeat already accepted testing.

Executable identity was obtained by read-only inspection of the installed Steam game, without starting it. Steam AppID 2161700, build ID 22672075; `P3R.exe`, Win64 PE machine 0x8664, resource file/product version 1.0.0.0, 366251968 bytes, PE timestamp 1775121465 matching the live log, SHA256 `88244bade1988eafd8ababd66d4c2227091c4c6c253e6455c7097b1b1e5299b8`. The installed executable is `/home/camo/.local/share/Steam/steamapps/common/P3R/P3R/Binaries/Win64/P3R.exe`; the game's prefix maps `S:` to that Steam library, matching the logged Windows path. The exact Wine/Proton tool version was not collected. The supplied log SHA256 is `4ce5a507d762bd0ed466199aea3c87f38b238863be2459de311683cb6bdaf9c6`; an ignored local copy is retained under `build/sdk-refactor/live-game/P3RFix.log`.

All implementation milestones and accepted verification requirements are complete. The final fresh inventory and baseline-to-HEAD diff checks pass; compiled sources still match the previously MSVC-tested and packaged snapshots, and submodule revisions remain unchanged. Only the completion record and changelog date changed, so another DLL build is unnecessary. Commit this completed plan first so its final evidence remains recoverable in Git, then remove `.agents/PLAN.md` as requested, commit/push, and open a PR targeting main for human review. Do not merge, tag, or publish a release.

## Context and Orientation

This section describes the preparation baseline. The completed implementation and current file paths are recorded in Outcomes & Retrospective: the full generated SDK and top-level support paths described below now exist only in baseline Git history. The final feature code uses `src/unreal/Integration.hpp`; final private layouts/runtime/support live in `src/unreal/detail/`.

The repository root is `/home/camo/Projects/P3RFix` in the preparation environment. On Windows, use the root of the same branch checkout as the working directory for every command below. P3RFix is a C++ shared library packaged as an `.asi` file. An ASI loader or Reloaded-II loads that library into Persona 3 Reload; this is not a standalone program that can be run on Linux to exercise the game hooks.

`src/dllmain.cpp` contains configuration, feature functions, hook installation, and the worker entry point `Main()`. A hook redirects or intercepts a game function using SafetyHook. `Main()` currently calls `Logging()`, `Configuration()`, `UpdateOffsets()`, `Resolution()`, `RenderTextures()`, `EnableConsole()`, and the other fixes in that order. `DllMain()` starts the worker thread. Keep that startup model during this work.

`src/helper.hpp` provides `Memory::PatternScan()` and `Memory::GetAbsolute()`. Pattern scanning finds a byte sequence in the game executable. `GetAbsolute()` resolves a signed 32-bit displacement relative to the instruction into an address. The signature strings and the displacement locations inside `UpdateOffsets()` are already the project's compatibility assumptions. Move or retain those existing signatures without changing them speculatively.

The sole SDK include in project feature code is `src/dllmain.cpp`'s `SDK/Engine_classes.hpp`. The direct SDK users are the `Engine` global, `UpdateOffsets()`, `UTextureRenderTarget2D_PostLoad_hk()`, and `EnableConsole()`. The `RTCaptureMidHook` inside `RenderTextures()` also writes two game fields using raw offsets. Route those writes through the same private render interface so that this feature's layout assumptions have a single home.

`src/SDK/Basic.hpp` defines the object registry, name representation, class/default-object lookup templates, flag values, and virtual dispatch helper. The object registry, called `GObjects`, contains pointers to allocated Unreal objects in chunks. `src/SDK/CoreUObject_classes.hpp` and `src/SDK/CoreUObject_functions.cpp` define the base object layouts, class inheritance checks, and function lookup. `src/SDK/Engine_functions.cpp` supplies engine instance lookup and the `SpawnObject` wrapper. `src/SDK/Engine_parameters.hpp` supplies the three-pointer parameter buffer for that reflected call.

`src/UnrealContainers.hpp` supplies Unreal array and string representations, `src/UtfN.hpp` converts UTF-16 text into UTF-8, and `src/PropertyFixup.hpp` and `src/NameCollisions.inl` support unrelated generated types. UTF-16 uses 16-bit text units as Windows does; UTF-8 is the string encoding used by the fix's logs. Preserve name suffix handling from `FName::ToString()`, which strips text preceding the final `/`.

`xmake.lua` selects C++ latest, compiles the fix and SafetyHook, and invokes CMake for Zydis. Windows release builds use Visual Studio 2022, a static C++ runtime, whole-program optimization, and removal of unreferenced code. These settings may already discard unused wrappers from the final binary, so repository and compiler input reduction are separate from binary size reduction. The four external submodules and their pinned revisions are outside the SDK replacement scope.

`create_release.ps1` requires Windows and PowerShell 7.2 or later. It builds Windows x64 release and produces the standalone and Reloaded-II ZIPs. `assets/r2-package/ModConfig.json` intentionally has a version placeholder; the packaging script replaces `ModVersion` in the staged package. `.github/workflows/build.yml` builds on `windows-2022` with xmake `3.1.1`; `.github/workflows/release.yml` reuses that workflow for tagged releases. `THIRD_PARTY_NOTICES.md`, `licenses/`, and the credits in `README.md` describe inherited SDK and support provenance.

## Plan of Work

### Milestone 1: Record the baseline and prune unreachable generated files

At this milestone's end, the existing implementation still runs, but only the generated files reachable from the build remain. Start by recording the source commit, submodule revisions, SDK file count and bytes, compiler inputs, Windows clean build duration, and `.asi` size. Keep local binaries and full logs under ignored `build/sdk-refactor/`; write concise observations into this plan. Build duration is an observation on a stated machine and configuration, not an acceptance threshold.

Add `tools/unreal_inventory.py`, a small Python 3 script using only the standard library. In `--phase legacy`, seed the dependency walk with all project `src/*.cpp` and the SDK source files explicitly listed in `xmake.lua`. Resolve quoted includes relative to the including file, recursively follow those inside `src`, distinguish reachable SDK files from other local files, and print counts, byte totals, and sorted retained/unused paths. A dependency closure here means every file reached by repeatedly following those includes. Report unresolved local includes rather than treating them as unused. Inspect the build file when the seed list changes; this script is an audit of this repository's simple include structure, not a complete C++ preprocessor. Confirm its result against a Windows compiler build before treating deletions as safe.

The expected retained SDK paths at the preparation commit are:

    src/SDK/AudioPlatformConfiguration_structs.hpp
    src/SDK/Basic.cpp
    src/SDK/Basic.hpp
    src/SDK/Chaos_structs.hpp
    src/SDK/CoreUObject_classes.hpp
    src/SDK/CoreUObject_functions.cpp
    src/SDK/CoreUObject_parameters.hpp
    src/SDK/CoreUObject_structs.hpp
    src/SDK/DeveloperSettings_classes.hpp
    src/SDK/Engine_classes.hpp
    src/SDK/Engine_functions.cpp
    src/SDK/Engine_parameters.hpp
    src/SDK/Engine_structs.hpp
    src/SDK/InputCore_structs.hpp
    src/SDK/PacketHandler_classes.hpp
    src/SDK/PhysicsCore_classes.hpp
    src/SDK/PhysicsCore_structs.hpp
    src/SDK/PropertyAccess_structs.hpp
    src/SDK/SlateCore_structs.hpp
    src/SDK/Slate_structs.hpp

Delete the other 930 tracked SDK files and the unused `src/SDK.hpp` using the reviewed inventory. Do not remove the reachable top-level support headers yet. Preserve generated contents of retained files. Do not rewrite Git history to recover old object storage; deletion reduces the current checkout, and the original dump remains recoverable from the baseline commit.

Run the inventory again and perform a clean Windows release build. Acceptance is 20 retained SDK files, no missing local includes, no deleted file still appearing in compiler inputs, and successful compilation and linking of the existing fix. Confirm a baseline game run where available. Record and commit this independently reviewable pruning milestone before extracting declarations.

### Milestone 2: Put feature callers behind the integration interface

At this milestone's end, `src/dllmain.cpp` uses a small public header while the integration implementation still delegates to the retained SDK. Create `src/unreal/Integration.hpp` and `src/unreal/Integration.cpp` with the interface specified below. The temporary SDK include belongs only in `Integration.cpp`. Add that source explicitly to `xmake.lua`; the current `src/*.cpp` pattern does not include nested directories.

Replace `UpdateOffsets()` with `InitializeUnrealIntegration()` at the same startup position. Continue discovering `GObjects` and `AppendString` with the existing signatures. Resolve absolute addresses and pass them in `RuntimeAddresses` to `InitializeConsoleRuntime()`; do not construct a base-relative offset when a discovery failed. In this temporary backend, initialize `SDK::UObject::GObjects` and `SDK::FName` through their existing manual initialization methods only after both discoveries have succeeded. The unused `ProcessEvent` scan can remain for this intermediate milestone, but must disappear at milestone 4.

Move engine search, default-object lookup, `SpawnObject`, viewport assignment, and console key conversion into `Integration.cpp`. Add null checks before using generated helpers that assume lookup success. Remove the `SDK::UEngine* Engine` global from `dllmain.cpp`; return its address only as a logging value in `ConsoleResult`. Keep the loop of at most 200 attempts with 100 ms sleeps in `EnableConsole()`. `TryEnableConsole()` performs one attempt and never sleeps. While waiting for the engine or viewport, return `Pending`. Once the attempt succeeds or returns a terminal failure, leave the retry loop and log the result. Failure to retrieve input settings after console creation remains a key-reporting failure, not a failure to create the console.

Change `UTextureRenderTarget2D_PostLoad_hk()` to receive an opaque `void*`, read a `RenderTargetInfo`, calculate the existing multiplier in feature code, and write the two sizes using `SetRenderTargetSize()`. An opaque pointer carries an address without exposing an engine class definition. Keep the original function's argument and return conventions and always forward to the original function once, including when the integration cannot read the target. Update the `stdcall` forwarding template to match the opaque argument. Keep `iRTCapX` and `iRTCapY` updates conditional on the RGBA16f render format. Replace the `ctx.rax + 0x1FC` and `ctx.rax + 0x200` writes with `SetCaptureSize()`.

The adapter should let `RenderTextures()` work even when console discovery is unavailable. `Main()` must continue through all unrelated fixes. Add direct standard-library includes in the files that use them, particularly `<algorithm>`, `<chrono>`, `<cstdint>`, and `<thread>` in feature code as needed; the SDK may currently provide accidental transitive includes.

Acceptance is a clean Windows build, no `SDK::` or `UC::` references and no SDK include in `src/dllmain.cpp`, successful compilation of a translation unit that includes only `Integration.hpp`, and the same healthy-path console and render observations as the baseline. The retained SDK should still compile during this milestone; record this as interface isolation, not final removal.

### Milestone 3: Prototype the minimal runtime and verify game memory boundaries

This is an additive prototyping milestone. At its end, a test executable can traverse synthetic game object memory, locate engine metadata, dispatch a fake `SpawnObject`, and retrieve a console key without compiling or linking generated SDK files. The production adapter continues using the old implementation until the prototype is verified.

Create `src/unreal/detail/Layouts.hpp`, `src/unreal/detail/Containers.hpp`, `src/unreal/detail/Runtime.hpp`, and `src/unreal/detail/Runtime.cpp`. Keep the existing attributed UTF conversion code in `src/unreal/detail/UtfN.hpp` if needed. Define only the memory records and operations listed under `Interfaces and Dependencies`. Use fixed-width integer types, named constants, and explicit padding. An application binary interface, or ABI, is the byte layout and function calling contract shared with the running game. Use flat records with standard C++ layout for ABI checks; these are descriptions of memory, not new game objects the fix owns. Prefer byte reads and writes using `std::memcpy` at asserted offsets rather than assuming a live C++ object of the fix's type occupies arbitrary game memory.

Implement only the lookup needed here: iterate the chunked registry; recognize class and function metadata by cast flags; find classes named `Engine`, `GameplayStatics`, and `InputSettings`; follow class `Super` pointers to identify an engine instance; exclude class default objects; and find `SpawnObject` by walking the `GameplayStatics` class's `Children` field chain. Default objects are game-owned template instances held by class metadata, distinct from the live engine instance.

The call to `SpawnObject` must use the `GameplayStatics` default object as the receiver. Fill a zero-initialized three-pointer parameter record with the console class and viewport as its outer, temporarily set the function's native flag `0x400`, call the receiver's virtual table entry `0x44`, and restore the original flags with a scoped cleanup object. Read the returned object and assign it to the viewport only after it is non-null. Check missing class, function, receiver, virtual table, and function entry before calling. Do not call a discovered direct `ProcessEvent` address.

Use the original `AppendString` signature with a fix-owned 1024-unit UTF-16 buffer, reset the count after every conversion, preserve UTF-8 conversion and final-slash handling, and ensure repeated calls do not retain previous output. The buffer must stay fix-owned and must not be freed through a different allocator. Check returned pointer, capacity, and count against the supplied buffer and report a conversion failure if the contract differs. Such checks cannot prevent a faulty game callback from reallocating before return; validate the inherited no-reallocation assumption in-game. If it fails, record the evidence and revise this plan to use a verified game allocation/free pair before proceeding; do not guess an allocator.

Add `tests/unreal_integration_tests.cpp` and a `unreal-integration-tests` xmake executable target with `set_default(false)`. During this milestone it may exercise the private runtime directly, avoiding the temporary SDK-backed public implementation. Compile it with MSVC x64 and the same language and static-runtime settings as the fix, using only minimal integration sources and extracted support. It must have no dependency on SafetyHook, Zydis, spdlog, a game executable, or a new test framework. Use a small test runner that returns a nonzero exit code on any failed check and works in release mode without relying on disabled `assert()` calls.

Construct fixtures as independent byte buffers and populate the documented game offsets, rather than deriving expected offsets from the implementation's records. Supply a fake name callback through `RuntimeAddresses::appendName` and a fake virtual table callback that captures receiver and parameters and writes a configured return object. Test registry holes and the index transition from `0xFFFF` to `0x10000`; default-object exclusion and inherited engine classes; missing metadata; exact spawn receiver, argument ordering, return assignment, and restoration of function flags; name conversion reuse; and console key reporting after successful creation. Confirm that missing runtime dependencies return a status and perform no callback or write. Keep null checks scoped to known addresses and metadata; they do not establish safety for arbitrary non-null pointers or guarantee compatibility with unknown game builds.

Promote the prototype only after these tests and compile-time layout assertions pass. If it disagrees with the legacy implementation on the same synthetic healthy inputs, inspect the retained SDK and correct the extraction before switching production. Record the specific tests and observed outcome here. Discard any test-only duplicate runtime code after the production switch; the final tests must exercise the real integration implementation.

### Milestone 4: Switch production to the minimal runtime and retire the dump

At this milestone's end, the production DLL compiles only the small integration layer and `src/SDK/` no longer exists. Change `Integration.cpp` to delegate console operations to `detail::InitializeRuntime()` and `detail::TryEnableConsole()`, and implement render field access through the minimal layout records. Link `src/unreal/detail/Runtime.cpp` explicitly in both production and test targets. Change the tests to call the public interface wherever the behavior is public; keep lower-level tests for chunk traversal and metadata checks where they provide distinct evidence.

Remove the three SDK sources from the production `add_files()` declaration and delete the remaining generated directory. Remove the unused direct `ProcessEvent` scan, SDK offset storage, `src/PropertyFixup.hpp`, and `src/NameCollisions.inl`. Extract the necessary array/string subset into private support and delete the old top-level `src/UnrealContainers.hpp` and `src/UtfN.hpp` after all references have moved. The final `Containers.hpp` needs only the array header, read operations, and name-string buffer support; do not carry maps, sets, weak object pointers, generated class caches, or other unused SDK facilities.

Preserve attribution comments on extracted code. Rewrite the inherited SDK section in `THIRD_PARTY_NOTICES.md` to describe the retained private layouts and support accurately. Update `README.md` credits and add a short maintainer explanation of where game layouts now live, which features use them, and how to run their tests. Keep the recorded licensing status and applicable license copies for retained derived code; moving or trimming a file does not establish a new upstream license. Do not perform an unrelated dependency upgrade or licensing research project.

Extend `tools/unreal_inventory.py --phase minimal` to report the current integration file count and bytes and to fail if any legacy SDK source/include reference remains in `src/` or `xmake.lua`, if the old paths still exist, or if a required final integration source is missing from build inputs. Check active code and build configuration; historical discussion in this plan may legitimately name deleted files. Require the combined source under `src/unreal/` to remain below 256 KiB. That budget accommodates attributed UTF support and makes accidental dump retention visible; any necessary increase must be justified with specific retained functionality in this plan.

Acceptance is passing tests, a clean Windows build with no SDK compiler inputs, a source inventory below the budget, and successful console/render checks with the minimal backend. Record the removal diff and compiler evidence. Deleting the SDK without proving its consumers still work does not complete this milestone.

### Milestone 5: Validate behavior and prepare the 1.3.0 packages

At this milestone's end, automated checks, both Windows configurations, and the game scenarios below have evidence, and both local packages contain the correctly versioned fix. Set the `fix_version` default in `xmake.lua` and the `P3RFIX_VERSION` fallback in `src/dllmain.cpp` to `1.3.0`. Add a `1.3.0` changelog entry describing the completed maintenance change and necessary missing-dependency behavior. Use the actual completion date for a dated release entry; do not invent test results or overwrite the historical `1.2.5` entry.

Update current-version examples in `README.md`, the argument-error example in `create_release.ps1`, the example tag description in `.github/workflows/release.yml`, and the version placeholder example in `.github/ISSUE_TEMPLATE/bug_report.yml`. Preserve `assets/r2-package/ModConfig.json`'s version placeholder and the packaging script's substitution behavior. Keep `release_body.md`'s existing replacement tokens; add release prose there only if its established format requires it. Do not change tag-trigger semantics or publish as part of this refactor.

Add a test step before packaging in `.github/workflows/build.yml`. Configure xmake for Windows x64 using the resolved workflow version, build and run `unreal-integration-tests`, and run the minimal inventory check using the runner's Python. Continue using the pinned tools and existing checkout. Do not make the test target a default DLL build input, package the test executable, or bypass the existing version/source checks. Because the release workflow reuses this build workflow, the same tests will then protect tagged builds as well.

Run debug and release integration tests, perform a clean release build, and run `create_release.ps1 -Version 1.3.0`. Inspect both archives, the embedded log version, and the staged Reloaded-II metadata. Compare the final source size, compiler inputs, clean build duration, and DLL bytes against milestone 1 using the same machine and build mode. Record the conditions and measurements even if the DLL shrinks only slightly.

Exercise the game scenarios in `Validation and Acceptance` using the baseline and final DLLs with identical configurations. Record game executable identity, platform, renderer settings, and logs. If Windows or the game is unavailable to the implementing contributor, finish independent code and tests that can run, leave the affected acceptance items unchecked, and record the missing environment precisely. Automated fixture tests cannot substitute for a real hooked game run.

## Concrete Steps

All shell commands below run from the repository root. This branch already exists; do not create it again on resume. Start by verifying the current branch and changes, then update submodules at their recorded revisions if needed:

    cd /home/camo/Projects/P3RFix
    git status --short --branch
    git rev-parse HEAD
    git submodule status
    git submodule update --init --recursive

On Windows, first `Set-Location` to the corresponding checkout root and use PowerShell 7.2 or later with Visual Studio 2022 C++ build tools, Windows SDK, CMake, Python 3, and xmake available. Use the existing workflow's xmake version `3.1.1` when comparing measurements. Once the milestone 1 inventory script exists:

    python tools/unreal_inventory.py --phase legacy
    xmake f -y -p windows -a x64 -m release --vs=2022 --fix_version=1.2.5
    xmake clean P3RFix
    xmake build -y -v P3RFix
    Get-Item build/windows/x64/release/P3RFix.asi

Expected initial inventory values are `950` total SDK files, `20` reachable SDK files, and `930` unused SDK files. After pruning, expect `20` total and reachable SDK files and `0` unused files. Keep the retained-path list and build exit status as evidence. Record baseline timing with `Measure-Command` around the same build command, ensure the command exit status is checked, and state whether the Zydis build was already warm. A clean fix build and a full dependency build have different costs.

After adding the test target, use the following commands for milestone 3 and subsequent work. During the prototype, document the private runtime source list; after milestone 4 these commands must test the final public implementation:

    xmake f -y -p windows -a x64 -m debug --vs=2022 --fix_version=1.3.0
    xmake build -y -v unreal-integration-tests
    xmake run unreal-integration-tests
    xmake build -y -v P3RFix
    xmake f -y -p windows -a x64 -m release --vs=2022 --fix_version=1.3.0
    xmake build -y -v unreal-integration-tests
    xmake run unreal-integration-tests
    xmake clean P3RFix
    xmake build -y -v P3RFix
    python tools/unreal_inventory.py --phase minimal

The final test runner prints named check results followed by `69/69 checks passed` and exits zero only when every check passes; that count was observed with MSVC in both debug and release. The additive prototype previously passed 60/60 console checks before promotion. In minimal mode, the inventory must report zero legacy references and less than `262144` bytes under `src/unreal/`. A verbose production build must list the new integration implementation files and no `src/SDK/*` files. Clear stale objects through `xmake clean P3RFix`; source deletion followed only by incremental linking is insufficient evidence.

Final package commands, still from the Windows checkout root. On the supplied VM with multiple installed Visual Studio versions, first set an isolated task-global selection, clear the project configuration, and check the selected version. This was the actual successful recovery; generic Windows users with only VS 2022 may omit the selection setup:

    $env:XMAKE_GLOBALDIR = Join-Path $env:USERPROFILE 'Projects/P3RFix-sdk-refactor/tools/xmake-global-vs2022'
    xmake g -y --vs=2022
    xmake f -c -y -p windows -a x64 -m release --fix_version=1.3.0
    Get-Content .xmake/windows/x64/xmake.conf | Select-String 'fix_version|vs\s*='
    xmake clean P3RFix

Then run:

    ./create_release.ps1 -Version 1.3.0
    Get-Item build/P3RFix_1.3.0.zip, build/P3RFix_Reloaded-II.zip
    # Inspect ModConfig.json inside build/P3RFix_Reloaded-II.zip; staging is removed on success.
    git diff --check
    git status --short --branch

Expect both ZIP files to exist, archived Reloaded-II `ModVersion` to equal `1.3.0`, and a game startup log reporting `P3RFix` version `1.3.0`. Inspect archive entries using `System.IO.Compression.ZipFile` or another locally available archive reader. Both packages must contain the same final `P3RFix.asi`, configuration, notices, and applicable licenses; the standalone package includes the existing pinned loader, while Reloaded-II includes `ModConfig.json`. No SDK dump, test executable, inventory script, or plan belongs in either archive. No tag, GitHub release, merge, or release publication is part of these steps. The user separately authorized pushing the implementation branch to origin for human review.

## Validation and Acceptance

The structural acceptance is that `src/SDK/`, `src/SDK.hpp`, and the obsolete top-level support files are absent, `src/dllmain.cpp` includes only the integration's public header for these operations, and the clean compiler input list contains neither generated SDK source nor headers. The final private integration is below the stated source budget, and every retained field or dispatch constant has a documented origin. The inventory and a clean compiler build provide complementary evidence.

The native test executable must verify binary boundaries with independent expected offsets. A render fixture populated at `0x180`, `0x184`, and `0x19B` must return its sizes and correctly distinguish format `6` from another format. Size updates must modify only the two size fields, leaving neighboring sentinel bytes unchanged. Capture updates must write only `0x1FC` and `0x200`. Null objects must cause no writes. These checks are necessary because an incorrect offset can compile and corrupt unrelated game fields.

The object fixtures must cross a chunk boundary, contain null entries and null chunks, reject invalid indices and inconsistent counts, and find a derived engine without selecting its class default object. A fake `SpawnObject` call must observe the gameplay default object as receiver, console class at parameter offset `0`, viewport at `8`, and returned object at `16`; verify native flags during dispatch and their original value afterward. A null result must leave the viewport console unchanged. Missing registry/name dependencies and missing class, function, default object, or dispatch entry must make no call and return a useful status. A later attempt must work after a legitimate pending engine becomes available. Repeated successful attempts during one startup must not create multiple console objects.

The string fixtures must exercise repeated conversions, final-slash name handling, a numbered name, empty/invalid returned string metadata, and valid Unicode including a UTF-16 surrogate pair. A surrogate pair is two 16-bit units used together for characters outside the basic range. Console key reporting must distinguish a bound key, an empty key array, unavailable input settings, and unavailable conversion; the console remains enabled after key-reporting failures. Run checks in both debug and release because release optimizations and disabled assertions can hide mistakes in a naive test runner.

For healthy-path game verification, record the executable filename, file version if present, SHA-256, store/build identity if known, and whether the scenario uses base content or Episode Aigis content. Never infer support for an untested game build from the mod's release number. Compare baseline and final logs with matching settings. In the deployed configuration, set `[Enable Console] Enabled = true`, launch the game, and verify the existing engine discovery and console-created messages. Press the reported configured key, confirm the console opens, and use a harmless display/query command supported by that installed game; record the actual command and result. Configure a different bound console key through the game's `Input.ini`, confirm that the logged key and actual key agree, and restore the original file afterward.

For render texture verification, use `[Render Texture Resolution] Enabled = true` and `Multiplier = 1` first at 1920 by 1080 and then at a supported higher resolution such as 2560 by 1440 with 100% screen percentage. Use the old DLL to establish target-dependent sizes; at the higher resolution a 1920 by 1080 target, when present, should scale according to the existing formula. Repeat with `Multiplier = 1.5` and with a configured screen percentage, comparing final results against the baseline rather than changing the formula. Navigate menus displaying character models and back into gameplay, observe the old/new size logs, confirm RGBA16f capture dimensions follow the resized target, and check for missing, distorted, or unstable images. Exercise values hitting the existing `0.25` and `4.0` clamp limits and confirm the same behavior. Avoid assuming every render target starts at the same resolution.

For feature independence, run once with console disabled and render scaling enabled, once with console enabled and render scaling disabled, and once with both disabled. Exercise a deliberately missing dependency through synthetic callbacks rather than patching the game executable. The missing-console-runtime scenario must emit a precise failure and allow resolution, HUD, aspect ratio, framerate, focus, intro, and mouse configuration paths to continue. Smoke-check these unrelated enabled fixes in a normal game run, including borderless resizing/minimizing and subsequent restoration, because this repository's previous release changed resolution tracking. These are regression observations, not permission to refactor those features.

Acceptance is complete only when structural checks, meaningful native tests, clean MSVC builds, package/version checks, and the applicable game scenarios have recorded outcomes. State any untested platform or game variant plainly. There is no existing automated test suite for these integration behaviors at the preparation commit; the new tests exercise the extracted boundary and must be observed running, not merely added to source control.

## Idempotence and Recovery

Inspection, inventory, tests, and clean builds may be repeated. Keep measurement output under `build/sdk-refactor/`, which is covered by the existing ignored build directory. Capture a baseline binary before overwriting normal build output. Package creation replaces package staging and ZIP outputs by design; preserve any baseline evidence separately under the ignored measurement directory.

Commit completed implementation milestones with this plan updated, so each successful intermediate state can be restored. Do not rewrite unrelated changes or dependency revisions. Before removing files, compare the inventory and review the diff. If pruning removes a required generated file, retrieve that specific path from `e119244314f345dada02914a01a504a169bc0c16`, fix the inventory, rerun the clean build, and record the finding. Avoid restoring the whole working tree or using a destructive reset when other work is present.

Keep the legacy backend active until the minimal prototype passes its checks. If switching causes a regression, restore the affected integration/build paths from the last verified milestone or revert the isolated switch commit, then investigate with the synthetic fixtures and game logs. The baseline commit retains the full dump for reference; no second checked-in copy, SDK submodule, download dependency, or history rewrite is necessary.

`InitializeConsoleRuntime()` must clear all integration-owned lookup caches before installing a new set of addresses. This lets tests create independent fixtures without retaining pointers from a previous test. In production, initialize once on the existing worker before using the console runtime. Render operations are stateless. Do not reinitialize runtime state concurrently with game callbacks. Preserve the existing startup lifetime assumptions; solving garbage collection, runtime DLL unload, game-thread scheduling, or arbitrary invalid pointer recovery is outside this task.

Save the deployed game's original `P3RFix.asi`, `P3RFix.ini`, and any edited `Input.ini` before manual comparisons, then restore them when checks finish. If an environment or game check is unavailable, record it as remaining work and continue independent implementation; do not claim a failed or missing validation has passed.

## Artifacts and Notes

The preparation baseline is commit `e119244314f345dada02914a01a504a169bc0c16`. The branch is `refactor/minimal-unreal-integration`, and release intent is `1.3.0`. The measured 58,516,425-byte SDK directory is approximately 55.8 MiB; the reachable 13,330,488-byte subset is approximately 12.7 MiB. Sizes are the sum of file content bytes, not allocated filesystem blocks or Git object storage.

Submodule revisions were verified unchanged at preparation and final audit: `external/inipp` = `3f224f1eed7a67d5d7e5fc8cab72de02a056b966`, `external/safetyhook` = `983ba5c4b72866c8ed5020b6d57b7108fd1622c2`, `external/spdlog` = `f1d748e5e3edfa4b1778edea003bac94781bc7b7`, `external/zydis` = `f2ad85f92fc6645a642053882eaf0e95693977e9`, and nested Zycore = `75a36c45ae1ad382b0f4e0ede0af84c11ee69928`.

The final production source declaration should express the relevant inputs explicitly, alongside the existing dependencies:

    add_files("src/*.cpp", "external/safetyhook/src/*.cpp",
              "src/unreal/Integration.cpp", "src/unreal/detail/Runtime.cpp")

Keep concise actual transcripts below this paragraph as work proceeds: inventory before/after, the real test count, clean build result and conditions, final binary size, package metadata, and the game log excerpt showing version, console construction, bound key, and render texture size changes. Do not record desired outputs as though they were observed. Retain original attributions in copied code and record the baseline source symbol beside each extracted layout or operation.

Observed automated evidence:

    Total SDK: 950 files, 58516425 bytes
    Reachable SDK: 20 files, 13330488 bytes
    Unused SDK: 930 files, 45185937 bytes
    Unresolved local includes/build inputs: 0

    Focused integration: 7 files, 79466 bytes
    Legacy references: 0
    Minimal inventory: PASS

    MSVC debug: 69/69 checks passed
    MSVC release: 69/69 checks passed
    debug: 11 compiled production sources; zero SDK or obsolete support compiler dependencies
    release: 11 compiled production sources; zero SDK or obsolete support compiler dependencies

Final package evidence:

    P3RFix_1.3.0.zip: 878059 bytes
    SHA256 e7eb9217a17fc2e7370b1d91a52f4db7411940f98f8c6b11395b528e8722defd
    P3RFix_Reloaded-II.zip: 482537 bytes
    SHA256 7f373866614c8a1efc4761369d9edf4b6642ba48e457306e7732c69dc918038e
    Shared ASI: 1200128 bytes
    SHA256 59562389cb5a81ebb5271f21ed71c1624f4b139a85a066fd3f1f5731c8976343
    Package contents and shared ASI/configuration/notices/licenses: PASS
    Reloaded-II ModVersion: 1.3.0
    Embedded version literal: PASS; actual user game startup log: v1.3.0 loaded

The local handoff files are `build/P3RFix_1.3.0.zip`, `build/P3RFix_Reloaded-II.zip`, and baseline `build/sdk-refactor/baseline-windows/P3RFix-baseline-1.2.5.asi` (SHA256 `b60c846120176ce2c6d0a7774e87737d96b91a91838df5fb190387e65548a6d3`). The VM retains the task checkout and evidence at `C:\Users\camo\Projects\P3RFix-sdk-refactor\pruned`, with package artifacts in its build directory. Exact VM selection commands are also preserved locally in ignored `build/sdk-refactor/package-vs2022.ps1`.

Live log excerpts (user-operated game):

    P3RFix v1.3.0 loaded.
    Enable Console: Console object constructed.
    Enable Console: Console enabled - access it using the 'Tilde' key.
    Current Resolution: Resolution: 5120x1440
    Render Texture 2D Resolution: fRenTexResMulti = 1.3333334
    Render Texture 2D Resolution: Old render texture resolution = 1920x1080
    Render Texture 2D Resolution: New render texture resolution = 2560x1440

## Interfaces and Dependencies

The public API is in `src/unreal/Integration.hpp`, in namespace `p3r::unreal`. It includes only standard-library headers needed for the following declarations; it does not include Windows headers, private layouts, generated SDK files, or inherited containers. Define:

    struct RuntimeAddresses {
        void* objectArray = nullptr;
        void* appendName = nullptr;
    };

    enum class InitStatus { Ready, MissingObjectArray, MissingNameAppender };

    struct RenderTargetInfo {
        std::int32_t width = 0;
        std::int32_t height = 0;
        bool isRgba16f = false;
    };

    enum class ConsoleStatus {
        Pending, Enabled, RuntimeUnavailable, LookupFailed, SpawnFailed
    };

    enum class ConsoleKeyStatus {
        Available, Unbound, InputSettingsUnavailable, NameUnavailable
    };

    struct ConsoleResult {
        ConsoleStatus status = ConsoleStatus::Pending;
        ConsoleKeyStatus keyStatus = ConsoleKeyStatus::InputSettingsUnavailable;
        std::uintptr_t engineAddress = 0;
        std::string keyName;
        std::string diagnostic;
    };

    InitStatus InitializeConsoleRuntime(RuntimeAddresses addresses) noexcept;
    std::optional<RenderTargetInfo> ReadRenderTarget(const void* object) noexcept;
    bool SetRenderTargetSize(void* object, std::int32_t width,
                             std::int32_t height) noexcept;
    bool SetCaptureSize(void* object, std::int32_t width,
                        std::int32_t height) noexcept;
    ConsoleResult TryEnableConsole();

`RuntimeAddresses::objectArray` is the absolute address of the registry structure, not its first object or first chunk. `appendName` is the absolute function address resolved from the existing call displacement. Initialization returns the missing dependency status before enabling console operations and never creates an executable-base fallback. If both are missing, report the object-array status and include both discovery failures in caller logs. Initialization resets cached classes, function, engine, and completed console result. Public read/write functions return failure for null pointers; callers must supply an object from the existing valid hook context. Positive dimension validation may reject invalid writes, but must not introduce a new scaling policy.

`TryEnableConsole()` returns `Pending` while engine metadata, an engine instance, its console class, or its viewport is not yet present during startup. Once those are available, missing gameplay metadata or dispatch machinery returns `LookupFailed` with a concrete diagnostic; a null spawn return is `SpawnFailed`. An uninitialized runtime is `RuntimeUnavailable`. Success sets `Enabled` even if input settings or key conversion failed, with the key-status field explaining the separate reporting result. Cache success for repeated calls during that startup so a retry cannot create a second console. The caller owns the retry timing, timeout message, and logging. The runtime does not own game objects, sleep, install hooks, or parse the INI.

`src/unreal/detail/Runtime.hpp` exposes only `detail::InitializeRuntime(RuntimeAddresses) noexcept` and `detail::TryEnableConsole()` to the public implementation, plus a narrowly scoped object-index helper if the registry tests need it. Both share the public status/value types. All class and function lookup helpers otherwise remain private to `Runtime.cpp`. `Integration.cpp` supplies the public wrappers and render memory operations; no SDK-compatible public class hierarchy is required.

`src/unreal/detail/Layouts.hpp` defines the following necessary memory records, with `sizeof`, `alignof`, and `offsetof` static assertions for every field the implementation touches. Full retained records must match the listed sizes. Prefix views may stop after their last needed field; name them as views and do not claim their size is the entire game class. Alignment must preserve the accessed field layout. Document original generated types beside the replacement definitions:

    UObject: size 0x28, alignment 8.
        VTable 0x00; Flags int32 0x08; Index int32 0x0C;
        Class pointer 0x10; FName 0x18; Outer pointer 0x20.
    UField: full size 0x30; Next pointer 0x28.
    UStruct: full size 0xB0; Super pointer 0x40; Children pointer 0x48.
    UClass: full size 0x230; CastFlags uint64 0xD0;
        DefaultObject pointer 0x118.
    UFunction: full size 0xE0; FunctionFlags uint32 0xB0.
    FName: size 8, alignment 4; ComparisonIndex int32 0; Number uint32 4.
    FUObjectItem: size 0x18, alignment 8; Object pointer 0.
    TUObjectArray: size 0x20, alignment 8; chunk-table pointer 0;
        MaxElements int32 0x10; NumElements int32 0x14;
        MaxChunks int32 0x18; NumChunks int32 0x1C.
        Elements per chunk: 0x10000.
    Array/string header: size 0x10, alignment 8;
        Data pointer 0; element count int32 8; capacity int32 12.
    FKey: size 0x18, alignment 8; KeyName FName 0.
    SpawnObject parameters: size 0x18, alignment 8;
        console class pointer 0; outer/viewport pointer 8;
        return object pointer 16.
    Engine view: ConsoleClass pointer 0xF0; GameViewport pointer 0x780.
    Viewport view: ViewportConsole pointer 0x40.
    Input settings view: ConsoleKeys array header 0x130.
    Render target view: SizeX int32 0x180; SizeY int32 0x184;
        RenderTargetFormat uint8 0x19B; original class alignment 16.
    Capture write locations: int32 width 0x1FC; int32 height 0x200.
    Constants: ClassDefaultObject flag 0x10;
        class cast flag 0x20; function cast flag 0x80000;
        native function flag 0x400; ProcessEvent table index 0x44;
        RGBA16f render target format 6.

Read registry counts and chunk pointers only after initialization, reject negative or inconsistent counts, and check index, table, and chunk pointers before traversal. A null entry is a normal hole, not a fatal lookup error. Preserve the identity pointer treatment from `TUObjectArray::DecryptPtr`; there is no pointer decryption algorithm to invent. Compare numeric flag masks exactly; the legacy enum's `operator&` tests mask inclusion rather than serving as a generic bitwise expression.

`Containers.hpp` retains only the attributed array/string representation and operations used by this runtime. Game-owned registry arrays, console keys, class metadata, engine objects, and spawn return objects are borrowed: the fix neither resizes nor frees them. Fix-owned name scratch storage has a clearly separate owner and lifetime. On the supported Windows build, verify `sizeof(void*) == 8` and `sizeof(wchar_t) == 2`; fail compilation on an incompatible target instead of interpreting the wrong layout. The preparation Linux environment may run inventory checks, but cannot establish the Windows ABI by compiling with its default four-byte `wchar_t`.

The final module uses the C++ standard library and retained private text helpers. Feature code continues using the existing spdlog, inipp, SafetyHook, Zydis, and Windows API dependencies at their pinned revisions. Do not add Unreal Engine source, a replacement full SDK, a reflection framework, a runtime download, or a new package manager. Future features should add the specific required field or operation with provenance and a meaningful boundary test, rather than importing another dump.

## Plan Revision Notes

2026-10-01: Created this self-contained plan from repository inspection and the approved staged direction for P3RFix `1.3.0`. Defined the private/public boundary, conservative console dispatch choice, memory layout evidence, rollback path, validation environments, and final package expectations. Implementation remains pending.

2026-10-01: Reviewed the plan's required sections, pending implementation milestones, source-derived layout values, command examples, and version references. Confirmed the branch creation time from Git and made the initialization call explicit so the intermediate adapter and final API agree.

2026-10-01 05:27Z: Began implementation, recorded VM authorization and the user-run game-test boundary, and clarified branch push authorization. No in-game evidence exists yet.

2026-10-01 05:32Z: Recorded reproducible baseline and pruning evidence, tool versions and environment, and the unchanged binary size. Milestone 1 is independently build-verified; the user must still perform baseline/final game comparisons.

2026-10-01 05:33Z: Recorded milestone 1 commit, prototype/final test separation, UTF helper portability fixes, packaging cleanup discovery, and truthful unreleased changelog status.

2026-10-01 05:34Z: Recorded milestone 2 clean Windows/interface evidence and inventory failure checks. The prototype is additive and will remain separate until MSVC fixture tests pass.

2026-10-01 05:35Z: Recorded MSVC debug/release prototype fixture success and actual compiler dependency audits. The additive prototype is now eligible for promotion; render/public wrapper tests follow in milestone 4.

2026-10-01 05:40Z: Promoted the verified private runtime locally, removed remaining SDK/support paths, enabled all public tests, and fixed the metadata conversion diagnostic gap found in review. Final Windows builds/tests are running; version defaults/docs/package checks follow after that verified switch is committed.

2026-10-01 05:42Z: Recorded final minimal public suite/build/inventory/compiler evidence. Production removal and boundary checks pass, while live-game acceptance remains explicitly incomplete. Next commit sets version defaults and final package/CI metadata.

2026-10-01 05:51Z: Reread the entire plan and audited every requirement against implementation/evidence. Independent code, MSVC, inventory, provenance, CI syntax, version and package checks passed; only mandatory user-run game evidence and its completion date remain. Documented VS selection recovery, artifact hashes, exact handoff paths, unchanged dependency revisions, and concrete resumption steps.

2026-10-01 05:54Z: Pushed and verified the human-review branch, with all implementation saved in short milestone commits. This final execution-record checkpoint changes only the plan; the tested source and artifacts remain unchanged. Resume with user game results for the pending live acceptance, then correct any discrepancies and finish the actual completion record.

2026-10-01 06:30Z: Completed final acceptance from the user-run test and inspected log; obtained matching executable hash/version/Steam build through read-only inspection; recorded exact log observations and user-attested scope without inventing details. Dated changelog and audited all requirements. User requested deletion of this completed plan after committing its final evidence and opening a review PR; no game was run by an agent.
