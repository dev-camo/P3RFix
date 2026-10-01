# P3RFix 1.4.0 release plan: four focused maintenance changes

This release should make automatic menu texture scaling consistent, reject invalid raw-input packets safely, install independent fixes before the optional console startup wait, and replace the release ZIPs' scattered license files with one complete `LICENSES` file. Players should retain the working console, the validated viewport-based ultrawide behavior, their existing configuration, and the current installation method.

Target release: **1.4.0**. Baseline examined: **1.3.0**, repository commit `6d55ef6`, on 2026-10-01. Execution began from branch `1.4.0`, commit `d352a6b`, with only this untracked plan in the working tree. No target release date is assigned and no release has been published.

This ExecPlan is a living document. Update `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` as implementation and validation proceed. Source line numbers are orientation aids from the examined baseline; find the named functions again if intervening changes move them.

## Purpose / Big Picture

The four changes belong in a small maintenance release, separate from the four larger changes proposed for 1.5.0. Automatic render-target scaling should retain the intended native-quality baseline on low-height displays while honoring a user's explicit multiplier. A failed Windows raw-input read should have no effect on the camera and should not lead to parsing uninitialized or incomplete storage. Enabling the developer console should not put unrelated hook installation behind its engine-discovery retry loop. Installation should leave one readable licensing document alongside the existing package files, with the project's own license and every retained dependency notice together instead of an entire `licenses/` directory.

A render target is a texture that the game draws into before displaying it elsewhere, such as a menu character preview or a captured menu background. A hook is a small interception installed at a game or Windows function so P3RFix can inspect or adjust behavior. These changes affect P3RFix's decisions around those existing hooks; they do not replace the game's renderer or input system.

Success requires meaningful automated checks for the scaling and packet-handling behavior, inspection of both archives for complete consolidated notices, and an actual game comparison against 1.3.0. Compilation alone does not establish compatibility with a game executable, show that a menu texture still looks correct, or establish that the packaged license bundle is complete.

## Release scope and established facts

The release contains exactly four primary changes: automatic render-target scaling, raw-input read validation, startup ordering, and consolidated release licensing. Version metadata, release notes, and narrowly necessary checks accompany those changes. Keep existing configuration sections and keys intact. No new player-facing setting is necessary for these changes.

The maintainer has verified that the developer console works in 1.3.0. The historical console crash in the shipped upstream 1.2.4 release is not an outstanding confirmed bug in the current baseline. Its cause remains unconfirmed: the newer bundled Ultimate ASI Loader and the eleven upstream commits between 1.2.4 and the maintenance takeover are possible explanations. Do not attribute that recovery to the focused Unreal refactor without evidence.

The maintainer has also validated the viewport-based ultrawide fix through extended gameplay. Commit `cc2582b36b8c3fe37a47781ea5fc98638c3f75b9`, shipped in 1.2.5, reads the actual viewport dimensions. Preserve that acquisition path and its handling of empty or minimized viewports. Do not restore requested-resolution tracking, multiply width by screen percentage to guess a display width, or replace it with the desktop-width workaround from Codeberg PR #5.

The shipping installation method and package placement are established as correct. A historical user installing files in the wrong location is not a reason to redesign the archives. The maintainer has separately requested one deliberate package-content improvement: replace the licensing directory and separate licensing documents with a single `LICENSES` file. Preserve the established binary/configuration destinations, archive names, loader placement, and installation procedure while making that specific change. If documentation is clarified while preparing this release, describe the actual package accurately and keep the final files beside `P3R.exe` for the standalone installation.

The binary remains a **Windows x64 ASI built with MSVC**. Preserve the existing Visual Studio 2022, Windows SDK, xmake, CMake, PowerShell, dependency pins, and release workflows. Proton testing means testing the Windows binary under Proton; it does not mean adding a Linux binary or Linux cross-compilation. Codeberg PR #5 and its recommendations are excluded from this work.

Separate gameplay/menu/background frame-rate limits, new HUD/FOV controls, live configuration, mouse sensitivity redesign, smoothing, per-target graphics budgets, and new platform support are not part of 1.4.0.

## Progress

- [x] (2026-10-01) Reviewed the three original proposals against the 1.3.0 source and preserved the maintainer's current validation and build constraints.
- [x] (2026-10-01) Renamed and expanded the former `pre-v2-fixes.md` notes into a targeted 1.4.0 release plan.
- [x] (2026-10-01) Added the maintainer's fourth release change: one generated `LICENSES` document in each ZIP, preserving existing notices and the source license files.
- [x] (2026-10-01) Read the entire plan, repository instructions, and ExecPlan skill; inspected the baseline source and submodule pins. Assigned independent scaling, packet-reader, and licensing implementation work while integrating the hooks and Windows validation centrally.
- [x] (2026-10-01) Implemented height-based scaling and checked dimension conversion; Windows debug/release behavior tests cover native retention, explicit undersampling, invalid inputs, truncation, and integer boundaries.
- [x] (2026-10-01) Implemented checked raw-input reading, an allocation-failure seam, and shared delta application. Independent literal Windows x64 fixtures cover two-call failures, malformed/truncated packets, signed motion, ignored flags, and unchanged accumulators/ownership on rejection.
- [x] (2026-10-01) Moved the unchanged console initialization call after all independent installers and added the attempt-complete marker. The development harness is complete; real startup/console observations remain.
- [x] (2026-10-01) Temporary Windows harness passed 8/8 using verbatim production bodies for `Main`, `EnableConsole`, and the two changed callbacks. Console-disabled, ready, unavailable, delayed, and 200-attempt timeout cases pass; message/render adapters forward once and preserve memory/delta boundaries. Remaining: maintainer startup/console observations in game.
- [ ] Verify console-enabled and disabled startup, intro skipping, and repeated console access in game.
- [x] (2026-10-01) Implemented the thirteen-input `LICENSES` generator and archive assertions in the existing release script; fault fixtures and real package validation are complete.
- [x] (2026-10-01) Licensing fixtures reject missing, empty, and unlisted notices; preserve BOM/Unicode/trailing-space/no-final-newline handling; reproduce identical document bytes; and reject legacy, incomplete, duplicate-location, missing-INI, and wrong-loader archive contents. Both real ZIPs pass production assertions and an independent Python comparison of all thirteen complete normalized inputs.
- [x] (2026-10-01) Updated README, AGENTS, third-party introduction, changelog, and release body. PowerShell ScriptAnalyzer reports zero warning/error diagnostics; selected new C++ files pass clang-format verification and the diff has no whitespace errors.
- [x] (2026-10-01) Built the ASI with VS2022/MSVC 14.44.35207 on the authorized VM in debug and release. In each configuration: Unreal 69/69, behavior 35/35, minimal inventory PASS. Behavior sources compile with `/W4 /WX`.
- [ ] Compare menu rendering, input, console operation, intro skipping, and ultrawide behavior in the game.
- [x] (2026-10-01) Prepared 1.4.0 binary defaults, unchanged Reloaded-II version substitution, generated release notes, and both actual Windows archives. Fresh temporary extractions and comparison with shipped 1.3.0 archives preserve all mod-file destinations and loader bytes. Live installation validation remains outstanding.
- [x] (2026-10-01) Prepared `/tmp/P3RFix/TESTING.txt`, `/tmp/P3RFix/RESULTS.txt`, both candidate ZIPs, baseline ZIP copies, extracted inventories, and the game-results directory. Runtime/package source is commit `d133132`; ASI and document hashes are recorded in the form and persistent `docs/1.4.0-validation.txt`.
- [x] (2026-10-01) Audited every implementation/automated requirement against current source, transcripts, and actual archive bytes. All automated gates are satisfied; game and actual installation evidence remains missing.
- [ ] Record maintainer live validation and release outcome, resolve failures or unavailable required coverage, remove this plan in the final commit only after testing/confirmation, and push the completed branch. Human review/approval remains required before merge; publication is a subsequent release operation.

