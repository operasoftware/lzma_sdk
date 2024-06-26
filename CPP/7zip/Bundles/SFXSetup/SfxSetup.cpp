// Main.cpp

#include "StdAfx.h"

#include "../../../../C/DllSecur.h"

#include "../../../Common/MyWindows.h"
#include "../../../Common/MyInitGuid.h"

#include "../../../Common/CommandLineParser.h"
#include "../../../Common/StringConvert.h"
#include "../../../Common/TextConfig.h"

#include "../../../Windows/DLL.h"
#include "../../../Windows/ErrorMsg.h"
#include "../../../Windows/FileDir.h"
#include "../../../Windows/FileFind.h"
#include "../../../Windows/FileIO.h"
#include "../../../Windows/FileName.h"
#include "../../../Windows/NtCheck.h"
#include "../../../Windows/ResourceString.h"

#include "../../UI/Explorer/MyMessages.h"

#include "ExtractEngine.h"

#ifdef OPERA_CUSTOM_CODE
#include "opera/Files.h"
#include "opera/Flags.h"
#include "opera/PayloadManager.h"

#include <chrono>
#include <filesystem>
#include <thread>
#endif // OPERA_CUSTOM_CODE

#include "resource.h"

using namespace NWindows;
using namespace NFile;
using namespace NDir;

extern
HINSTANCE g_hInstance;
HINSTANCE g_hInstance;

static CFSTR const kTempDirPrefix = FTEXT("7zS");

#define MY_SHELL_EXECUTE

static bool ReadDataString(CFSTR fileName, LPCSTR startID,
    LPCSTR endID, AString &stringResult)
{
  stringResult.Empty();
  NIO::CInFile inFile;
  if (!inFile.Open(fileName))
    return false;
  const size_t kBufferSize = (1 << 12);

  Byte buffer[kBufferSize];
  const unsigned signatureStartSize = MyStringLen(startID);
  const unsigned signatureEndSize = MyStringLen(endID);

// maxPos prevents reading too long files before encountering expected data.
// SFX itself should be smaller than 1MB, anything above is probably archive to
// be extracted. Debug builds size limit will be raised to 16 MB.
#ifdef _DEBUG
  const unsigned maxPos = (1 << 24);
#else
  const unsigned maxPos = (1 << 20);
#endif // _DEBUG


  size_t numBytesPrev = 0;
  bool writeMode = false;
  UInt64 posTotal = 0;
  for (;;)
  {
    if (posTotal > maxPos)
      return (stringResult.IsEmpty());
    const size_t numReadBytes = kBufferSize - numBytesPrev;
    size_t processedSize;
    if (!inFile.ReadFull(buffer + numBytesPrev, numReadBytes, processedSize))
      return false;
    if (processedSize == 0)
      return true;
    const size_t numBytesInBuffer = numBytesPrev + processedSize;
    UInt32 pos = 0;
    for (;;)
    {
      if (writeMode)
      {
        if (pos + signatureEndSize > numBytesInBuffer)
          break;
        if (memcmp(buffer + pos, endID, signatureEndSize) == 0)
          return true;
        const Byte b = buffer[pos];
        if (b == 0)
          return false;
        stringResult += (char)b;
        pos++;
      }
      else
      {
        if (pos + signatureStartSize > numBytesInBuffer)
          break;
        if (memcmp(buffer + pos, startID, signatureStartSize) == 0)
        {
          writeMode = true;
          pos += signatureStartSize;
        }
        else
          pos++;
      }
    }
    numBytesPrev = numBytesInBuffer - pos;
    posTotal += pos;
    memmove(buffer, buffer + pos, numBytesPrev);
  }
}

static char kStartID[] = { ',','!','@','I','n','s','t','a','l','l','@','!','U','T','F','-','8','!', 0 };
static char kEndID[]   = { ',','!','@','I','n','s','t','a','l','l','E','n','d','@','!', 0 };

static struct CInstallIDInit
{
  CInstallIDInit()
  {
    kStartID[0] = ';';
    kEndID[0] = ';';
  }
} g_CInstallIDInit;


#if defined(_WIN32) && defined(_UNICODE) && !defined(_WIN64) && !defined(UNDER_CE)
#define NT_CHECK_FAIL_ACTION ShowErrorMessage(L"Unsupported Windows version"); return 1;
#endif

static void ShowErrorMessageSpec(const UString &name)
{
  UString message = NError::MyFormatMessage(::GetLastError());
  const int pos = message.Find(L"%1");
  if (pos >= 0)
  {
    message.Delete((unsigned)pos, 2);
    message.Insert((unsigned)pos, name);
  }
  ShowErrorMessage(NULL, message);
}

