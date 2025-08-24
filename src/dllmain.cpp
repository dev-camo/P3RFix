#include "stdafx.h"
#include "helper.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <inipp/inipp.h>
#include <safetyhook.hpp>

#include "SDK/Engine_classes.hpp"

#define spdlog_confparse(var) spdlog::info("Config Parse: {}: {}", #var, var)

HMODULE exeModule = GetModuleHandle(NULL);
HMODULE thisModule;

// Fix details
std::string sFixName = "P3RFix";
std::string sFixVersion = "1.2.5";
std::filesystem::path FixPath;

// Ini
inipp::Ini<char> ini;
std::string sConfigFile = sFixName + ".ini";

// Logger
std::shared_ptr<spdlog::logger> logger;
std::string sLogFile = sFixName + ".log";
std::filesystem::path ExePath;
std::string sExeName;

// Ini Variables
bool bCustomResolution = false;
int iCustomResX = -1;
int iCustomResY = -1;
bool bHUDFix = false;
bool bAspectFix = false;
bool bSkipLogos = false;
int iSkipLogos = -1;
bool bUncapMenuFPS = false;
bool bAdjustFPSCap = false;
float fFramerateCap = 120.0f;
bool bPauseOnFocusLoss = false;
bool bEnableConsole = false;
bool bScreenPercentage = false;
float fScreenPercentage = 100.0f;
bool bRenTexResMulti = false;
float fRenTexResUserMulti = 1.0f;
bool bMouseFix = false;
bool bIgnoreGamepad = false;
float fMouseMultiplierX = 1.0f;
float fMouseMultiplierY = 1.0f;

// Aspect ratio / FOV / HUD
std::pair<int, int> DesktopDimensions = { 0, 0 };
const float fPi = 3.1415926535f;
const float fNativeAspect = 16.0f / 9.0f;
float fAspectRatio = 16.0f / 9.0f;
float fAspectMultiplier = 1.0f;
float fHUDWidth = 1920.0f;
float fHUDWidthOffset = 0.0f;
float fHUDHeight = 1080.0f;
float fHUDHeightOffset = 0.0f;

// Variables
int iCurrentResX = 0;
int iCurrentResY = 0;
SDK::UEngine* Engine = nullptr;
bool bIntroSkipHasRun = false;
int iFadeStatus = 0;
float fRenTexResMulti = 1.0f;
int iRTCapX = 1920;
int iRTCapY = 1080;
float fRawMouseX = 0.0f;
float fRawMouseY = 0.0f;
bool bLastValidInputWasFromMouse = false;
bool bCameraShouldMoveHasRunThisFrame = false;
float* fInputVectorPtr = 0;

void CalculateAspectRatio(bool bLog)
{
    if (iCurrentResX <= 0 || iCurrentResY <= 0)
        return;

    // Calculate aspect ratio
    fAspectRatio = static_cast<float>(iCurrentResX) / iCurrentResY;
    fAspectMultiplier = fAspectRatio / fNativeAspect;

    // HUD (wider)
    fHUDWidth = static_cast<float>(iCurrentResY) * fNativeAspect;
    fHUDHeight = static_cast<float>(iCurrentResY);
    fHUDWidthOffset =  (static_cast<float>(iCurrentResX) - fHUDWidth) / 2.0f;
    fHUDHeightOffset = 0.0f;

    // HUD (narrow)
    if (fAspectRatio < fNativeAspect) {
        fHUDWidth =  static_cast<float>(iCurrentResX);
        fHUDHeight =  static_cast<float>(iCurrentResX) / fNativeAspect;
        fHUDWidthOffset = 0.0f;
        fHUDHeightOffset =  (static_cast<float>(iCurrentResY) - fHUDHeight) / 2.0f;
    }

    if (bLog) {
        spdlog::info("----------");
        spdlog::info("Current Resolution: Resolution: {:d}x{:d}", iCurrentResX, iCurrentResY);
        spdlog::info("Current Resolution: fAspectRatio: {}", fAspectRatio);
        spdlog::info("Current Resolution: fAspectMultiplier: {}", fAspectMultiplier);
        spdlog::info("Current Resolution: fHUDWidth: {}", fHUDWidth);
        spdlog::info("Current Resolution: fHUDHeight: {}", fHUDHeight);
        spdlog::info("Current Resolution: fHUDWidthOffset: {}", fHUDWidthOffset);
        spdlog::info("Current Resolution: fHUDHeightOffset: {}", fHUDHeightOffset);
        spdlog::info("----------");
    }
}

void Logging()
{
    // Get path to DLL
    WCHAR dllPath[_MAX_PATH] = {0};
    GetModuleFileNameW(thisModule, dllPath, MAX_PATH);
    FixPath = dllPath;
    FixPath = FixPath.remove_filename();

    // Get game name and exe path
    WCHAR gameExePath[_MAX_PATH] = {0};
    GetModuleFileNameW(exeModule, gameExePath, MAX_PATH);
    ExePath = gameExePath;
    sExeName = ExePath.filename().string();
    ExePath = ExePath.remove_filename();

    // Spdlog initialisation
    try
    {
        // Truncate existing log file
        std::ofstream file(ExePath / sLogFile, std::ios::trunc);
        if (file.is_open()) file.close();

        // Create single log file that's size-limited to 10MB
        logger = std::make_shared<spdlog::logger>(sFixName, std::make_shared<spdlog::sinks::rotating_file_sink_st>(ExePath.string() + sLogFile, 10 * 1024 * 1024, 1));
        spdlog::set_default_logger(logger);
        spdlog::flush_on(spdlog::level::debug);
        spdlog::set_level(spdlog::level::info); 
        
        spdlog::info("----------");
        spdlog::info("{:s} v{:s} loaded.", sFixName, sFixVersion);
        spdlog::info("----------");
        spdlog::info("Log file: {}", (FixPath / sLogFile).string());
        spdlog::info("----------");
        spdlog::info("Module Name: {:s}", sExeName);
        spdlog::info("Module Path: {}", ExePath.string());
        spdlog::info("Module Address: 0x{:x}", (uintptr_t)exeModule);
        spdlog::info("Module Timestamp: {:d}", Memory::ModuleTimestamp(exeModule));
        spdlog::info("----------");
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        AllocConsole();
        FILE *dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        std::cout << "Log initialisation failed: " << ex.what() << std::endl;
        FreeLibraryAndExitThread(thisModule, 1);
    }
}