## Surprises & Discoveries

The baseline render-target guard did more than contain an obvious repeated variable. It read `iCurrentResX <= 1920 || iCurrentResX <= 1080`, which is equivalent to the first comparison alone. Because the multiplier calculation uses vertical rendering resolution, a mechanical replacement of the second variable would still leave a width-dependent policy. This plan chooses an explicit vertical-resolution policy instead.

At 2560x720 and 100% screen percentage, the old automatic multiplier is approximately 0.667 because the width-only guard does not activate. At 1920x2160 and 100%, it instead forces the automatic multiplier to 1. These are results of the examined arithmetic, not claims that the maintainer reproduced a visual defect at those resolutions.

The baseline raw-input path did not check the size-query result. It logged an unexpected read result but continued to access the packet. Its `new BYTE[dwSize]` null check also did not handle the exception normally raised by an unsuccessful ordinary C++ allocation. Packet validation therefore covers the entire read path, rather than only adding a return after the old log line.

The console retry loop runs on P3RFix's initialization worker. It does not itself pause the entire game thread. Its observable ordering problem is that later independent fixes are installed only after the retry finishes. Preserve that distinction in release notes.

The baseline packaging loop in `create_release.ps1` copied `LICENSE.md`, `THIRD_PARTY_NOTICES.md`, and the entire `licenses/` directory into both release packages. The repository has eleven component-license files under `licenses/`, in addition to the project's own license and the provenance document. Some component files contain additional notices or exceptions: the MinHook file includes the Hacker Disassembler Engine notices, and the fmt file includes its optional binary exception. Combining only a generic MIT paragraph would lose material already present in the release inputs.

The Windows VM at `camo@192.168.122.141` accepts SSH and uses a Windows command shell. Tool locations and a dedicated validation checkout have been established below. Game execution is expressly reserved for the maintainer; automated validation stops at a concrete package and test handoff.

The validation tools from the prior 1.3.0 work are available at `C:\Users\camo\Projects\P3RFix-sdk-refactor\tools`: portable PowerShell, xmake 3.1.1, Python 3.13.7, and Git. CMake is bundled with `C:\BuildTools2022`, and MSVC is 14.44.35207. This release uses a separate checkout at `C:\Users\camo\Projects\P3RFix-1.4.0\source`, with transcripts under its sibling `evidence` directory. The earlier baseline/refactor checkouts are preserved.

The first behavior-test build exposed Windows `min`/`max` macros after including the raw-input SDK header. Defining `NOMINMAX` in the test runner, as the production precompiled header already does, fixed the conflict. Both complete Windows configurations subsequently passed. SSH command length also limits large encoded PowerShell scripts; the handoff helper now uploads scripts and executes `-File` instead.

The empty-notice fixture found that .NET's culture-sensitive `StartsWith` treats the encoding marker as ignorable, causing an empty string to pass that check and fail at `Substring(1)`. An ordinal comparison now produces the required path-specific empty-input error. The fixture output must also be outside the fixture's `licenses/` directory: Windows treats a root file named `LICENSES` and that source directory as the same name. Production already separates source inputs and staging output correctly.

The shipped 1.3.0 INI is 2,954 bytes (CRLF) and this Linux-sourced Windows validation package's INI is 2,876 bytes (LF). An independent comparison confirms that line endings are the only difference; no section, key, value, or comment changed. The document generator normalizes all maintained notice inputs, so both packages have identical 25,315-byte `LICENSES` files regardless of input line endings.

## Decision Log

Decision: target the original three behavior fixes and the subsequently requested licensing cleanup at 1.4.0 rather than a pre-v2 backlog. Rationale: the maintainer wants a concrete maintenance release and separate treatment from the 1.5.0 feature proposals. Date/author: 2026-10-01, maintainer direction recorded by Codex.

Decision: the automatic multiplier has a minimum of 1, and explicit user multipliers are applied afterward. Rationale: preserve native render-target quality automatically without removing the user's deliberate ability to undersample. Date/author: 2026-10-01, proposed implementation design by Codex.

Decision: preserve the current raw-mouse interpretation in this release. Rationale: packet-read safety can be established independently of the larger camera and device-handoff work proposed for 1.5.0. Date/author: 2026-10-01, proposed implementation design by Codex.

Decision: solve console-related ordering by moving its existing wait after independent installation on the same initialization worker. Rationale: another thread or a console implementation rewrite is unnecessary for the stated problem and would add concurrency risk to a working feature. Date/author: 2026-10-01, proposed implementation design by Codex.

Decision: retain Windows x64/MSVC, the current build tooling, working viewport discovery, working console behavior, archive names, and binary/configuration placement; change only the specified licensing payload. Rationale: these are established project constraints and validated behavior, with an explicit maintainer request to simplify release licensing. Date/author: 2026-10-01, maintainer direction recorded by Codex.

Decision: use the exact extensionless filename `LICENSES` for the combined release document. Rationale: the plural name describes the project's license plus multiple component licenses and notices without implying that one license governs every component. The maintainer authorized either `LICENSE` or `LICENSES`; selecting one spelling keeps both packages and their documentation consistent. Date/author: 2026-10-01, maintainer direction and proposed filename recorded by Codex.

Decision: generate `LICENSES` from the existing checked-in license texts and provenance during packaging, using the existing PowerShell script, and put the same combined contents in both package formats. Rationale: keeping editable source notices avoids a second manually maintained copy of every license, while one shared generated output prevents package-specific omissions. Loader sections will explicitly state that the loader binary is bundled only with the standalone package. Date/author: 2026-10-01, proposed implementation design by Codex.

Decision: reserve actual game runs and install-method compatibility confirmation for the maintainer, and retain this plan until that confirmation arrives. Rationale: the execution instructions prohibit the agent from running the game and require user validation before deleting the plan or completing/pushing the branch. Prepare instructions, packages, and a result form under `/tmp/P3RFix/`. Date/author: 2026-10-01, maintainer instruction recorded by Codex.

Decision: expose a small nothrow raw-input allocator seam and delta-application helper, with a separate fixture header alongside the scaling cases. Rationale: allocation failure can be proven without exhausting memory, and tests exercise the shipped accumulator behavior rather than duplicating it. Date/author: 2026-10-01, Codex.

