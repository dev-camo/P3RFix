# Third-party notices

P3RFix retains its original MIT license and copyright notice in [LICENSE.md](LICENSE.md), with an additional copyright notice for current maintenance. Third-party components retain their own licenses and attribution. Both release packages include this notice and the `licenses/` directory.

## Components compiled into P3RFix.asi

The Git submodule revisions determine the sources used for each build. The license copies below come from those revisions.

| Component | Revision | License copy |
| --- | --- | --- |
| [inipp](https://github.com/mcmtroffaes/inipp/tree/3f224f1eed7a67d5d7e5fc8cab72de02a056b966) | `3f224f1eed7a67d5d7e5fc8cab72de02a056b966` | [MIT](licenses/inipp/LICENSE.txt) |
| [spdlog](https://github.com/gabime/spdlog/tree/f1d748e5e3edfa4b1778edea003bac94781bc7b7) | `f1d748e5e3edfa4b1778edea003bac94781bc7b7` (1.15.3) | [MIT](licenses/spdlog/LICENSE) |
| [{fmt}](https://github.com/gabime/spdlog/tree/f1d748e5e3edfa4b1778edea003bac94781bc7b7/include/spdlog/fmt/bundled) | 11.2.0, bundled with spdlog | [MIT with optional binary exception](licenses/fmt/LICENSE) |
| [SafetyHook](https://github.com/cursey/safetyhook/tree/983ba5c4b72866c8ed5020b6d57b7108fd1622c2) | `983ba5c4b72866c8ed5020b6d57b7108fd1622c2` | [Boost Software License 1.0](licenses/SafetyHook/LICENSE) |
| [Zydis](https://github.com/zyantific/zydis/tree/f2ad85f92fc6645a642053882eaf0e95693977e9) | `f2ad85f92fc6645a642053882eaf0e95693977e9` | [MIT](licenses/Zydis/LICENSE) |
| [Zycore](https://github.com/zyantific/zycore-c/tree/75a36c45ae1ad382b0f4e0ede0af84c11ee69928) | `75a36c45ae1ad382b0f4e0ede0af84c11ee69928`, nested Zydis dependency | [MIT](licenses/Zycore/LICENSE) |

## Standalone ASI loader

The standalone ZIP contains the unmodified Windows x64 [Ultimate ASI Loader v9.7.4](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/tag/v9.7.4) binary, renamed from `dinput8.dll` to `dsound.dll`. Its source revision is `6b440669144c4a0bef5718ab155df160d231cd42`. The loader is omitted from the Reloaded-II ZIP.

The loader's [MIT license](licenses/Ultimate-ASI-Loader/LICENSE) is included, along with licenses for the dependencies used by its x64 build:

| Component | Source revision | License copy |
| --- | --- | --- |
| [miniz](https://github.com/ThirteenAG/Ultimate-ASI-Loader/tree/6b440669144c4a0bef5718ab155df160d231cd42/external/miniz) | Bundled with loader v9.7.4 | [MIT](licenses/Ultimate-ASI-Loader/miniz-LICENSE) |
| [MinHook and Hacker Disassembler Engine](https://github.com/TsudaKageyu/minhook/tree/d94c64d32ea37bc4f5ee47d580709f70c6fb6080) | `d94c64d32ea37bc4f5ee47d580709f70c6fb6080`, via injector | [BSD 2-Clause notices](licenses/Ultimate-ASI-Loader/MinHook-LICENSE.txt) |
| [injector utility / FunctionHookMinHook](https://github.com/ThirteenAG/injector/tree/3a384e8d1b575c09383b0fab8bd92e34cb654949/utility) | `3a384e8d1b575c09383b0fab8bd92e34cb654949` | [MIT](licenses/Ultimate-ASI-Loader/injector-utility-LICENSE.txt) |

The loader also contains LINK/2012's [Unhandled Exception Tracer](https://github.com/ThirteenAG/Ultimate-ASI-Loader/blob/6b440669144c4a0bef5718ab155df160d231cd42/source/exception.hpp), whose source notice offers it for use in the public domain.

## Retained Unreal integration layouts and support

The private memory layouts and lookup/dispatch operations under `src/unreal/detail/` were extracted from the original [Dumper-7](https://github.com/Encryqed/Dumper-7) SDK at P3RFix commit `e119244314f345dada02914a01a504a169bc0c16`. The full dump has been removed from the current tree. Retained array/string support in `Containers.hpp` derives from [Fischsalat's UnrealContainers](https://github.com/Fischsalat/UnrealContainers); private UTF conversion support in `UtfN.hpp` credits [Fischsalat's UTF-N](https://github.com/Fischsalat/UTF-N). Original source symbols and attribution comments accompany the extracted code.

The current [UnrealContainers MIT license](https://github.com/Fischsalat/UnrealContainers/blob/80b835813ab46c0b298cf9398b22eeaaa9e10aeb/LICENSE) is copied to [licenses/UnrealContainers/LICENSE](licenses/UnrealContainers/LICENSE). Dumper-7 and UTF-N did not declare an upstream license in their repository trees when this notice was prepared on September 30, 2026; the inherited attribution comments are retained.