void Configuration()
{
    // Inipp initialisation
    std::ifstream iniFile(FixPath / sConfigFile);
    if (!iniFile) {
        AllocConsole();
        FILE *dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        std::cout << "" << sFixName.c_str() << " v" << sFixVersion.c_str() << " loaded." << std::endl;
        std::cout << "ERROR: Could not locate config file." << std::endl;
        std::cout << "ERROR: Make sure " << sConfigFile.c_str() << " is located in " << FixPath.string().c_str() << std::endl;
        spdlog::error("ERROR: Could not locate config file {}", sConfigFile);
        spdlog::shutdown();
        FreeLibraryAndExitThread(thisModule, 1);
    }
    else {
        spdlog::info("Config file: {}", (FixPath / sConfigFile).string());
        ini.parse(iniFile);
    }

    // Parse config
    ini.strip_trailing_comments();
    spdlog::info("----------");

    // Read ini file
    inipp::get_value(ini.sections["Custom Resolution"], "Enabled", bCustomResolution);
    inipp::get_value(ini.sections["Custom Resolution"], "Width", iCustomResX);
    inipp::get_value(ini.sections["Custom Resolution"], "Height", iCustomResY);
    inipp::get_value(ini.sections["Intro Skip"], "SkipLogos", bSkipLogos);
    inipp::get_value(ini.sections["Intro Skip"], "SkipTo", iSkipLogos);
    inipp::get_value(ini.sections["Pause on Focus Loss"], "Enabled", bPauseOnFocusLoss);
    inipp::get_value(ini.sections["Uncap 60FPS Menus"], "Enabled", bUncapMenuFPS);
    inipp::get_value(ini.sections["Enable Console"], "Enabled", bEnableConsole);
    inipp::get_value(ini.sections["Fix HUD"], "Enabled", bHUDFix);
    inipp::get_value(ini.sections["Fix Aspect Ratio"], "Enabled", bAspectFix);
    inipp::get_value(ini.sections["Screen Percentage"], "Enabled", bScreenPercentage);
    inipp::get_value(ini.sections["Screen Percentage"], "Value", fScreenPercentage);
    inipp::get_value(ini.sections["Render Texture Resolution"], "Enabled", bRenTexResMulti);
    inipp::get_value(ini.sections["Render Texture Resolution"], "Multiplier", fRenTexResUserMulti);
    inipp::get_value(ini.sections["FPS Cap"], "AdjustFPSCap", bAdjustFPSCap);
    inipp::get_value(ini.sections["FPS Cap"], "Framerate", fFramerateCap);
    inipp::get_value(ini.sections["Mouse Fix"], "Enabled", bMouseFix);
    inipp::get_value(ini.sections["Mouse Fix"], "IgnoreGamepad", bIgnoreGamepad);
    inipp::get_value(ini.sections["Mouse Fix"], "MouseMultiplierX", fMouseMultiplierX);
    inipp::get_value(ini.sections["Mouse Fix"], "MouseMultiplierY", fMouseMultiplierY);

    // Log config parse
    spdlog_confparse(bCustomResolution);
    spdlog_confparse(iCustomResX);
    spdlog_confparse(iCustomResY);
    spdlog_confparse(bSkipLogos);
    spdlog_confparse(iSkipLogos);
    if (iSkipLogos < 1 || iSkipLogos > 3) {
        iSkipLogos = std::clamp(iSkipLogos, 1, 3);
        spdlog::warn("Config Parse: iSkipLogos value invalid, clamped to {}", iSkipLogos);
    }
    spdlog_confparse(bUncapMenuFPS);
    spdlog_confparse(bPauseOnFocusLoss);
    spdlog_confparse(bEnableConsole);
    spdlog_confparse(bHUDFix);
    spdlog_confparse(bAspectFix);
    spdlog_confparse(bScreenPercentage);
    spdlog_confparse(fScreenPercentage);
    if (fScreenPercentage < (float)10 || fScreenPercentage > (float)400) {
        fScreenPercentage = std::clamp(fScreenPercentage, (float)10, (float)400);
        spdlog::warn("Config Parse: fScreenPercentage value invalid, clamped to {}", fScreenPercentage);
    }
    spdlog_confparse(bRenTexResMulti);
    spdlog_confparse(fRenTexResUserMulti);
    if (fRenTexResUserMulti < 0.25f || fRenTexResUserMulti > 4.0f) {
        fRenTexResUserMulti = std::clamp(fRenTexResUserMulti, (float)0.25, (float)4);
        spdlog::warn("Config Parse: fRenTexResUserMulti value invalid, clamped to {}", fRenTexResUserMulti);
    }
    spdlog_confparse(bAdjustFPSCap);
    spdlog_confparse(fFramerateCap);
    spdlog_confparse(bMouseFix);
    spdlog_confparse(bIgnoreGamepad);
    spdlog_confparse(fMouseMultiplierX);
    spdlog_confparse(fMouseMultiplierY);

    spdlog::info("----------");
}