Decision: rate-limit invalid screen-percentage and user-multiplier warnings independently, without treating an unknown startup height as an error. Rationale: early viewport discovery is expected and must not suppress reporting a later invalid runtime observation. Date/author: 2026-10-01, Codex.

Decision: use PowerShell's information stream for packaging diagnostics and a singular internal `Assert-PackageContent` name. Rationale: keep helper return bytes separate from diagnostics and pass ScriptAnalyzer without suppression or a new release dependency. The analyzer is installed only in the VM's disposable validation tools. Date/author: 2026-10-01, Codex.

## Context and Orientation

The repository examined is `/home/camo/Projects/P3RFix`. On a Windows build machine, use that machine's checkout path rather than attempting to use this Linux path. The source produces a Windows ASI: a DLL with an `.asi` filename loaded into the game by the standalone loader or Reloaded-II.

`src/dllmain.cpp` contains configuration parsing, startup, signature discovery, and most hook callbacks. `Main` is the initialization worker created by `DllMain`. `Resolution` installs the viewport-size hook and screen-percentage hook. `UTextureRenderTarget2D_PostLoad_hk` calculates new texture dimensions and forwards to the original game function. `PeekMessageW_Injected` observes messages after the original Windows message function returns.

`src/unreal/Integration.hpp` and `src/unreal/Integration.cpp` expose focused render-target and console operations. Private memory layouts and reflected calls live in `src/unreal/detail/`. The generated SDK is no longer a build dependency. Keep that separation. The existing independent byte fixtures in `tests/unreal_integration_tests.cpp` verify memory boundaries and console behavior; they do not currently test the scaling decision or Windows packet-reader failure paths.

`xmake.lua` defines the ASI, `unreal-integration-tests`, and new `fix-behavior-tests` targets. The behavior runner loads `tests/render_scaling_tests.hpp` and `tests/raw_mouse_packet_tests.hpp`. `create_release.ps1` builds and packages a version supplied through `-Version`. The version default is also present in `xmake.lua` and as a fallback macro in `src/dllmain.cpp`. `.github/workflows/build.yml` checks inventory and runs both test targets in debug/release before packaging. `P3RFix.ini`, `README.md`, `CHANGELOG.md`, `release_body.md`, and `assets/r2-package/ModConfig.json` supply the player-facing configuration, documentation, and package metadata.

`LICENSE.md` contains P3RFix's MIT license and the original/current-maintenance copyright notices. `THIRD_PARTY_NOTICES.md` identifies compiled dependencies, the pinned standalone loader and its dependencies, and retained Unreal integration provenance. The eleven component texts under `licenses/` are the inputs to preserve in full. They remain source-repository files; the release script will combine their contents into one package document instead of copying the source directory into a player's game installation.

Use small independent helpers for arithmetic and packet validation so tests exercise the shipped implementation. Keep the hook callbacks as adapters around those helpers. Do not turn this release into a general reorganization of `dllmain.cpp`.

## Change 1: make automatic render-target scaling consistent

### Current behavior and defect

The relevant calculation begins around `src/dllmain.cpp:378`:

    float fOptimalRenTexResMulti =
        (iCurrentResY * (fScreenPercentage / 100)) / 1080;

    if (iCurrentResX <= 1920 || iCurrentResX <= 1080)
        fOptimalRenTexResMulti = 1.0f;

The function then applies `[Render Texture Resolution].Multiplier`, clamps the result to 0.25 through 4, multiplies the original target's width and height, and updates the capture-size globals when the target is RGBA16F. RGBA16F is the floating-point texture format used by the existing code to select those capture dimensions.

The problem is the mismatch between the height-derived calculation and the redundant width-only guard. The release should define the automatic baseline consistently for standard, ultrawide, and narrow resolutions.

### Required policy

For a positive viewport height and finite positive screen percentage, calculate the effective vertical rendering resolution as:

    effectiveHeight = viewportHeight * screenPercentage / 100.0
    automaticMultiplier = max(1.0, effectiveHeight / 1080.0)
    finalMultiplier = clamp(automaticMultiplier * userMultiplier, 0.25, 4.0)

Apply the upper limit to the final combined multiplier. Do not clamp the automatic multiplier to 4 before applying the user value: a large effective height combined with a multiplier below 1 should retain its intended combined meaning.

The user multiplier remains relative to the automatic baseline. A value of 1 uses automatic scaling; a value of 0.5 halves that baseline; a value of 2 doubles it, subject to the final limit. This preserves the existing documented meaning of the setting.

If the viewport height has not yet become valid, use an automatic baseline of 1 until a valid height is available. The existing viewport code already keeps the last valid dimensions during minimization; do not reset those dimensions here. A finite nonpositive or nonfinite screen percentage cannot safely drive arithmetic; use 100 for this calculation and report the invalid observation without changing the user's INI on disk. Use 1 for an invalid nonpositive or nonfinite user multiplier at this boundary. Ordinary finite user values should continue to be constrained to the documented 0.25-to-4 range by configuration validation.

These defensive fallbacks cover unexpected runtime input to the helper. They are not an invitation to change all configuration parsing in 1.4.0.

### Proposed implementation boundary

Create `src/render/Scaling.hpp` and `src/render/Scaling.cpp` with a small pure arithmetic interface under `p3r::render`. A pure function makes its decision only from its arguments; it does not read game memory, perform a pattern scan, or change globals.

Use the following interface, with a result field or equivalent diagnostic status for sanitized inputs:

    struct ScaleResult {
        double automaticMultiplier;
        float finalMultiplier;
        bool usedFallback;
    };

    ScaleResult ComputeRenderTargetScale(
        int viewportHeight,
        float screenPercentage,
        float userMultiplier) noexcept;

    struct ScaledDimensions {
        std::int32_t width;
        std::int32_t height;
    };

    std::optional<ScaledDimensions> ScaleDimensions(
        std::int32_t width,
        std::int32_t height,
        float multiplier) noexcept;

Have `UTextureRenderTarget2D_PostLoad_hk` use this helper and retain its existing integration calls. Calculate in a sufficiently wide floating-point type, check finiteness and integer bounds before narrowing dimensions, and reject nonpositive source dimensions. Preserve the current truncation toward zero for positive products, with a minimum of one pixel for a valid deliberately undersampled target. A floating-point value outside the `int32_t` range must never be cast and written to game memory.

On rejected dimensions, leave the target untouched and forward to the original function exactly once. Keep the RGBA16F capture-size behavior. Do not add target-category selection, a graphics-memory budget, or rescaling of already-created targets in this release. Those require different feature work.

### Expected arithmetic examples

At 1280x720, 1280x800, 1920x1080, or 2560x720 with 100% screen percentage and user multiplier 1, the final multiplier is 1. At 3440x1440 or 5120x1440 with the same settings, it is approximately 1.333333. At 3840x2160 it is 2. At 1920x2160 it is also 2; width does not decide the vertical-quality baseline.