namespace {
void TrimPathSeparatorSuffix(UString& path) {
  if (!path.IsEmpty() && IS_PATH_SEPAR(path.Back())) {
    path.DeleteBack();
  }
}

// Parses that install dir exist, or can be created.
// %%S will be replaced with sfx file parent dir.
// %%C will be replaced with current working directory.
bool HandleInstallPath(UString& installPath) {
  if (installPath.IsEmpty()) {
    return true;
  }

  TrimPathSeparatorSuffix(installPath);

  UString path;
  GetCurrentDir(path);
  TrimPathSeparatorSuffix(path);
  installPath.Replace(L"%%C", path);

  path = NDLL::GetModuleDirPrefix();
  TrimPathSeparatorSuffix(path);
  installPath.Replace(L"%%S", path);

  return CreateComplexDir(installPath);
}

#ifdef OPERA_CUSTOM_CODE
// Function responsible for reading data that server might have added, and
// updating switches so data can be passed to executed application.
void HandlePayload(UString& switches,
                   const FString& sfxPath,
                   const FString& installPath) {
  const auto haveTrackingDataFlag =
      switches.Find(opera::flags::ServerTrackingBlob()) != -1;
  const auto haveCustomizationPackageFlag =
      switches.Find(opera::flags::CustomizationPackage()) != -1;

  // Both types of data are provided in command line.
  // Skip searching for payload.
  if (haveTrackingDataFlag && haveCustomizationPackageFlag) {
    return;
  }

  const auto payloadManager = opera::PayloadManager(sfxPath.Ptr());
  const auto& trackingData = payloadManager.GetTrackingData();
  const auto& customizationPackage = payloadManager.GetCustomizationPackage();
  if (!haveTrackingDataFlag && trackingData.GetStatus()) {
    switches.Add_Space_if_NotEmpty();
    switches += opera::flags::ServerTrackingBlob();
    switches += UString(trackingData.GetData().c_str());
  }

  if (!haveCustomizationPackageFlag && customizationPackage.GetStatus()) {
    const auto customizationPackagePath =
        std::filesystem::path(installPath.Ptr()) / "customization_package.bin";
    opera::files::Write(customizationPackagePath,
                        customizationPackage.GetData());
    switches.Add_Space_if_NotEmpty();
    switches += opera::flags::CustomizationPackage();
    switches +=
        UString((L"\"" + customizationPackagePath.wstring() + L"\"").c_str());
  }
}
#endif  // OPERA_CUSTOM_CODE
}  // namespace



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
    #ifdef UNDER_CE
    LPWSTR
    #else
    LPSTR
    #endif
    /* lpCmdLine */,int /* nCmdShow */)
{
  g_hInstance = (HINSTANCE)hInstance;

  NT_CHECK

  #ifdef _WIN32
  LoadSecurityDlls();
  #endif

  // InitCommonControls();

  UString archiveName, switches;
  #ifdef MY_SHELL_EXECUTE
  UString executeFile, executeParameters;
  #endif
  NCommandLineParser::SplitCommandLine(GetCommandLineW(), archiveName, switches);

  FString fullPath;
  NDLL::MyGetModuleFileName(fullPath);

  switches.Trim();
  bool assumeYes = false;
  if (switches.IsPrefixedBy_Ascii_NoCase("-y"))
  {
    assumeYes = true;
    switches = switches.Ptr(2);
    switches.Trim();
  }

  // TODO investigate.
  // Reading config seams to not work in debug.
  // Empty string will be considered a broken configuration, as it would unpack
  // archive, but do nothing with it and remove it afterwards.
  AString config;
  if (!ReadDataString(fullPath, kStartID, kEndID, config) || config.IsEmpty())
  {
    if (!assumeYes)
      ShowErrorMessage(L"Can't load config info");
    return 1;
  }

  UString appLaunched;
  UString installPath;
  bool showProgress = false;
  {
    CObjectVector<CTextConfigPair> pairs;
    if (!GetTextConfig(config, pairs))
    {
      if (!assumeYes)
        ShowErrorMessage(L"Config failed");
      return 1;
    }
    const UString friendlyName = GetTextConfigValue(pairs, "Title");
    const UString installPrompt = GetTextConfigValue(pairs, "BeginPrompt");
    const UString progress = GetTextConfigValue(pairs, "Progress");

    if (progress.IsEqualTo_Ascii_NoCase("yes"))
      showProgress = true;
    if (!installPrompt.IsEmpty() && !assumeYes)
    {
      if (MessageBoxW(NULL, installPrompt, friendlyName, MB_YESNO |
          MB_ICONQUESTION) != IDYES)
        return 0;
    }
    appLaunched = GetTextConfigValue(pairs, "RunProgram");

    #ifdef MY_SHELL_EXECUTE
    executeFile = GetTextConfigValue(pairs, "ExecuteFile");
    executeParameters = GetTextConfigValue(pairs, "ExecuteParameters");
    #endif

    installPath = GetTextConfigValue(pairs, "InstallPath");
    if (!HandleInstallPath(installPath))
    {
	  if (!assumeYes)
		ShowErrorMessage(L"Install path is incorrect");
      return 1;
    }

  }

  CTempDir tempDir;
  // If install path is provided the temp dir will not be created.
  // Temporary location is automatically removed, but one explicitly provided
  // will not.
  if (installPath.IsEmpty())
  {
    if (!tempDir.Create(kTempDirPrefix))
    {
      ShowErrorMessage(L"Cannot create temp folder archive", assumeYes);
      return 1;
    }

    installPath = tempDir.GetPath();
  }

  CCodecs *codecs = new CCodecs;
  CMyComPtr<IUnknown> compressCodecsInfo = codecs;
  {
    const HRESULT result = codecs->Load();
    if (result != S_OK)
    {
      ShowErrorMessage(L"Cannot load codecs");
      return 1;
    }
  }

  {
    bool isCorrupt = false;
    UString errorMessage;
    HRESULT result = ExtractArchive(codecs, fullPath, installPath, showProgress,
      isCorrupt, errorMessage);

    if (result != S_OK)
    {
      if (!assumeYes)
      {
        if (result == S_FALSE || isCorrupt)
        {
          NWindows::MyLoadString(IDS_EXTRACTION_ERROR_MESSAGE, errorMessage);
          result = E_FAIL;
        }
        if (result != E_ABORT)
        {
          if (errorMessage.IsEmpty())
            errorMessage = NError::MyFormatMessage(result);
          ::MessageBoxW(NULL, errorMessage, NWindows::MyLoadString(IDS_EXTRACTION_ERROR_TITLE), MB_ICONERROR);
        }
      }
      return 1;
    }
  }

  #ifndef UNDER_CE
  CCurrentDirRestorer currentDirRestorer;
  if (!SetCurrentDir(installPath))
    return 1;
  #endif

  DWORD exitCode = 0;
  HANDLE hProcess = NULL;
#ifdef MY_SHELL_EXECUTE
  if (!executeFile.IsEmpty())
  {
    CSysString filePath (GetSystemString(executeFile));
    SHELLEXECUTEINFO execInfo;
    execInfo.cbSize = sizeof(execInfo);
    execInfo.fMask = SEE_MASK_NOCLOSEPROCESS
      #ifndef UNDER_CE
      | SEE_MASK_FLAG_DDEWAIT
      #endif
      ;
    execInfo.hwnd = NULL;
    execInfo.lpVerb = NULL;
    execInfo.lpFile = filePath;

    if (!switches.IsEmpty())
    {
      executeParameters.Add_Space_if_NotEmpty();
      executeParameters += switches;
    }

    const CSysString parametersSys (GetSystemString(executeParameters));
    if (parametersSys.IsEmpty())
      execInfo.lpParameters = NULL;
    else
      execInfo.lpParameters = parametersSys;

    execInfo.lpDirectory = NULL;
    execInfo.nShow = SW_SHOWNORMAL;
    execInfo.hProcess = NULL;
    /* BOOL success = */ ::ShellExecuteEx(&execInfo);
    UINT32 result = (UINT32)(UINT_PTR)execInfo.hInstApp;
    if (result <= 32)
    {
      if (!assumeYes)
        ShowErrorMessage(L"Cannot open file");
      return 1;
    }
    hProcess = execInfo.hProcess;
  }
  else
#endif
  {
    if (appLaunched.IsEmpty())
      return 0;
    appLaunched.Replace(L"%%T", installPath);
    const UString appNameForError = appLaunched;

#ifdef OPERA_CUSTOM_CODE
    HandlePayload(switches, fullPath, installPath);
#endif // OPERA_CUSTOM_CODE

    if (!switches.IsEmpty())
    {
      appLaunched.Add_Space();
      appLaunched += switches;
    }

    STARTUPINFO startupInfo;
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpReserved = NULL;
    startupInfo.lpDesktop = NULL;
    startupInfo.lpTitle = NULL;
    startupInfo.dwFlags = 0;
    startupInfo.cbReserved2 = 0;
    startupInfo.lpReserved2 = NULL;

    PROCESS_INFORMATION processInformation;

    const CSysString appLaunchedSys (GetSystemString(appLaunched));

    const BOOL createResult = CreateProcess(NULL,
      appLaunchedSys.Ptr_non_const(),
        NULL, NULL, FALSE, 0, NULL, NULL /*tempDir.GetPath() */,
        &startupInfo, &processInformation);
    if (createResult == 0)
    {
      if (!assumeYes)
      {
        // we print name of exe file, if error message is
        // ERROR_BAD_EXE_FORMAT: "%1 is not a valid Win32 application".
        ShowErrorMessageSpec(appNameForError);
      }
      return 1;
    }
    ::CloseHandle(processInformation.hThread);
    hProcess = processInformation.hProcess;
  }
  if (hProcess)
  {
    WaitForSingleObject(hProcess, INFINITE);
    GetExitCodeProcess(hProcess, &exitCode);
    ::CloseHandle(hProcess);

#ifdef OPERA_CUSTOM_CODE
    // TODO investigate temporary files not being removed unless delay is added.
    // This is probably because process starts another instance that is not awaited.
    // Windows blocks file, and removal fails.
    std::this_thread::sleep_for(std::chrono::seconds(1));
#endif // OPERA_CUSTOM_CODE
  }
  return exitCode;
}