void UpdateOffsets()
{
    // GObjects
    std::uint8_t* GObjectsScanResult = Memory::PatternScan(exeModule, "48 8B ?? ?? ?? ?? ?? 48 8B ?? ?? 48 8D ?? ?? EB ?? 33 ?? 8B ?? ?? C1 ??");
    if (GObjectsScanResult) {
        spdlog::info("Offsets: GObjects: Address is {:s}+{:x}", sExeName.c_str(), GObjectsScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        std::uint8_t* GObjectsAddr = Memory::GetAbsolute(GObjectsScanResult + 0x3);
        SDK::Offsets::GObjects = static_cast<UC::uint32>(GObjectsAddr - reinterpret_cast<std::uint8_t*>(exeModule));
        spdlog::info("Offsets: GObjects: 0x{:x}", SDK::Offsets::GObjects);
    }
    else {
        spdlog::error("Offsets: GObjects: Pattern scan failed.");
    }

    // AppendString
    std::uint8_t* AppendStringScanResult = Memory::PatternScan(exeModule, "48 89 ?? ?? ?? E8 ?? ?? ?? ?? 48 8B ?? ?? 48 85 ?? 75 ?? 48 8B ?? ?? ?? 48 8B ??");
    if (AppendStringScanResult) {
        spdlog::info("Offsets: AppendString: Address is {:s}+{:x}", sExeName.c_str(), AppendStringScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        std::uint8_t* AppendStringAddr = Memory::GetAbsolute(AppendStringScanResult + 0x6);
        SDK::Offsets::AppendString = static_cast<UC::uint32>(AppendStringAddr - reinterpret_cast<std::uint8_t*>(exeModule));
        spdlog::info("Offsets: AppendString: 0x{:x}", SDK::Offsets::AppendString);
    }
    else {
        spdlog::error("Offsets: AppendString: Pattern scan failed.");
    }

    // ProcessEvent
    std::uint8_t* ProcessEventScanResult = Memory::PatternScan(exeModule, "40 ?? 56 57 41 ?? 41 ?? 41 ?? 41 ?? 48 81 ?? ?? ?? ?? ?? 48 8D ?? ?? ?? 48 89 ?? ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 33 ?? 48 89 ?? ?? ?? ?? ?? 8B ?? ?? 45 33 ??");
    if (ProcessEventScanResult) {
        spdlog::info("Offsets: ProcessEvent: Address is {:s}+{:x}", sExeName.c_str(), ProcessEventScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        SDK::Offsets::ProcessEvent = static_cast<UC::uint32>(ProcessEventScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        spdlog::info("Offsets: ProcessEvent: 0x{:x}", SDK::Offsets::ProcessEvent);
    }
    else {
        spdlog::error("Offsets: ProcessEvent: Pattern scan failed.");
    }

    spdlog::info("----------");
}

void Resolution()
{
    if (bCustomResolution)
    {
        // Grab desktop resolution
        DesktopDimensions = Util::GetPhysicalDesktopDimensions();

        // Use desktop resolution if custom resolution is set automatic/invalid
        if (iCustomResX <= 0 || iCustomResY <= 0) {
            iCustomResX = DesktopDimensions.first;
            iCustomResY = DesktopDimensions.second;
        }

        CalculateAspectRatio(true);

        // Apply custom resolution
        std::uint8_t* ApplyResolutionScanResult = Memory::PatternScan(exeModule, "39 ?? ?? ?? ?? 00 75 ?? 48 ?? ?? 48 ?? ?? 20 39 ?? ?? ?? ?? 00 74 ??");
        if (ApplyResolutionScanResult) {
            // Need 4 bytes aligned twice
            static struct Resolution 
            {
                int Width = iCustomResX;
                int Height = iCustomResY;
            } CustomResolution;

            spdlog::info("Custom Resolution: Address is {:s}+{:x}", sExeName.c_str(), ApplyResolutionScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid ApplyResolutionMidHook{};
            ApplyResolutionMidHook = safetyhook::create_mid(ApplyResolutionScanResult,
                [](SafetyHookContext& ctx) {
                    ctx.rdx = *(uint64_t*)&CustomResolution;
                });
        }
        else {
            spdlog::error("Custom Resolution: Pattern scan failed.");
        }
    }

    // Get current resolution
    std::uint8_t* CurrentResolutionScanResult = Memory::PatternScan(exeModule, "44 89 ?? ?? ?? ?? ?? 44 89 ?? ?? ?? ?? ?? 44 89 ?? ?? ?? ?? ??  88 ?? ?? ?? ?? ??");
    if (CurrentResolutionScanResult) {
        spdlog::info("Current Resolution: Address is {:s}+{:x}", sExeName.c_str(), CurrentResolutionScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        static SafetyHookMid CurrentResolutionMidHook{};
        CurrentResolutionMidHook = safetyhook::create_mid(CurrentResolutionScanResult,
            [](SafetyHookContext& ctx) {
                int iResX = (int)ctx.r13;
                int iResY = (int)ctx.r12;

                // Log resolution
                if (iResX != iCurrentResX || iResY != iCurrentResY) {
                    iCurrentResX = iResX;
                    iCurrentResY = iResY;
                    CalculateAspectRatio(true);
                }
            });
    }
    else {
        spdlog::error("Current Resolution: Pattern scan failed.");
    }

    // Screen Percentage
    std::uint8_t* ScreenPercentageScanResult = Memory::PatternScan(exeModule, "0F ?? ?? F3 0F ?? ?? ?? 0F ?? ?? F3 0F ?? ?? ?? ?? ?? ?? 0F ?? ?? 77 ?? F3 0F ?? ?? ?? ?? ?? ?? 48 ?? ?? ?? ?? 48 ?? ?? 20 5F C3");
    if (ScreenPercentageScanResult) {
        spdlog::info("Screen Percentage: Address is {:s}+{:x}", sExeName.c_str(), ScreenPercentageScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        static SafetyHookMid ScreenPercentageMidHook{};
        ScreenPercentageMidHook = safetyhook::create_mid(ScreenPercentageScanResult + 0x3,
            [](SafetyHookContext& ctx) {
                if (bScreenPercentage) {
                    *reinterpret_cast<float*>(ctx.rdi + (ctx.rbx * 4)) = (float)fScreenPercentage;
                }
                else {
                    // Grab screen percentage value if not applied by user.
                    if (fScreenPercentage != *reinterpret_cast<float*>(ctx.rdi + (ctx.rbx * 4)))
                        fScreenPercentage = *reinterpret_cast<float*>(ctx.rdi + (ctx.rbx * 4));
                }
            });
    }
    else {
        spdlog::error("Screen Percentage: Pattern scan failed.");
    }
}

SafetyHookInline RenTexPostLoad{};
void* RenTexPostLoad_Hooked(std::uint8_t* thisptr)
{
    // Calculate optimal resolution multiplier assuming target is 1080p
    // Screen percentage is only retrieved when the hook is run, meaning that on first boot we have to assume it is 100%
    float fOptimalRenTexResMulti = (iCustomResY * (fScreenPercentage / 100)) / 1080;

    if (iCustomResX <= 1920 || iCustomResX <= 1080)
        fOptimalRenTexResMulti = 1.0f; // Avoid lowering resolution of render targets when resolution is <1080p.

    if (fRenTexResUserMulti == 1.0f)
        fRenTexResMulti = fOptimalRenTexResMulti; // If set to 1, use the calculated optimal multiplier.
    else
        fRenTexResMulti = fOptimalRenTexResMulti * fRenTexResUserMulti;  // If not set to 1, then multiply on top using user defined value.

    if (fRenTexResMulti < 0.25f || fRenTexResMulti > 4.0f) {
        fRenTexResMulti = std::clamp(fRenTexResMulti, (float)0.25, (float)4);
        spdlog::warn("Render Texture 2D Resolution: fRenTexResMulti value invalid, clamped to {}", fRenTexResMulti);
    }
    spdlog::info("Render Texture 2D Resolution: fRenTexResMulti = {}", fRenTexResMulti);

    std::uint32_t* SizeX = (std::uint32_t*)(thisptr + 0x180);
    std::uint32_t* SizeY = (std::uint32_t*)(thisptr + 0x184);
    std::uint8_t* RTFormat = (std::uint8_t*)(thisptr + 0x19B);
    std::uint32_t* LightingGUID = (std::uint32_t*)(thisptr + 0x68);

    spdlog::info("Render Texture 2D Resolution: Old render texture resolution = {}x{}", *SizeX, *SizeY);

    *SizeX *= fRenTexResMulti;
    *SizeY *= fRenTexResMulti;

    if (*RTFormat == 6) {
        iRTCapX = *SizeX;
        iRTCapY = *SizeY;
    }

    spdlog::info("Render Texture 2D Resolution: New render texture resolution = {}x{}", *SizeX, *SizeY);

    // Run original function
    return RenTexPostLoad.stdcall<void*>(thisptr);
}

void RenderTextures()
{
    if (bRenTexResMulti)
    {
        // Render Texture 2D Resolution
        std::uint8_t* RenTex2DScanResult = Memory::PatternScan(exeModule, "8B ?? ?? ?? 00 00 44 ?? ?? ?? ?? ?? ?? 41 ?? ?? 8B ?? ?? ?? 00 00 44 ?? ?? ?? 66 ?? ?? ??");
        if (RenTex2DScanResult) {
            RenTexPostLoad = safetyhook::create_inline(reinterpret_cast<void*>(RenTex2DScanResult), RenTexPostLoad_Hooked);
            spdlog::info("Render Textures: 2D Resolution: Address is {:s}+{:x}", sExeName.c_str(), RenTex2DScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
        }
        else {
            spdlog::error("Render Textures: 2D Resolution: Pattern scan failed.");
        }

        // RT_Capture
        std::uint8_t* RTCaptureScanResult = Memory::PatternScan(exeModule, "C7 ?? ?? ?? ?? 00 80 07 00 00 C7 ?? ?? ?? ?? 00 38 04 00 00 49 ?? ??");
        if (RTCaptureScanResult) {
            spdlog::info("Render Textures: RT_Capture: Address is {:s}+{:x}", sExeName.c_str(), RTCaptureScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid RTCaptureMidHook{};
            RTCaptureMidHook = safetyhook::create_mid(RTCaptureScanResult + 0x14,
                [](SafetyHookContext& ctx) {
                    if (ctx.rax + 0x1FC && ctx.rax + 0x200) {
                        *reinterpret_cast<int*>(ctx.rax + 0x1FC) = iRTCapX;
                        *reinterpret_cast<int*>(ctx.rax + 0x200) = iRTCapY;
                    }
                });
        }
        else {
            spdlog::error("Render Textures: RT_Capture: Pattern scan failed.");
        }
    }
}

void EnableConsole()
{ 
    if (bEnableConsole) 
    {
        // Get GEngine
        for (int i = 0; i < 200; ++i) { // 20s
            Engine = SDK::UEngine::GetEngine();

            if (Engine && Engine->ConsoleClass && Engine->GameViewport)
                break;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!Engine || !Engine->ConsoleClass || !Engine->GameViewport) {
            spdlog::error("Enable Console: Failed to find GEngine address after 20 seconds.");
            return;
        }

        spdlog::info("Enable Console: GEngine address = 0x{:x}", reinterpret_cast<uintptr_t>(Engine));

        // Construct console
        SDK::UObject* NewObject = SDK::UGameplayStatics::SpawnObject(Engine->ConsoleClass, Engine->GameViewport);
        if (NewObject) {
            Engine->GameViewport->ViewportConsole = static_cast<SDK::UConsole*>(NewObject);
            spdlog::info("Enable Console: Console object constructed.");
        }
        else {
            spdlog::error("Enable Console: Failed to construct console object.");
            return;
        }

        // Get input settings
        SDK::UInputSettings* InputSettings = SDK::UInputSettings::GetDefaultObj();

        if (InputSettings) {
            if (InputSettings->ConsoleKeys && InputSettings->ConsoleKeys.Num() > 0) {
                spdlog::info("Enable Console: Console enabled - access it using the '{}' key.", InputSettings->ConsoleKeys[0].KeyName.ToString());
            }
            else {
                spdlog::error("Enable Console: Console enabled but no console key is bound.\nAdd this to %LOCALAPPDATA%\\P3R\\Saved\\Config\\Windows\\Input.ini -\n[/Script/Engine.InputSettings]\nConsoleKeys = Tilde\nAlter the key from 'Tidle' if necessary.");
            }
        }
        else {
            spdlog::error("Enable Console: Failed to retreive input settings.");
        }
    }
}

void IntroSkip()
{
    if (bSkipLogos)
    {
        // Skip intro
        std::uint8_t* CautionSkipScanResult = Memory::PatternScan(exeModule, "FF ?? ?? 32 C0 48 ?? ?? ?? ?? 48 ?? ?? ?? ?? 0F ?? ?? ?? ?? 48 ?? ?? ?? 5F C3");
        std::uint8_t* IntroSkipScanResult = Memory::PatternScan(exeModule, "B0 03 0F ?? ?? ?? ?? ?? ?? 00 44 ?? ?? ?? ?? ?? ?? ?? 00");
        std::uint8_t* OpeningMovieScanResult = Memory::PatternScan(exeModule, "80 ?? ?? 00 0F ?? ?? ?? ?? ?? ?? 00 74 ?? F3 ?? ?? ?? ?? ??");
        std::uint8_t* NetworkCheckSkipScanResult = Memory::PatternScan(exeModule, "48 ?? ?? 8B ?? 3C 85 ?? 0F 84 ?? ?? ?? ??");
        std::uint8_t* NetworkDialogSkipScanResult = Memory::PatternScan(exeModule, "F7 ?? ?? F7 FF FF FF 0F ?? ?? C3");
        if (CautionSkipScanResult && IntroSkipScanResult && OpeningMovieScanResult && NetworkCheckSkipScanResult && NetworkDialogSkipScanResult)
        {
            // Skip caution screens
            spdlog::info("Intro Skip: Caution: Address is {:s}+{:x}", sExeName.c_str(), CautionSkipScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            Memory::PatchBytes(CautionSkipScanResult + 0x2, "\xB0\x03", 2);

            // Enable network features
            spdlog::info("Intro Skip: Network Check: Address is {:s}+{:x}", sExeName.c_str(), NetworkCheckSkipScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid NetworkCheckMidHook{};
            NetworkCheckMidHook = safetyhook::create_mid(NetworkCheckSkipScanResult + 0x3,
                [](SafetyHookContext& ctx) {
                    if (ctx.rcx + 0x3C && !bIntroSkipHasRun) {
                        if (*reinterpret_cast<int*>(ctx.rcx + 0x3C) == 0)
                            *reinterpret_cast<int*>(ctx.rcx + 0x3C) = 3; // Enable network features

                        if (*reinterpret_cast<int*>(ctx.rcx + 0x3C) == 5)
                            bIntroSkipHasRun = true;
                    }
                });

            // Skip network dialog to confirm
            spdlog::info("Intro Skip: Network Dialog: Address is {:s}+{:x}", sExeName.c_str(), NetworkDialogSkipScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid NetworkDialogMidHook{};
            NetworkDialogMidHook = safetyhook::create_mid(NetworkDialogSkipScanResult + 0xA,
                [](SafetyHookContext& ctx) {
                    if (!bIntroSkipHasRun)
                        ctx.rax |= 1;                               
                });

            switch (iSkipLogos) {
            case 1:
                iSkipLogos = 4; // Opening Movie
                break;
            case 2:
                iSkipLogos = 5; // Main Menu
                break;
            case 3:
                iSkipLogos = 8; // Load Save
                break;
            }

            // Fix softlock if skipping to opening movies
            spdlog::info("Intro Skip: Opening Movie: Address is {:s}+{:x}", sExeName.c_str(), OpeningMovieScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            if (iSkipLogos == 4) {
                Memory::PatchBytes(OpeningMovieScanResult + 0xD, "\x1A", 1);
            }

            // Skip logos to title state
            spdlog::info("Intro Skip: Logo Skip: Address is {:s}+{:x}", sExeName.c_str(), IntroSkipScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid IntroSkipMidHook{};
            IntroSkipMidHook = safetyhook::create_mid(IntroSkipScanResult + 0x2,
                [](SafetyHookContext& ctx) {
                    if (iSkipLogos != 4) {
                        ctx.rax &= ~(0xFF);
                        ctx.rax |= iSkipLogos;
                    }
                });
        }
        else
        {
            spdlog::error("Intro Skip: Pattern scan(s) failed.");
        }     
    }
}

void AspectRatioFOV()
{
    if (bAspectFix) 
    {
        // Aspect Ratio / FOV
        std::uint8_t* AspectRatioFOVScanResult = Memory::PatternScan(exeModule, "F3 0F ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? 0F ?? ?? ?? ?? ?? ?? 33 ?? ??");
        if (AspectRatioFOVScanResult) {
            spdlog::info("Aspect Ratio/FOV: Address is {:s}+{:x}", sExeName.c_str(), AspectRatioFOVScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid FOVMidHook{};
            FOVMidHook = safetyhook::create_mid(AspectRatioFOVScanResult,
                [](SafetyHookContext& ctx) {
                    // Fix vert- FOV
                    if (fAspectRatio > fNativeAspect)
                        ctx.xmm0.f32[0] = atanf(tanf(ctx.xmm0.f32[0] * (fPi / 360)) / fNativeAspect * fAspectRatio) * (360 / fPi);
                });

            static SafetyHookMid AspectRatioMidHook{};
            AspectRatioMidHook = safetyhook::create_mid(AspectRatioFOVScanResult + 0xB,
                [](SafetyHookContext& ctx) {
                    ctx.rax = static_cast<uintptr_t>(std::bit_cast<uint32_t>(fAspectRatio));
                });
        }
        else {
            spdlog::error("Aspect Ratio/FOV: Pattern scan failed.");
        } 
        
        // Aspect Ratio Check
        std::uint8_t* AspectRatioCheckScanResult = Memory::PatternScan(exeModule, "41 ?? 01 00 00 00 85 ?? 0F 8E ?? ?? ?? ?? 8B ?? ?? 85 ??");
        if (AspectRatioCheckScanResult) {
            spdlog::info("Aspect Ratio Check: Address is {:s}+{:x}", sExeName.c_str(), AspectRatioCheckScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            Memory::PatchBytes(AspectRatioCheckScanResult + 0x9, "\x85", 1);
            spdlog::info("Aspect Ratio Check: Instruction patched.");
        }
        else {
            spdlog::error("Aspect Ratio Check: Pattern scan failed.");
        }
    }    
}

void HUDFix()
{
    if (bHUDFix || bRenTexResMulti)
    {
        // HUD Rect
        std::uint8_t* HUDRectScanResult = Memory::PatternScan(exeModule, "F3 0F ?? ?? ?? ?? ?? ?? F3 41 ?? ?? ?? ?? 0F 28 ?? ?? ?? 66 0F ?? ?? F3 0F ?? ??");
        if (HUDRectScanResult) {
            spdlog::info("HUD: Rect: Address is {:s}+{:x}", sExeName.c_str(), HUDRectScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid HUDRectMidHook{};
            HUDRectMidHook = safetyhook::create_mid(HUDRectScanResult,
                [](SafetyHookContext& ctx) {
                    ctx.xmm10.f32[0] += ctx.xmm2.f32[0];        // Width + width offset
                    ctx.xmm2.f32[0] = ctx.xmm4.f32[0] = 0.0f;   // Width/height offset
                });
        }
        else {
            spdlog::error("HUD: Rect: Pattern scan failed.");
        }

        // FFWD effect
        std::uint8_t* FFWDEffectScanResult = Memory::PatternScan(exeModule, "F3 0F ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F 28 ?? ?? ?? ?? ?? ?? 0F 28 ?? ?? ?? ?? ?? ?? 44 0F 28 ?? ?? ?? ?? ?? ?? 44 0F 28 ?? ?? ?? ?? ?? ?? 48 81 ?? ?? ?? ?? ??");
        if (FFWDEffectScanResult) {
            spdlog::info("HUD: FFWD Effect: Address is {:s}+{:x}", sExeName.c_str(), FFWDEffectScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid FFWDEffectMidHook{};
            FFWDEffectMidHook = safetyhook::create_mid(FFWDEffectScanResult,
                [](SafetyHookContext& ctx) {
                    if (fAspectRatio > fNativeAspect) {
                        float Width = 1080.0f * fAspectRatio;
                        float WidthOffset = Width - 1920.0f;

                        ctx.xmm3.f32[0] = Width;
                        *reinterpret_cast<float*>(ctx.rsp + 0x38) = Width;
                        ctx.xmm1.f32[0] = -WidthOffset;
                        *reinterpret_cast<float*>(ctx.rsp + 0x28) = -WidthOffset;
                    }
                    else if (fAspectRatio < fNativeAspect) {
                        // TODO
                    }
                });
        }
        else {
            spdlog::error("HUD: FFWD Effect: Pattern scan failed.");
        }

        // Get fade status
        std::uint8_t* FadeStatusScanResult = Memory::PatternScan(exeModule, "40 ?? 48 ?? ?? ?? 4C ?? ?? ?? ?? ?? 00 0F ?? ?? 0F ?? ?? ?? ?? 0F ?? ?? 48 ?? ??");
        if (FadeStatusScanResult) {
            spdlog::info("HUD: Fade Status: Address is {:s}+{:x}", sExeName.c_str(), FadeStatusScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid FadeStatusMidHook{};
            FadeStatusMidHook = safetyhook::create_mid(FadeStatusScanResult,
                [](SafetyHookContext& ctx) {
                    iFadeStatus = *reinterpret_cast<int*>(ctx.rcx + 0x30);
                });
        }
        else {
            spdlog::error("HUD: Fade Status: Pattern scan failed.");
        }

        // Fades
        std::uint8_t* FadesScanResult = Memory::PatternScan(exeModule, "BA 06 00 00 00 F3 0F ?? ?? ?? ?? E8 ?? ?? ?? ?? F3 0F ?? ?? ?? ??");
        if (FadesScanResult) {
            spdlog::info("HUD: Fades: Address is {:s}+{:x}", sExeName.c_str(), FadesScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            std::uint8_t* FadesAddress = Memory::GetAbsolute(FadesScanResult + 0xC);
            spdlog::info("HUD: Fades: Function address is {:s}+{:x}", sExeName.c_str(), FadesAddress - reinterpret_cast<std::uint8_t*>(exeModule));

            static SafetyHookMid GlobalFadesMidHook{};
            GlobalFadesMidHook = safetyhook::create_mid(FadesAddress,
                [](SafetyHookContext& ctx) {
                    // Only modify values when we are in a fade transition
                    if (iFadeStatus != 0) {
                        if (ctx.xmm2.f32[0] == 0.0f && ctx.xmm3.f32[0] == 0.0f && ctx.xmm10.f32[0] != 1920.0f) {
                            if (fAspectRatio > fNativeAspect)
                                ctx.xmm2.f32[0] = -(fHUDWidthOffset * 2);
                            else if (fAspectRatio < fNativeAspect)
                                ctx.xmm3.f32[0] = -(fHUDHeightOffset * 2);
                        }

                        if (ctx.xmm2.f32[0] == (float)1920 && ctx.xmm3.f32[0] == 0.0f && ctx.xmm10.f32[0] != 1920.0f) {
                            if (fAspectRatio > fNativeAspect)
                                ctx.xmm2.f32[0] = 1080 * fAspectRatio;
                            else if (fAspectRatio < fNativeAspect)
                                ctx.xmm3.f32[0] = -(fHUDHeightOffset * 2);
                        }

                        if (ctx.xmm2.f32[0] == 0.0f && ctx.xmm3.f32[0] == (float)1080 && ctx.xmm10.f32[0] != 1920.0f) {
                            if (fAspectRatio > fNativeAspect)
                                ctx.xmm2.f32[0] = -(fHUDWidthOffset * 2);
                            else if (fAspectRatio < fNativeAspect)
                                ctx.xmm3.f32[0] = 1920 / fAspectRatio;
                        }

                        if (ctx.xmm2.f32[0] == (float)1920 && ctx.xmm3.f32[0] == (float)1080 && ctx.xmm10.f32[0] != 1920.0f) {
                            if (fAspectRatio > fNativeAspect)
                                ctx.xmm2.f32[0] = 1080 * fAspectRatio;
                            else if (fAspectRatio < fNativeAspect)
                                ctx.xmm3.f32[0] = 1920 / fAspectRatio;
                        }
                    }
                });
        }
        else {
            spdlog::error("HUD: Fades: Pattern scan failed.");
        }

        /*
        // Fade: FadePgColorOut
        std::uint8_t* FadePgColorOutScanResult = Memory::PatternScan(exeModule, "89 ?? ?? ?? 33 ?? F3 0F ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F ?? ?? ?? 66 0F ?? ?? ?? 66 0F ?? ?? ??");
        if (FadePgColorOutScanResult) {
            spdlog::info("HUD: Fade: PgColorOut: Address is {:s}+{:x}", sExeName.c_str(), FadePgColorOutScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid FadePgColorOut1MidHook{};
            FadePgColorOut1MidHook = safetyhook::create_mid(FadePgColorOutScanResult,
                [](SafetyHookContext& ctx) {
                    if (fAspectRatio > fNativeAspect)
                        ctx.xmm2.f32[0] = 1920.0f - (1080.0f * fAspectRatio);
                    else if (fAspectRatio < fNativeAspect)
                        ctx.xmm3.f32[0] = 1080.0f - (1920.0f / fAspectRatio);
                });

            static SafetyHookMid FadePgColorOut2MidHook{};
            FadePgColorOut2MidHook = safetyhook::create_mid(FadePgColorOutScanResult + 0x42,
                [](SafetyHookContext& ctx) {
                    if (fAspectRatio > fNativeAspect)
                        ctx.xmm2.f32[0] = 1080.0f * fAspectRatio;
                    else if (fAspectRatio < fNativeAspect)
                        ctx.xmm3.f32[0] = 1920.0f / fAspectRatio;
                });

            static SafetyHookMid FadePgColorOut3MidHook{};
            FadePgColorOut3MidHook = safetyhook::create_mid(FadePgColorOutScanResult + 0x9E,
                [](SafetyHookContext& ctx) {
                    if (fAspectRatio > fNativeAspect)
                        ctx.xmm2.f32[0] = 1920.0f - (1080.0f * fAspectRatio);
                    else if (fAspectRatio < fNativeAspect)
                        ctx.xmm3.f32[0] = 1080.0f - (1920.0f / fAspectRatio);
                });

            static SafetyHookMid FadePgColorOut4MidHook{};
            FadePgColorOut4MidHook = safetyhook::create_mid(FadePgColorOutScanResult + 0xD9,
                [](SafetyHookContext& ctx) {
                     if (fAspectRatio > fNativeAspect)
                        ctx.xmm2.f32[0] = 1080.0f * fAspectRatio;
                    else if (fAspectRatio < fNativeAspect)
                        ctx.xmm3.f32[0] = 1920.0f / fAspectRatio;
                });
        }
        else {
            spdlog::error("HUD: Fade: PgColorOut: Pattern scan failed.");
        }
        */
    }
}

void Framerate()
{
    if (bUncapMenuFPS)
    {
        // Menu 60 FPS Cap
        std::uint8_t* MenuFPSCapScanResult = Memory::PatternScan(exeModule, "3B ?? 74 ?? E8 ?? ?? ?? ?? 48 ?? ?? 41 ?? 01 8B ?? E8 ?? ?? ?? ??");
        if (MenuFPSCapScanResult) {
            spdlog::info("Framerate: Menu FPS Cap: Address is {:s}+{:x}", sExeName.c_str(), MenuFPSCapScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            Memory::PatchBytes(MenuFPSCapScanResult + 0x2, "\xEB", 1);
        }
        else {
            spdlog::error("Framerate: Menu FPS Cap: Pattern scan failed.");
        }
    }

    if (bAdjustFPSCap)
    {
        // FPS Cap
        std::uint8_t* FPSCapScanResult = Memory::PatternScan(exeModule, "3B ?? ?? ?? ?? ?? 0F ?? ?? F3 0F ?? ?? ?? EB ?? 0F ?? ?? 48 ?? ?? ?? ?? 0F ?? ?? ?? ??");
        if (FPSCapScanResult) {
            spdlog::info("Framerate: FPS Cap: Address is {:s}+{:x}", sExeName.c_str(), FPSCapScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid FPSCapMidHook{};
            FPSCapMidHook = safetyhook::create_mid(FPSCapScanResult + 0x13,
                [](SafetyHookContext& ctx) {
                    ctx.xmm0.f32[0] = fFramerateCap;
                });
        }
        else
        {
            spdlog::error("Framerate: FPS Cap: Pattern scan failed.");
        }
    }
}

void WindowFocus()
{
    if (!bPauseOnFocusLoss) 
    {
        // ApplicationWindowState.OnFocusChangeBP    
        std::uint8_t* OnFocusChangeScanResult = Memory::PatternScan(exeModule, "0F ?? ?? 48 8B ?? E8 ?? ?? ?? ?? 48 85 ?? 0F 84 ?? ?? ?? ?? 48 89 ?? ?? ?? E8 ?? ?? ?? ??");
        if (OnFocusChangeScanResult) {
            spdlog::info("Window Focus: Address is {:s}+{:x}", sExeName.c_str(), OnFocusChangeScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid OnFocusChangeMidHook{};
            OnFocusChangeMidHook = safetyhook::create_mid(OnFocusChangeScanResult,
                [](SafetyHookContext& ctx) {
                    ctx.rdx = (ctx.rdx & ~0xFF) | 0x01;
                });
        }
        else {
            spdlog::error("Window Focus: Pattern scan failed.");
        }
    }
}

static SafetyHookInline RegisterRawInputDevicesHook{};
BOOL __stdcall RegisterRawInputDevices_Injected(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize) {
    // RawInputDevices pointer is const so we copy it into our new sneaky array
    RAWINPUTDEVICE* pTamperedRawInputDevices = new RAWINPUTDEVICE[uiNumDevices];
    memcpy(pTamperedRawInputDevices, pRawInputDevices, uiNumDevices * cbSize);

    for (int i = 0; i < uiNumDevices; i++) {
        // If we unregister the mouse, no we didn't :)
        // usUsagePage and usUsage are specific to mouse inputs (Related to HID documentation)
        // dwFlags least significant bit is whether device(s) are to be unregistered
        if (pTamperedRawInputDevices[i].usUsagePage == 0x1 && pTamperedRawInputDevices[i].usUsage == 0x2 && pTamperedRawInputDevices[i].dwFlags & 0b1)
        {
            spdlog::info("Mouse Fix: Tampering with mouse unregister!");
            pTamperedRawInputDevices[i].dwFlags &= ~0b1;
        }
    }

    auto output = RegisterRawInputDevicesHook.call<BOOL>(pTamperedRawInputDevices, uiNumDevices, cbSize); //Do original call, but don't unregister anything
    delete[] pTamperedRawInputDevices; //Clean up the sneaky
    return output;
}

static SafetyHookInline PeekMessageWHook{};
UINT __stdcall PeekMessageW_Injected(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    // Alrighty boys, we need to basically write our own input handling so buckle up
    auto output = PeekMessageWHook.call<UINT>(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    if (output && wRemoveMsg & 0b1) {
        // Only read (valid) messages on consumption so we don't accidentally double-process anything
        // Game seems to process raw mouse input despite unregistering it (thank God) so we don't have to fight it for messages
        if (lpMsg->message == WM_INPUT) {
            // We've just received an input message.
            UINT dwSize = 0;
            GetRawInputData((HRAWINPUT)lpMsg->lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)); //Get size of rawinput structure

            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == NULL)
                return output; // No Raw input data, abort current message injection
            
            if (GetRawInputData((HRAWINPUT)lpMsg->lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) //Actually retrieve raw input data into lpb
                spdlog::debug("Win32 API Error: GetRawInputData does not return correct size!"); //Doubt this'll happen but might as well log it
            
            RAWINPUT* raw = (RAWINPUT*)lpb;
            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                // We got the good stuff, mouse input data!
                if (!raw->data.mouse.usFlags) {
                    if (raw->data.mouse.lLastX || raw->data.mouse.lLastY) {
                        bLastValidInputWasFromMouse = true;
                    }
                    // These numbers were completely just eyeballed. TODO: if you're bored you can find more "accurate" numbers
                    fRawMouseX += raw->data.mouse.lLastX / 1200.0f;
                    fRawMouseY += raw->data.mouse.lLastY / 1200.0f;
                }
            }
            delete[] lpb;
        }
    }
    return output;
}

void MouseFix()
{
    if (bMouseFix)
    {
        // These *are* technically separate fixes, but doing only one of them leads to wacky results with the camera movement
        std::uint8_t* CheckIfCameraShouldMoveScanResult = Memory::PatternScan(exeModule, "?? 8D ?? ?? ?? F3 ?? 0F 59 ?? ?? 8B ?? F3 0F 59 ?? F3 ?? 0F 11 ?? ?? ?? F3 0F 11 ?? ?? ?? ?? 8B ?? FF ?? ?? ?? ?? ??");
        std::uint8_t* DefaultCameraInputSpyScanResult = Memory::PatternScan(exeModule, "0F 83 ?? ?? ?? ?? ?? 8B ?? ?? 8B ?? FF ?? ?? ?? ?? ?? F3 0F 10 ?? ?? F3 0F 10 ?? F3 0F 10");
        std::uint8_t* RemovePitchSmoothingScanResult = Memory::PatternScan(exeModule, "F3 0F 58 ?? E8 ?? ?? ?? ?? F3 0F 10 ?? ?? ?? ?? ?? ?? 8B ?? 0F 2F");
        std::uint8_t* RemoveYawSmoothingScanResult = Memory::PatternScan(exeModule, "?? 8B ?? ?? 8B ?? 0F 28 ?? FF ?? ?? ?? ?? ?? ?? 89 ?? ?? ?? ?? ?? E9");
        if (CheckIfCameraShouldMoveScanResult && DefaultCameraInputSpyScanResult && RemovePitchSmoothingScanResult && RemoveYawSmoothingScanResult) {
            // Old gamepad detection broke once raw input events were enabled. Presumably Unreal started treating mouse inputs
            // properly and thus it did the same processing as with a gamepad.

            // Part 1 v2: Scrap the game's mouse input handling entirely and just make our own from the Windows raw input API
            spdlog::info("Mouse Fix - Win32 RegisterRawInputDevices Hook: Address is {:x}", (uintptr_t)RegisterRawInputDevices);
            RegisterRawInputDevicesHook = safetyhook::create_inline(RegisterRawInputDevices, RegisterRawInputDevices_Injected); //Prevent game from disabling raw input. No I don't know why it does this
            // Unreal seems to use PeekMessageW to get events from windows and nothing else. Works for me
            // I don't want to accidentally consume messages (thus preventing the game getting them) so we can intercept here
            // With this we can spy on all messages (including raw input). The game still processes raw input events even though they're usually not enabled
            spdlog::info("Mouse Fix - Win32 PeekMessageW Hook: Address is {:x}", (uintptr_t)PeekMessageW);
            PeekMessageWHook = safetyhook::create_inline(PeekMessageW, PeekMessageW_Injected);

            spdlog::info("Mouse Fix - Dialogue Detection: Address is {:s}+{:x}", sExeName.c_str(), CheckIfCameraShouldMoveScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid CheckIfCameraShouldMoveHook{};
            CheckIfCameraShouldMoveHook = safetyhook::create_mid(CheckIfCameraShouldMoveScanResult,
                [](SafetyHookContext& ctx) {
                    // So it turns out one of my old hooks conveniently is only called when the camera should move?
                    // Horrendously untested but whatever it solves my immediate problem
                    if (bCameraShouldMoveHasRunThisFrame) {
                        // Failsafe check. This being true means our mouse fix didn't run last frame for some reason
                        // To avoid wacky buildups of raw mouse values we'll reset them here
                        fRawMouseX = 0.0f;
                        fRawMouseY = 0.0f;
                    }
                    bCameraShouldMoveHasRunThisFrame = true;
                });

            // Part 2: Bypass the camera smoothing calculations when processing player input
            spdlog::info("Mouse Fix - Default Camera Input Spy: Address is {:s}+{:x}", sExeName.c_str(), DefaultCameraInputSpyScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid DefaultCameraInputSpyHook{};
            DefaultCameraInputSpyHook = safetyhook::create_mid(DefaultCameraInputSpyScanResult,
                [](SafetyHookContext& ctx) {
                    // This is a convenient spot to read the default game input for the camera
                    fInputVectorPtr = reinterpret_cast<float*>(ctx.rax + 0x25c); //(X,Y,Z)

                    // New gamepad detection. Relatively simple: if game says we move but mouse no move, then something else must be controlling the game
                    if ((fInputVectorPtr[0] != 0.0f || fInputVectorPtr[1] != 0.0f) && (fRawMouseX == 0.0f && fRawMouseY == 0.0f) && bIgnoreGamepad)
                    {
                        // Input vector is non-zero but mouse input is zero. Gamepad detected!
                        bLastValidInputWasFromMouse = false;
                    }

                    // Deadzone while strafing fix
                    // Normally this only jumps when the game input vector's magnitude is > 0.01, which doesn't account for precise mouse movements
                    // Normal check is still carried out before this hook though
                    if (fRawMouseX != 0.0f || fRawMouseY != 0.0f) {
                        // Next instruction is a JNC
                        ctx.rflags = ctx.rflags & ~0b1; //Set Carry to 0, ensuring we jump (doing player input instead of strafe camera)
                    }
                });

            // I purposely only replace these 2 calls to the original camera calculation to preserve features like
            // holding left and right turning the camera or the camera moving back to neutral on slopes sometimes(?)
            // when there's no input
            spdlog::info("Mouse Fix - Remove Pitch SmoothCam: Address is {:s}+{:x}", sExeName.c_str(), RemovePitchSmoothingScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid RemovePitchSmoothingHook;
            RemovePitchSmoothingHook = safetyhook::create_mid(RemovePitchSmoothingScanResult,
                [](SafetyHookContext& ctx) {
                    // Documentation time:
                    // As far as I know: XMM6 is the pitch value to be added. XMM12 is P3R's Camera Speed multiplier
                    // XMM10 is the deltaTime for this frame but the rawMouse value is already cumulative throughout the frame so you don't need to use it
                    if (bLastValidInputWasFromMouse && bCameraShouldMoveHasRunThisFrame)
                    {
                        // Structure that stores camera behavior parameters.
                        // Values in order: (Speed, Accel, Decel, Press, Release, CurrentSpeed, CurrentAccel)
                        // No, I don't really know what Press and Release are, they seemed to be something to do modifying acceleration over time
                        // CurrentSpeed and CurrentAccel are the speed that the camera is supposed to move at this frame
                        float* fPitchParams = reinterpret_cast<float*>(ctx.rbx + 0x104); 

                        ctx.xmm6.f32[0] = -fRawMouseY * fPitchParams[0] * ctx.xmm12.f32[0] * fMouseMultiplierY;

                        // Zeroing these out because later frames act on them and move the camera further (doing a smooth deceleration)
                        fPitchParams[5] = 0.0f;
                        fPitchParams[6] = 0.0f;
                    }
                    fRawMouseY = 0.0f; //Reset raw mouse for next frame
                });

            spdlog::info("Mouse Fix - Remove Yaw SmoothCam: Address is {:s}+{:x}", sExeName.c_str(), RemoveYawSmoothingScanResult - reinterpret_cast<std::uint8_t*>(exeModule));
            static SafetyHookMid RemoveYawSmoothingHook;
            RemoveYawSmoothingHook = safetyhook::create_mid(RemoveYawSmoothingScanResult,
                // Coincidentally all the registers are the same for this hook as in the above one.
                // XMM0 is the default game speed calculation if you want that for some reason. It was overwritten in the last hook but here it's available
                [](SafetyHookContext& ctx) {
                    if (bLastValidInputWasFromMouse && bCameraShouldMoveHasRunThisFrame) {
                        // See previous hook for details
                        float* fYawParams = reinterpret_cast<float*>(ctx.rbx + 0xe8);

                        ctx.xmm6.f32[0] = fRawMouseX * fYawParams[0] * ctx.xmm12.f32[0] * fMouseMultiplierX;

                        fYawParams[5] = 0.0f;
                        fYawParams[6] = 0.0f;
                    }
                    fRawMouseX = 0.0f;

                    // Frame (as far as we care) has ended. Reset dialogue check state
                    bCameraShouldMoveHasRunThisFrame = false;
                });
        }
        else {
            spdlog::error("Mouse Fix: Pattern scan(s) failed");
        }
    }
}

DWORD __stdcall Main(void*)
{
    Logging();
    Configuration();
    UpdateOffsets();
    Resolution();
    RenderTextures();
    EnableConsole();
    IntroSkip();
    AspectRatioFOV();
    HUDFix();
    Framerate();
    WindowFocus();
    MouseFix();
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        thisModule = hModule;
        HANDLE mainHandle = CreateThread(NULL, 0, Main, 0, NULL, 0);
        if (mainHandle)
        {
            SetThreadPriority(mainHandle, THREAD_PRIORITY_HIGHEST); // set our Main thread priority higher than the games thread
            CloseHandle(mainHandle);
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