At a 2160-pixel viewport height with 50% screen percentage, the automatic baseline is 1. At a 1440-pixel height with 150%, it is 2. A user multiplier of 0.5 applied to the 1440p/100% baseline yields approximately 0.666667, demonstrating that explicit undersampling still works. A combined multiplier above 4 is capped at 4.

A 1920x1080 source target becomes 2560x1440 for the normal 1440p/100%/1 case and 3840x2160 for the 2160p/100%/1 case. Use tolerances for intermediate floating-point comparisons. Expected integer dimensions should be checked directly.

### Validation and acceptance for this change

Add tests with independent expected answers for native-baseline retention, height-driven scaling, screen-percentage interaction, user undersampling, combined clamping, invalid arithmetic inputs, invalid source dimensions, one-pixel minimums, and integer overflow rejection. Include a case that fails under the old guard, such as 2560x720 with automatic scaling.

In the actual game, compare at least one below-1080p display mode, a 1440p ultrawide mode, and a 2160p mode against the baseline. Inspect menu model previews and captured backgrounds, record old and new target sizes from the log, and watch for unexpected hitching. Verify that changing aspect ratio alone at the same vertical rendering resolution does not alter the quality baseline.

Acceptance requires correct calculated sizes, original-function forwarding on every path, no unrelated viewport change, and no new visual or loading regression. The release notes should describe the scaling guard correction; they should not claim a universal performance improvement.

## Change 2: validate raw-input reads before parsing

### Current behavior and defect

The relevant path begins around `src/dllmain.cpp:855`. It queries `dwSize`, allocates that many bytes, performs a second read, and then interprets the buffer even if the read fails or is incomplete.

The Windows API contract is specific: a successful size query with a null data pointer returns zero; a successful data read returns the copied byte count; an error returns `(UINT)-1`. The buffer must have at least 32-bit alignment. Those facts are summarized here so implementation does not depend on external context. [Microsoft GetRawInputData reference](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getrawinputdata).

### Required behavior

A failed size query, zero or undersized required buffer, allocation failure, failed data read, copied count outside the allocated capacity, inconsistent packet header, or truncated mouse payload must result in no P3RFix camera update. Return the original `PeekMessageW` result. Observing a bad packet must not consume another Windows message or prevent the game from performing its own handling.

Keep the queried allocation capacity separate from the size variable passed back to the second API call. Store the read's returned count before comparing it. Do not depend on the evaluation order of an API call and a comparison against a variable that the call can modify.

Validate that the copied bytes contain a `RAWINPUTHEADER`. Inspect the header through a checked copy or properly aligned storage. Validate its declared packet size against the copied count and capacity. If the type is mouse, require enough bytes for the mouse payload at the Windows-defined union offset before accessing movement. Non-mouse packets need no camera processing.

For 1.4.0, retain the current movement-selection behavior: only the packets accepted by the existing `usFlags == 0` condition contribute to the custom relative mouse movement. Changing the interpretation of other valid flags belongs to 1.5.0. Retain signed X/Y values, the existing division by 1200, mouse-activity behavior for valid nonzero motion, and exactly-once accumulation when the message is removed.

Use scoped ownership for the buffer. An allocation failure must not throw through a Windows interception callback. Keep any catch localized to allocation/read preparation; avoid a broad catch that hides unrelated defects.

### Proposed implementation boundary

Create `src/input/RawMousePacket.hpp` and `src/input/RawMousePacket.cpp`. Use Windows SDK structures directly; do not create packed replicas of them. Keep the helper independent of SafetyHook, logging, and game-memory layouts.

Define a reader function type compatible with the actual Windows function so tests can supply deterministic read outcomes:

    using RawInputReader =
        UINT (WINAPI*)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);

    struct MouseDelta {
        LONG x;
        LONG y;
    };

    enum class PacketStatus {
        RelativeMouse,
        Ignored,
        Invalid,
        AllocationFailed
    };

    struct PacketResult {
        PacketStatus status;
        MouseDelta delta;
    };

    using RawInputBuffer = std::unique_ptr<std::uint32_t[]>;
    using RawInputAllocator = RawInputBuffer (*)(std::size_t wordCount) noexcept;

    PacketResult ReadRelativeMousePacket(
        HRAWINPUT input,
        RawInputReader reader,
        RawInputAllocator allocator = nullptr) noexcept;

    void ApplyRelativeMousePacket(
        const PacketResult& packet,
        float& accumulatedX,
        float& accumulatedY,
        bool& lastValidInputWasFromMouse) noexcept;

Production passes `GetRawInputData`; tests pass a controlled implementation of that same contract. An aligned array of 32-bit words is one simple way to own the requested byte capacity. Check the rounding calculation before allocating. Parse only the bytes the API reports as copied.

The hook remains responsible for applying the returned valid delta to the existing accumulators. Invalid, ignored, and allocation-failure results do not alter movement or ownership state. Keep original-call forwarding at the top of the message hook as it is today. Preserve the original result on every early return.

Log invalid reads without flooding the log on repeated device events. Allocation/read failures can include an available error code captured at the point of failure, but do not invent an error code for a structurally short packet. Normal ignored keyboard or nonrelative packets are not errors.

### Validation and acceptance for this change

Use fixtures that exercise the production reader with a successful two-call mouse read, a size-query error, zero queried size, a queried size smaller than the header, a data-read error, a short data read, an oversized returned count, a header declaring more data than was copied, and a truncated mouse payload. Add valid non-mouse and currently ignored flag cases.

Use both positive and negative motion, including zero motion. Confirm that a valid delta is accumulated once and that an invalid result leaves existing accumulated X/Y and the active-input flag unchanged. The allocation-failure path should be verified through an appropriately small allocation seam or a controlled failing allocator if practical; do not attempt to exhaust system memory as a test.

In game, enable the existing mouse fix and check movement, strafing, dialogue, menus, and controller switching against 1.3.0. This release's valid-input camera response should remain equivalent. A changed sensitivity curve or newly introduced smoothing would violate its scope.

Acceptance requires no parsing outside the copied packet, no leaked storage on early returns, no exception escaping the callback, no duplicated message processing, and preserved valid-input behavior.

## Change 3: remove the console wait from independent installation

### Current behavior and ordering

In the baseline, `Main` calls `EnableConsole` after `RenderTextures` and before `IntroSkip`, `AspectRatioFOV`, `HUDFix`, `Framerate`, `WindowFocus`, and `MouseFix`.

`EnableConsole` returns immediately when disabled. When enabled, it retries up to 200 pending results with a 100 ms sleep between attempts. That is the existing approximately 20-second pending window, plus lookup work. A missing dependency or other nonpending error already exits the loop early.

### Proposed order and preserved behavior

Keep initialization on the existing worker and use this order:

    Logging
    Configuration
    InitializeUnrealIntegration
    Resolution
    RenderTextures
    IntroSkip
    AspectRatioFOV
    HUDFix
    Framerate
    WindowFocus
    MouseFix
    EnableConsole

Check that no moved installer relies on a constructed console. The examined source provides no such dependency: these functions scan and install their own hooks. Render-target access is also independent of console-runtime discovery through the focused integration.

Preserve the console retry count, pending interpretation, result reporting, key reporting, and console construction behavior. Do not add a second worker, call game-object construction from another new thread, or replace the console implementation. Do not move work into `DllMain`; it should remain the small loader entry point that starts the existing worker.

Add a concise initialization-complete log marker after independent installation and before console initialization. Its wording must distinguish attempting independent installation from every requested feature succeeding. Individual scan errors still need their existing reporting.

### Validation and acceptance for this change

Check source dependencies and actual startup logs. With the console disabled, startup behavior should remain equivalent. With it enabled and ready, all independent installation attempts should appear before console success/key reporting. With dependencies unavailable, independent attempts should still precede the console error and the game should retain those successfully installed features.

For a genuinely delayed pending console, verify the ordering through a controlled startup observation or development-only test harness. A harness may model pending console discovery without touching game memory, but do not retain a special delay toggle in the shipped player configuration merely to test this reorder. Do not build a large startup abstraction just to assert a literal function list.

Test intro skipping with the console enabled so time-sensitive hooks have a chance to act at boot. Also verify that the working console still opens with its configured game binding and that repeated access works. This change should improve availability of independent fixes during the pending period, not change the console's own functionality.

## Change 4: consolidate all release licensing into one `LICENSES` file

### Current package contents and requested outcome

The baseline release script requires `LICENSE.md`, `THIRD_PARTY_NOTICES.md`, and `licenses/`, then copies all of them into both package staging directories. The two archive formats therefore carry multiple separate licensing documents and a nested directory that becomes extra installation clutter when extracted.

Replace that distribution payload with exactly one extensionless `LICENSES` document at each package's existing content root. It must contain the complete P3RFix license, dependency license texts, and retained attribution/provenance. Neither ZIP should separately contain `LICENSE.md`, `THIRD_PARTY_NOTICES.md`, or any `licenses/` entry. Keep the archive root, any established game-relative nesting, ASI/INI/loader destinations, and Reloaded-II metadata placement unchanged. The new file occupies the same package level where the existing top-level licensing documents belong; it is not a reason to add another enclosing folder.

The source checkout retains `LICENSE.md`, `THIRD_PARTY_NOTICES.md`, and the component files under `licenses/`. The requested simplification concerns what players download and extract. Generating the release document from those source inputs makes future maintenance easier and preserves the repository's existing license references.

### Contents and component coverage

Start the combined document with a short plain-text introduction explaining that it collects P3RFix's license and the notices for components used or credited by the release. Explain that each component retains its own terms and that loader notices describe the loader bundled with the standalone ZIP. Put the P3RFix license first, followed by the provenance/attribution section and individually labeled component sections.

The P3RFix section must preserve both copyright lines from `LICENSE.md`, including the original author's credit. The provenance section must retain the component identities, pinned revisions/version information, the distinction between compiled dependencies and the standalone loader, and the existing retained Unreal integration attribution. The loader remains Ultimate ASI Loader v9.7.4 with the existing SHA256 verification and DLL naming; this change does not upgrade or replace it.

Preserve the entire text of every component file, including copyrights, grants, conditions, disclaimers, embedded subcomponent notices, and exceptions. Do not replace several MIT variants with one shared MIT paragraph, drop the Boost text because the package also includes MIT-licensed software, or omit MinHook's additional Hacker Disassembler Engine notices. Preserve fmt's optional exception as part of its source text even though this plan does not depend on invoking it.

Retain the existing Dumper-7, UnrealContainers, UTF-N, and Unhandled Exception Tracer provenance as written and correctly attributed. Moving notices does not assign a new license to inherited code or change what the existing provenance document says about the source trees examined when it was prepared. Keep original source attribution comments in place.

Use this explicit ordered input manifest inside `create_release.ps1`; it defines thirteen source documents, comprising twelve full license texts and one provenance document:

    LICENSE.md
    THIRD_PARTY_NOTICES.md
    licenses/inipp/LICENSE.txt
    licenses/spdlog/LICENSE
    licenses/fmt/LICENSE
    licenses/SafetyHook/LICENSE
    licenses/Zydis/LICENSE
    licenses/Zycore/LICENSE
    licenses/UnrealContainers/LICENSE
    licenses/Ultimate-ASI-Loader/LICENSE
    licenses/Ultimate-ASI-Loader/miniz-LICENSE
    licenses/Ultimate-ASI-Loader/MinHook-LICENSE.txt
    licenses/Ultimate-ASI-Loader/injector-utility-LICENSE.txt

Give each input a readable component heading and its source-repository path. Those path labels describe where the maintained source text lives; they do not claim that the original individual files are also shipped. Distinguish the P3RFix license section from the provenance section and distinguish the four loader-related inputs from components compiled into the ASI.

Use the same combined document in both packages. In the introduction and loader sections, state plainly that Reloaded-II does not include the standalone loader binary. Including the same clearly qualified loader notices in both archives preserves existing notice coverage and keeps generation/validation simple; it must not suggest that Reloaded-II loads or requires `dsound.dll` from this package.

### Readability and text preservation

Write ordinary UTF-8 text without a byte-order mark at the beginning of the combined file. A byte-order mark is an encoding marker that some source text files contain; stripping that marker from an input prevents invisible markers from appearing between sections. Preserve Unicode author names and meaningful text. Use one consistent newline convention and a final newline so the document opens cleanly in common Windows editors.

Place clear separators and component titles outside the copied text. License payloads may normalize line endings and remove only an initial encoding marker; do not trim lines, reflow paragraphs, remove trailing spaces as a side effect of broad cleanup, summarize wording, or rewrite license terms. If a source file has no final newline, add a separating newline before the next section so the last sentence and next heading do not run together.

`THIRD_PARTY_NOTICES.md` currently contains Markdown links to repository-local license paths. Preserve their source/provenance meaning by explaining, immediately before the included notice, that repository-relative paths refer to the source checkout and the full license texts follow in this same document. Update that source document's package-description sentence to say releases carry one combined `LICENSES` file. This avoids telling readers that the old `licenses/` tree exists inside the new ZIP while retaining the repository's useful navigable references.

Do not depend on release-page links or an internet connection to provide the actual license terms. Links and source revision identifiers supplement the copied texts; a player reading the extracted document offline should have all of the license wording already present.

### Implementation within the existing release script

Add a small `Write-CombinedLicenseFile` helper inside `create_release.ps1`. Its inputs are the repository root, the explicit ordered section records, and a destination path. Each section record contains a component title and repository-relative input filename. The function validates every input as a nonempty readable file, constructs the complete text in the manifest order, and writes one output. Do not create a new packaging framework, add a tool dependency, introduce a new build target, or download license texts at release time.

Validate these inputs before building or creating either final ZIP so a missing document fails the release with a message naming its path. Treat a missing or empty required license/provenance input as an error rather than silently skipping it. Compare the source `licenses/` file inventory with the manifest: if a component notice is added or removed, packaging should report the mismatch and require the manifest to be updated deliberately. This prevents a future checked-in notice from being forgotten merely because the original list was hardcoded.

Generate the combined file once at `build/package-staging/LICENSES`. Copy it into both `$StandaloneDirectory` and `$ReloadedDirectory` alongside the existing ASI and INI. Replace only the baseline loop's separate `LICENSE.md`/`THIRD_PARTY_NOTICES.md` copy and recursive `licenses/` copy. Keep the loader copy, metadata substitution, compression destinations, release-body generation, existing source-input validation, and staging cleanup behavior.

Keep the helper's section order deterministic, using the explicit manifest rather than filesystem enumeration order. Do not insert a current timestamp, machine path, username, locale-dependent string, or dynamically fetched revision into the document. Repeated generation from the same source texts must produce the same `LICENSES` bytes. Reproducibility here concerns the document contents; ZIP container timestamps need not become a separate release project.

Update `README.md` and the existing `THIRD_PARTY_NOTICES.md` introduction to distinguish source license files from the single document shipped in releases. Add a concise 1.4.0 changelog item explaining the archive cleanup. Add release-body wording where it helps describe the actual archive contents; retain the established extraction instructions and game directory. No ASI behavior, player setting, loader configuration, or dependency revision changes as part of this item.

### Verification of generated contents and both archives

Verify each generated section against its maintained input after only the declared encoding/newline normalization. Identify sections by their manifest records and unique headings, rather than checking only that the output contains the word `MIT` or that a filename exists. A present but incomplete file fails acceptance. Check that all thirteen source documents appear once in the intended order and that the final output is not empty.

Add a focused package-content assertion in the same PowerShell script after both archives are created, using .NET's built-in ZIP reader. Open each ZIP, normalize its entry separators for comparison, and require exactly one file with the expected `LICENSES` path at the established package root. Check entry names case-insensitively for a `licenses` directory or descendant and for the separate top-level `LICENSE.md` and `THIRD_PARTY_NOTICES.md` documents; reject any of those legacy licensing outputs. Do not reject unrelated files merely because their names contain the substring `license`.

Read the actual archived `LICENSES` entry and compare its bytes to the generated reference. Require the two package documents to match each other. In the same archive inspection, verify that the established ASI/INI entries retain their paths, that the standalone loader appears only in the standalone package, and that Reloaded-II retains the expected `ModConfig.json`. Preserve the script's existing build/loader-input checks as well. Close every ZIP stream and entry stream before the script performs staging cleanup or another package run.

These assertions run as part of the existing packaging invocation, so the current build and tagged-release workflows exercise them without another CI platform or utility. A short diagnostic should identify which archive and which notice/entry failed. A validation failure must stop packaging successfully completing and prevent publishing through the existing workflow; it must not emit a reassuring success message for incomplete archives.

For fault-path verification during implementation, test the document helper against a disposable source fixture with one required notice missing, one empty, and one unexpected file under the fixture's `licenses/` directory. These should fail with the relevant path and leave no accepted output. Include a source without a final newline and an input with an initial encoding marker to confirm separator/encoding handling. Keep fixtures in temporary build output; do not delete or edit real repository licenses merely to provoke an error. This is a small packaging-boundary check, not another C++ test framework.

### Installation and upgrade behavior

Fresh installation should extract one `LICENSES` document instead of the old licensing directory and separate documents. Compare actual ZIP inventories before and after implementation to show that the ASI, active INI, standalone loader, and Reloaded-II metadata retain their established package destinations. Validate extraction through both supported install methods. Licensing consolidation must not introduce an extra folder that changes where the game loads the mod.

Extraction over an older installation does not remove files the older archive already created. Do not claim that an ordinary ZIP update automatically cleans an existing `licenses/` directory or old top-level notice files. The release note may state that older notice files can remain after an overwrite update and that the new combined file is the release's complete notice document. Do not introduce an installer, automatic deletion routine, or broad cleanup instruction that could affect licensing files belonging to another mod.

Acceptance requires one complete readable `LICENSES` file in each archive, no separately packaged licensing tree/documents, full preservation of the maintained texts and attribution, matching documents in both formats, repeatable generation, and unchanged binary installation behavior.

## Plan of Work and Milestones

### Milestone 1: isolate and correct render-scaling decisions

Implement `src/render/Scaling.hpp` and `src/render/Scaling.cpp`, wire the existing render-target callback to them, and create arithmetic/dimension tests with independent expected values. At the end of this milestone, an isolated Windows test executable should demonstrate the intended baseline and explicit multiplier behavior.

Add a focused `fix-behavior-tests` target in the existing `xmake.lua`. This is a necessary test target in the current tooling, not a build-platform change. Compile the new helper into the ASI by listing its source under the existing target; the current `src/*.cpp` pattern does not recursively include subdirectories.

### Milestone 2: make packet handling fail safely

Implement the checked reader and wire it into `PeekMessageW_Injected`. Extend `tests/fix_behavior_tests.cpp` with the controlled Windows reader fixtures. Keep tests linked to the actual helper instead of duplicating its validation logic in a test implementation.

At the end of this milestone, read failures and truncated payloads should be reproducible in tests with no camera update, while valid signed movement produces the expected delta. The existing game memory fixture target remains separate and unchanged in purpose.

### Milestone 3: move the optional console wait

Move the existing call in `Main`, add the concise installation marker, and review the resulting dependency order. Verify normal startup in game, then capture console-disabled, console-enabled, and unavailable/deferred-console evidence as described above.

At the end of this milestone, independently installed features should no longer wait behind console discovery, and the confirmed working console should remain working.

### Milestone 4: consolidate and verify the licensing payload

Implement the ordered document generator and archive assertions inside `create_release.ps1`, update the source/release descriptions, and exercise the missing/empty/extra-input and encoding/separator checks with disposable fixtures. Run the packaging script and inspect both actual ZIPs. At the end of this milestone, each archive should contain exactly one complete `LICENSES` document with the same bytes and no legacy licensing directory or separate notice documents.

Keep the source license texts intact and record the input manifest, document hash, and archive-entry evidence. Confirm the loader remains confined to the standalone package and extraction still places each supported package's actual mod files where its installation instructions expect them.

### Milestone 5: prepare and validate the release

Update the version fallback/default to 1.4.0, add a truthful `CHANGELOG.md` entry, and adjust build examples and release text where necessary. Preserve the existing Reloaded-II metadata substitution and pinned loader.

Run the automated checks and live comparisons, then build both archives with the consolidated licensing checks enabled. Inspect their actual contents and validate installation through each supported method. An archive inspection proves package contents; it does not prove game compatibility. Record both kinds of evidence.

## Concrete Steps

Use a Windows x64 development environment with Visual Studio 2022 C++ build tools, Windows SDK, CMake, PowerShell 7.2 or newer, and the project's existing xmake. Run commands from the repository root. Initialize the already-pinned submodules if this checkout has not done so:

    git submodule update --init --recursive

For debug checks after the new behavior target exists:

    xmake f -y -p windows -a x64 -m debug --fix_version=1.4.0
    xmake build -y unreal-integration-tests
    xmake run unreal-integration-tests
    xmake build -y fix-behavior-tests
    xmake run fix-behavior-tests
    python tools/unreal_inventory.py --phase minimal

Repeat in release mode:

    xmake f -y -p windows -a x64 -m release --fix_version=1.4.0
    xmake build -y unreal-integration-tests
    xmake run unreal-integration-tests
    xmake build -y fix-behavior-tests
    xmake run fix-behavior-tests
    python tools/unreal_inventory.py --phase minimal

Both test executables must return zero and report all checks passing. The existing Unreal test runner prints `[PASS]` or `[FAIL]` per case and ends with a passed/total count. Give the new runner the same straightforward result convention. Record the actual counts after implementation; no count is claimed by this plan.

Extend the existing build workflow's debug/release validation step to run the new target too. Keep its platform, pinned actions, dependencies, permissions, packaging flow, and release handoff intact.

Once code and release metadata are ready, build staged archives using the existing script:

    ./create_release.ps1 -Version 1.4.0

Expected release outputs are `build/P3RFix_1.4.0.zip`, `build/P3RFix_Reloaded-II.zip`, and `build/release_body.md`. The ASI is produced at `build/windows/x64/release/P3RFix.asi`. The script's staging directories are disposable build output; do not put user settings or validation captures there.

The packaging invocation must also validate the combined licensing manifest and both ZIP contents before reporting success. For a separate manual inspection, use the existing PowerShell/.NET environment from the repository root:

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    foreach ($ArchivePath in @('build/P3RFix_1.4.0.zip', 'build/P3RFix_Reloaded-II.zip')) {
        $Archive = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $ArchivePath).Path)
        try {
            Write-Host "Contents of $ArchivePath"
            $Archive.Entries | Select-Object FullName, Length
        } finally {
            $Archive.Dispose()
        }
    }

Expect one nonempty `LICENSES` entry at each established package root and no `licenses/`, `LICENSE.md`, or `THIRD_PARTY_NOTICES.md` entry. Inspect the extracted document in a Windows text editor and confirm that the project, compiled-dependency, retained-provenance, and standalone-loader sections are readable. The automatic assertions prove section coverage and matching contents; this manual step proves the layout and text are understandable to a reader. Archive entry listings and checks described here are expected future verification, not already captured evidence.

Actual publishing is a subsequent release operation after validation. Use the project's existing tag-driven workflow and a new 1.4.0 tag on the exact tested commit. Do not move an existing release tag or imply that writing this plan published a release.

## Validation and Acceptance

The maintenance release is acceptable when the three behavior fixes and the fourth licensing/package change are implemented, their scope remains narrow, the existing integration/inventory checks and new focused checks pass, both packages contain one complete `LICENSES` document, and the game shows no regression in the validated console or viewport behavior.

For presentation, test 16:9 and at least one wide viewport at the same vertical resolution, below-1080p rendering, normal 1440p automatic scaling, and 2160p automatic scaling. Compare old/new menu targets and captures. Include borderless resizing and minimization so the unchanged viewport hook is exercised.

For input, use mouse fix disabled as a baseline, then enabled with ordinary mouse movement and a controller present. Test device switching and opening/closing menus. Automated invalid-packet tests establish the safety improvement; do not pretend it is necessary or reliable to cause an actual bad Windows packet during play.

For startup, test intro skipping and console access together. Keep log evidence showing independent installation before the optional console result. In Proton, validate the shipped Windows binary with the existing loader override and console setting. Do not revive the historical 1.2.4 crash as a current issue merely because it appears in an upstream thread.

Where Steam and Xbox/MS Store executable variants are available, record which variant was actually tested. An untested variant remains untested; passing a Steam run does not imply a WinGDK run. The release can record unavailable coverage honestly, but must not replace it with an invented compatibility claim.

For each live case, record the mod build/commit, game executable identifier, loader/install method, relevant settings, viewport size, action sequence, result, and a short log or visual comparison. Use a stable saved-game scene where possible so before/after presentation is comparable.

For packaging, record both ZIP inventories, the consolidated document hash, and the thirteen-input coverage result. Demonstrate that omitted/empty/unlisted notice inputs fail the helper or package checks, that the final two documents match, and that the old licensing payload is absent. Fresh extraction should show the single document; an overwrite upgrade may retain old files from a previous install, as explicitly explained in Change 4.

## Idempotence and Recovery

Build and test steps may be repeated in the checkout. The package script recreates its own staging/output files. Preserve useful release artifacts and captured evidence outside that disposable staging directory before rerunning it.

Before swapping a game installation's ASI or loader for validation, keep recoverable copies of the working baseline files and the player's INI. Restore that exact baseline if a regression appears. Do not overwrite or delete unrelated mods, game configuration, or saves.

Each primary change should be independently reviewable and revertible. If render scaling fails live comparison, revert that helper/wiring while retaining independently verified packet safety and startup ordering. If a feature requires an unrelated redesign, move it out of this maintenance release rather than expanding its scope silently.

License generation is repeatable and writes only to disposable package staging/output. If a license input fails validation, fix the manifest or maintained source text and regenerate the packages; do not bypass the check or hand-edit only one ZIP. If consolidation itself needs to be reverted, restore the old script copy operations with the matching documentation and keep the source license files intact. Do not ship a package without either the validated combined document or the previous complete notice payload.

Do not add generated SDK files or new memory-layout fields for the three runtime fixes or for licensing consolidation. Existing integration tests already cover render writes and console boundaries. If an actual discovered requirement changes that assumption, document the evidence and revise this plan before implementing broader memory access.

## Artifacts and Notes

Keep implementation evidence with the release work: the final source diff, debug and release test output, inventory result, startup-order log excerpt, representative render-target sizes, actual-game comparison notes, the combined-license input manifest/hash, and both archive-content inspections.

Captured automated evidence is under `/tmp/P3RFix/evidence/`, mirrored from `C:\Users\camo\Projects\P3RFix-1.4.0\evidence`. The candidate archives and generated release text are under `/tmp/P3RFix/packages/`. `windows-validation.txt` records both ASI builds and both sets of 69/69 integration and 35/35 behavior results; `hook-harness.txt` records 8/8 development-only observations; `license-checks.txt`, `powershell-lint.txt`, and `packaging.txt` record the packaging checks. `archive-inspection.txt` independently verifies exact inventories, all thirteen payloads, configuration/loader preservation, Reloaded-II metadata, x64 PE identity, and matching documents/binaries. No actual-game evidence exists yet.

`docs/1.4.0-validation.txt` permanently records the verified candidate and remaining gates. `/tmp/P3RFix/TESTING.txt` provides concrete install/rollback instructions and the rendering, input, startup, viewport, and package cases; `/tmp/P3RFix/RESULTS.txt` records common environment/build identifiers and per-case outcomes. Baseline archives are under `/tmp/P3RFix/baseline/`, and the maintainer can store logs/images under `/tmp/P3RFix/game-results/`. The candidate runtime/packaging source commit is `d133132`; subsequent handoff/evidence commits change documentation only. The supplied Steam `P3R.exe` was read for identification (SHA256 `88244bade1988eafd8ababd66d4c2227091c4c6c253e6455c7097b1b1e5299b8`), never executed.

The verified standalone inventory is `dsound.dll` (1,198,304 bytes), `LICENSES` (25,315), `P3RFix.asi` (1,203,200), and `P3RFix.ini` (2,876), all at the archive root. Reloaded-II replaces `dsound.dll` with `ModConfig.json` (945 bytes), retaining the other three paths. Both documents have SHA256 `f953af8a4c4e88df3c592a3fbf8dbec9cb37bddba1b17633b9fa38d3e2f1aa54`; the ASI hash is `c64347fb3d0f687a44bda30a5f7e796a538a956cee7c2b51a48d56dd882f91cf`. ZIP hashes are recorded in `archive-inspection.txt` and may change if packaging is repeated; compare actual bytes before handoff.

The following are expected example results, not captured output:

    [PASS] automatic scale retains native baseline on a wide 720p viewport
    [PASS] explicit undersampling applies after the automatic baseline
    [PASS] failed raw-input read produces no mouse delta
    [PASS] truncated mouse payload is rejected
    N/N checks passed

An expected startup sequence is:

    Independent fix installation attempts completed.
    Enable Console: Console object constructed.
    Enable Console: Console enabled - access it using the reported key.

Use the actual console log wording and key from the resulting build. The first marker proves order, while individual installation logs establish which features succeeded.

An illustrative packaging result is:

    Combined licensing document: all 13 source documents included.
    P3RFix_1.4.0.zip: one LICENSES document; no separate licensing directory/documents.
    P3RFix_Reloaded-II.zip: one LICENSES document; no separate licensing directory/documents.
    Both packaged LICENSES documents match the generated reference.

This transcript is an expected result, not captured output. Use the implemented script's actual wording and output hashes in the release evidence.

Draft release-note content should describe the corrected vertical-quality baseline, checked raw-input reads, earlier independent installation, and one combined `LICENSES` document replacing the old licensing folder/separate documents in release ZIPs. Do not include unreleased 1.5.0 controls, claim the old console crash was repaired here, or describe the viewport fix as new in 1.4.0.

## Interfaces and Dependencies

Keep using the existing `p3r::unreal::ReadRenderTarget`, `SetRenderTargetSize`, `SetCaptureSize`, and console interface. No generated SDK or additional library is needed.

The proposed new sources are `src/render/Scaling.hpp`, `src/render/Scaling.cpp`, `src/input/RawMousePacket.hpp`, `src/input/RawMousePacket.cpp`, and `tests/fix_behavior_tests.cpp`. Add them explicitly to the appropriate existing xmake targets. The behavior tests may link the already-used Windows `user32` library for the SDK/API boundary; they should not depend on the game, hook installation, or Zydis.

Use the current C++ standard setting and MSVC ABI. ABI means the binary rules for calling functions and laying out SDK structures. In particular, keep the correct Windows calling convention for the injected reader and use the Windows SDK's own `RAWINPUTHEADER` and `RAWMOUSE` definitions.

The helper sources and adjacent fixture headers now exist and use the interfaces described above. Future changes to these boundaries must update the decisions, commands, and interface descriptions together so this plan remains an accurate resume record.

Licensing consolidation uses only the existing PowerShell 7.2-or-newer release environment and built-in .NET text/ZIP support. The ordered generator and archive-content assertion live in `create_release.ps1`; no new external dependency, compiler, source target, or runtime DLL is required. `LICENSE.md`, `THIRD_PARTY_NOTICES.md`, and all eleven source component-license files remain the maintained inputs. `LICENSES` is generated package content, not another independent source-of-truth file to edit by hand.

## Outcomes & Retrospective

All four changes are implemented. Both Windows configurations build the ASI and pass Unreal 69/69, behavior 35/35, and the minimal inventory; the development harness passes 8/8. Both real ZIPs pass licensing and destination checks, independent thirteen-input comparisons, and fresh temporary extraction. PowerShell lint reports zero warning/error diagnostics. README, repository instructions, changelog, and release text describe the new behavior and package contract; the changelog truthfully marks 1.4.0 unreleased. These results establish arithmetic, simulated API failures, synthetic adapter ordering, and archive contents; they do not establish gameplay or loader compatibility.

The prepared candidate has reached the user-required game-test handoff. Implementation and automated/package verification are finished and committed, but release completion is unproven because actual game/installation results are missing. This is essential external validation reserved for the maintainer, not a failed build or an exhausted implementation path. The smallest input needed is the completed `/tmp/P3RFix/RESULTS.txt` with supporting startup/render logs and representative visual comparisons.

Resume from the current branch and inspect the worktree, plan, persistent validation record, current package hashes, and any returned user results. Local validation scripts and console output are under `/tmp/P3RFix/`; `remote.py` uploads a PowerShell script to the dedicated VM directory and runs it. Next: reconcile each live acceptance case with the maintainer's evidence, investigate any regression, and obtain coverage for unverified required cases. Update this plan and `docs/1.4.0-validation.txt` with actual findings. After successful user validation/confirmation, remove this plan in the final commit and push branch `1.4.0` to origin. Do not run the game, delete this plan early, push before completion, merge without human approval, or infer a passed case from missing/unavailable evidence. No release tag or publication has been attempted.

Revision note, 2026-10-01: renamed the original pre-v2 notes for a 1.4.0 release and expanded them into a self-contained maintenance plan at the maintainer's request. The scaling policy, failure handling, startup ordering, implementation boundaries, validation, and release criteria are now explicit; the established console, viewport, packaging, and MSVC constraints are retained.

Revision note, 2026-10-01: added the maintainer's fourth 1.4.0 change, replacing the release licensing folder and separate license/notice files with one generated `LICENSES` document in both existing package formats. Expanded the scope, decisions, milestones, commands, archive/content validation, upgrade behavior, recovery, and release criteria to cover complete notice preservation. Source licensing files and build tooling remain unchanged; the separate 1.5.0 plan's baseline references now inherit this packaging contract.

Revision note, 2026-10-01: recorded the recovered implementation state, completed Windows validation, test seams, and dedicated VM/tool paths. Split implementation from outstanding live validation so future continuations cannot mistake compilation or synthetic tests for confirmed gameplay.

Revision note, 2026-10-01: recorded verified licensing fault paths, the actual package inventories and hashes, zero-diagnostic linting, and production-body development harness results. Live game and installation checks remain explicit release gates; no evidence has been inferred from compilation or archive success.

Revision note, 2026-10-01: completed the concrete maintainer handoff and permanent candidate validation record. The release remains unreleased and the goal remains active pending actual game/installation results; final plan removal and branch push retain the user's explicit testing/confirmation gate.

Revision note, 2026-10-01: the final pre-handoff reread reconciled stale baseline wording and progress notes with the implemented state and documented the allocator/delta application interfaces in full. This is documentation alignment only; acceptance criteria and candidate runtime/package bytes are unchanged.
