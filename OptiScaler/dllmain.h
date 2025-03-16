#pragma once
#include "pch.h"
#include "Config.h"

#include "detours/detours.h"

#include <dxgi1_6.h>
#include <DbgHelp.h>

#pragma region DXGI definitions

typedef HRESULT(*PFN_CREATE_DXGI_FACTORY)(REFIID riid, IDXGIFactory** ppFactory);
typedef HRESULT(*PFN_CREATE_DXGI_FACTORY_1)(REFIID riid, IDXGIFactory1** ppFactory);
typedef HRESULT(*PFN_CREATE_DXGI_FACTORY_2)(UINT Flags, REFIID riid, IDXGIFactory2** ppFactory);
typedef HRESULT(*PFN_DECLARE_ADAPTER_REMOVAL_SUPPORT)();
typedef HRESULT(*PFN_GET_DEBUG_INTERFACE)(UINT Flags, REFIID riid, void** ppDebug);

typedef HRESULT(*PFN_GetDesc)(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc);
typedef HRESULT(*PFN_GetDesc1)(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc);
typedef HRESULT(*PFN_GetDesc2)(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc);
typedef HRESULT(*PFN_GetDesc3)(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc);

typedef HRESULT(*PFN_EnumAdapterByGpuPreference)(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IDXGIAdapter** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapterByLuid)(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IDXGIAdapter** ppvAdapter);
typedef HRESULT(*PFN_EnumAdapters1)(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter);
typedef HRESULT(*PFN_EnumAdapters)(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter);

inline static PFN_GetDesc ptrGetDesc = nullptr;
inline static PFN_GetDesc1 ptrGetDesc1 = nullptr;
inline static PFN_GetDesc2 ptrGetDesc2 = nullptr;
inline static PFN_GetDesc3 ptrGetDesc3 = nullptr;

inline static PFN_EnumAdapters ptrEnumAdapters = nullptr;
inline static PFN_EnumAdapters1 ptrEnumAdapters1 = nullptr;
inline static PFN_EnumAdapterByLuid ptrEnumAdapterByLuid = nullptr;
inline static PFN_EnumAdapterByGpuPreference ptrEnumAdapterByGpuPreference = nullptr;

inline bool skipGetModuleHandle = false;
inline bool skipHighPerfCheck = false;

void AttachToAdapter(IUnknown* unkAdapter);
void AttachToFactory(IUnknown* unkFactory);

#pragma endregion

#pragma region Structs

struct shared
{
    HMODULE dll;
    FARPROC DllCanUnloadNow;
    FARPROC DllGetClassObject;
    FARPROC DllRegisterServer;
    FARPROC DllUnregisterServer;
    FARPROC DebugSetMute;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        skipGetModuleHandle = true;
        DllCanUnloadNow = GetProcAddress(module, "DllCanUnloadNow");
        DllGetClassObject = GetProcAddress(module, "DllGetClassObject");
        DllRegisterServer = GetProcAddress(module, "DllRegisterServer");
        DllUnregisterServer = GetProcAddress(module, "DllUnregisterServer");
        DebugSetMute = GetProcAddress(module, "DebugSetMute");
        skipGetModuleHandle = false;
    }
} shared;

struct wininet_dll
{
    HMODULE dll;
    FARPROC AppCacheCheckManifest;
    FARPROC AppCacheCloseHandle;
    FARPROC AppCacheCreateAndCommitFile;
    FARPROC AppCacheDeleteGroup;
    FARPROC AppCacheDeleteIEGroup;
    FARPROC AppCacheDuplicateHandle;
    FARPROC AppCacheFinalize;
    FARPROC AppCacheFreeDownloadList;
    FARPROC AppCacheFreeGroupList;
    FARPROC AppCacheFreeIESpace;
    FARPROC AppCacheFreeSpace;
    FARPROC AppCacheGetDownloadList;
    FARPROC AppCacheGetFallbackUrl;
    FARPROC AppCacheGetGroupList;
    FARPROC AppCacheGetIEGroupList;
    FARPROC AppCacheGetInfo;
    FARPROC AppCacheGetManifestUrl;
    FARPROC AppCacheLookup;
    FARPROC CommitUrlCacheEntryA;
    FARPROC CommitUrlCacheEntryBinaryBlob;
    FARPROC CommitUrlCacheEntryW;
    FARPROC CreateMD5SSOHash;
    FARPROC CreateUrlCacheContainerA;
    FARPROC CreateUrlCacheContainerW;
    FARPROC CreateUrlCacheEntryA;
    FARPROC CreateUrlCacheEntryExW;
    FARPROC CreateUrlCacheEntryW;
    FARPROC CreateUrlCacheGroup;
    FARPROC DeleteIE3Cache;
    FARPROC DeleteUrlCacheContainerA;
    FARPROC DeleteUrlCacheContainerW;
    FARPROC DeleteUrlCacheEntry;
    FARPROC DeleteUrlCacheEntryA;
    FARPROC DeleteUrlCacheEntryW;
    FARPROC DeleteUrlCacheGroup;
    FARPROC DeleteWpadCacheForNetworks;
    FARPROC DetectAutoProxyUrl;
    FARPROC DispatchAPICall;
    FARPROC DllInstall;
    FARPROC FindCloseUrlCache;
    FARPROC FindFirstUrlCacheContainerA;
    FARPROC FindFirstUrlCacheContainerW;
    FARPROC FindFirstUrlCacheEntryA;
    FARPROC FindFirstUrlCacheEntryExA;
    FARPROC FindFirstUrlCacheEntryExW;
    FARPROC FindFirstUrlCacheEntryW;
    FARPROC FindFirstUrlCacheGroup;
    FARPROC FindNextUrlCacheContainerA;
    FARPROC FindNextUrlCacheContainerW;
    FARPROC FindNextUrlCacheEntryA;
    FARPROC FindNextUrlCacheEntryExA;
    FARPROC FindNextUrlCacheEntryExW;
    FARPROC FindNextUrlCacheEntryW;
    FARPROC FindNextUrlCacheGroup;
    FARPROC ForceNexusLookup;
    FARPROC ForceNexusLookupExW;
    FARPROC FreeUrlCacheSpaceA;
    FARPROC FreeUrlCacheSpaceW;
    FARPROC FtpCommandA;
    FARPROC FtpCommandW;
    FARPROC FtpCreateDirectoryA;
    FARPROC FtpCreateDirectoryW;
    FARPROC FtpDeleteFileA;
    FARPROC FtpDeleteFileW;
    FARPROC FtpFindFirstFileA;
    FARPROC FtpFindFirstFileW;
    FARPROC FtpGetCurrentDirectoryA;
    FARPROC FtpGetCurrentDirectoryW;
    FARPROC FtpGetFileA;
    FARPROC FtpGetFileEx;
    FARPROC FtpGetFileSize;
    FARPROC FtpGetFileW;
    FARPROC FtpOpenFileA;
    FARPROC FtpOpenFileW;
    FARPROC FtpPutFileA;
    FARPROC FtpPutFileEx;
    FARPROC FtpPutFileW;
    FARPROC FtpRemoveDirectoryA;
    FARPROC FtpRemoveDirectoryW;
    FARPROC FtpRenameFileA;
    FARPROC FtpRenameFileW;
    FARPROC FtpSetCurrentDirectoryA;
    FARPROC FtpSetCurrentDirectoryW;
    FARPROC _GetFileExtensionFromUrl;
    FARPROC GetProxyDllInfo;
    FARPROC GetUrlCacheConfigInfoA;
    FARPROC GetUrlCacheConfigInfoW;
    FARPROC GetUrlCacheEntryBinaryBlob;
    FARPROC GetUrlCacheEntryInfoA;
    FARPROC GetUrlCacheEntryInfoExA;
    FARPROC GetUrlCacheEntryInfoExW;
    FARPROC GetUrlCacheEntryInfoW;
    FARPROC GetUrlCacheGroupAttributeA;
    FARPROC GetUrlCacheGroupAttributeW;
    FARPROC GetUrlCacheHeaderData;
    FARPROC GopherCreateLocatorA;
    FARPROC GopherCreateLocatorW;
    FARPROC GopherFindFirstFileA;
    FARPROC GopherFindFirstFileW;
    FARPROC GopherGetAttributeA;
    FARPROC GopherGetAttributeW;
    FARPROC GopherGetLocatorTypeA;
    FARPROC GopherGetLocatorTypeW;
    FARPROC GopherOpenFileA;
    FARPROC GopherOpenFileW;
    FARPROC HttpAddRequestHeadersA;
    FARPROC HttpAddRequestHeadersW;
    FARPROC HttpCheckDavCompliance;
    FARPROC HttpCloseDependencyHandle;
    FARPROC HttpDuplicateDependencyHandle;
    FARPROC HttpEndRequestA;
    FARPROC HttpEndRequestW;
    FARPROC HttpGetServerCredentials;
    FARPROC HttpGetTunnelSocket;
    FARPROC HttpIsHostHstsEnabled;
    FARPROC HttpOpenDependencyHandle;
    FARPROC HttpOpenRequestA;
    FARPROC HttpOpenRequestW;
    FARPROC HttpPushClose;
    FARPROC HttpPushEnable;
    FARPROC HttpPushWait;
    FARPROC HttpQueryInfoA;
    FARPROC HttpQueryInfoW;
    FARPROC HttpSendRequestA;
    FARPROC HttpSendRequestExA;
    FARPROC HttpSendRequestExW;
    FARPROC HttpSendRequestW;
    FARPROC HttpWebSocketClose;
    FARPROC HttpWebSocketCompleteUpgrade;
    FARPROC HttpWebSocketQueryCloseStatus;
    FARPROC HttpWebSocketReceive;
    FARPROC HttpWebSocketSend;
    FARPROC HttpWebSocketShutdown;
    FARPROC IncrementUrlCacheHeaderData;
    FARPROC InternetAlgIdToStringA;
    FARPROC InternetAlgIdToStringW;
    FARPROC InternetAttemptConnect;
    FARPROC InternetAutodial;
    FARPROC InternetAutodialCallback;
    FARPROC InternetAutodialHangup;
    FARPROC InternetCanonicalizeUrlA;
    FARPROC InternetCanonicalizeUrlW;
    FARPROC InternetCheckConnectionA;
    FARPROC InternetCheckConnectionW;
    FARPROC InternetClearAllPerSiteCookieDecisions;
    FARPROC InternetCloseHandle;
    FARPROC InternetCombineUrlA;
    FARPROC InternetCombineUrlW;
    FARPROC InternetConfirmZoneCrossing;
    FARPROC InternetConfirmZoneCrossingA;
    FARPROC InternetConfirmZoneCrossingW;
    FARPROC InternetConnectA;
    FARPROC InternetConnectW;
    FARPROC InternetConvertUrlFromWireToWideChar;
    FARPROC InternetCrackUrlA;
    FARPROC InternetCrackUrlW;
    FARPROC InternetCreateUrlA;
    FARPROC InternetCreateUrlW;
    FARPROC InternetDial;
    FARPROC InternetDialA;
    FARPROC InternetDialW;
    FARPROC InternetEnumPerSiteCookieDecisionA;
    FARPROC InternetEnumPerSiteCookieDecisionW;
    FARPROC InternetErrorDlg;
    FARPROC InternetFindNextFileA;
    FARPROC InternetFindNextFileW;
    FARPROC InternetFortezzaCommand;
    FARPROC InternetFreeCookies;
    FARPROC InternetFreeProxyInfoList;
    FARPROC InternetGetCertByURL;
    FARPROC InternetGetCertByURLA;
    FARPROC InternetGetConnectedState;
    FARPROC InternetGetConnectedStateEx;
    FARPROC InternetGetConnectedStateExA;
    FARPROC InternetGetConnectedStateExW;
    FARPROC InternetGetCookieA;
    FARPROC InternetGetCookieEx2;
    FARPROC InternetGetCookieExA;
    FARPROC InternetGetCookieExW;
    FARPROC InternetGetCookieW;
    FARPROC InternetGetLastResponseInfoA;
    FARPROC InternetGetLastResponseInfoW;
    FARPROC InternetGetPerSiteCookieDecisionA;
    FARPROC InternetGetPerSiteCookieDecisionW;
    FARPROC InternetGetProxyForUrl;
    FARPROC InternetGetSecurityInfoByURL;
    FARPROC InternetGetSecurityInfoByURLA;
    FARPROC InternetGetSecurityInfoByURLW;
    FARPROC InternetGoOnline;
    FARPROC InternetGoOnlineA;
    FARPROC InternetGoOnlineW;
    FARPROC InternetHangUp;
    FARPROC InternetInitializeAutoProxyDll;
    FARPROC InternetLockRequestFile;
    FARPROC InternetOpenA;
    FARPROC InternetOpenUrlA;
    FARPROC InternetOpenUrlW;
    FARPROC InternetOpenW;
    FARPROC InternetQueryDataAvailable;
    FARPROC InternetQueryFortezzaStatus;
    FARPROC InternetQueryOptionA;
    FARPROC InternetQueryOptionW;
    FARPROC InternetReadFile;
    FARPROC InternetReadFileExA;
    FARPROC InternetReadFileExW;
    FARPROC InternetSecurityProtocolToStringA;
    FARPROC InternetSecurityProtocolToStringW;
    FARPROC InternetSetCookieA;
    FARPROC InternetSetCookieEx2;
    FARPROC InternetSetCookieExA;
    FARPROC InternetSetCookieExW;
    FARPROC InternetSetCookieW;
    FARPROC InternetSetDialState;
    FARPROC InternetSetDialStateA;
    FARPROC InternetSetDialStateW;
    FARPROC InternetSetFilePointer;
    FARPROC InternetSetOptionA;
    FARPROC InternetSetOptionExA;
    FARPROC InternetSetOptionExW;
    FARPROC InternetSetOptionW;
    FARPROC InternetSetPerSiteCookieDecisionA;
    FARPROC InternetSetPerSiteCookieDecisionW;
    FARPROC InternetSetStatusCallback;
    FARPROC InternetSetStatusCallbackA;
    FARPROC InternetSetStatusCallbackW;
    FARPROC InternetShowSecurityInfoByURL;
    FARPROC InternetShowSecurityInfoByURLA;
    FARPROC InternetShowSecurityInfoByURLW;
    FARPROC InternetTimeFromSystemTime;
    FARPROC InternetTimeFromSystemTimeA;
    FARPROC InternetTimeFromSystemTimeW;
    FARPROC InternetTimeToSystemTime;
    FARPROC InternetTimeToSystemTimeA;
    FARPROC InternetTimeToSystemTimeW;
    FARPROC InternetUnlockRequestFile;
    FARPROC InternetWriteFile;
    FARPROC InternetWriteFileExA;
    FARPROC InternetWriteFileExW;
    FARPROC IsHostInProxyBypassList;
    FARPROC IsUrlCacheEntryExpiredA;
    FARPROC IsUrlCacheEntryExpiredW;
    FARPROC LoadUrlCacheContent;
    FARPROC ParseX509EncodedCertificateForListBoxEntry;
    FARPROC PrivacyGetZonePreferenceW;
    FARPROC PrivacySetZonePreferenceW;
    FARPROC ReadUrlCacheEntryStream;
    FARPROC ReadUrlCacheEntryStreamEx;
    FARPROC RegisterUrlCacheNotification;
    FARPROC ResumeSuspendedDownload;
    FARPROC RetrieveUrlCacheEntryFileA;
    FARPROC RetrieveUrlCacheEntryFileW;
    FARPROC RetrieveUrlCacheEntryStreamA;
    FARPROC RetrieveUrlCacheEntryStreamW;
    FARPROC RunOnceUrlCache;
    FARPROC SetUrlCacheConfigInfoA;
    FARPROC SetUrlCacheConfigInfoW;
    FARPROC SetUrlCacheEntryGroup;
    FARPROC SetUrlCacheEntryGroupA;
    FARPROC SetUrlCacheEntryGroupW;
    FARPROC SetUrlCacheEntryInfoA;
    FARPROC SetUrlCacheEntryInfoW;
    FARPROC SetUrlCacheGroupAttributeA;
    FARPROC SetUrlCacheGroupAttributeW;
    FARPROC SetUrlCacheHeaderData;
    FARPROC ShowCertificate;
    FARPROC ShowClientAuthCerts;
    FARPROC ShowSecurityInfo;
    FARPROC ShowX509EncodedCertificate;
    FARPROC UnlockUrlCacheEntryFile;
    FARPROC UnlockUrlCacheEntryFileA;
    FARPROC UnlockUrlCacheEntryFileW;
    FARPROC UnlockUrlCacheEntryStream;
    FARPROC UpdateUrlCacheContentPath;
    FARPROC UrlCacheCheckEntriesExist;
    FARPROC UrlCacheCloseEntryHandle;
    FARPROC UrlCacheContainerSetEntryMaximumAge;
    FARPROC UrlCacheCreateContainer;
    FARPROC UrlCacheFindFirstEntry;
    FARPROC UrlCacheFindNextEntry;
    FARPROC UrlCacheFreeEntryInfo;
    FARPROC UrlCacheFreeGlobalSpace;
    FARPROC UrlCacheGetContentPaths;
    FARPROC UrlCacheGetEntryInfo;
    FARPROC UrlCacheGetGlobalCacheSize;
    FARPROC UrlCacheGetGlobalLimit;
    FARPROC UrlCacheReadEntryStream;
    FARPROC UrlCacheReloadSettings;
    FARPROC UrlCacheRetrieveEntryFile;
    FARPROC UrlCacheRetrieveEntryStream;
    FARPROC UrlCacheServer;
    FARPROC UrlCacheSetGlobalLimit;
    FARPROC UrlCacheUpdateEntryExtraData;
    FARPROC UrlZonesDetach;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        skipGetModuleHandle = true;
        AppCacheCheckManifest = GetProcAddress(dll, "AppCacheCheckManifest");
        AppCacheCloseHandle = GetProcAddress(dll, "AppCacheCloseHandle");
        AppCacheCreateAndCommitFile = GetProcAddress(dll, "AppCacheCreateAndCommitFile");
        AppCacheDeleteGroup = GetProcAddress(dll, "AppCacheDeleteGroup");
        AppCacheDeleteIEGroup = GetProcAddress(dll, "AppCacheDeleteIEGroup");
        AppCacheDuplicateHandle = GetProcAddress(dll, "AppCacheDuplicateHandle");
        AppCacheFinalize = GetProcAddress(dll, "AppCacheFinalize");
        AppCacheFreeDownloadList = GetProcAddress(dll, "AppCacheFreeDownloadList");
        AppCacheFreeGroupList = GetProcAddress(dll, "AppCacheFreeGroupList");
        AppCacheFreeIESpace = GetProcAddress(dll, "AppCacheFreeIESpace");
        AppCacheFreeSpace = GetProcAddress(dll, "AppCacheFreeSpace");
        AppCacheGetDownloadList = GetProcAddress(dll, "AppCacheGetDownloadList");
        AppCacheGetFallbackUrl = GetProcAddress(dll, "AppCacheGetFallbackUrl");
        AppCacheGetGroupList = GetProcAddress(dll, "AppCacheGetGroupList");
        AppCacheGetIEGroupList = GetProcAddress(dll, "AppCacheGetIEGroupList");
        AppCacheGetInfo = GetProcAddress(dll, "AppCacheGetInfo");
        AppCacheGetManifestUrl = GetProcAddress(dll, "AppCacheGetManifestUrl");
        AppCacheLookup = GetProcAddress(dll, "AppCacheLookup");
        CommitUrlCacheEntryA = GetProcAddress(dll, "CommitUrlCacheEntryA");
        CommitUrlCacheEntryBinaryBlob = GetProcAddress(dll, "CommitUrlCacheEntryBinaryBlob");
        CommitUrlCacheEntryW = GetProcAddress(dll, "CommitUrlCacheEntryW");
        CreateMD5SSOHash = GetProcAddress(dll, "CreateMD5SSOHash");
        CreateUrlCacheContainerA = GetProcAddress(dll, "CreateUrlCacheContainerA");
        CreateUrlCacheContainerW = GetProcAddress(dll, "CreateUrlCacheContainerW");
        CreateUrlCacheEntryA = GetProcAddress(dll, "CreateUrlCacheEntryA");
        CreateUrlCacheEntryExW = GetProcAddress(dll, "CreateUrlCacheEntryExW");
        CreateUrlCacheEntryW = GetProcAddress(dll, "CreateUrlCacheEntryW");
        CreateUrlCacheGroup = GetProcAddress(dll, "CreateUrlCacheGroup");
        DeleteIE3Cache = GetProcAddress(dll, "DeleteIE3Cache");
        DeleteUrlCacheContainerA = GetProcAddress(dll, "DeleteUrlCacheContainerA");
        DeleteUrlCacheContainerW = GetProcAddress(dll, "DeleteUrlCacheContainerW");
        DeleteUrlCacheEntry = GetProcAddress(dll, "DeleteUrlCacheEntry");
        DeleteUrlCacheEntryA = GetProcAddress(dll, "DeleteUrlCacheEntryA");
        DeleteUrlCacheEntryW = GetProcAddress(dll, "DeleteUrlCacheEntryW");
        DeleteUrlCacheGroup = GetProcAddress(dll, "DeleteUrlCacheGroup");
        DeleteWpadCacheForNetworks = GetProcAddress(dll, "DeleteWpadCacheForNetworks");
        DetectAutoProxyUrl = GetProcAddress(dll, "DetectAutoProxyUrl");
        DispatchAPICall = GetProcAddress(dll, "DispatchAPICall");
        DllInstall = GetProcAddress(dll, "DllInstall");
        FindCloseUrlCache = GetProcAddress(dll, "FindCloseUrlCache");
        FindFirstUrlCacheContainerA = GetProcAddress(dll, "FindFirstUrlCacheContainerA");
        FindFirstUrlCacheContainerW = GetProcAddress(dll, "FindFirstUrlCacheContainerW");
        FindFirstUrlCacheEntryA = GetProcAddress(dll, "FindFirstUrlCacheEntryA");
        FindFirstUrlCacheEntryExA = GetProcAddress(dll, "FindFirstUrlCacheEntryExA");
        FindFirstUrlCacheEntryExW = GetProcAddress(dll, "FindFirstUrlCacheEntryExW");
        FindFirstUrlCacheEntryW = GetProcAddress(dll, "FindFirstUrlCacheEntryW");
        FindFirstUrlCacheGroup = GetProcAddress(dll, "FindFirstUrlCacheGroup");
        FindNextUrlCacheContainerA = GetProcAddress(dll, "FindNextUrlCacheContainerA");
        FindNextUrlCacheContainerW = GetProcAddress(dll, "FindNextUrlCacheContainerW");
        FindNextUrlCacheEntryA = GetProcAddress(dll, "FindNextUrlCacheEntryA");
        FindNextUrlCacheEntryExA = GetProcAddress(dll, "FindNextUrlCacheEntryExA");
        FindNextUrlCacheEntryExW = GetProcAddress(dll, "FindNextUrlCacheEntryExW");
        FindNextUrlCacheEntryW = GetProcAddress(dll, "FindNextUrlCacheEntryW");
        FindNextUrlCacheGroup = GetProcAddress(dll, "FindNextUrlCacheGroup");
        ForceNexusLookup = GetProcAddress(dll, "ForceNexusLookup");
        ForceNexusLookupExW = GetProcAddress(dll, "ForceNexusLookupExW");
        FreeUrlCacheSpaceA = GetProcAddress(dll, "FreeUrlCacheSpaceA");
        FreeUrlCacheSpaceW = GetProcAddress(dll, "FreeUrlCacheSpaceW");
        FtpCommandA = GetProcAddress(dll, "FtpCommandA");
        FtpCommandW = GetProcAddress(dll, "FtpCommandW");
        FtpCreateDirectoryA = GetProcAddress(dll, "FtpCreateDirectoryA");
        FtpCreateDirectoryW = GetProcAddress(dll, "FtpCreateDirectoryW");
        FtpDeleteFileA = GetProcAddress(dll, "FtpDeleteFileA");
        FtpDeleteFileW = GetProcAddress(dll, "FtpDeleteFileW");
        FtpFindFirstFileA = GetProcAddress(dll, "FtpFindFirstFileA");
        FtpFindFirstFileW = GetProcAddress(dll, "FtpFindFirstFileW");
        FtpGetCurrentDirectoryA = GetProcAddress(dll, "FtpGetCurrentDirectoryA");
        FtpGetCurrentDirectoryW = GetProcAddress(dll, "FtpGetCurrentDirectoryW");
        FtpGetFileA = GetProcAddress(dll, "FtpGetFileA");
        FtpGetFileEx = GetProcAddress(dll, "FtpGetFileEx");
        FtpGetFileSize = GetProcAddress(dll, "FtpGetFileSize");
        FtpGetFileW = GetProcAddress(dll, "FtpGetFileW");
        FtpOpenFileA = GetProcAddress(dll, "FtpOpenFileA");
        FtpOpenFileW = GetProcAddress(dll, "FtpOpenFileW");
        FtpPutFileA = GetProcAddress(dll, "FtpPutFileA");
        FtpPutFileEx = GetProcAddress(dll, "FtpPutFileEx");
        FtpPutFileW = GetProcAddress(dll, "FtpPutFileW");
        FtpRemoveDirectoryA = GetProcAddress(dll, "FtpRemoveDirectoryA");
        FtpRemoveDirectoryW = GetProcAddress(dll, "FtpRemoveDirectoryW");
        FtpRenameFileA = GetProcAddress(dll, "FtpRenameFileA");
        FtpRenameFileW = GetProcAddress(dll, "FtpRenameFileW");
        FtpSetCurrentDirectoryA = GetProcAddress(dll, "FtpSetCurrentDirectoryA");
        FtpSetCurrentDirectoryW = GetProcAddress(dll, "FtpSetCurrentDirectoryW");
        _GetFileExtensionFromUrl = GetProcAddress(dll, "_GetFileExtensionFromUrl");
        GetProxyDllInfo = GetProcAddress(dll, "GetProxyDllInfo");
        GetUrlCacheConfigInfoA = GetProcAddress(dll, "GetUrlCacheConfigInfoA");
        GetUrlCacheConfigInfoW = GetProcAddress(dll, "GetUrlCacheConfigInfoW");
        GetUrlCacheEntryBinaryBlob = GetProcAddress(dll, "GetUrlCacheEntryBinaryBlob");
        GetUrlCacheEntryInfoA = GetProcAddress(dll, "GetUrlCacheEntryInfoA");
        GetUrlCacheEntryInfoExA = GetProcAddress(dll, "GetUrlCacheEntryInfoExA");
        GetUrlCacheEntryInfoExW = GetProcAddress(dll, "GetUrlCacheEntryInfoExW");
        GetUrlCacheEntryInfoW = GetProcAddress(dll, "GetUrlCacheEntryInfoW");
        GetUrlCacheGroupAttributeA = GetProcAddress(dll, "GetUrlCacheGroupAttributeA");
        GetUrlCacheGroupAttributeW = GetProcAddress(dll, "GetUrlCacheGroupAttributeW");
        GetUrlCacheHeaderData = GetProcAddress(dll, "GetUrlCacheHeaderData");
        GopherCreateLocatorA = GetProcAddress(dll, "GopherCreateLocatorA");
        GopherCreateLocatorW = GetProcAddress(dll, "GopherCreateLocatorW");
        GopherFindFirstFileA = GetProcAddress(dll, "GopherFindFirstFileA");
        GopherFindFirstFileW = GetProcAddress(dll, "GopherFindFirstFileW");
        GopherGetAttributeA = GetProcAddress(dll, "GopherGetAttributeA");
        GopherGetAttributeW = GetProcAddress(dll, "GopherGetAttributeW");
        GopherGetLocatorTypeA = GetProcAddress(dll, "GopherGetLocatorTypeA");
        GopherGetLocatorTypeW = GetProcAddress(dll, "GopherGetLocatorTypeW");
        GopherOpenFileA = GetProcAddress(dll, "GopherOpenFileA");
        GopherOpenFileW = GetProcAddress(dll, "GopherOpenFileW");
        HttpAddRequestHeadersA = GetProcAddress(dll, "HttpAddRequestHeadersA");
        HttpAddRequestHeadersW = GetProcAddress(dll, "HttpAddRequestHeadersW");
        HttpCheckDavCompliance = GetProcAddress(dll, "HttpCheckDavCompliance");
        HttpCloseDependencyHandle = GetProcAddress(dll, "HttpCloseDependencyHandle");
        HttpDuplicateDependencyHandle = GetProcAddress(dll, "HttpDuplicateDependencyHandle");
        HttpEndRequestA = GetProcAddress(dll, "HttpEndRequestA");
        HttpEndRequestW = GetProcAddress(dll, "HttpEndRequestW");
        HttpGetServerCredentials = GetProcAddress(dll, "HttpGetServerCredentials");
        HttpGetTunnelSocket = GetProcAddress(dll, "HttpGetTunnelSocket");
        HttpIsHostHstsEnabled = GetProcAddress(dll, "HttpIsHostHstsEnabled");
        HttpOpenDependencyHandle = GetProcAddress(dll, "HttpOpenDependencyHandle");
        HttpOpenRequestA = GetProcAddress(dll, "HttpOpenRequestA");
        HttpOpenRequestW = GetProcAddress(dll, "HttpOpenRequestW");
        HttpPushClose = GetProcAddress(dll, "HttpPushClose");
        HttpPushEnable = GetProcAddress(dll, "HttpPushEnable");
        HttpPushWait = GetProcAddress(dll, "HttpPushWait");
        HttpQueryInfoA = GetProcAddress(dll, "HttpQueryInfoA");
        HttpQueryInfoW = GetProcAddress(dll, "HttpQueryInfoW");
        HttpSendRequestA = GetProcAddress(dll, "HttpSendRequestA");
        HttpSendRequestExA = GetProcAddress(dll, "HttpSendRequestExA");
        HttpSendRequestExW = GetProcAddress(dll, "HttpSendRequestExW");
        HttpSendRequestW = GetProcAddress(dll, "HttpSendRequestW");
        HttpWebSocketClose = GetProcAddress(dll, "HttpWebSocketClose");
        HttpWebSocketCompleteUpgrade = GetProcAddress(dll, "HttpWebSocketCompleteUpgrade");
        HttpWebSocketQueryCloseStatus = GetProcAddress(dll, "HttpWebSocketQueryCloseStatus");
        HttpWebSocketReceive = GetProcAddress(dll, "HttpWebSocketReceive");
        HttpWebSocketSend = GetProcAddress(dll, "HttpWebSocketSend");
        HttpWebSocketShutdown = GetProcAddress(dll, "HttpWebSocketShutdown");
        IncrementUrlCacheHeaderData = GetProcAddress(dll, "IncrementUrlCacheHeaderData");
        InternetAlgIdToStringA = GetProcAddress(dll, "InternetAlgIdToStringA");
        InternetAlgIdToStringW = GetProcAddress(dll, "InternetAlgIdToStringW");
        InternetAttemptConnect = GetProcAddress(dll, "InternetAttemptConnect");
        InternetAutodial = GetProcAddress(dll, "InternetAutodial");
        InternetAutodialCallback = GetProcAddress(dll, "InternetAutodialCallback");
        InternetAutodialHangup = GetProcAddress(dll, "InternetAutodialHangup");
        InternetCanonicalizeUrlA = GetProcAddress(dll, "InternetCanonicalizeUrlA");
        InternetCanonicalizeUrlW = GetProcAddress(dll, "InternetCanonicalizeUrlW");
        InternetCheckConnectionA = GetProcAddress(dll, "InternetCheckConnectionA");
        InternetCheckConnectionW = GetProcAddress(dll, "InternetCheckConnectionW");
        InternetClearAllPerSiteCookieDecisions = GetProcAddress(dll, "InternetClearAllPerSiteCookieDecisions");
        InternetCloseHandle = GetProcAddress(dll, "InternetCloseHandle");
        InternetCombineUrlA = GetProcAddress(dll, "InternetCombineUrlA");
        InternetCombineUrlW = GetProcAddress(dll, "InternetCombineUrlW");
        InternetConfirmZoneCrossing = GetProcAddress(dll, "InternetConfirmZoneCrossing");
        InternetConfirmZoneCrossingA = GetProcAddress(dll, "InternetConfirmZoneCrossingA");
        InternetConfirmZoneCrossingW = GetProcAddress(dll, "InternetConfirmZoneCrossingW");
        InternetConnectA = GetProcAddress(dll, "InternetConnectA");
        InternetConnectW = GetProcAddress(dll, "InternetConnectW");
        InternetConvertUrlFromWireToWideChar = GetProcAddress(dll, "InternetConvertUrlFromWireToWideChar");
        InternetCrackUrlA = GetProcAddress(dll, "InternetCrackUrlA");
        InternetCrackUrlW = GetProcAddress(dll, "InternetCrackUrlW");
        InternetCreateUrlA = GetProcAddress(dll, "InternetCreateUrlA");
        InternetCreateUrlW = GetProcAddress(dll, "InternetCreateUrlW");
        InternetDial = GetProcAddress(dll, "InternetDial");
        InternetDialA = GetProcAddress(dll, "InternetDialA");
        InternetDialW = GetProcAddress(dll, "InternetDialW");
        InternetEnumPerSiteCookieDecisionA = GetProcAddress(dll, "InternetEnumPerSiteCookieDecisionA");
        InternetEnumPerSiteCookieDecisionW = GetProcAddress(dll, "InternetEnumPerSiteCookieDecisionW");
        InternetErrorDlg = GetProcAddress(dll, "InternetErrorDlg");
        InternetFindNextFileA = GetProcAddress(dll, "InternetFindNextFileA");
        InternetFindNextFileW = GetProcAddress(dll, "InternetFindNextFileW");
        InternetFortezzaCommand = GetProcAddress(dll, "InternetFortezzaCommand");
        InternetFreeCookies = GetProcAddress(dll, "InternetFreeCookies");
        InternetFreeProxyInfoList = GetProcAddress(dll, "InternetFreeProxyInfoList");
        InternetGetCertByURL = GetProcAddress(dll, "InternetGetCertByURL");
        InternetGetCertByURLA = GetProcAddress(dll, "InternetGetCertByURLA");
        InternetGetConnectedState = GetProcAddress(dll, "InternetGetConnectedState");
        InternetGetConnectedStateEx = GetProcAddress(dll, "InternetGetConnectedStateEx");
        InternetGetConnectedStateExA = GetProcAddress(dll, "InternetGetConnectedStateExA");
        InternetGetConnectedStateExW = GetProcAddress(dll, "InternetGetConnectedStateExW");
        InternetGetCookieA = GetProcAddress(dll, "InternetGetCookieA");
        InternetGetCookieEx2 = GetProcAddress(dll, "InternetGetCookieEx2");
        InternetGetCookieExA = GetProcAddress(dll, "InternetGetCookieExA");
        InternetGetCookieExW = GetProcAddress(dll, "InternetGetCookieExW");
        InternetGetCookieW = GetProcAddress(dll, "InternetGetCookieW");
        InternetGetLastResponseInfoA = GetProcAddress(dll, "InternetGetLastResponseInfoA");
        InternetGetLastResponseInfoW = GetProcAddress(dll, "InternetGetLastResponseInfoW");
        InternetGetPerSiteCookieDecisionA = GetProcAddress(dll, "InternetGetPerSiteCookieDecisionA");
        InternetGetPerSiteCookieDecisionW = GetProcAddress(dll, "InternetGetPerSiteCookieDecisionW");
        InternetGetProxyForUrl = GetProcAddress(dll, "InternetGetProxyForUrl");
        InternetGetSecurityInfoByURL = GetProcAddress(dll, "InternetGetSecurityInfoByURL");
        InternetGetSecurityInfoByURLA = GetProcAddress(dll, "InternetGetSecurityInfoByURLA");
        InternetGetSecurityInfoByURLW = GetProcAddress(dll, "InternetGetSecurityInfoByURLW");
        InternetGoOnline = GetProcAddress(dll, "InternetGoOnline");
        InternetGoOnlineA = GetProcAddress(dll, "InternetGoOnlineA");
        InternetGoOnlineW = GetProcAddress(dll, "InternetGoOnlineW");
        InternetHangUp = GetProcAddress(dll, "InternetHangUp");
        InternetInitializeAutoProxyDll = GetProcAddress(dll, "InternetInitializeAutoProxyDll");
        InternetLockRequestFile = GetProcAddress(dll, "InternetLockRequestFile");
        InternetOpenA = GetProcAddress(dll, "InternetOpenA");
        InternetOpenUrlA = GetProcAddress(dll, "InternetOpenUrlA");
        InternetOpenUrlW = GetProcAddress(dll, "InternetOpenUrlW");
        InternetOpenW = GetProcAddress(dll, "InternetOpenW");
        InternetQueryDataAvailable = GetProcAddress(dll, "InternetQueryDataAvailable");
        InternetQueryFortezzaStatus = GetProcAddress(dll, "InternetQueryFortezzaStatus");
        InternetQueryOptionA = GetProcAddress(dll, "InternetQueryOptionA");
        InternetQueryOptionW = GetProcAddress(dll, "InternetQueryOptionW");
        InternetReadFile = GetProcAddress(dll, "InternetReadFile");
        InternetReadFileExA = GetProcAddress(dll, "InternetReadFileExA");
        InternetReadFileExW = GetProcAddress(dll, "InternetReadFileExW");
        InternetSecurityProtocolToStringA = GetProcAddress(dll, "InternetSecurityProtocolToStringA");
        InternetSecurityProtocolToStringW = GetProcAddress(dll, "InternetSecurityProtocolToStringW");
        InternetSetCookieA = GetProcAddress(dll, "InternetSetCookieA");
        InternetSetCookieEx2 = GetProcAddress(dll, "InternetSetCookieEx2");
        InternetSetCookieExA = GetProcAddress(dll, "InternetSetCookieExA");
        InternetSetCookieExW = GetProcAddress(dll, "InternetSetCookieExW");
        InternetSetCookieW = GetProcAddress(dll, "InternetSetCookieW");
        InternetSetDialState = GetProcAddress(dll, "InternetSetDialState");
        InternetSetDialStateA = GetProcAddress(dll, "InternetSetDialStateA");
        InternetSetDialStateW = GetProcAddress(dll, "InternetSetDialStateW");
        InternetSetFilePointer = GetProcAddress(dll, "InternetSetFilePointer");
        InternetSetOptionA = GetProcAddress(dll, "InternetSetOptionA");
        InternetSetOptionExA = GetProcAddress(dll, "InternetSetOptionExA");
        InternetSetOptionExW = GetProcAddress(dll, "InternetSetOptionExW");
        InternetSetOptionW = GetProcAddress(dll, "InternetSetOptionW");
        InternetSetPerSiteCookieDecisionA = GetProcAddress(dll, "InternetSetPerSiteCookieDecisionA");
        InternetSetPerSiteCookieDecisionW = GetProcAddress(dll, "InternetSetPerSiteCookieDecisionW");
        InternetSetStatusCallback = GetProcAddress(dll, "InternetSetStatusCallback");
        InternetSetStatusCallbackA = GetProcAddress(dll, "InternetSetStatusCallbackA");
        InternetSetStatusCallbackW = GetProcAddress(dll, "InternetSetStatusCallbackW");
        InternetShowSecurityInfoByURL = GetProcAddress(dll, "InternetShowSecurityInfoByURL");
        InternetShowSecurityInfoByURLA = GetProcAddress(dll, "InternetShowSecurityInfoByURLA");
        InternetShowSecurityInfoByURLW = GetProcAddress(dll, "InternetShowSecurityInfoByURLW");
        InternetTimeFromSystemTime = GetProcAddress(dll, "InternetTimeFromSystemTime");
        InternetTimeFromSystemTimeA = GetProcAddress(dll, "InternetTimeFromSystemTimeA");
        InternetTimeFromSystemTimeW = GetProcAddress(dll, "InternetTimeFromSystemTimeW");
        InternetTimeToSystemTime = GetProcAddress(dll, "InternetTimeToSystemTime");
        InternetTimeToSystemTimeA = GetProcAddress(dll, "InternetTimeToSystemTimeA");
        InternetTimeToSystemTimeW = GetProcAddress(dll, "InternetTimeToSystemTimeW");
        InternetUnlockRequestFile = GetProcAddress(dll, "InternetUnlockRequestFile");
        InternetWriteFile = GetProcAddress(dll, "InternetWriteFile");
        InternetWriteFileExA = GetProcAddress(dll, "InternetWriteFileExA");
        InternetWriteFileExW = GetProcAddress(dll, "InternetWriteFileExW");
        IsHostInProxyBypassList = GetProcAddress(dll, "IsHostInProxyBypassList");
        IsUrlCacheEntryExpiredA = GetProcAddress(dll, "IsUrlCacheEntryExpiredA");
        IsUrlCacheEntryExpiredW = GetProcAddress(dll, "IsUrlCacheEntryExpiredW");
        LoadUrlCacheContent = GetProcAddress(dll, "LoadUrlCacheContent");
        ParseX509EncodedCertificateForListBoxEntry = GetProcAddress(dll, "ParseX509EncodedCertificateForListBoxEntry");
        PrivacyGetZonePreferenceW = GetProcAddress(dll, "PrivacyGetZonePreferenceW");
        PrivacySetZonePreferenceW = GetProcAddress(dll, "PrivacySetZonePreferenceW");
        ReadUrlCacheEntryStream = GetProcAddress(dll, "ReadUrlCacheEntryStream");
        ReadUrlCacheEntryStreamEx = GetProcAddress(dll, "ReadUrlCacheEntryStreamEx");
        RegisterUrlCacheNotification = GetProcAddress(dll, "RegisterUrlCacheNotification");
        ResumeSuspendedDownload = GetProcAddress(dll, "ResumeSuspendedDownload");
        RetrieveUrlCacheEntryFileA = GetProcAddress(dll, "RetrieveUrlCacheEntryFileA");
        RetrieveUrlCacheEntryFileW = GetProcAddress(dll, "RetrieveUrlCacheEntryFileW");
        RetrieveUrlCacheEntryStreamA = GetProcAddress(dll, "RetrieveUrlCacheEntryStreamA");
        RetrieveUrlCacheEntryStreamW = GetProcAddress(dll, "RetrieveUrlCacheEntryStreamW");
        RunOnceUrlCache = GetProcAddress(dll, "RunOnceUrlCache");
        SetUrlCacheConfigInfoA = GetProcAddress(dll, "SetUrlCacheConfigInfoA");
        SetUrlCacheConfigInfoW = GetProcAddress(dll, "SetUrlCacheConfigInfoW");
        SetUrlCacheEntryGroup = GetProcAddress(dll, "SetUrlCacheEntryGroup");
        SetUrlCacheEntryGroupA = GetProcAddress(dll, "SetUrlCacheEntryGroupA");
        SetUrlCacheEntryGroupW = GetProcAddress(dll, "SetUrlCacheEntryGroupW");
        SetUrlCacheEntryInfoA = GetProcAddress(dll, "SetUrlCacheEntryInfoA");
        SetUrlCacheEntryInfoW = GetProcAddress(dll, "SetUrlCacheEntryInfoW");
        SetUrlCacheGroupAttributeA = GetProcAddress(dll, "SetUrlCacheGroupAttributeA");
        SetUrlCacheGroupAttributeW = GetProcAddress(dll, "SetUrlCacheGroupAttributeW");
        SetUrlCacheHeaderData = GetProcAddress(dll, "SetUrlCacheHeaderData");
        ShowCertificate = GetProcAddress(dll, "ShowCertificate");
        ShowClientAuthCerts = GetProcAddress(dll, "ShowClientAuthCerts");
        ShowSecurityInfo = GetProcAddress(dll, "ShowSecurityInfo");
        ShowX509EncodedCertificate = GetProcAddress(dll, "ShowX509EncodedCertificate");
        UnlockUrlCacheEntryFile = GetProcAddress(dll, "UnlockUrlCacheEntryFile");
        UnlockUrlCacheEntryFileA = GetProcAddress(dll, "UnlockUrlCacheEntryFileA");
        UnlockUrlCacheEntryFileW = GetProcAddress(dll, "UnlockUrlCacheEntryFileW");
        UnlockUrlCacheEntryStream = GetProcAddress(dll, "UnlockUrlCacheEntryStream");
        UpdateUrlCacheContentPath = GetProcAddress(dll, "UpdateUrlCacheContentPath");
        UrlCacheCheckEntriesExist = GetProcAddress(dll, "UrlCacheCheckEntriesExist");
        UrlCacheCloseEntryHandle = GetProcAddress(dll, "UrlCacheCloseEntryHandle");
        UrlCacheContainerSetEntryMaximumAge = GetProcAddress(dll, "UrlCacheContainerSetEntryMaximumAge");
        UrlCacheCreateContainer = GetProcAddress(dll, "UrlCacheCreateContainer");
        UrlCacheFindFirstEntry = GetProcAddress(dll, "UrlCacheFindFirstEntry");
        UrlCacheFindNextEntry = GetProcAddress(dll, "UrlCacheFindNextEntry");
        UrlCacheFreeEntryInfo = GetProcAddress(dll, "UrlCacheFreeEntryInfo");
        UrlCacheFreeGlobalSpace = GetProcAddress(dll, "UrlCacheFreeGlobalSpace");
        UrlCacheGetContentPaths = GetProcAddress(dll, "UrlCacheGetContentPaths");
        UrlCacheGetEntryInfo = GetProcAddress(dll, "UrlCacheGetEntryInfo");
        UrlCacheGetGlobalCacheSize = GetProcAddress(dll, "UrlCacheGetGlobalCacheSize");
        UrlCacheGetGlobalLimit = GetProcAddress(dll, "UrlCacheGetGlobalLimit");
        UrlCacheReadEntryStream = GetProcAddress(dll, "UrlCacheReadEntryStream");
        UrlCacheReloadSettings = GetProcAddress(dll, "UrlCacheReloadSettings");
        UrlCacheRetrieveEntryFile = GetProcAddress(dll, "UrlCacheRetrieveEntryFile");
        UrlCacheRetrieveEntryStream = GetProcAddress(dll, "UrlCacheRetrieveEntryStream");
        UrlCacheServer = GetProcAddress(dll, "UrlCacheServer");
        UrlCacheSetGlobalLimit = GetProcAddress(dll, "UrlCacheSetGlobalLimit");
        UrlCacheUpdateEntryExtraData = GetProcAddress(dll, "UrlCacheUpdateEntryExtraData");
        UrlZonesDetach = GetProcAddress(dll, "UrlZonesDetach");
        skipGetModuleHandle = false;
    }
} wininet;

struct version_dll
{
    HMODULE dll;
    FARPROC GetFileVersionInfoA;
    FARPROC GetFileVersionInfoByHandle;
    FARPROC GetFileVersionInfoExA;
    FARPROC GetFileVersionInfoExW;
    FARPROC GetFileVersionInfoSizeA;
    FARPROC GetFileVersionInfoSizeExA;
    FARPROC GetFileVersionInfoSizeExW;
    FARPROC GetFileVersionInfoSizeW;
    FARPROC GetFileVersionInfoW;
    FARPROC VerFindFileA;
    FARPROC VerFindFileW;
    FARPROC VerInstallFileA;
    FARPROC VerInstallFileW;
    FARPROC VerLanguageNameA;
    FARPROC VerLanguageNameW;
    FARPROC VerQueryValueA;
    FARPROC VerQueryValueW;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        skipGetModuleHandle = true;
        GetFileVersionInfoA = GetProcAddress(dll, "GetFileVersionInfoA");
        GetFileVersionInfoByHandle = GetProcAddress(dll, "GetFileVersionInfoByHandle");
        GetFileVersionInfoExA = GetProcAddress(dll, "GetFileVersionInfoExA");
        GetFileVersionInfoExW = GetProcAddress(dll, "GetFileVersionInfoExW");
        GetFileVersionInfoSizeA = GetProcAddress(dll, "GetFileVersionInfoSizeA");
        GetFileVersionInfoSizeExA = GetProcAddress(dll, "GetFileVersionInfoSizeExA");
        GetFileVersionInfoSizeExW = GetProcAddress(dll, "GetFileVersionInfoSizeExW");
        GetFileVersionInfoSizeW = GetProcAddress(dll, "GetFileVersionInfoSizeW");
        GetFileVersionInfoW = GetProcAddress(dll, "GetFileVersionInfoW");
        VerFindFileA = GetProcAddress(dll, "VerFindFileA");
        VerFindFileW = GetProcAddress(dll, "VerFindFileW");
        VerInstallFileA = GetProcAddress(dll, "VerInstallFileA");
        VerInstallFileW = GetProcAddress(dll, "VerInstallFileW");
        VerLanguageNameA = GetProcAddress(dll, "VerLanguageNameA");
        VerLanguageNameW = GetProcAddress(dll, "VerLanguageNameW");
        VerQueryValueA = GetProcAddress(dll, "VerQueryValueA");
        VerQueryValueW = GetProcAddress(dll, "VerQueryValueW");
        skipGetModuleHandle = false;
    }
} version;

struct winmm_dll
{
    HMODULE dll;
    FARPROC CloseDriver;
    FARPROC DefDriverProc;
    FARPROC DriverCallback;
    FARPROC DrvGetModuleHandle;
    FARPROC GetDriverModuleHandle;
    FARPROC NotifyCallbackData;
    FARPROC OpenDriver;
    FARPROC PlaySound;
    FARPROC PlaySoundA;
    FARPROC PlaySoundW;
    FARPROC SendDriverMessage;
    FARPROC WOW32DriverCallback;
    FARPROC WOW32ResolveMultiMediaHandle;
    FARPROC WOWAppExit;
    FARPROC aux32Message;
    FARPROC auxGetDevCapsA;
    FARPROC auxGetDevCapsW;
    FARPROC auxGetNumDevs;
    FARPROC auxGetVolume;
    FARPROC auxOutMessage;
    FARPROC auxSetVolume;
    FARPROC joy32Message;
    FARPROC joyConfigChanged;
    FARPROC joyGetDevCapsA;
    FARPROC joyGetDevCapsW;
    FARPROC joyGetNumDevs;
    FARPROC joyGetPos;
    FARPROC joyGetPosEx;
    FARPROC joyGetThreshold;
    FARPROC joyReleaseCapture;
    FARPROC joySetCapture;
    FARPROC joySetThreshold;
    FARPROC mci32Message;
    FARPROC mciDriverNotify;
    FARPROC mciDriverYield;
    FARPROC mciExecute;
    FARPROC mciFreeCommandResource;
    FARPROC mciGetCreatorTask;
    FARPROC mciGetDeviceIDA;
    FARPROC mciGetDeviceIDFromElementIDA;
    FARPROC mciGetDeviceIDFromElementIDW;
    FARPROC mciGetDeviceIDW;
    FARPROC mciGetDriverData;
    FARPROC mciGetErrorStringA;
    FARPROC mciGetErrorStringW;
    FARPROC mciGetYieldProc;
    FARPROC mciLoadCommandResource;
    FARPROC mciSendCommandA;
    FARPROC mciSendCommandW;
    FARPROC mciSendStringA;
    FARPROC mciSendStringW;
    FARPROC mciSetDriverData;
    FARPROC mciSetYieldProc;
    FARPROC mid32Message;
    FARPROC midiConnect;
    FARPROC midiDisconnect;
    FARPROC midiInAddBuffer;
    FARPROC midiInClose;
    FARPROC midiInGetDevCapsA;
    FARPROC midiInGetDevCapsW;
    FARPROC midiInGetErrorTextA;
    FARPROC midiInGetErrorTextW;
    FARPROC midiInGetID;
    FARPROC midiInGetNumDevs;
    FARPROC midiInMessage;
    FARPROC midiInOpen;
    FARPROC midiInPrepareHeader;
    FARPROC midiInReset;
    FARPROC midiInStart;
    FARPROC midiInStop;
    FARPROC midiInUnprepareHeader;
    FARPROC midiOutCacheDrumPatches;
    FARPROC midiOutCachePatches;
    FARPROC midiOutClose;
    FARPROC midiOutGetDevCapsA;
    FARPROC midiOutGetDevCapsW;
    FARPROC midiOutGetErrorTextA;
    FARPROC midiOutGetErrorTextW;
    FARPROC midiOutGetID;
    FARPROC midiOutGetNumDevs;
    FARPROC midiOutGetVolume;
    FARPROC midiOutLongMsg;
    FARPROC midiOutMessage;
    FARPROC midiOutOpen;
    FARPROC midiOutPrepareHeader;
    FARPROC midiOutReset;
    FARPROC midiOutSetVolume;
    FARPROC midiOutShortMsg;
    FARPROC midiOutUnprepareHeader;
    FARPROC midiStreamClose;
    FARPROC midiStreamOpen;
    FARPROC midiStreamOut;
    FARPROC midiStreamPause;
    FARPROC midiStreamPosition;
    FARPROC midiStreamProperty;
    FARPROC midiStreamRestart;
    FARPROC midiStreamStop;
    FARPROC mixerClose;
    FARPROC mixerGetControlDetailsA;
    FARPROC mixerGetControlDetailsW;
    FARPROC mixerGetDevCapsA;
    FARPROC mixerGetDevCapsW;
    FARPROC mixerGetID;
    FARPROC mixerGetLineControlsA;
    FARPROC mixerGetLineControlsW;
    FARPROC mixerGetLineInfoA;
    FARPROC mixerGetLineInfoW;
    FARPROC mixerGetNumDevs;
    FARPROC mixerMessage;
    FARPROC mixerOpen;
    FARPROC mixerSetControlDetails;
    FARPROC mmDrvInstall;
    FARPROC mmGetCurrentTask;
    FARPROC mmTaskBlock;
    FARPROC mmTaskCreate;
    FARPROC mmTaskSignal;
    FARPROC mmTaskYield;
    FARPROC mmioAdvance;
    FARPROC mmioAscend;
    FARPROC mmioClose;
    FARPROC mmioCreateChunk;
    FARPROC mmioDescend;
    FARPROC mmioFlush;
    FARPROC mmioGetInfo;
    FARPROC mmioInstallIOProcA;
    FARPROC mmioInstallIOProcW;
    FARPROC mmioOpenA;
    FARPROC mmioOpenW;
    FARPROC mmioRead;
    FARPROC mmioRenameA;
    FARPROC mmioRenameW;
    FARPROC mmioSeek;
    FARPROC mmioSendMessage;
    FARPROC mmioSetBuffer;
    FARPROC mmioSetInfo;
    FARPROC mmioStringToFOURCCA;
    FARPROC mmioStringToFOURCCW;
    FARPROC mmioWrite;
    FARPROC mmsystemGetVersion;
    FARPROC mod32Message;
    FARPROC mxd32Message;
    FARPROC sndPlaySoundA;
    FARPROC sndPlaySoundW;
    FARPROC tid32Message;
    FARPROC timeBeginPeriod;
    FARPROC timeEndPeriod;
    FARPROC timeGetDevCaps;
    FARPROC timeGetSystemTime;
    FARPROC timeGetTime;
    FARPROC timeKillEvent;
    FARPROC timeSetEvent;
    FARPROC waveInAddBuffer;
    FARPROC waveInClose;
    FARPROC waveInGetDevCapsA;
    FARPROC waveInGetDevCapsW;
    FARPROC waveInGetErrorTextA;
    FARPROC waveInGetErrorTextW;
    FARPROC waveInGetID;
    FARPROC waveInGetNumDevs;
    FARPROC waveInGetPosition;
    FARPROC waveInMessage;
    FARPROC waveInOpen;
    FARPROC waveInPrepareHeader;
    FARPROC waveInReset;
    FARPROC waveInStart;
    FARPROC waveInStop;
    FARPROC waveInUnprepareHeader;
    FARPROC waveOutBreakLoop;
    FARPROC waveOutClose;
    FARPROC waveOutGetDevCapsA;
    FARPROC waveOutGetDevCapsW;
    FARPROC waveOutGetErrorTextA;
    FARPROC waveOutGetErrorTextW;
    FARPROC waveOutGetID;
    FARPROC waveOutGetNumDevs;
    FARPROC waveOutGetPitch;
    FARPROC waveOutGetPlaybackRate;
    FARPROC waveOutGetPosition;
    FARPROC waveOutGetVolume;
    FARPROC waveOutMessage;
    FARPROC waveOutOpen;
    FARPROC waveOutPause;
    FARPROC waveOutPrepareHeader;
    FARPROC waveOutReset;
    FARPROC waveOutRestart;
    FARPROC waveOutSetPitch;
    FARPROC waveOutSetPlaybackRate;
    FARPROC waveOutSetVolume;
    FARPROC waveOutUnprepareHeader;
    FARPROC waveOutWrite;
    FARPROC wid32Message;
    FARPROC wod32Message;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        skipGetModuleHandle = true;
        CloseDriver = GetProcAddress(dll, "CloseDriver");
        DefDriverProc = GetProcAddress(dll, "DefDriverProc");
        DriverCallback = GetProcAddress(dll, "DriverCallback");
        DrvGetModuleHandle = GetProcAddress(dll, "DrvGetModuleHandle");
        GetDriverModuleHandle = GetProcAddress(dll, "GetDriverModuleHandle");
        NotifyCallbackData = GetProcAddress(dll, "NotifyCallbackData");
        OpenDriver = GetProcAddress(dll, "OpenDriver");
        PlaySound = GetProcAddress(dll, "PlaySound");
        PlaySoundA = GetProcAddress(dll, "PlaySoundA");
        PlaySoundW = GetProcAddress(dll, "PlaySoundW");
        SendDriverMessage = GetProcAddress(dll, "SendDriverMessage");
        WOW32DriverCallback = GetProcAddress(dll, "WOW32DriverCallback");
        WOW32ResolveMultiMediaHandle = GetProcAddress(dll, "WOW32ResolveMultiMediaHandle");
        WOWAppExit = GetProcAddress(dll, "WOWAppExit");
        aux32Message = GetProcAddress(dll, "aux32Message");
        auxGetDevCapsA = GetProcAddress(dll, "auxGetDevCapsA");
        auxGetDevCapsW = GetProcAddress(dll, "auxGetDevCapsW");
        auxGetNumDevs = GetProcAddress(dll, "auxGetNumDevs");
        auxGetVolume = GetProcAddress(dll, "auxGetVolume");
        auxOutMessage = GetProcAddress(dll, "auxOutMessage");
        auxSetVolume = GetProcAddress(dll, "auxSetVolume");
        joy32Message = GetProcAddress(dll, "joy32Message");
        joyConfigChanged = GetProcAddress(dll, "joyConfigChanged");
        joyGetDevCapsA = GetProcAddress(dll, "joyGetDevCapsA");
        joyGetDevCapsW = GetProcAddress(dll, "joyGetDevCapsW");
        joyGetNumDevs = GetProcAddress(dll, "joyGetNumDevs");
        joyGetPos = GetProcAddress(dll, "joyGetPos");
        joyGetPosEx = GetProcAddress(dll, "joyGetPosEx");
        joyGetThreshold = GetProcAddress(dll, "joyGetThreshold");
        joyReleaseCapture = GetProcAddress(dll, "joyReleaseCapture");
        joySetCapture = GetProcAddress(dll, "joySetCapture");
        joySetThreshold = GetProcAddress(dll, "joySetThreshold");
        mci32Message = GetProcAddress(dll, "mci32Message");
        mciDriverNotify = GetProcAddress(dll, "mciDriverNotify");
        mciDriverYield = GetProcAddress(dll, "mciDriverYield");
        mciExecute = GetProcAddress(dll, "mciExecute");
        mciFreeCommandResource = GetProcAddress(dll, "mciFreeCommandResource");
        mciGetCreatorTask = GetProcAddress(dll, "mciGetCreatorTask");
        mciGetDeviceIDA = GetProcAddress(dll, "mciGetDeviceIDA");
        mciGetDeviceIDFromElementIDA = GetProcAddress(dll, "mciGetDeviceIDFromElementIDA");
        mciGetDeviceIDFromElementIDW = GetProcAddress(dll, "mciGetDeviceIDFromElementIDW");
        mciGetDeviceIDW = GetProcAddress(dll, "mciGetDeviceIDW");
        mciGetDriverData = GetProcAddress(dll, "mciGetDriverData");
        mciGetErrorStringA = GetProcAddress(dll, "mciGetErrorStringA");
        mciGetErrorStringW = GetProcAddress(dll, "mciGetErrorStringW");
        mciGetYieldProc = GetProcAddress(dll, "mciGetYieldProc");
        mciLoadCommandResource = GetProcAddress(dll, "mciLoadCommandResource");
        mciSendCommandA = GetProcAddress(dll, "mciSendCommandA");
        mciSendCommandW = GetProcAddress(dll, "mciSendCommandW");
        mciSendStringA = GetProcAddress(dll, "mciSendStringA");
        mciSendStringW = GetProcAddress(dll, "mciSendStringW");
        mciSetDriverData = GetProcAddress(dll, "mciSetDriverData");
        mciSetYieldProc = GetProcAddress(dll, "mciSetYieldProc");
        mid32Message = GetProcAddress(dll, "mid32Message");
        midiConnect = GetProcAddress(dll, "midiConnect");
        midiDisconnect = GetProcAddress(dll, "midiDisconnect");
        midiInAddBuffer = GetProcAddress(dll, "midiInAddBuffer");
        midiInClose = GetProcAddress(dll, "midiInClose");
        midiInGetDevCapsA = GetProcAddress(dll, "midiInGetDevCapsA");
        midiInGetDevCapsW = GetProcAddress(dll, "midiInGetDevCapsW");
        midiInGetErrorTextA = GetProcAddress(dll, "midiInGetErrorTextA");
        midiInGetErrorTextW = GetProcAddress(dll, "midiInGetErrorTextW");
        midiInGetID = GetProcAddress(dll, "midiInGetID");
        midiInGetNumDevs = GetProcAddress(dll, "midiInGetNumDevs");
        midiInMessage = GetProcAddress(dll, "midiInMessage");
        midiInOpen = GetProcAddress(dll, "midiInOpen");
        midiInPrepareHeader = GetProcAddress(dll, "midiInPrepareHeader");
        midiInReset = GetProcAddress(dll, "midiInReset");
        midiInStart = GetProcAddress(dll, "midiInStart");
        midiInStop = GetProcAddress(dll, "midiInStop");
        midiInUnprepareHeader = GetProcAddress(dll, "midiInUnprepareHeader");
        midiOutCacheDrumPatches = GetProcAddress(dll, "midiOutCacheDrumPatches");
        midiOutCachePatches = GetProcAddress(dll, "midiOutCachePatches");
        midiOutClose = GetProcAddress(dll, "midiOutClose");
        midiOutGetDevCapsA = GetProcAddress(dll, "midiOutGetDevCapsA");
        midiOutGetDevCapsW = GetProcAddress(dll, "midiOutGetDevCapsW");
        midiOutGetErrorTextA = GetProcAddress(dll, "midiOutGetErrorTextA");
        midiOutGetErrorTextW = GetProcAddress(dll, "midiOutGetErrorTextW");
        midiOutGetID = GetProcAddress(dll, "midiOutGetID");
        midiOutGetNumDevs = GetProcAddress(dll, "midiOutGetNumDevs");
        midiOutGetVolume = GetProcAddress(dll, "midiOutGetVolume");
        midiOutLongMsg = GetProcAddress(dll, "midiOutLongMsg");
        midiOutMessage = GetProcAddress(dll, "midiOutMessage");
        midiOutOpen = GetProcAddress(dll, "midiOutOpen");
        midiOutPrepareHeader = GetProcAddress(dll, "midiOutPrepareHeader");
        midiOutReset = GetProcAddress(dll, "midiOutReset");
        midiOutSetVolume = GetProcAddress(dll, "midiOutSetVolume");
        midiOutShortMsg = GetProcAddress(dll, "midiOutShortMsg");
        midiOutUnprepareHeader = GetProcAddress(dll, "midiOutUnprepareHeader");
        midiStreamClose = GetProcAddress(dll, "midiStreamClose");
        midiStreamOpen = GetProcAddress(dll, "midiStreamOpen");
        midiStreamOut = GetProcAddress(dll, "midiStreamOut");
        midiStreamPause = GetProcAddress(dll, "midiStreamPause");
        midiStreamPosition = GetProcAddress(dll, "midiStreamPosition");
        midiStreamProperty = GetProcAddress(dll, "midiStreamProperty");
        midiStreamRestart = GetProcAddress(dll, "midiStreamRestart");
        midiStreamStop = GetProcAddress(dll, "midiStreamStop");
        mixerClose = GetProcAddress(dll, "mixerClose");
        mixerGetControlDetailsA = GetProcAddress(dll, "mixerGetControlDetailsA");
        mixerGetControlDetailsW = GetProcAddress(dll, "mixerGetControlDetailsW");
        mixerGetDevCapsA = GetProcAddress(dll, "mixerGetDevCapsA");
        mixerGetDevCapsW = GetProcAddress(dll, "mixerGetDevCapsW");
        mixerGetID = GetProcAddress(dll, "mixerGetID");
        mixerGetLineControlsA = GetProcAddress(dll, "mixerGetLineControlsA");
        mixerGetLineControlsW = GetProcAddress(dll, "mixerGetLineControlsW");
        mixerGetLineInfoA = GetProcAddress(dll, "mixerGetLineInfoA");
        mixerGetLineInfoW = GetProcAddress(dll, "mixerGetLineInfoW");
        mixerGetNumDevs = GetProcAddress(dll, "mixerGetNumDevs");
        mixerMessage = GetProcAddress(dll, "mixerMessage");
        mixerOpen = GetProcAddress(dll, "mixerOpen");
        mixerSetControlDetails = GetProcAddress(dll, "mixerSetControlDetails");
        mmDrvInstall = GetProcAddress(dll, "mmDrvInstall");
        mmGetCurrentTask = GetProcAddress(dll, "mmGetCurrentTask");
        mmTaskBlock = GetProcAddress(dll, "mmTaskBlock");
        mmTaskCreate = GetProcAddress(dll, "mmTaskCreate");
        mmTaskSignal = GetProcAddress(dll, "mmTaskSignal");
        mmTaskYield = GetProcAddress(dll, "mmTaskYield");
        mmioAdvance = GetProcAddress(dll, "mmioAdvance");
        mmioAscend = GetProcAddress(dll, "mmioAscend");
        mmioClose = GetProcAddress(dll, "mmioClose");
        mmioCreateChunk = GetProcAddress(dll, "mmioCreateChunk");
        mmioDescend = GetProcAddress(dll, "mmioDescend");
        mmioFlush = GetProcAddress(dll, "mmioFlush");
        mmioGetInfo = GetProcAddress(dll, "mmioGetInfo");
        mmioInstallIOProcA = GetProcAddress(dll, "mmioInstallIOProcA");
        mmioInstallIOProcW = GetProcAddress(dll, "mmioInstallIOProcW");
        mmioOpenA = GetProcAddress(dll, "mmioOpenA");
        mmioOpenW = GetProcAddress(dll, "mmioOpenW");
        mmioRead = GetProcAddress(dll, "mmioRead");
        mmioRenameA = GetProcAddress(dll, "mmioRenameA");
        mmioRenameW = GetProcAddress(dll, "mmioRenameW");
        mmioSeek = GetProcAddress(dll, "mmioSeek");
        mmioSendMessage = GetProcAddress(dll, "mmioSendMessage");
        mmioSetBuffer = GetProcAddress(dll, "mmioSetBuffer");
        mmioSetInfo = GetProcAddress(dll, "mmioSetInfo");
        mmioStringToFOURCCA = GetProcAddress(dll, "mmioStringToFOURCCA");
        mmioStringToFOURCCW = GetProcAddress(dll, "mmioStringToFOURCCW");
        mmioWrite = GetProcAddress(dll, "mmioWrite");
        mmsystemGetVersion = GetProcAddress(dll, "mmsystemGetVersion");
        mod32Message = GetProcAddress(dll, "mod32Message");
        mxd32Message = GetProcAddress(dll, "mxd32Message");
        sndPlaySoundA = GetProcAddress(dll, "sndPlaySoundA");
        sndPlaySoundW = GetProcAddress(dll, "sndPlaySoundW");
        tid32Message = GetProcAddress(dll, "tid32Message");
        timeBeginPeriod = GetProcAddress(dll, "timeBeginPeriod");
        timeEndPeriod = GetProcAddress(dll, "timeEndPeriod");
        timeGetDevCaps = GetProcAddress(dll, "timeGetDevCaps");
        timeGetSystemTime = GetProcAddress(dll, "timeGetSystemTime");
        timeGetTime = GetProcAddress(dll, "timeGetTime");
        timeKillEvent = GetProcAddress(dll, "timeKillEvent");
        timeSetEvent = GetProcAddress(dll, "timeSetEvent");
        waveInAddBuffer = GetProcAddress(dll, "waveInAddBuffer");
        waveInClose = GetProcAddress(dll, "waveInClose");
        waveInGetDevCapsA = GetProcAddress(dll, "waveInGetDevCapsA");
        waveInGetDevCapsW = GetProcAddress(dll, "waveInGetDevCapsW");
        waveInGetErrorTextA = GetProcAddress(dll, "waveInGetErrorTextA");
        waveInGetErrorTextW = GetProcAddress(dll, "waveInGetErrorTextW");
        waveInGetID = GetProcAddress(dll, "waveInGetID");
        waveInGetNumDevs = GetProcAddress(dll, "waveInGetNumDevs");
        waveInGetPosition = GetProcAddress(dll, "waveInGetPosition");
        waveInMessage = GetProcAddress(dll, "waveInMessage");
        waveInOpen = GetProcAddress(dll, "waveInOpen");
        waveInPrepareHeader = GetProcAddress(dll, "waveInPrepareHeader");
        waveInReset = GetProcAddress(dll, "waveInReset");
        waveInStart = GetProcAddress(dll, "waveInStart");
        waveInStop = GetProcAddress(dll, "waveInStop");
        waveInUnprepareHeader = GetProcAddress(dll, "waveInUnprepareHeader");
        waveOutBreakLoop = GetProcAddress(dll, "waveOutBreakLoop");
        waveOutClose = GetProcAddress(dll, "waveOutClose");
        waveOutGetDevCapsA = GetProcAddress(dll, "waveOutGetDevCapsA");
        waveOutGetDevCapsW = GetProcAddress(dll, "waveOutGetDevCapsW");
        waveOutGetErrorTextA = GetProcAddress(dll, "waveOutGetErrorTextA");
        waveOutGetErrorTextW = GetProcAddress(dll, "waveOutGetErrorTextW");
        waveOutGetID = GetProcAddress(dll, "waveOutGetID");
        waveOutGetNumDevs = GetProcAddress(dll, "waveOutGetNumDevs");
        waveOutGetPitch = GetProcAddress(dll, "waveOutGetPitch");
        waveOutGetPlaybackRate = GetProcAddress(dll, "waveOutGetPlaybackRate");
        waveOutGetPosition = GetProcAddress(dll, "waveOutGetPosition");
        waveOutGetVolume = GetProcAddress(dll, "waveOutGetVolume");
        waveOutMessage = GetProcAddress(dll, "waveOutMessage");
        waveOutOpen = GetProcAddress(dll, "waveOutOpen");
        waveOutPause = GetProcAddress(dll, "waveOutPause");
        waveOutPrepareHeader = GetProcAddress(dll, "waveOutPrepareHeader");
        waveOutReset = GetProcAddress(dll, "waveOutReset");
        waveOutRestart = GetProcAddress(dll, "waveOutRestart");
        waveOutSetPitch = GetProcAddress(dll, "waveOutSetPitch");
        waveOutSetPlaybackRate = GetProcAddress(dll, "waveOutSetPlaybackRate");
        waveOutSetVolume = GetProcAddress(dll, "waveOutSetVolume");
        waveOutUnprepareHeader = GetProcAddress(dll, "waveOutUnprepareHeader");
        waveOutWrite = GetProcAddress(dll, "waveOutWrite");
        wid32Message = GetProcAddress(dll, "wid32Message");
        wod32Message = GetProcAddress(dll, "wod32Message");
        skipGetModuleHandle = false;
    }
} winmm;

struct winhttp_dll
{
    HMODULE dll;
    FARPROC Private1;
    FARPROC SvchostPushServiceGlobals;
    FARPROC WinHttpAddRequestHeaders;
    FARPROC WinHttpAddRequestHeadersEx;
    FARPROC WinHttpAutoProxySvcMain;
    FARPROC WinHttpCheckPlatform;
    FARPROC WinHttpCloseHandle;
    FARPROC WinHttpConnect;
    FARPROC WinHttpConnectionDeletePolicyEntries;
    FARPROC WinHttpConnectionDeleteProxyInfo;
    FARPROC WinHttpConnectionFreeNameList;
    FARPROC WinHttpConnectionFreeProxyInfo;
    FARPROC WinHttpConnectionFreeProxyList;
    FARPROC WinHttpConnectionGetNameList;
    FARPROC WinHttpConnectionGetProxyInfo;
    FARPROC WinHttpConnectionGetProxyList;
    FARPROC WinHttpConnectionOnlyConvert;
    FARPROC WinHttpConnectionOnlyReceive;
    FARPROC WinHttpConnectionOnlySend;
    FARPROC WinHttpConnectionSetPolicyEntries;
    FARPROC WinHttpConnectionSetProxyInfo;
    FARPROC WinHttpConnectionUpdateIfIndexTable;
    FARPROC WinHttpCrackUrl;
    FARPROC WinHttpCreateProxyResolver;
    FARPROC WinHttpCreateUrl;
    FARPROC WinHttpDetectAutoProxyConfigUrl;
    FARPROC WinHttpFreeProxyResult;
    FARPROC WinHttpFreeProxyResultEx;
    FARPROC WinHttpFreeProxySettings;
    FARPROC WinHttpFreeProxySettingsEx;
    FARPROC WinHttpFreeQueryConnectionGroupResult;
    FARPROC WinHttpGetDefaultProxyConfiguration;
    FARPROC WinHttpGetIEProxyConfigForCurrentUser;
    FARPROC WinHttpGetProxyForUrl;
    FARPROC WinHttpGetProxyForUrlEx;
    FARPROC WinHttpGetProxyForUrlEx2;
    FARPROC WinHttpGetProxyForUrlHvsi;
    FARPROC WinHttpGetProxyResult;
    FARPROC WinHttpGetProxyResultEx;
    FARPROC WinHttpGetProxySettingsEx;
    FARPROC WinHttpGetProxySettingsResultEx;
    FARPROC WinHttpGetProxySettingsVersion;
    FARPROC WinHttpGetTunnelSocket;
    FARPROC WinHttpOpen;
    FARPROC WinHttpOpenRequest;
    FARPROC WinHttpPacJsWorkerMain;
    FARPROC WinHttpProbeConnectivity;
    FARPROC WinHttpQueryAuthSchemes;
    FARPROC WinHttpQueryConnectionGroup;
    FARPROC WinHttpQueryDataAvailable;
    FARPROC WinHttpQueryHeaders;
    FARPROC WinHttpQueryHeadersEx;
    FARPROC WinHttpQueryOption;
    FARPROC WinHttpReadData;
    FARPROC WinHttpReadDataEx;
    FARPROC WinHttpReadProxySettings;
    FARPROC WinHttpReadProxySettingsHvsi;
    FARPROC WinHttpReceiveResponse;
    FARPROC WinHttpRegisterProxyChangeNotification;
    FARPROC WinHttpResetAutoProxy;
    FARPROC WinHttpSaveProxyCredentials;
    FARPROC WinHttpSendRequest;
    FARPROC WinHttpSetCredentials;
    FARPROC WinHttpSetDefaultProxyConfiguration;
    FARPROC WinHttpSetOption;
    FARPROC WinHttpSetProxySettingsPerUser;
    FARPROC WinHttpSetSecureLegacyServersAppCompat;
    FARPROC WinHttpSetStatusCallback;
    FARPROC WinHttpSetTimeouts;
    FARPROC WinHttpTimeFromSystemTime;
    FARPROC WinHttpTimeToSystemTime;
    FARPROC WinHttpUnregisterProxyChangeNotification;
    FARPROC WinHttpWebSocketClose;
    FARPROC WinHttpWebSocketCompleteUpgrade;
    FARPROC WinHttpWebSocketQueryCloseStatus;
    FARPROC WinHttpWebSocketReceive;
    FARPROC WinHttpWebSocketSend;
    FARPROC WinHttpWebSocketShutdown;
    FARPROC WinHttpWriteData;
    FARPROC WinHttpWriteProxySettings;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        skipGetModuleHandle = true;
        Private1 = GetProcAddress(dll, "Private1");
        SvchostPushServiceGlobals = GetProcAddress(dll, "SvchostPushServiceGlobals");
        WinHttpAddRequestHeaders = GetProcAddress(dll, "WinHttpAddRequestHeaders");
        WinHttpAddRequestHeadersEx = GetProcAddress(dll, "WinHttpAddRequestHeadersEx");
        WinHttpAutoProxySvcMain = GetProcAddress(dll, "WinHttpAutoProxySvcMain");
        WinHttpCheckPlatform = GetProcAddress(dll, "WinHttpCheckPlatform");
        WinHttpCloseHandle = GetProcAddress(dll, "WinHttpCloseHandle");
        WinHttpConnect = GetProcAddress(dll, "WinHttpConnect");
        WinHttpConnectionDeletePolicyEntries = GetProcAddress(dll, "WinHttpConnectionDeletePolicyEntries");
        WinHttpConnectionDeleteProxyInfo = GetProcAddress(dll, "WinHttpConnectionDeleteProxyInfo");
        WinHttpConnectionFreeNameList = GetProcAddress(dll, "WinHttpConnectionFreeNameList");
        WinHttpConnectionFreeProxyInfo = GetProcAddress(dll, "WinHttpConnectionFreeProxyInfo");
        WinHttpConnectionFreeProxyList = GetProcAddress(dll, "WinHttpConnectionFreeProxyList");
        WinHttpConnectionGetNameList = GetProcAddress(dll, "WinHttpConnectionGetNameList");
        WinHttpConnectionGetProxyInfo = GetProcAddress(dll, "WinHttpConnectionGetProxyInfo");
        WinHttpConnectionGetProxyList = GetProcAddress(dll, "WinHttpConnectionGetProxyList");
        WinHttpConnectionOnlyConvert = GetProcAddress(dll, "WinHttpConnectionOnlyConvert");
        WinHttpConnectionOnlyReceive = GetProcAddress(dll, "WinHttpConnectionOnlyReceive");
        WinHttpConnectionOnlySend = GetProcAddress(dll, "WinHttpConnectionOnlySend");
        WinHttpConnectionSetPolicyEntries = GetProcAddress(dll, "WinHttpConnectionSetPolicyEntries");
        WinHttpConnectionSetProxyInfo = GetProcAddress(dll, "WinHttpConnectionSetProxyInfo");
        WinHttpConnectionUpdateIfIndexTable = GetProcAddress(dll, "WinHttpConnectionUpdateIfIndexTable");
        WinHttpCrackUrl = GetProcAddress(dll, "WinHttpCrackUrl");
        WinHttpCreateProxyResolver = GetProcAddress(dll, "WinHttpCreateProxyResolver");
        WinHttpCreateUrl = GetProcAddress(dll, "WinHttpCreateUrl");
        WinHttpDetectAutoProxyConfigUrl = GetProcAddress(dll, "WinHttpDetectAutoProxyConfigUrl");
        WinHttpFreeProxyResult = GetProcAddress(dll, "WinHttpFreeProxyResult");
        WinHttpFreeProxyResultEx = GetProcAddress(dll, "WinHttpFreeProxyResultEx");
        WinHttpFreeProxySettings = GetProcAddress(dll, "WinHttpFreeProxySettings");
        WinHttpFreeProxySettingsEx = GetProcAddress(dll, "WinHttpFreeProxySettingsEx");
        WinHttpFreeQueryConnectionGroupResult = GetProcAddress(dll, "WinHttpFreeQueryConnectionGroupResult");
        WinHttpGetDefaultProxyConfiguration = GetProcAddress(dll, "WinHttpGetDefaultProxyConfiguration");
        WinHttpGetIEProxyConfigForCurrentUser = GetProcAddress(dll, "WinHttpGetIEProxyConfigForCurrentUser");
        WinHttpGetProxyForUrl = GetProcAddress(dll, "WinHttpGetProxyForUrl");
        WinHttpGetProxyForUrlEx = GetProcAddress(dll, "WinHttpGetProxyForUrlEx");
        WinHttpGetProxyForUrlEx2 = GetProcAddress(dll, "WinHttpGetProxyForUrlEx2");
        WinHttpGetProxyForUrlHvsi = GetProcAddress(dll, "WinHttpGetProxyForUrlHvsi");
        WinHttpGetProxyResult = GetProcAddress(dll, "WinHttpGetProxyResult");
        WinHttpGetProxyResultEx = GetProcAddress(dll, "WinHttpGetProxyResultEx");
        WinHttpGetProxySettingsEx = GetProcAddress(dll, "WinHttpGetProxySettingsEx");
        WinHttpGetProxySettingsResultEx = GetProcAddress(dll, "WinHttpGetProxySettingsResultEx");
        WinHttpGetProxySettingsVersion = GetProcAddress(dll, "WinHttpGetProxySettingsVersion");
        WinHttpGetTunnelSocket = GetProcAddress(dll, "WinHttpGetTunnelSocket");
        WinHttpOpen = GetProcAddress(dll, "WinHttpOpen");
        WinHttpOpenRequest = GetProcAddress(dll, "WinHttpOpenRequest");
        WinHttpPacJsWorkerMain = GetProcAddress(dll, "WinHttpPacJsWorkerMain");
        WinHttpProbeConnectivity = GetProcAddress(dll, "WinHttpProbeConnectivity");
        WinHttpQueryAuthSchemes = GetProcAddress(dll, "WinHttpQueryAuthSchemes");
        WinHttpQueryConnectionGroup = GetProcAddress(dll, "WinHttpQueryConnectionGroup");
        WinHttpQueryDataAvailable = GetProcAddress(dll, "WinHttpQueryDataAvailable");
        WinHttpQueryHeaders = GetProcAddress(dll, "WinHttpQueryHeaders");
        WinHttpQueryHeadersEx = GetProcAddress(dll, "WinHttpQueryHeadersEx");
        WinHttpQueryOption = GetProcAddress(dll, "WinHttpQueryOption");
        WinHttpReadData = GetProcAddress(dll, "WinHttpReadData");
        WinHttpReadDataEx = GetProcAddress(dll, "WinHttpReadDataEx");
        WinHttpReadProxySettings = GetProcAddress(dll, "WinHttpReadProxySettings");
        WinHttpReadProxySettingsHvsi = GetProcAddress(dll, "WinHttpReadProxySettingsHvsi");
        WinHttpReceiveResponse = GetProcAddress(dll, "WinHttpReceiveResponse");
        WinHttpRegisterProxyChangeNotification = GetProcAddress(dll, "WinHttpRegisterProxyChangeNotification");
        WinHttpResetAutoProxy = GetProcAddress(dll, "WinHttpResetAutoProxy");
        WinHttpSaveProxyCredentials = GetProcAddress(dll, "WinHttpSaveProxyCredentials");
        WinHttpSendRequest = GetProcAddress(dll, "WinHttpSendRequest");
        WinHttpSetCredentials = GetProcAddress(dll, "WinHttpSetCredentials");
        WinHttpSetDefaultProxyConfiguration = GetProcAddress(dll, "WinHttpSetDefaultProxyConfiguration");
        WinHttpSetOption = GetProcAddress(dll, "WinHttpSetOption");
        WinHttpSetProxySettingsPerUser = GetProcAddress(dll, "WinHttpSetProxySettingsPerUser");
        WinHttpSetSecureLegacyServersAppCompat = GetProcAddress(dll, "WinHttpSetSecureLegacyServersAppCompat");
        WinHttpSetStatusCallback = GetProcAddress(dll, "WinHttpSetStatusCallback");
        WinHttpSetTimeouts = GetProcAddress(dll, "WinHttpSetTimeouts");
        WinHttpTimeFromSystemTime = GetProcAddress(dll, "WinHttpTimeFromSystemTime");
        WinHttpTimeToSystemTime = GetProcAddress(dll, "WinHttpTimeToSystemTime");
        WinHttpUnregisterProxyChangeNotification = GetProcAddress(dll, "WinHttpUnregisterProxyChangeNotification");
        WinHttpWebSocketClose = GetProcAddress(dll, "WinHttpWebSocketClose");
        WinHttpWebSocketCompleteUpgrade = GetProcAddress(dll, "WinHttpWebSocketCompleteUpgrade");
        WinHttpWebSocketQueryCloseStatus = GetProcAddress(dll, "WinHttpWebSocketQueryCloseStatus");
        WinHttpWebSocketReceive = GetProcAddress(dll, "WinHttpWebSocketReceive");
        WinHttpWebSocketSend = GetProcAddress(dll, "WinHttpWebSocketSend");
        WinHttpWebSocketShutdown = GetProcAddress(dll, "WinHttpWebSocketShutdown");
        WinHttpWriteData = GetProcAddress(dll, "WinHttpWriteData");
        WinHttpWriteProxySettings = GetProcAddress(dll, "WinHttpWriteProxySettings");
        skipGetModuleHandle = false;
    }
} winhttp;

struct dxgi_dll
{
    HMODULE dll = nullptr;

    PFN_CREATE_DXGI_FACTORY CreateDxgiFactory;
    PFN_CREATE_DXGI_FACTORY_1 CreateDxgiFactory1;
    PFN_CREATE_DXGI_FACTORY_2 CreateDxgiFactory2;

    PFN_DECLARE_ADAPTER_REMOVAL_SUPPORT dxgiDeclareAdapterRemovalSupport = nullptr;
    PFN_GET_DEBUG_INTERFACE dxgiGetDebugInterface = nullptr;
    FARPROC dxgiApplyCompatResolutionQuirking = nullptr;
    FARPROC dxgiCompatString = nullptr;
    FARPROC dxgiCompatValue = nullptr;
    FARPROC dxgiD3D10CreateDevice = nullptr;
    FARPROC dxgiD3D10CreateLayeredDevice = nullptr;
    FARPROC dxgiD3D10GetLayeredDeviceSize = nullptr;
    FARPROC dxgiD3D10RegisterLayers = nullptr;
    FARPROC dxgiD3D10ETWRundown = nullptr;
    FARPROC dxgiDumpJournal = nullptr;
    FARPROC dxgiReportAdapterConfiguration = nullptr;
    FARPROC dxgiPIXBeginCapture = nullptr;
    FARPROC dxgiPIXEndCapture = nullptr;
    FARPROC dxgiPIXGetCaptureState = nullptr;
    FARPROC dxgiSetAppCompatStringPointer = nullptr;
    FARPROC dxgiUpdateHMDEmulationStatus = nullptr;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        skipGetModuleHandle = true;
        CreateDxgiFactory = (PFN_CREATE_DXGI_FACTORY)GetProcAddress(module, "CreateDXGIFactory");
        CreateDxgiFactory1 = (PFN_CREATE_DXGI_FACTORY_1)GetProcAddress(module, "CreateDXGIFactory1");
        CreateDxgiFactory2 = (PFN_CREATE_DXGI_FACTORY_2)GetProcAddress(module, "CreateDXGIFactory2");

        dxgiDeclareAdapterRemovalSupport = (PFN_DECLARE_ADAPTER_REMOVAL_SUPPORT)GetProcAddress(module, "DXGIDeclareAdapterRemovalSupport");
        dxgiGetDebugInterface = (PFN_GET_DEBUG_INTERFACE)GetProcAddress(module, "DXGIGetDebugInterface1");
        //dxgiApplyCompatResolutionQuirking = GetProcAddress(module, "ApplyCompatResolutionQuirking");
        //dxgiCompatString = GetProcAddress(module, "CompatString");
        //dxgiCompatValue = GetProcAddress(module, "CompatValue");
        //dxgiD3D10CreateDevice = GetProcAddress(module, "DXGID3D10CreateDevice");
        //dxgiD3D10CreateLayeredDevice = GetProcAddress(module, "DXGID3D10CreateLayeredDevice");
        //dxgiD3D10GetLayeredDeviceSize = GetProcAddress(module, "DXGID3D10GetLayeredDeviceSize");
        //dxgiD3D10RegisterLayers = GetProcAddress(module, "DXGID3D10RegisterLayers");
        //dxgiD3D10ETWRundown = GetProcAddress(module, "DXGID3D10ETWRundown");
        //dxgiDumpJournal = GetProcAddress(module, "DXGIDumpJournal");
        //dxgiReportAdapterConfiguration = GetProcAddress(module, "DXGIReportAdapterConfiguration");
        //dxgiPIXBeginCapture = GetProcAddress(module, "PIXBeginCapture");
        //dxgiPIXEndCapture = GetProcAddress(module, "PIXEndCapture");
        //dxgiPIXGetCaptureState = GetProcAddress(module, "PIXGetCaptureState");
        //dxgiSetAppCompatStringPointer = GetProcAddress(module, "SetAppCompatStringPointer");
        //dxgiUpdateHMDEmulationStatus = GetProcAddress(module, "UpdateHMDEmulationStatus");
        skipGetModuleHandle = false;
    }
} dxgi;

struct dbghelp_dll
{
    HMODULE dll = nullptr;

    FARPROC dbghelpvc7fpo = nullptr;
    FARPROC dbghelpsymsrv = nullptr;
    FARPROC dbghelpsym = nullptr;
    FARPROC dbghelpstackdbg = nullptr;
    FARPROC dbghelpstack_force_ebp = nullptr;
    FARPROC dbghelpsrcfiles = nullptr;
    FARPROC dbghelpoptdbgdumpaddr = nullptr;
    FARPROC dbghelpoptdbgdump = nullptr;
    FARPROC dbghelpomap = nullptr;
    FARPROC dbghelplminfo = nullptr;
    FARPROC dbghelplmi = nullptr;
    FARPROC dbghelpitoldyouso = nullptr;
    FARPROC dbghelpinlinedbg = nullptr;
    FARPROC dbghelphomedir = nullptr;
    FARPROC dbghelpfptr = nullptr;
    FARPROC dbghelpdh = nullptr;
    FARPROC dbghelpdbghelp = nullptr;
    FARPROC dbghelpchksym = nullptr;
    FARPROC dbghelpblock = nullptr;
    FARPROC dbghelp_EFN_DumpImage = nullptr;
    FARPROC dbghelpWinDbgExtensionDllInit = nullptr;
    FARPROC dbghelpUnDecorateSymbolNameW = nullptr;
    FARPROC dbghelpUnDecorateSymbolName = nullptr;
    FARPROC dbghelpSymUnloadModule64 = nullptr;
    FARPROC dbghelpSymUnloadModule = nullptr;
    FARPROC dbghelpSymUnDName64 = nullptr;
    FARPROC dbghelpSymUnDName = nullptr;
    FARPROC dbghelpSymSrvStoreSupplementW = nullptr;
    FARPROC dbghelpSymSrvStoreSupplement = nullptr;
    FARPROC dbghelpSymSrvStoreFileW = nullptr;
    FARPROC dbghelpSymSrvStoreFile = nullptr;
    FARPROC dbghelpSymSrvIsStoreW = nullptr;
    FARPROC dbghelpSymSrvIsStore = nullptr;
    FARPROC dbghelpSymSrvGetSupplementW = nullptr;
    FARPROC dbghelpSymSrvGetSupplement = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexesW = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexes = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexStringW = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexString = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexInfoW = nullptr;
    FARPROC dbghelpSymSrvGetFileIndexInfo = nullptr;
    FARPROC dbghelpSymSrvDeltaNameW = nullptr;
    FARPROC dbghelpSymSrvDeltaName = nullptr;
    FARPROC dbghelpSymSetSearchPathW = nullptr;
    FARPROC dbghelpSymSetSearchPath = nullptr;
    FARPROC dbghelpSymSetScopeFromInlineContext = nullptr;
    FARPROC dbghelpSymSetScopeFromIndex = nullptr;
    FARPROC dbghelpSymSetScopeFromAddr = nullptr;
    FARPROC dbghelpSymSetParentWindow = nullptr;
    FARPROC dbghelpSymSetOptions = nullptr;
    FARPROC dbghelpSymSetHomeDirectoryW = nullptr;
    FARPROC dbghelpSymSetHomeDirectory = nullptr;
    FARPROC dbghelpSymSetExtendedOption = nullptr;
    FARPROC dbghelpSymSetDiaSession = nullptr;
    FARPROC dbghelpSymSetContext = nullptr;
    FARPROC dbghelpSymSearchW = nullptr;
    FARPROC dbghelpSymSearch = nullptr;
    FARPROC dbghelpSymRegisterFunctionEntryCallback64 = nullptr;
    FARPROC dbghelpSymRegisterFunctionEntryCallback = nullptr;
    FARPROC dbghelpSymRegisterCallbackW64 = nullptr;
    FARPROC dbghelpSymRegisterCallback64 = nullptr;
    FARPROC dbghelpSymRegisterCallback = nullptr;
    FARPROC dbghelpSymRefreshModuleList = nullptr;
    FARPROC dbghelpSymQueryInlineTrace = nullptr;
    FARPROC dbghelpSymPrevW = nullptr;
    FARPROC dbghelpSymPrev = nullptr;
    FARPROC dbghelpSymNextW = nullptr;
    FARPROC dbghelpSymNext = nullptr;
    FARPROC dbghelpSymMatchStringW = nullptr;
    FARPROC dbghelpSymMatchStringA = nullptr;
    FARPROC dbghelpSymMatchString = nullptr;
    FARPROC dbghelpSymMatchFileNameW = nullptr;
    FARPROC dbghelpSymMatchFileName = nullptr;
    FARPROC dbghelpSymLoadModuleExW = nullptr;
    FARPROC dbghelpSymLoadModuleEx = nullptr;
    FARPROC dbghelpSymLoadModule64 = nullptr;
    FARPROC dbghelpSymLoadModule = nullptr;
    FARPROC dbghelpSymInitializeW = nullptr;
    FARPROC dbghelpSymInitialize = nullptr;
    FARPROC dbghelpSymGetUnwindInfo = nullptr;
    FARPROC dbghelpSymGetTypeInfoEx = nullptr;
    FARPROC dbghelpSymGetTypeInfo = nullptr;
    FARPROC dbghelpSymGetTypeFromNameW = nullptr;
    FARPROC dbghelpSymGetTypeFromName = nullptr;
    FARPROC dbghelpSymGetSymbolFileW = nullptr;
    FARPROC dbghelpSymGetSymbolFile = nullptr;
    FARPROC dbghelpSymGetSymPrev64 = nullptr;
    FARPROC dbghelpSymGetSymPrev = nullptr;
    FARPROC dbghelpSymGetSymNext64 = nullptr;
    FARPROC dbghelpSymGetSymNext = nullptr;
    FARPROC dbghelpSymGetSymFromName64 = nullptr;
    FARPROC dbghelpSymGetSymFromName = nullptr;
    FARPROC dbghelpSymGetSymFromAddr64 = nullptr;
    FARPROC dbghelpSymGetSymFromAddr = nullptr;
    FARPROC dbghelpSymGetSourceVarFromTokenW = nullptr;
    FARPROC dbghelpSymGetSourceVarFromToken = nullptr;
    FARPROC dbghelpSymGetSourceFileW = nullptr;
    FARPROC dbghelpSymGetSourceFileTokenW = nullptr;
    FARPROC dbghelpSymGetSourceFileToken = nullptr;
    FARPROC dbghelpSymGetSourceFileFromTokenW = nullptr;
    FARPROC dbghelpSymGetSourceFileFromToken = nullptr;
    FARPROC dbghelpSymGetSourceFileChecksumW = nullptr;
    FARPROC dbghelpSymGetSourceFileChecksum = nullptr;
    FARPROC dbghelpSymGetSourceFile = nullptr;
    FARPROC dbghelpSymGetSearchPathW = nullptr;
    FARPROC dbghelpSymGetSearchPath = nullptr;
    FARPROC dbghelpSymGetScopeW = nullptr;
    FARPROC dbghelpSymGetScope = nullptr;
    FARPROC dbghelpSymGetOptions = nullptr;
    FARPROC dbghelpSymGetOmaps = nullptr;
    FARPROC dbghelpSymGetOmapBlockBase = nullptr;
    FARPROC dbghelpSymGetModuleInfoW64 = nullptr;
    FARPROC dbghelpSymGetModuleInfoW = nullptr;
    FARPROC dbghelpSymGetModuleInfo64 = nullptr;
    FARPROC dbghelpSymGetModuleInfo = nullptr;
    FARPROC dbghelpSymGetModuleBase64 = nullptr;
    FARPROC dbghelpSymGetModuleBase = nullptr;
    FARPROC dbghelpSymGetLinePrevW64 = nullptr;
    FARPROC dbghelpSymGetLinePrevEx = nullptr;
    FARPROC dbghelpSymGetLinePrev64 = nullptr;
    FARPROC dbghelpSymGetLinePrev = nullptr;
    FARPROC dbghelpSymGetLineNextW64 = nullptr;
    FARPROC dbghelpSymGetLineNextEx = nullptr;
    FARPROC dbghelpSymGetLineNext64 = nullptr;
    FARPROC dbghelpSymGetLineNext = nullptr;
    FARPROC dbghelpSymGetLineFromNameW64 = nullptr;
    FARPROC dbghelpSymGetLineFromNameEx = nullptr;
    FARPROC dbghelpSymGetLineFromName64 = nullptr;
    FARPROC dbghelpSymGetLineFromName = nullptr;
    FARPROC dbghelpSymGetLineFromInlineContextW = nullptr;
    FARPROC dbghelpSymGetLineFromInlineContext = nullptr;
    FARPROC dbghelpSymGetLineFromAddrW64 = nullptr;
    FARPROC dbghelpSymGetLineFromAddrEx = nullptr;
    FARPROC dbghelpSymGetLineFromAddr64 = nullptr;
    FARPROC dbghelpSymGetLineFromAddr = nullptr;
    FARPROC dbghelpSymGetHomeDirectoryW = nullptr;
    FARPROC dbghelpSymGetHomeDirectory = nullptr;
    FARPROC dbghelpSymGetFileLineOffsets64 = nullptr;
    FARPROC dbghelpSymGetExtendedOption = nullptr;
    FARPROC dbghelpSymGetDiaSession = nullptr;
    FARPROC dbghelpSymFunctionTableAccess64AccessRoutines = nullptr;
    FARPROC dbghelpSymFunctionTableAccess64 = nullptr;
    FARPROC dbghelpSymFunctionTableAccess = nullptr;
    FARPROC dbghelpSymFromTokenW = nullptr;
    FARPROC dbghelpSymFromToken = nullptr;
    FARPROC dbghelpSymFromNameW = nullptr;
    FARPROC dbghelpSymFromName = nullptr;
    FARPROC dbghelpSymFromInlineContextW = nullptr;
    FARPROC dbghelpSymFromInlineContext = nullptr;
    FARPROC dbghelpSymFromIndexW = nullptr;
    FARPROC dbghelpSymFromIndex = nullptr;
    FARPROC dbghelpSymFromAddrW = nullptr;
    FARPROC dbghelpSymFromAddr = nullptr;
    FARPROC dbghelpSymFreeDiaString = nullptr;
    FARPROC dbghelpSymFindFileInPathW = nullptr;
    FARPROC dbghelpSymFindFileInPath = nullptr;
    FARPROC dbghelpSymFindExecutableImageW = nullptr;
    FARPROC dbghelpSymFindExecutableImage = nullptr;
    FARPROC dbghelpSymFindDebugInfoFileW = nullptr;
    FARPROC dbghelpSymFindDebugInfoFile = nullptr;
    FARPROC dbghelpSymEnumerateSymbolsW64 = nullptr;
    FARPROC dbghelpSymEnumerateSymbolsW = nullptr;
    FARPROC dbghelpSymEnumerateSymbols64 = nullptr;
    FARPROC dbghelpSymEnumerateSymbols = nullptr;
    FARPROC dbghelpSymEnumerateModulesW64 = nullptr;
    FARPROC dbghelpSymEnumerateModules64 = nullptr;
    FARPROC dbghelpSymEnumerateModules = nullptr;
    FARPROC dbghelpSymEnumTypesW = nullptr;
    FARPROC dbghelpSymEnumTypesByNameW = nullptr;
    FARPROC dbghelpSymEnumTypesByName = nullptr;
    FARPROC dbghelpSymEnumTypes = nullptr;
    FARPROC dbghelpSymEnumSymbolsW = nullptr;
    FARPROC dbghelpSymEnumSymbolsForAddrW = nullptr;
    FARPROC dbghelpSymEnumSymbolsForAddr = nullptr;
    FARPROC dbghelpSymEnumSymbolsExW = nullptr;
    FARPROC dbghelpSymEnumSymbolsEx = nullptr;
    FARPROC dbghelpSymEnumSymbols = nullptr;
    FARPROC dbghelpSymEnumSym = nullptr;
    FARPROC dbghelpSymEnumSourceLinesW = nullptr;
    FARPROC dbghelpSymEnumSourceLines = nullptr;
    FARPROC dbghelpSymEnumSourceFilesW = nullptr;
    FARPROC dbghelpSymEnumSourceFiles = nullptr;
    FARPROC dbghelpSymEnumSourceFileTokens = nullptr;
    FARPROC dbghelpSymEnumProcesses = nullptr;
    FARPROC dbghelpSymEnumLinesW = nullptr;
    FARPROC dbghelpSymEnumLines = nullptr;
    FARPROC dbghelpSymDeleteSymbolW = nullptr;
    FARPROC dbghelpSymDeleteSymbol = nullptr;
    FARPROC dbghelpSymCompareInlineTrace = nullptr;
    FARPROC dbghelpSymCleanup = nullptr;
    FARPROC dbghelpSymAllocDiaString = nullptr;
    FARPROC dbghelpSymAddrIncludeInlineTrace = nullptr;
    FARPROC dbghelpSymAddSymbolW = nullptr;
    FARPROC dbghelpSymAddSymbol = nullptr;
    FARPROC dbghelpSymAddSourceStreamW = nullptr;
    FARPROC dbghelpSymAddSourceStreamA = nullptr;
    FARPROC dbghelpSymAddSourceStream = nullptr;
    FARPROC dbghelpStackWalkEx = nullptr;
    FARPROC dbghelpStackWalk64 = nullptr;
    FARPROC dbghelpStackWalk = nullptr;
    FARPROC dbghelpSetSymLoadError = nullptr;
    FARPROC dbghelpSetCheckUserInterruptShared = nullptr;
    FARPROC dbghelpSearchTreeForFileW = nullptr;
    FARPROC dbghelpSearchTreeForFile = nullptr;
    FARPROC dbghelpReportSymbolLoadSummary = nullptr;
    FARPROC dbghelpRemoveInvalidModuleList = nullptr;
    FARPROC dbghelpRangeMapWrite = nullptr;
    FARPROC dbghelpRangeMapRemove = nullptr;
    FARPROC dbghelpRangeMapRead = nullptr;
    FARPROC dbghelpRangeMapFree = nullptr;
    FARPROC dbghelpRangeMapCreate = nullptr;
    FARPROC dbghelpRangeMapAddPeImageSections = nullptr;
    FARPROC dbghelpMiniDumpWriteDump = nullptr;
    FARPROC dbghelpMiniDumpReadDumpStream = nullptr;
    FARPROC dbghelpMakeSureDirectoryPathExists = nullptr;
    FARPROC dbghelpImagehlpApiVersionEx = nullptr;
    FARPROC dbghelpImagehlpApiVersion = nullptr;
    FARPROC dbghelpImageRvaToVa = nullptr;
    FARPROC dbghelpImageRvaToSection = nullptr;
    FARPROC dbghelpImageNtHeader = nullptr;
    FARPROC dbghelpImageDirectoryEntryToDataEx = nullptr;
    FARPROC dbghelpImageDirectoryEntryToData = nullptr;
    FARPROC dbghelpGetTimestampForLoadedLibrary = nullptr;
    FARPROC dbghelpGetSymLoadError = nullptr;
    FARPROC dbghelpFindFileInSearchPath = nullptr;
    FARPROC dbghelpFindFileInPath = nullptr;
    FARPROC dbghelpFindExecutableImageExW = nullptr;
    FARPROC dbghelpFindExecutableImageEx = nullptr;
    FARPROC dbghelpFindExecutableImage = nullptr;
    FARPROC dbghelpFindDebugInfoFileExW = nullptr;
    FARPROC dbghelpFindDebugInfoFileEx = nullptr;
    FARPROC dbghelpFindDebugInfoFile = nullptr;
    FARPROC dbghelpExtensionApiVersion = nullptr;
    FARPROC dbghelpEnumerateLoadedModulesW64 = nullptr;
    FARPROC dbghelpEnumerateLoadedModulesExW = nullptr;
    FARPROC dbghelpEnumerateLoadedModulesEx = nullptr;
    FARPROC dbghelpEnumerateLoadedModules64 = nullptr;
    FARPROC dbghelpEnumerateLoadedModules = nullptr;
    FARPROC dbghelpEnumDirTreeW = nullptr;
    FARPROC dbghelpEnumDirTree = nullptr;
    FARPROC dbghelpDbgHelpCreateUserDumpW = nullptr;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        skipGetModuleHandle = true;
        dbghelpvc7fpo = GetProcAddress(dll, "vc7fpo");
        dbghelpsymsrv = GetProcAddress(dll, "symsrv");
        dbghelpsym = GetProcAddress(dll, "sym");
        dbghelpstackdbg = GetProcAddress(dll, "stackdbg");
        dbghelpstack_force_ebp = GetProcAddress(dll, "stack_force_ebp");
        dbghelpsrcfiles = GetProcAddress(dll, "srcfiles");
        dbghelpoptdbgdumpaddr = GetProcAddress(dll, "optdbgdumpaddr");
        dbghelpoptdbgdump = GetProcAddress(dll, "optdbgdump");
        dbghelpomap = GetProcAddress(dll, "omap");
        dbghelplminfo = GetProcAddress(dll, "lminfo");
        dbghelplmi = GetProcAddress(dll, "lmi");
        dbghelpitoldyouso = GetProcAddress(dll, "itoldyouso");
        dbghelpinlinedbg = GetProcAddress(dll, "inlinedbg");
        dbghelphomedir = GetProcAddress(dll, "homedir");
        dbghelpfptr = GetProcAddress(dll, "fptr");
        dbghelpdh = GetProcAddress(dll, "dh");
        dbghelpdbghelp = GetProcAddress(dll, "dbghelp");
        dbghelpchksym = GetProcAddress(dll, "chksym");
        dbghelpblock = GetProcAddress(dll, "block");
        dbghelp_EFN_DumpImage = GetProcAddress(dll, "_EFN_DumpImage");
        dbghelpWinDbgExtensionDllInit = GetProcAddress(dll, "WinDbgExtensionDllInit");
        dbghelpUnDecorateSymbolNameW = GetProcAddress(dll, "UnDecorateSymbolNameW");
        dbghelpUnDecorateSymbolName = GetProcAddress(dll, "UnDecorateSymbolName");
        dbghelpSymUnloadModule64 = GetProcAddress(dll, "SymUnloadModule64");
        dbghelpSymUnloadModule = GetProcAddress(dll, "SymUnloadModule");
        dbghelpSymUnDName64 = GetProcAddress(dll, "SymUnDName64");
        dbghelpSymUnDName = GetProcAddress(dll, "SymUnDName");
        dbghelpSymSrvStoreSupplementW = GetProcAddress(dll, "SymSrvStoreSupplementW");
        dbghelpSymSrvStoreSupplement = GetProcAddress(dll, "SymSrvStoreSupplement");
        dbghelpSymSrvStoreFileW = GetProcAddress(dll, "SymSrvStoreFileW");
        dbghelpSymSrvStoreFile = GetProcAddress(dll, "SymSrvStoreFile");
        dbghelpSymSrvIsStoreW = GetProcAddress(dll, "SymSrvIsStoreW");
        dbghelpSymSrvIsStore = GetProcAddress(dll, "SymSrvIsStore");
        dbghelpSymSrvGetSupplementW = GetProcAddress(dll, "SymSrvGetSupplementW");
        dbghelpSymSrvGetSupplement = GetProcAddress(dll, "SymSrvGetSupplement");
        dbghelpSymSrvGetFileIndexesW = GetProcAddress(dll, "SymSrvGetFileIndexesW");
        dbghelpSymSrvGetFileIndexes = GetProcAddress(dll, "SymSrvGetFileIndexes");
        dbghelpSymSrvGetFileIndexStringW = GetProcAddress(dll, "SymSrvGetFileIndexStringW");
        dbghelpSymSrvGetFileIndexString = GetProcAddress(dll, "SymSrvGetFileIndexString");
        dbghelpSymSrvGetFileIndexInfoW = GetProcAddress(dll, "SymSrvGetFileIndexInfoW");
        dbghelpSymSrvGetFileIndexInfo = GetProcAddress(dll, "SymSrvGetFileIndexInfo");
        dbghelpSymSrvDeltaNameW = GetProcAddress(dll, "SymSrvDeltaNameW");
        dbghelpSymSrvDeltaName = GetProcAddress(dll, "SymSrvDeltaName");
        dbghelpSymSetSearchPathW = GetProcAddress(dll, "SymSetSearchPathW");
        dbghelpSymSetSearchPath = GetProcAddress(dll, "SymSetSearchPath");
        dbghelpSymSetScopeFromInlineContext = GetProcAddress(dll, "SymSetScopeFromInlineContext");
        dbghelpSymSetScopeFromIndex = GetProcAddress(dll, "SymSetScopeFromIndex");
        dbghelpSymSetScopeFromAddr = GetProcAddress(dll, "SymSetScopeFromAddr");
        dbghelpSymSetParentWindow = GetProcAddress(dll, "SymSetParentWindow");
        dbghelpSymSetOptions = GetProcAddress(dll, "SymSetOptions");
        dbghelpSymSetHomeDirectoryW = GetProcAddress(dll, "SymSetHomeDirectoryW");
        dbghelpSymSetHomeDirectory = GetProcAddress(dll, "SymSetHomeDirectory");
        dbghelpSymSetExtendedOption = GetProcAddress(dll, "SymSetExtendedOption");
        dbghelpSymSetDiaSession = GetProcAddress(dll, "SymSetDiaSession");
        dbghelpSymSetContext = GetProcAddress(dll, "SymSetContext");
        dbghelpSymSearchW = GetProcAddress(dll, "SymSearchW");
        dbghelpSymSearch = GetProcAddress(dll, "SymSearch");
        dbghelpSymRegisterFunctionEntryCallback64 = GetProcAddress(dll, "SymRegisterFunctionEntryCallback64");
        dbghelpSymRegisterFunctionEntryCallback = GetProcAddress(dll, "SymRegisterFunctionEntryCallback");
        dbghelpSymRegisterCallbackW64 = GetProcAddress(dll, "SymRegisterCallbackW64");
        dbghelpSymRegisterCallback64 = GetProcAddress(dll, "SymRegisterCallback64");
        dbghelpSymRegisterCallback = GetProcAddress(dll, "SymRegisterCallback");
        dbghelpSymRefreshModuleList = GetProcAddress(dll, "SymRefreshModuleList");
        dbghelpSymQueryInlineTrace = GetProcAddress(dll, "SymQueryInlineTrace");
        dbghelpSymPrevW = GetProcAddress(dll, "SymPrevW");
        dbghelpSymPrev = GetProcAddress(dll, "SymPrev");
        dbghelpSymNextW = GetProcAddress(dll, "SymNextW");
        dbghelpSymNext = GetProcAddress(dll, "SymNext");
        dbghelpSymMatchStringW = GetProcAddress(dll, "SymMatchStringW");
        dbghelpSymMatchStringA = GetProcAddress(dll, "SymMatchStringA");
        dbghelpSymMatchString = GetProcAddress(dll, "SymMatchString");
        dbghelpSymMatchFileNameW = GetProcAddress(dll, "SymMatchFileNameW");
        dbghelpSymMatchFileName = GetProcAddress(dll, "SymMatchFileName");
        dbghelpSymLoadModuleExW = GetProcAddress(dll, "SymLoadModuleExW");
        dbghelpSymLoadModuleEx = GetProcAddress(dll, "SymLoadModuleEx");
        dbghelpSymLoadModule64 = GetProcAddress(dll, "SymLoadModule64");
        dbghelpSymLoadModule = GetProcAddress(dll, "SymLoadModule");
        dbghelpSymInitializeW = GetProcAddress(dll, "SymInitializeW");
        dbghelpSymInitialize = GetProcAddress(dll, "SymInitialize");
        dbghelpSymGetUnwindInfo = GetProcAddress(dll, "SymGetUnwindInfo");
        dbghelpSymGetTypeInfoEx = GetProcAddress(dll, "SymGetTypeInfoEx");
        dbghelpSymGetTypeInfo = GetProcAddress(dll, "SymGetTypeInfo");
        dbghelpSymGetTypeFromNameW = GetProcAddress(dll, "SymGetTypeFromNameW");
        dbghelpSymGetTypeFromName = GetProcAddress(dll, "SymGetTypeFromName");
        dbghelpSymGetSymbolFileW = GetProcAddress(dll, "SymGetSymbolFileW");
        dbghelpSymGetSymbolFile = GetProcAddress(dll, "SymGetSymbolFile");
        dbghelpSymGetSymPrev64 = GetProcAddress(dll, "SymGetSymPrev64");
        dbghelpSymGetSymPrev = GetProcAddress(dll, "SymGetSymPrev");
        dbghelpSymGetSymNext64 = GetProcAddress(dll, "SymGetSymNext64");
        dbghelpSymGetSymNext = GetProcAddress(dll, "SymGetSymNext");
        dbghelpSymGetSymFromName64 = GetProcAddress(dll, "SymGetSymFromName64");
        dbghelpSymGetSymFromName = GetProcAddress(dll, "SymGetSymFromName");
        dbghelpSymGetSymFromAddr64 = GetProcAddress(dll, "SymGetSymFromAddr64");
        dbghelpSymGetSymFromAddr = GetProcAddress(dll, "SymGetSymFromAddr");
        dbghelpSymGetSourceVarFromTokenW = GetProcAddress(dll, "SymGetSourceVarFromTokenW");
        dbghelpSymGetSourceVarFromToken = GetProcAddress(dll, "SymGetSourceVarFromToken");
        dbghelpSymGetSourceFileW = GetProcAddress(dll, "SymGetSourceFileW");
        dbghelpSymGetSourceFileTokenW = GetProcAddress(dll, "SymGetSourceFileTokenW");
        dbghelpSymGetSourceFileToken = GetProcAddress(dll, "SymGetSourceFileToken");
        dbghelpSymGetSourceFileFromTokenW = GetProcAddress(dll, "SymGetSourceFileFromTokenW");
        dbghelpSymGetSourceFileFromToken = GetProcAddress(dll, "SymGetSourceFileFromToken");
        dbghelpSymGetSourceFileChecksumW = GetProcAddress(dll, "SymGetSourceFileChecksumW");
        dbghelpSymGetSourceFileChecksum = GetProcAddress(dll, "SymGetSourceFileChecksum");
        dbghelpSymGetSourceFile = GetProcAddress(dll, "SymGetSourceFile");
        dbghelpSymGetSearchPathW = GetProcAddress(dll, "SymGetSearchPathW");
        dbghelpSymGetSearchPath = GetProcAddress(dll, "SymGetSearchPath");
        dbghelpSymGetScopeW = GetProcAddress(dll, "SymGetScopeW");
        dbghelpSymGetScope = GetProcAddress(dll, "SymGetScope");
        dbghelpSymGetOptions = GetProcAddress(dll, "SymGetOptions");
        dbghelpSymGetOmaps = GetProcAddress(dll, "SymGetOmaps");
        dbghelpSymGetOmapBlockBase = GetProcAddress(dll, "SymGetOmapBlockBase");
        dbghelpSymGetModuleInfoW64 = GetProcAddress(dll, "SymGetModuleInfoW64");
        dbghelpSymGetModuleInfoW = GetProcAddress(dll, "SymGetModuleInfoW");
        dbghelpSymGetModuleInfo64 = GetProcAddress(dll, "SymGetModuleInfo64");
        dbghelpSymGetModuleInfo = GetProcAddress(dll, "SymGetModuleInfo");
        dbghelpSymGetModuleBase64 = GetProcAddress(dll, "SymGetModuleBase64");
        dbghelpSymGetModuleBase = GetProcAddress(dll, "SymGetModuleBase");
        dbghelpSymGetLinePrevW64 = GetProcAddress(dll, "SymGetLinePrevW64");
        dbghelpSymGetLinePrevEx = GetProcAddress(dll, "SymGetLinePrevEx");
        dbghelpSymGetLinePrev64 = GetProcAddress(dll, "SymGetLinePrev64");
        dbghelpSymGetLinePrev = GetProcAddress(dll, "SymGetLinePrev");
        dbghelpSymGetLineNextW64 = GetProcAddress(dll, "SymGetLineNextW64");
        dbghelpSymGetLineNextEx = GetProcAddress(dll, "SymGetLineNextEx");
        dbghelpSymGetLineNext64 = GetProcAddress(dll, "SymGetLineNext64");
        dbghelpSymGetLineNext = GetProcAddress(dll, "SymGetLineNext");
        dbghelpSymGetLineFromNameW64 = GetProcAddress(dll, "SymGetLineFromNameW64");
        dbghelpSymGetLineFromNameEx = GetProcAddress(dll, "SymGetLineFromNameEx");
        dbghelpSymGetLineFromName64 = GetProcAddress(dll, "SymGetLineFromName64");
        dbghelpSymGetLineFromName = GetProcAddress(dll, "SymGetLineFromName");
        dbghelpSymGetLineFromInlineContextW = GetProcAddress(dll, "SymGetLineFromInlineContextW");
        dbghelpSymGetLineFromInlineContext = GetProcAddress(dll, "SymGetLineFromInlineContext");
        dbghelpSymGetLineFromAddrW64 = GetProcAddress(dll, "SymGetLineFromAddrW64");
        dbghelpSymGetLineFromAddrEx = GetProcAddress(dll, "SymGetLineFromAddrEx");
        dbghelpSymGetLineFromAddr64 = GetProcAddress(dll, "SymGetLineFromAddr64");
        dbghelpSymGetLineFromAddr = GetProcAddress(dll, "SymGetLineFromAddr");
        dbghelpSymGetHomeDirectoryW = GetProcAddress(dll, "SymGetHomeDirectoryW");
        dbghelpSymGetHomeDirectory = GetProcAddress(dll, "SymGetHomeDirectory");
        dbghelpSymGetFileLineOffsets64 = GetProcAddress(dll, "SymGetFileLineOffsets64");
        dbghelpSymGetExtendedOption = GetProcAddress(dll, "SymGetExtendedOption");
        dbghelpSymGetDiaSession = GetProcAddress(dll, "SymGetDiaSession");
        dbghelpSymFunctionTableAccess64AccessRoutines = GetProcAddress(dll, "SymFunctionTableAccess64AccessRoutines");
        dbghelpSymFunctionTableAccess64 = GetProcAddress(dll, "SymFunctionTableAccess64");
        dbghelpSymFunctionTableAccess = GetProcAddress(dll, "SymFunctionTableAccess");
        dbghelpSymFromTokenW = GetProcAddress(dll, "SymFromTokenW");
        dbghelpSymFromToken = GetProcAddress(dll, "SymFromToken");
        dbghelpSymFromNameW = GetProcAddress(dll, "SymFromNameW");
        dbghelpSymFromName = GetProcAddress(dll, "SymFromName");
        dbghelpSymFromInlineContextW = GetProcAddress(dll, "SymFromInlineContextW");
        dbghelpSymFromInlineContext = GetProcAddress(dll, "SymFromInlineContext");
        dbghelpSymFromIndexW = GetProcAddress(dll, "SymFromIndexW");
        dbghelpSymFromIndex = GetProcAddress(dll, "SymFromIndex");
        dbghelpSymFromAddrW = GetProcAddress(dll, "SymFromAddrW");
        dbghelpSymFromAddr = GetProcAddress(dll, "SymFromAddr");
        dbghelpSymFreeDiaString = GetProcAddress(dll, "SymFreeDiaString");
        dbghelpSymFindFileInPathW = GetProcAddress(dll, "SymFindFileInPathW");
        dbghelpSymFindFileInPath = GetProcAddress(dll, "SymFindFileInPath");
        dbghelpSymFindExecutableImageW = GetProcAddress(dll, "SymFindExecutableImageW");
        dbghelpSymFindExecutableImage = GetProcAddress(dll, "SymFindExecutableImage");
        dbghelpSymFindDebugInfoFileW = GetProcAddress(dll, "SymFindDebugInfoFileW");
        dbghelpSymFindDebugInfoFile = GetProcAddress(dll, "SymFindDebugInfoFile");
        dbghelpSymEnumerateSymbolsW64 = GetProcAddress(dll, "SymEnumerateSymbolsW64");
        dbghelpSymEnumerateSymbolsW = GetProcAddress(dll, "SymEnumerateSymbolsW");
        dbghelpSymEnumerateSymbols64 = GetProcAddress(dll, "SymEnumerateSymbols64");
        dbghelpSymEnumerateSymbols = GetProcAddress(dll, "SymEnumerateSymbols");
        dbghelpSymEnumerateModulesW64 = GetProcAddress(dll, "SymEnumerateModulesW64");
        dbghelpSymEnumerateModules64 = GetProcAddress(dll, "SymEnumerateModules64");
        dbghelpSymEnumerateModules = GetProcAddress(dll, "SymEnumerateModules");
        dbghelpSymEnumTypesW = GetProcAddress(dll, "SymEnumTypesW");
        dbghelpSymEnumTypesByNameW = GetProcAddress(dll, "SymEnumTypesByNameW");
        dbghelpSymEnumTypesByName = GetProcAddress(dll, "SymEnumTypesByName");
        dbghelpSymEnumTypes = GetProcAddress(dll, "SymEnumTypes");
        dbghelpSymEnumSymbolsW = GetProcAddress(dll, "SymEnumSymbolsW");
        dbghelpSymEnumSymbolsForAddrW = GetProcAddress(dll, "SymEnumSymbolsForAddrW");
        dbghelpSymEnumSymbolsForAddr = GetProcAddress(dll, "SymEnumSymbolsForAddr");
        dbghelpSymEnumSymbolsExW = GetProcAddress(dll, "SymEnumSymbolsExW");
        dbghelpSymEnumSymbolsEx = GetProcAddress(dll, "SymEnumSymbolsEx");
        dbghelpSymEnumSymbols = GetProcAddress(dll, "SymEnumSymbols");
        dbghelpSymEnumSym = GetProcAddress(dll, "SymEnumSym");
        dbghelpSymEnumSourceLinesW = GetProcAddress(dll, "SymEnumSourceLinesW");
        dbghelpSymEnumSourceLines = GetProcAddress(dll, "SymEnumSourceLines");
        dbghelpSymEnumSourceFilesW = GetProcAddress(dll, "SymEnumSourceFilesW");
        dbghelpSymEnumSourceFiles = GetProcAddress(dll, "SymEnumSourceFiles");
        dbghelpSymEnumSourceFileTokens = GetProcAddress(dll, "SymEnumSourceFileTokens");
        dbghelpSymEnumProcesses = GetProcAddress(dll, "SymEnumProcesses");
        dbghelpSymEnumLinesW = GetProcAddress(dll, "SymEnumLinesW");
        dbghelpSymEnumLines = GetProcAddress(dll, "SymEnumLines");
        dbghelpSymDeleteSymbolW = GetProcAddress(dll, "SymDeleteSymbolW");
        dbghelpSymDeleteSymbol = GetProcAddress(dll, "SymDeleteSymbol");
        dbghelpSymCompareInlineTrace = GetProcAddress(dll, "SymCompareInlineTrace");
        dbghelpSymCleanup = GetProcAddress(dll, "SymCleanup");
        dbghelpSymAllocDiaString = GetProcAddress(dll, "SymAllocDiaString");
        dbghelpSymAddrIncludeInlineTrace = GetProcAddress(dll, "SymAddrIncludeInlineTrace");
        dbghelpSymAddSymbolW = GetProcAddress(dll, "SymAddSymbolW");
        dbghelpSymAddSymbol = GetProcAddress(dll, "SymAddSymbol");
        dbghelpSymAddSourceStreamW = GetProcAddress(dll, "SymAddSourceStreamW");
        dbghelpSymAddSourceStreamA = GetProcAddress(dll, "SymAddSourceStreamA");
        dbghelpSymAddSourceStream = GetProcAddress(dll, "SymAddSourceStream");
        dbghelpStackWalkEx = GetProcAddress(dll, "StackWalkEx");
        dbghelpStackWalk64 = GetProcAddress(dll, "StackWalk64");
        dbghelpStackWalk = GetProcAddress(dll, "StackWalk");
        dbghelpSetSymLoadError = GetProcAddress(dll, "SetSymLoadError");
        dbghelpSetCheckUserInterruptShared = GetProcAddress(dll, "SetCheckUserInterruptShared");
        dbghelpSearchTreeForFileW = GetProcAddress(dll, "SearchTreeForFileW");
        dbghelpSearchTreeForFile = GetProcAddress(dll, "SearchTreeForFile");
        dbghelpReportSymbolLoadSummary = GetProcAddress(dll, "ReportSymbolLoadSummary");
        dbghelpRemoveInvalidModuleList = GetProcAddress(dll, "RemoveInvalidModuleList");
        dbghelpRangeMapWrite = GetProcAddress(dll, "RangeMapWrite");
        dbghelpRangeMapRemove = GetProcAddress(dll, "RangeMapRemove");
        dbghelpRangeMapRead = GetProcAddress(dll, "RangeMapRead");
        dbghelpRangeMapFree = GetProcAddress(dll, "RangeMapFree");
        dbghelpRangeMapCreate = GetProcAddress(dll, "RangeMapCreate");
        dbghelpRangeMapAddPeImageSections = GetProcAddress(dll, "RangeMapAddPeImageSections");
        dbghelpMiniDumpWriteDump = GetProcAddress(dll, "MiniDumpWriteDump");
        dbghelpMiniDumpReadDumpStream = GetProcAddress(dll, "MiniDumpReadDumpStream");
        dbghelpMakeSureDirectoryPathExists = GetProcAddress(dll, "MakeSureDirectoryPathExists");
        dbghelpImagehlpApiVersionEx = GetProcAddress(dll, "ImagehlpApiVersionEx");
        dbghelpImagehlpApiVersion = GetProcAddress(dll, "ImagehlpApiVersion");
        dbghelpImageRvaToVa = GetProcAddress(dll, "ImageRvaToVa");
        dbghelpImageRvaToSection = GetProcAddress(dll, "ImageRvaToSection");
        dbghelpImageNtHeader = GetProcAddress(dll, "ImageNtHeader");
        dbghelpImageDirectoryEntryToDataEx = GetProcAddress(dll, "ImageDirectoryEntryToDataEx");
        dbghelpImageDirectoryEntryToData = GetProcAddress(dll, "ImageDirectoryEntryToData");
        dbghelpGetTimestampForLoadedLibrary = GetProcAddress(dll, "GetTimestampForLoadedLibrary");
        dbghelpGetSymLoadError = GetProcAddress(dll, "GetSymLoadError");
        dbghelpFindFileInSearchPath = GetProcAddress(dll, "FindFileInSearchPath");
        dbghelpFindFileInPath = GetProcAddress(dll, "FindFileInPath");
        dbghelpFindExecutableImageExW = GetProcAddress(dll, "FindExecutableImageExW");
        dbghelpFindExecutableImageEx = GetProcAddress(dll, "FindExecutableImageEx");
        dbghelpFindExecutableImage = GetProcAddress(dll, "FindExecutableImage");
        dbghelpFindDebugInfoFileExW = GetProcAddress(dll, "FindDebugInfoFileExW");
        dbghelpFindDebugInfoFileEx = GetProcAddress(dll, "FindDebugInfoFileEx");
        dbghelpFindDebugInfoFile = GetProcAddress(dll, "FindDebugInfoFile");
        dbghelpExtensionApiVersion = GetProcAddress(dll, "ExtensionApiVersion");
        dbghelpEnumerateLoadedModulesW64 = GetProcAddress(dll, "EnumerateLoadedModulesW64");
        dbghelpEnumerateLoadedModulesExW = GetProcAddress(dll, "EnumerateLoadedModulesExW");
        dbghelpEnumerateLoadedModulesEx = GetProcAddress(dll, "EnumerateLoadedModulesEx");
        dbghelpEnumerateLoadedModules64 = GetProcAddress(dll, "EnumerateLoadedModules64");
        dbghelpEnumerateLoadedModules = GetProcAddress(dll, "EnumerateLoadedModules");
        dbghelpEnumDirTreeW = GetProcAddress(dll, "EnumDirTreeW");
        dbghelpEnumDirTree = GetProcAddress(dll, "EnumDirTree");
        dbghelpDbgHelpCreateUserDumpW = GetProcAddress(dll, "DbgHelpCreateUserDumpW");
        skipGetModuleHandle = false;
    }
} dbghelp;

#pragma endregion

#pragma region DXGI Adapter methods

bool SkipSpoofing()
{
    auto skip = !Config::Instance()->DxgiSpoofing.value_or_default() || State::Instance().skipSpoofing; // || State::Instance().isRunningOnLinux;

    if (skip)
        LOG_TRACE("DxgiSpoofing: {}, skipSpoofing: {}, skipping spoofing",
                  Config::Instance()->DxgiSpoofing.value_or_default(), State::Instance().skipSpoofing);

    HANDLE process = GetCurrentProcess();

    if (!skip && Config::Instance()->DxgiBlacklist.has_value() && process != nullptr)
    {
        skip = true;

        /*
        * File based spoofing
        SymInitialize(process, NULL, TRUE);

        // Capture stack trace
        const int maxFrames = 64;
        void* stack[maxFrames];
        USHORT frames = CaptureStackBackTrace(0, maxFrames, stack, NULL);

        for (USHORT i = 0; i < frames; ++i)
        {
            DWORD64 address = (DWORD64)(stack[i]);
            DWORD64 moduleBase = SymGetModuleBase64(process, address);

            if (moduleBase)
            {
                char moduleName[MAX_PATH];
                if (GetModuleFileNameA((HINSTANCE)moduleBase, moduleName, MAX_PATH))
                {
                    std::string module(moduleName);
                    LOG_TRACE("ModuleName: {}", module);

                    auto pos = module.rfind("\\sl.");

                    if (pos != std::string::npos)
                    {
                        LOG_INFO("spoofing for: {0}", module);
                        skip = false;
                        break;
                    }
                }
            }
        }

        SymCleanup(process);
        */

        // Walk the call stack to find the DLL that is calling the hooked function
        void* callers[100];

        unsigned short frames = CaptureStackBackTrace(0, 100, callers, NULL);
        //HANDLE process = GetCurrentProcess();

        if (SymInitialize(process, NULL, TRUE))
        {
            SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);

            if (symbol != nullptr)
            {
                symbol->MaxNameLen = 255;
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

                for (unsigned int i = 0; i < frames; i++)
                {
                    if (SymFromAddr(process, (DWORD64)callers[i], 0, symbol))
                    {
                        auto sn = std::string(symbol->Name);
                        auto pos = Config::Instance()->DxgiBlacklist.value().rfind(sn);

                        LOG_DEBUG("checking for: {0} ({1})", sn, i);

                        if (pos != std::string::npos)
                        {
                            LOG_INFO("spoofing for: {0}", sn);
                            skip = false;
                            break;
                        }
                    }
                }

                free(symbol);
            }

            SymCleanup(process);
        }

        if (skip)
            LOG_DEBUG("skipping spoofing, blacklisting active");
    }

    return skip;
}

HRESULT WINAPI detGetDesc3(IDXGIAdapter4* This, DXGI_ADAPTER_DESC3* pDesc)
{
    auto result = ptrGetDesc3(This, pDesc);
    if (result == S_OK)
    {
        if (pDesc->VendorId != 0x1414 && !State::Instance().adapterDescs.contains(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart))
        {
            std::wstring szName(pDesc->Description);
            std::string descStr = std::format("Adapter: {}, VRAM: {} MB", wstring_to_string(szName), pDesc->DedicatedVideoMemory / (1024 * 1024));
            LOG_INFO("{}", descStr);
            State::Instance().adapterDescs.insert_or_assign(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart, descStr);
        }

        if (Config::Instance()->DxgiVRAM.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiVRAM.value() * 1024 * 1024 * 1024;

        if (!SkipSpoofing() && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or_default())
        {
            pDesc->VendorId = 0x10de;
            pDesc->DeviceId = 0x2684;

            auto szName = Config::Instance()->SpoofedGPUName.value_or_default();
            std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
            std::wcscpy(pDesc->Description, szName.c_str());

#ifdef _DEBUG
            LOG_DEBUG("spoofing");
#endif 
        }
    }

    AttachToAdapter(This);

    return result;
}

HRESULT WINAPI detGetDesc2(IDXGIAdapter2* This, DXGI_ADAPTER_DESC2* pDesc)
{
    auto result = ptrGetDesc2(This, pDesc);
    if (result == S_OK)
    {
        if (pDesc->VendorId != 0x1414 && !State::Instance().adapterDescs.contains(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart))
        {
            std::wstring szName(pDesc->Description);
            std::string descStr = std::format("Adapter: {}, VRAM: {} MB", wstring_to_string(szName), pDesc->DedicatedVideoMemory / (1024 * 1024));
            LOG_INFO("{}", descStr);
            State::Instance().adapterDescs.insert_or_assign(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart, descStr);
        }

        if (Config::Instance()->DxgiVRAM.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiVRAM.value() * 1024 * 1024 * 1024;

        if (!SkipSpoofing() && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or_default())
        {
            pDesc->VendorId = 0x10de;
            pDesc->DeviceId = 0x2684;

            auto szName = Config::Instance()->SpoofedGPUName.value_or_default();
            std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
            std::wcscpy(pDesc->Description, szName.c_str());

#ifdef _DEBUG
            LOG_DEBUG("spoofing");
#endif 
        }
    }

    AttachToAdapter(This);

    return result;
}

HRESULT WINAPI detGetDesc1(IDXGIAdapter1* This, DXGI_ADAPTER_DESC1* pDesc)
{
    auto result = ptrGetDesc1(This, pDesc);
    if (result == S_OK)
    {
        if (pDesc->VendorId != 0x1414 && !State::Instance().adapterDescs.contains(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart))
        {
            std::wstring szName(pDesc->Description);
            std::string descStr = std::format("Adapter: {}, VRAM: {} MB", wstring_to_string(szName), pDesc->DedicatedVideoMemory / (1024 * 1024));
            LOG_INFO("{}", descStr);
            State::Instance().adapterDescs.insert_or_assign(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart, descStr);
        }

        if (Config::Instance()->DxgiVRAM.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiVRAM.value() * 1024 * 1024 * 1024;

        if (!SkipSpoofing() && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or_default())
        {
            pDesc->VendorId = 0x10de;
            pDesc->DeviceId = 0x2684;

            auto szName = Config::Instance()->SpoofedGPUName.value_or_default();
            std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
            std::wcscpy(pDesc->Description, szName.c_str());

#ifdef _DEBUG
            LOG_DEBUG("spoofing");
#endif
        }
    }

    AttachToAdapter(This);

    return result;
}

HRESULT WINAPI detGetDesc(IDXGIAdapter* This, DXGI_ADAPTER_DESC* pDesc)
{
    auto result = ptrGetDesc(This, pDesc);
    if (result == S_OK)
    {
        if (pDesc->VendorId != 0x1414 && !State::Instance().adapterDescs.contains(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart))
        {
            std::wstring szName(pDesc->Description);
            std::string descStr = std::format("Adapter: {}, VRAM: {} MB", wstring_to_string(szName), pDesc->DedicatedVideoMemory / (1024 * 1024));
            LOG_INFO("{}", descStr);
            State::Instance().adapterDescs.insert_or_assign(pDesc->AdapterLuid.HighPart | pDesc->AdapterLuid.LowPart, descStr);
        }

        if (Config::Instance()->DxgiVRAM.has_value())
            pDesc->DedicatedVideoMemory = (UINT64)Config::Instance()->DxgiVRAM.value() * 1024 * 1024 * 1024;

        if (!SkipSpoofing() && pDesc->VendorId != 0x1414 && Config::Instance()->DxgiSpoofing.value_or_default())
        {
            pDesc->VendorId = 0x10de;
            pDesc->DeviceId = 0x2684;

            auto szName = Config::Instance()->SpoofedGPUName.value_or_default();
            std::memset(pDesc->Description, 0, sizeof(pDesc->Description));
            std::wcscpy(pDesc->Description, szName.c_str());

#ifdef _DEBUG
            LOG_DEBUG("spoofing");
#endif
        }
    }

    AttachToAdapter(This);

    return result;
}

#pragma endregion

#pragma region DXGI Factory methods

HRESULT WINAPI detEnumAdapterByGpuPreference(IDXGIFactory6* This, UINT Adapter, DXGI_GPU_PREFERENCE GpuPreference, REFIID riid, IDXGIAdapter** ppvAdapter)
{
    AttachToFactory(This);

    State::Instance().skipDxgiLoadChecks = true;
    auto result = ptrEnumAdapterByGpuPreference(This, Adapter, GpuPreference, riid, ppvAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        AttachToAdapter(*ppvAdapter);

    return result;
}

HRESULT WINAPI detEnumAdapterByLuid(IDXGIFactory4* This, LUID AdapterLuid, REFIID riid, IDXGIAdapter** ppvAdapter)
{
    AttachToFactory(This);

    State::Instance().skipDxgiLoadChecks = true;
    auto result = ptrEnumAdapterByLuid(This, AdapterLuid, riid, ppvAdapter);
    State::Instance().skipDxgiLoadChecks = false;

    if (result == S_OK)
        AttachToAdapter(*ppvAdapter);

    return result;
}

HRESULT WINAPI detEnumAdapters1(IDXGIFactory1* This, UINT Adapter, IDXGIAdapter1** ppAdapter)
{
    LOG_TRACE("dllmain");

    AttachToFactory(This);

    HRESULT result = S_OK;

    if (!skipHighPerfCheck && Config::Instance()->PreferDedicatedGpu.value_or_default())
    {
        if (Config::Instance()->PreferFirstDedicatedGpu.value_or_default() && Adapter > 0)
        {
            LOG_DEBUG("{}, returning not found", Adapter);
            return DXGI_ERROR_NOT_FOUND;
        }

        IDXGIFactory6* factory6 = nullptr;
        if (This->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK && factory6 != nullptr)
        {
            LOG_DEBUG("Trying to select high performance adapter ({})", Adapter);

            skipHighPerfCheck = true;
            State::Instance().skipDxgiLoadChecks = true;
            result = ptrEnumAdapterByGpuPreference(factory6, Adapter, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter), (IDXGIAdapter**)ppAdapter);
            State::Instance().skipDxgiLoadChecks = false;
            skipHighPerfCheck = false;

            if (result != S_OK)
            {
                LOG_ERROR("Can't get high performance adapter: {:X}, fallback to standard method", Adapter);
                result = ptrEnumAdapters1(This, Adapter, ppAdapter);
            }

            if (result == S_OK)
            {
                DXGI_ADAPTER_DESC desc;
                State::Instance().skipSpoofing = true;
                if ((*ppAdapter)->GetDesc(&desc) == S_OK)
                {
                    std::wstring name(desc.Description);
                    LOG_DEBUG("High performance adapter ({}) will be used", wstring_to_string(name));
                }
                else
                {
                    LOG_DEBUG("High performance adapter (Can't get description!) will be used");
                }
                State::Instance().skipSpoofing = false;
            }

            factory6->Release();
        }
    }
    else
    {
        State::Instance().skipDxgiLoadChecks = true;
        result = ptrEnumAdapters1(This, Adapter, ppAdapter);
        State::Instance().skipDxgiLoadChecks = false;
    }

    if (result == S_OK)
        AttachToAdapter(*ppAdapter);

    return result;
}

HRESULT WINAPI detEnumAdapters(IDXGIFactory* This, UINT Adapter, IDXGIAdapter** ppAdapter)
{
    AttachToFactory(This);

    HRESULT result = S_OK;

    if (!skipHighPerfCheck && Config::Instance()->PreferDedicatedGpu.value_or_default())
    {
        if (Config::Instance()->PreferFirstDedicatedGpu.value_or_default() && Adapter > 0)
        {
            LOG_DEBUG("{}, returning not found", Adapter);
            return DXGI_ERROR_NOT_FOUND;
        }

        IDXGIFactory6* factory6 = nullptr;
        if (This->QueryInterface(IID_PPV_ARGS(&factory6)) == S_OK && factory6 != nullptr)
        {
            LOG_DEBUG("Trying to select high performance adapter ({})", Adapter);

            skipHighPerfCheck = true;
            State::Instance().skipDxgiLoadChecks = true;
            result = ptrEnumAdapterByGpuPreference(factory6, Adapter, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter), ppAdapter);
            State::Instance().skipDxgiLoadChecks = false;
            skipHighPerfCheck = false;

            if (result != S_OK)
            {
                LOG_ERROR("Can't get high performance adapter: {:X}, fallback to standard method", Adapter);
                result = ptrEnumAdapters(This, Adapter, ppAdapter);
            }

            if (result == S_OK)
            {
                DXGI_ADAPTER_DESC desc;
                State::Instance().skipSpoofing = true;
                if ((*ppAdapter)->GetDesc(&desc) == S_OK)
                {
                    std::wstring name(desc.Description);
                    LOG_DEBUG("Adapter ({}) will be used", wstring_to_string(name));
                }
                else
                {
                    LOG_ERROR("Can't get adapter description!");
                }
                State::Instance().skipSpoofing = false;
            }

            factory6->Release();
        }
    }
    else
    {
        State::Instance().skipDxgiLoadChecks = true;
        result = ptrEnumAdapters(This, Adapter, ppAdapter);
        State::Instance().skipDxgiLoadChecks = false;
    }

    if (result == S_OK)
        AttachToAdapter(*ppAdapter);

    return result;
}

#pragma endregion

#pragma region DXGI methods

HRESULT _CreateDXGIFactory(REFIID riid, IDXGIFactory** ppFactory)
{
    State::Instance().skipDxgiLoadChecks = true;
    if (dxgi.CreateDxgiFactory) {
        HRESULT result = dxgi.CreateDxgiFactory(riid, ppFactory);
        State::Instance().skipDxgiLoadChecks = false;

        if (result == S_OK)
            AttachToFactory(*ppFactory);

        return result;
    }
    else 
    {
        LOG_ERROR("CreateDxgiFactory is NULL");
        State::Instance().skipDxgiLoadChecks = false;
        return DXGI_ERROR_INVALID_CALL;
    }
}

HRESULT _CreateDXGIFactory1(REFIID riid, IDXGIFactory1** ppFactory)
{
    State::Instance().skipDxgiLoadChecks = true;
    if (dxgi.CreateDxgiFactory1) {
        HRESULT result = dxgi.CreateDxgiFactory1(riid, ppFactory);
        State::Instance().skipDxgiLoadChecks = false;

        if (result == S_OK)
            AttachToFactory(*ppFactory);

        return result;
    }
    else 
    {
        LOG_ERROR("CreateDxgiFactory1 is NULL");
        State::Instance().skipDxgiLoadChecks = false;
        return DXGI_ERROR_INVALID_CALL;
    }
}

HRESULT _CreateDXGIFactory2(UINT Flags, REFIID riid, IDXGIFactory2** ppFactory)
{
    State::Instance().skipDxgiLoadChecks = true;
    if (dxgi.CreateDxgiFactory2) {
        HRESULT result = dxgi.CreateDxgiFactory2(Flags, riid, ppFactory);
        State::Instance().skipDxgiLoadChecks = false;

        if (result == S_OK)
            AttachToFactory(*ppFactory);

        return result;
    }
    else
    {
        LOG_ERROR("CreateDxgiFactory2 is NULL");
        State::Instance().skipDxgiLoadChecks = false;
        return DXGI_ERROR_INVALID_CALL;
    }
}

HRESULT _DXGIDeclareAdapterRemovalSupport()
{
    LOG_FUNC();
    return dxgi.dxgiDeclareAdapterRemovalSupport();
}

HRESULT _DXGIGetDebugInterface1(UINT Flags, REFIID riid, void** pDebug)
{
    LOG_FUNC();
    return dxgi.dxgiGetDebugInterface(Flags, riid, pDebug);
}

//void _ApplyCompatResolutionQuirking()
//{
//    LOG_FUNC();
//    dxgi.dxgiApplyCompatResolutionQuirking();
//}
//
//void _CompatString()
//{
//    LOG_FUNC();
//    dxgi.dxgiCompatString();
//}
//
//void _CompatValue()
//{
//    LOG_FUNC();
//    dxgi.dxgiCompatValue();
//}
//
//void _DXGID3D10CreateDevice()
//{
//    LOG_FUNC();
//    dxgi.dxgiD3D10CreateDevice();
//}
//
//void _DXGID3D10CreateLayeredDevice()
//{
//    LOG_FUNC();
//    dxgi.dxgiD3D10CreateLayeredDevice();
//}
//
//void _DXGID3D10GetLayeredDeviceSize()
//{
//    LOG_FUNC();
//    dxgi.dxgiD3D10GetLayeredDeviceSize();
//}
//
//void _DXGID3D10RegisterLayers()
//{
//    LOG_FUNC();
//    dxgi.dxgiD3D10RegisterLayers();
//}
//
//void _DXGID3D10ETWRundown()
//{
//    LOG_FUNC();
//    dxgi.dxgiD3D10ETWRundown();
//}
//
//void _DXGIDumpJournal()
//{
//    LOG_FUNC();
//    dxgi.dxgiDumpJournal();
//}
//
//void _DXGIReportAdapterConfiguration()
//{
//    LOG_FUNC();
//    dxgi.dxgiReportAdapterConfiguration();
//}
//
//void _PIXBeginCapture()
//{
//    LOG_FUNC();
//    dxgi.dxgiPIXBeginCapture();
//}
//
//void _PIXEndCapture()
//{
//    LOG_FUNC();
//    dxgi.dxgiPIXEndCapture();
//}
//
//void _PIXGetCaptureState()
//{
//    LOG_FUNC();
//    dxgi.dxgiPIXGetCaptureState();
//}
//
//void _SetAppCompatStringPointer()
//{
//    LOG_FUNC();
//    dxgi.dxgiSetAppCompatStringPointer();
//}
//
//void _UpdateHMDEmulationStatus()
//{
//    LOG_FUNC();
//    dxgi.dxgiUpdateHMDEmulationStatus();
//}

#pragma endregion

#pragma region DXGI Attach methods

void AttachToAdapter(IUnknown* unkAdapter)
{
    const GUID guid = { 0x907bf281,0xea3c,0x43b4,{0xa8,0xe4,0x9f,0x23,0x11,0x07,0xb4,0xff} };

    PVOID* pVTable = *(PVOID**)unkAdapter;

    bool dxvkStatus = State::Instance().isRunningOnDXVK;

    IDXGIAdapter* adapter = nullptr;
    bool adapterOk = unkAdapter->QueryInterface(__uuidof(IDXGIAdapter), (void**)&adapter) == S_OK;

    void* dxvkAdapter = nullptr;
    if (adapterOk && !State::Instance().isRunningOnDXVK && adapter->QueryInterface(guid, &dxvkAdapter) == S_OK)
    {
        State::Instance().isRunningOnDXVK = dxvkAdapter != nullptr;
        ((IDXGIAdapter*)dxvkAdapter)->Release();

        // Temporary fix for Linux & DXVK
        if (State::Instance().isRunningOnDXVK || State::Instance().isRunningOnLinux)
            Config::Instance()->UseHQFont.set_volatile_value(false);
    }

    if (State::Instance().isRunningOnDXVK != dxvkStatus)
        LOG_INFO("IDXGIVkInteropDevice interface (DXVK) {0}", State::Instance().isRunningOnDXVK ? "detected" : "not detected");

    if (ptrGetDesc == nullptr && adapterOk)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc = (PFN_GetDesc)pVTable[8];

        DetourAttach(&(PVOID&)ptrGetDesc, detGetDesc);

        DetourTransactionCommit();
    }

    if (adapter != nullptr)
        adapter->Release();

    IDXGIAdapter1* adapter1 = nullptr;
    if (ptrGetDesc1 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter1), (void**)&adapter1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc1 = (PFN_GetDesc1)pVTable[10];

        DetourAttach(&(PVOID&)ptrGetDesc1, detGetDesc1);

        DetourTransactionCommit();
    }

    if (adapter1 != nullptr)
        adapter1->Release();

    IDXGIAdapter2* adapter2 = nullptr;
    if (ptrGetDesc2 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter2), (void**)&adapter2) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc2 = (PFN_GetDesc2)pVTable[11];

        DetourAttach(&(PVOID&)ptrGetDesc2, detGetDesc2);

        DetourTransactionCommit();
    }

    if (adapter2 != nullptr)
        adapter2->Release();

    IDXGIAdapter4* adapter4 = nullptr;
    if (ptrGetDesc3 == nullptr && unkAdapter->QueryInterface(__uuidof(IDXGIAdapter4), (void**)&adapter4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrGetDesc3 = (PFN_GetDesc3)pVTable[18];

        DetourAttach(&(PVOID&)ptrGetDesc3, detGetDesc3);

        DetourTransactionCommit();
    }

    if (adapter4 != nullptr)
        adapter4->Release();
}

void AttachToFactory(IUnknown* unkFactory)
{
    PVOID* pVTable = *(PVOID**)unkFactory;

    IDXGIFactory* factory;
    if (ptrEnumAdapters == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory), (void**)&factory) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters = (PFN_EnumAdapters)pVTable[7];

        DetourAttach(&(PVOID&)ptrEnumAdapters, detEnumAdapters);

        DetourTransactionCommit();

        factory->Release();
    }

    IDXGIFactory1* factory1;
    if (ptrEnumAdapters1 == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory1), (void**)&factory1) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapters1 = (PFN_EnumAdapters1)pVTable[12];

        DetourAttach(&(PVOID&)ptrEnumAdapters1, detEnumAdapters1);

        DetourTransactionCommit();

        factory1->Release();
    }

    IDXGIFactory4* factory4;
    if (ptrEnumAdapterByLuid == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory4), (void**)&factory4) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByLuid = (PFN_EnumAdapterByLuid)pVTable[26];

        DetourAttach(&(PVOID&)ptrEnumAdapterByLuid, detEnumAdapterByLuid);

        DetourTransactionCommit();

        factory4->Release();
    }

    IDXGIFactory6* factory6;
    if (ptrEnumAdapterByGpuPreference == nullptr && unkFactory->QueryInterface(__uuidof(IDXGIFactory6), (void**)&factory6) == S_OK)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        ptrEnumAdapterByGpuPreference = (PFN_EnumAdapterByGpuPreference)pVTable[29];

        DetourAttach(&(PVOID&)ptrEnumAdapterByGpuPreference, detEnumAdapterByGpuPreference);

        DetourTransactionCommit();

        factory6->Release();
    }
}

#pragma endregion 

#pragma region Shared methods

void _DllCanUnloadNow() { shared.DllCanUnloadNow(); }
void _DllGetClassObject() { shared.DllGetClassObject(); }
void _DllRegisterServer() { shared.DllRegisterServer(); }
void _DllUnregisterServer() { shared.DllUnregisterServer(); }
void _DebugSetMute() { shared.DebugSetMute(); }

#pragma endregion 

#pragma region WinMM Methods

void _CloseDriver() { winmm.CloseDriver(); }
void _DefDriverProc() { winmm.DefDriverProc(); }
void _DriverCallback() { winmm.DriverCallback(); }
void _DrvGetModuleHandle() { winmm.DrvGetModuleHandle(); }
void _GetDriverModuleHandle() { winmm.GetDriverModuleHandle(); }
void _NotifyCallbackData() { winmm.NotifyCallbackData(); }
void _OpenDriver() { winmm.OpenDriver(); }
void _PlaySound() { winmm.PlaySound(); }
void _PlaySoundA() { winmm.PlaySoundA(); }
void _PlaySoundW() { winmm.PlaySoundW(); }
void _SendDriverMessage() { winmm.SendDriverMessage(); }
void _WOW32DriverCallback() { winmm.WOW32DriverCallback(); }
void _WOW32ResolveMultiMediaHandle() { winmm.WOW32ResolveMultiMediaHandle(); }
void _WOWAppExit() { winmm.WOWAppExit(); }
void _aux32Message() { winmm.aux32Message(); }
void _auxGetDevCapsA() { winmm.auxGetDevCapsA(); }
void _auxGetDevCapsW() { winmm.auxGetDevCapsW(); }
void _auxGetNumDevs() { winmm.auxGetNumDevs(); }
void _auxGetVolume() { winmm.auxGetVolume(); }
void _auxOutMessage() { winmm.auxOutMessage(); }
void _auxSetVolume() { winmm.auxSetVolume(); }
void _joy32Message() { winmm.joy32Message(); }
void _joyConfigChanged() { winmm.joyConfigChanged(); }
void _joyGetDevCapsA() { winmm.joyGetDevCapsA(); }
void _joyGetDevCapsW() { winmm.joyGetDevCapsW(); }
void _joyGetNumDevs() { winmm.joyGetNumDevs(); }
void _joyGetPos() { winmm.joyGetPos(); }
void _joyGetPosEx() { winmm.joyGetPosEx(); }
void _joyGetThreshold() { winmm.joyGetThreshold(); }
void _joyReleaseCapture() { winmm.joyReleaseCapture(); }
void _joySetCapture() { winmm.joySetCapture(); }
void _joySetThreshold() { winmm.joySetThreshold(); }
void _mci32Message() { winmm.mci32Message(); }
void _mciDriverNotify() { winmm.mciDriverNotify(); }
void _mciDriverYield() { winmm.mciDriverYield(); }
void _mciExecute() { winmm.mciExecute(); }
void _mciFreeCommandResource() { winmm.mciFreeCommandResource(); }
void _mciGetCreatorTask() { winmm.mciGetCreatorTask(); }
void _mciGetDeviceIDA() { winmm.mciGetDeviceIDA(); }
void _mciGetDeviceIDFromElementIDA() { winmm.mciGetDeviceIDFromElementIDA(); }
void _mciGetDeviceIDFromElementIDW() { winmm.mciGetDeviceIDFromElementIDW(); }
void _mciGetDeviceIDW() { winmm.mciGetDeviceIDW(); }
void _mciGetDriverData() { winmm.mciGetDriverData(); }
void _mciGetErrorStringA() { winmm.mciGetErrorStringA(); }
void _mciGetErrorStringW() { winmm.mciGetErrorStringW(); }
void _mciGetYieldProc() { winmm.mciGetYieldProc(); }
void _mciLoadCommandResource() { winmm.mciLoadCommandResource(); }
void _mciSendCommandA() { winmm.mciSendCommandA(); }
void _mciSendCommandW() { winmm.mciSendCommandW(); }
void _mciSendStringA() { winmm.mciSendStringA(); }
void _mciSendStringW() { winmm.mciSendStringW(); }
void _mciSetDriverData() { winmm.mciSetDriverData(); }
void _mciSetYieldProc() { winmm.mciSetYieldProc(); }
void _mid32Message() { winmm.mid32Message(); }
void _midiConnect() { winmm.midiConnect(); }
void _midiDisconnect() { winmm.midiDisconnect(); }
void _midiInAddBuffer() { winmm.midiInAddBuffer(); }
void _midiInClose() { winmm.midiInClose(); }
void _midiInGetDevCapsA() { winmm.midiInGetDevCapsA(); }
void _midiInGetDevCapsW() { winmm.midiInGetDevCapsW(); }
void _midiInGetErrorTextA() { winmm.midiInGetErrorTextA(); }
void _midiInGetErrorTextW() { winmm.midiInGetErrorTextW(); }
void _midiInGetID() { winmm.midiInGetID(); }
void _midiInGetNumDevs() { winmm.midiInGetNumDevs(); }
void _midiInMessage() { winmm.midiInMessage(); }
void _midiInOpen() { winmm.midiInOpen(); }
void _midiInPrepareHeader() { winmm.midiInPrepareHeader(); }
void _midiInReset() { winmm.midiInReset(); }
void _midiInStart() { winmm.midiInStart(); }
void _midiInStop() { winmm.midiInStop(); }
void _midiInUnprepareHeader() { winmm.midiInUnprepareHeader(); }
void _midiOutCacheDrumPatches() { winmm.midiOutCacheDrumPatches(); }
void _midiOutCachePatches() { winmm.midiOutCachePatches(); }
void _midiOutClose() { winmm.midiOutClose(); }
void _midiOutGetDevCapsA() { winmm.midiOutGetDevCapsA(); }
void _midiOutGetDevCapsW() { winmm.midiOutGetDevCapsW(); }
void _midiOutGetErrorTextA() { winmm.midiOutGetErrorTextA(); }
void _midiOutGetErrorTextW() { winmm.midiOutGetErrorTextW(); }
void _midiOutGetID() { winmm.midiOutGetID(); }
void _midiOutGetNumDevs() { winmm.midiOutGetNumDevs(); }
void _midiOutGetVolume() { winmm.midiOutGetVolume(); }
void _midiOutLongMsg() { winmm.midiOutLongMsg(); }
void _midiOutMessage() { winmm.midiOutMessage(); }
void _midiOutOpen() { winmm.midiOutOpen(); }
void _midiOutPrepareHeader() { winmm.midiOutPrepareHeader(); }
void _midiOutReset() { winmm.midiOutReset(); }
void _midiOutSetVolume() { winmm.midiOutSetVolume(); }
void _midiOutShortMsg() { winmm.midiOutShortMsg(); }
void _midiOutUnprepareHeader() { winmm.midiOutUnprepareHeader(); }
void _midiStreamClose() { winmm.midiStreamClose(); }
void _midiStreamOpen() { winmm.midiStreamOpen(); }
void _midiStreamOut() { winmm.midiStreamOut(); }
void _midiStreamPause() { winmm.midiStreamPause(); }
void _midiStreamPosition() { winmm.midiStreamPosition(); }
void _midiStreamProperty() { winmm.midiStreamProperty(); }
void _midiStreamRestart() { winmm.midiStreamRestart(); }
void _midiStreamStop() { winmm.midiStreamStop(); }
void _mixerClose() { winmm.mixerClose(); }
void _mixerGetControlDetailsA() { winmm.mixerGetControlDetailsA(); }
void _mixerGetControlDetailsW() { winmm.mixerGetControlDetailsW(); }
void _mixerGetDevCapsA() { winmm.mixerGetDevCapsA(); }
void _mixerGetDevCapsW() { winmm.mixerGetDevCapsW(); }
void _mixerGetID() { winmm.mixerGetID(); }
void _mixerGetLineControlsA() { winmm.mixerGetLineControlsA(); }
void _mixerGetLineControlsW() { winmm.mixerGetLineControlsW(); }
void _mixerGetLineInfoA() { winmm.mixerGetLineInfoA(); }
void _mixerGetLineInfoW() { winmm.mixerGetLineInfoW(); }
void _mixerGetNumDevs() { winmm.mixerGetNumDevs(); }
void _mixerMessage() { winmm.mixerMessage(); }
void _mixerOpen() { winmm.mixerOpen(); }
void _mixerSetControlDetails() { winmm.mixerSetControlDetails(); }
void _mmDrvInstall() { winmm.mmDrvInstall(); }
void _mmGetCurrentTask() { winmm.mmGetCurrentTask(); }
void _mmTaskBlock() { winmm.mmTaskBlock(); }
void _mmTaskCreate() { winmm.mmTaskCreate(); }
void _mmTaskSignal() { winmm.mmTaskSignal(); }
void _mmTaskYield() { winmm.mmTaskYield(); }
void _mmioAdvance() { winmm.mmioAdvance(); }
void _mmioAscend() { winmm.mmioAscend(); }
void _mmioClose() { winmm.mmioClose(); }
void _mmioCreateChunk() { winmm.mmioCreateChunk(); }
void _mmioDescend() { winmm.mmioDescend(); }
void _mmioFlush() { winmm.mmioFlush(); }
void _mmioGetInfo() { winmm.mmioGetInfo(); }
void _mmioInstallIOProcA() { winmm.mmioInstallIOProcA(); }
void _mmioInstallIOProcW() { winmm.mmioInstallIOProcW(); }
void _mmioOpenA() { winmm.mmioOpenA(); }
void _mmioOpenW() { winmm.mmioOpenW(); }
void _mmioRead() { winmm.mmioRead(); }
void _mmioRenameA() { winmm.mmioRenameA(); }
void _mmioRenameW() { winmm.mmioRenameW(); }
void _mmioSeek() { winmm.mmioSeek(); }
void _mmioSendMessage() { winmm.mmioSendMessage(); }
void _mmioSetBuffer() { winmm.mmioSetBuffer(); }
void _mmioSetInfo() { winmm.mmioSetInfo(); }
void _mmioStringToFOURCCA() { winmm.mmioStringToFOURCCA(); }
void _mmioStringToFOURCCW() { winmm.mmioStringToFOURCCW(); }
void _mmioWrite() { winmm.mmioWrite(); }
void _mmsystemGetVersion() { winmm.mmsystemGetVersion(); }
void _mod32Message() { winmm.mod32Message(); }
void _mxd32Message() { winmm.mxd32Message(); }
void _sndPlaySoundA() { winmm.sndPlaySoundA(); }
void _sndPlaySoundW() { winmm.sndPlaySoundW(); }
void _tid32Message() { winmm.tid32Message(); }
void _timeBeginPeriod() { winmm.timeBeginPeriod(); }
void _timeEndPeriod() { winmm.timeEndPeriod(); }
void _timeGetDevCaps() { winmm.timeGetDevCaps(); }
void _timeGetSystemTime() { winmm.timeGetSystemTime(); }
void _timeGetTime() { winmm.timeGetTime(); }
void _timeKillEvent() { winmm.timeKillEvent(); }
void _timeSetEvent() { winmm.timeSetEvent(); }
void _waveInAddBuffer() { winmm.waveInAddBuffer(); }
void _waveInClose() { winmm.waveInClose(); }
void _waveInGetDevCapsA() { winmm.waveInGetDevCapsA(); }
void _waveInGetDevCapsW() { winmm.waveInGetDevCapsW(); }
void _waveInGetErrorTextA() { winmm.waveInGetErrorTextA(); }
void _waveInGetErrorTextW() { winmm.waveInGetErrorTextW(); }
void _waveInGetID() { winmm.waveInGetID(); }
void _waveInGetNumDevs() { winmm.waveInGetNumDevs(); }
void _waveInGetPosition() { winmm.waveInGetPosition(); }
void _waveInMessage() { winmm.waveInMessage(); }
void _waveInOpen() { winmm.waveInOpen(); }
void _waveInPrepareHeader() { winmm.waveInPrepareHeader(); }
void _waveInReset() { winmm.waveInReset(); }
void _waveInStart() { winmm.waveInStart(); }
void _waveInStop() { winmm.waveInStop(); }
void _waveInUnprepareHeader() { winmm.waveInUnprepareHeader(); }
void _waveOutBreakLoop() { winmm.waveOutBreakLoop(); }
void _waveOutClose() { winmm.waveOutClose(); }
void _waveOutGetDevCapsA() { winmm.waveOutGetDevCapsA(); }
void _waveOutGetDevCapsW() { winmm.waveOutGetDevCapsW(); }
void _waveOutGetErrorTextA() { winmm.waveOutGetErrorTextA(); }
void _waveOutGetErrorTextW() { winmm.waveOutGetErrorTextW(); }
void _waveOutGetID() { winmm.waveOutGetID(); }
void _waveOutGetNumDevs() { winmm.waveOutGetNumDevs(); }
void _waveOutGetPitch() { winmm.waveOutGetPitch(); }
void _waveOutGetPlaybackRate() { winmm.waveOutGetPlaybackRate(); }
void _waveOutGetPosition() { winmm.waveOutGetPosition(); }
void _waveOutGetVolume() { winmm.waveOutGetVolume(); }
void _waveOutMessage() { winmm.waveOutMessage(); }
void _waveOutOpen() { winmm.waveOutOpen(); }
void _waveOutPause() { winmm.waveOutPause(); }
void _waveOutPrepareHeader() { winmm.waveOutPrepareHeader(); }
void _waveOutReset() { winmm.waveOutReset(); }
void _waveOutRestart() { winmm.waveOutRestart(); }
void _waveOutSetPitch() { winmm.waveOutSetPitch(); }
void _waveOutSetPlaybackRate() { winmm.waveOutSetPlaybackRate(); }
void _waveOutSetVolume() { winmm.waveOutSetVolume(); }
void _waveOutUnprepareHeader() { winmm.waveOutUnprepareHeader(); }
void _waveOutWrite() { winmm.waveOutWrite(); }
void _wid32Message() { winmm.wid32Message(); }
void _wod32Message() { winmm.wod32Message(); }

#pragma endregion 

#pragma region WinHTTP methods

void _Private1() { winhttp.Private1(); }
void _SvchostPushServiceGlobals() { winhttp.SvchostPushServiceGlobals(); }
void _WinHttpAddRequestHeaders() { winhttp.WinHttpAddRequestHeaders(); }
void _WinHttpAddRequestHeadersEx() { winhttp.WinHttpAddRequestHeadersEx(); }
void _WinHttpAutoProxySvcMain() { winhttp.WinHttpAutoProxySvcMain(); }
void _WinHttpCheckPlatform() { winhttp.WinHttpCheckPlatform(); }
void _WinHttpCloseHandle() { winhttp.WinHttpCloseHandle(); }
void _WinHttpConnect() { winhttp.WinHttpConnect(); }
void _WinHttpConnectionDeletePolicyEntries() { winhttp.WinHttpConnectionDeletePolicyEntries(); }
void _WinHttpConnectionDeleteProxyInfo() { winhttp.WinHttpConnectionDeleteProxyInfo(); }
void _WinHttpConnectionFreeNameList() { winhttp.WinHttpConnectionFreeNameList(); }
void _WinHttpConnectionFreeProxyInfo() { winhttp.WinHttpConnectionFreeProxyInfo(); }
void _WinHttpConnectionFreeProxyList() { winhttp.WinHttpConnectionFreeProxyList(); }
void _WinHttpConnectionGetNameList() { winhttp.WinHttpConnectionGetNameList(); }
void _WinHttpConnectionGetProxyInfo() { winhttp.WinHttpConnectionGetProxyInfo(); }
void _WinHttpConnectionGetProxyList() { winhttp.WinHttpConnectionGetProxyList(); }
void _WinHttpConnectionOnlyConvert() { winhttp.WinHttpConnectionOnlyConvert(); }
void _WinHttpConnectionOnlyReceive() { winhttp.WinHttpConnectionOnlyReceive(); }
void _WinHttpConnectionOnlySend() { winhttp.WinHttpConnectionOnlySend(); }
void _WinHttpConnectionSetPolicyEntries() { winhttp.WinHttpConnectionSetPolicyEntries(); }
void _WinHttpConnectionSetProxyInfo() { winhttp.WinHttpConnectionSetProxyInfo(); }
void _WinHttpConnectionUpdateIfIndexTable() { winhttp.WinHttpConnectionUpdateIfIndexTable(); }
void _WinHttpCrackUrl() { winhttp.WinHttpCrackUrl(); }
void _WinHttpCreateProxyResolver() { winhttp.WinHttpCreateProxyResolver(); }
void _WinHttpCreateUrl() { winhttp.WinHttpCreateUrl(); }
void _WinHttpDetectAutoProxyConfigUrl() { winhttp.WinHttpDetectAutoProxyConfigUrl(); }
void _WinHttpFreeProxyResult() { winhttp.WinHttpFreeProxyResult(); }
void _WinHttpFreeProxyResultEx() { winhttp.WinHttpFreeProxyResultEx(); }
void _WinHttpFreeProxySettings() { winhttp.WinHttpFreeProxySettings(); }
void _WinHttpFreeProxySettingsEx() { winhttp.WinHttpFreeProxySettingsEx(); }
void _WinHttpFreeQueryConnectionGroupResult() { winhttp.WinHttpFreeQueryConnectionGroupResult(); }
void _WinHttpGetDefaultProxyConfiguration() { winhttp.WinHttpGetDefaultProxyConfiguration(); }
void _WinHttpGetIEProxyConfigForCurrentUser() { winhttp.WinHttpGetIEProxyConfigForCurrentUser(); }
void _WinHttpGetProxyForUrl() { winhttp.WinHttpGetProxyForUrl(); }
void _WinHttpGetProxyForUrlEx() { winhttp.WinHttpGetProxyForUrlEx(); }
void _WinHttpGetProxyForUrlEx2() { winhttp.WinHttpGetProxyForUrlEx2(); }
void _WinHttpGetProxyForUrlHvsi() { winhttp.WinHttpGetProxyForUrlHvsi(); }
void _WinHttpGetProxyResult() { winhttp.WinHttpGetProxyResult(); }
void _WinHttpGetProxyResultEx() { winhttp.WinHttpGetProxyResultEx(); }
void _WinHttpGetProxySettingsEx() { winhttp.WinHttpGetProxySettingsEx(); }
void _WinHttpGetProxySettingsResultEx() { winhttp.WinHttpGetProxySettingsResultEx(); }
void _WinHttpGetProxySettingsVersion() { winhttp.WinHttpGetProxySettingsVersion(); }
void _WinHttpGetTunnelSocket() { winhttp.WinHttpGetTunnelSocket(); }
void _WinHttpOpen() { winhttp.WinHttpOpen(); }
void _WinHttpOpenRequest() { winhttp.WinHttpOpenRequest(); }
void _WinHttpPacJsWorkerMain() { winhttp.WinHttpPacJsWorkerMain(); }
void _WinHttpProbeConnectivity() { winhttp.WinHttpProbeConnectivity(); }
void _WinHttpQueryAuthSchemes() { winhttp.WinHttpQueryAuthSchemes(); }
void _WinHttpQueryConnectionGroup() { winhttp.WinHttpQueryConnectionGroup(); }
void _WinHttpQueryDataAvailable() { winhttp.WinHttpQueryDataAvailable(); }
void _WinHttpQueryHeaders() { winhttp.WinHttpQueryHeaders(); }
void _WinHttpQueryHeadersEx() { winhttp.WinHttpQueryHeadersEx(); }
void _WinHttpQueryOption() { winhttp.WinHttpQueryOption(); }
void _WinHttpReadData() { winhttp.WinHttpReadData(); }
void _WinHttpReadDataEx() { winhttp.WinHttpReadDataEx(); }
void _WinHttpReadProxySettings() { winhttp.WinHttpReadProxySettings(); }
void _WinHttpReadProxySettingsHvsi() { winhttp.WinHttpReadProxySettingsHvsi(); }
void _WinHttpReceiveResponse() { winhttp.WinHttpReceiveResponse(); }
void _WinHttpRegisterProxyChangeNotification() { winhttp.WinHttpRegisterProxyChangeNotification(); }
void _WinHttpResetAutoProxy() { winhttp.WinHttpResetAutoProxy(); }
void _WinHttpSaveProxyCredentials() { winhttp.WinHttpSaveProxyCredentials(); }
void _WinHttpSendRequest() { winhttp.WinHttpSendRequest(); }
void _WinHttpSetCredentials() { winhttp.WinHttpSetCredentials(); }
void _WinHttpSetDefaultProxyConfiguration() { winhttp.WinHttpSetDefaultProxyConfiguration(); }
void _WinHttpSetOption() { winhttp.WinHttpSetOption(); }
void _WinHttpSetProxySettingsPerUser() { winhttp.WinHttpSetProxySettingsPerUser(); }
void _WinHttpSetSecureLegacyServersAppCompat() { winhttp.WinHttpSetSecureLegacyServersAppCompat(); }
void _WinHttpSetStatusCallback() { winhttp.WinHttpSetStatusCallback(); }
void _WinHttpSetTimeouts() { winhttp.WinHttpSetTimeouts(); }
void _WinHttpTimeFromSystemTime() { winhttp.WinHttpTimeFromSystemTime(); }
void _WinHttpTimeToSystemTime() { winhttp.WinHttpTimeToSystemTime(); }
void _WinHttpUnregisterProxyChangeNotification() { winhttp.WinHttpUnregisterProxyChangeNotification(); }
void _WinHttpWebSocketClose() { winhttp.WinHttpWebSocketClose(); }
void _WinHttpWebSocketCompleteUpgrade() { winhttp.WinHttpWebSocketCompleteUpgrade(); }
void _WinHttpWebSocketQueryCloseStatus() { winhttp.WinHttpWebSocketQueryCloseStatus(); }
void _WinHttpWebSocketReceive() { winhttp.WinHttpWebSocketReceive(); }
void _WinHttpWebSocketSend() { winhttp.WinHttpWebSocketSend(); }
void _WinHttpWebSocketShutdown() { winhttp.WinHttpWebSocketShutdown(); }
void _WinHttpWriteData() { winhttp.WinHttpWriteData(); }
void _WinHttpWriteProxySettings() { winhttp.WinHttpWriteProxySettings(); }

#pragma endregion 

#pragma region WinInet methods

void _AppCacheCheckManifest() { wininet.AppCacheCheckManifest(); }
void _AppCacheCloseHandle() { wininet.AppCacheCloseHandle(); }
void _AppCacheCreateAndCommitFile() { wininet.AppCacheCreateAndCommitFile(); }
void _AppCacheDeleteGroup() { wininet.AppCacheDeleteGroup(); }
void _AppCacheDeleteIEGroup() { wininet.AppCacheDeleteIEGroup(); }
void _AppCacheDuplicateHandle() { wininet.AppCacheDuplicateHandle(); }
void _AppCacheFinalize() { wininet.AppCacheFinalize(); }
void _AppCacheFreeDownloadList() { wininet.AppCacheFreeDownloadList(); }
void _AppCacheFreeGroupList() { wininet.AppCacheFreeGroupList(); }
void _AppCacheFreeIESpace() { wininet.AppCacheFreeIESpace(); }
void _AppCacheFreeSpace() { wininet.AppCacheFreeSpace(); }
void _AppCacheGetDownloadList() { wininet.AppCacheGetDownloadList(); }
void _AppCacheGetFallbackUrl() { wininet.AppCacheGetFallbackUrl(); }
void _AppCacheGetGroupList() { wininet.AppCacheGetGroupList(); }
void _AppCacheGetIEGroupList() { wininet.AppCacheGetIEGroupList(); }
void _AppCacheGetInfo() { wininet.AppCacheGetInfo(); }
void _AppCacheGetManifestUrl() { wininet.AppCacheGetManifestUrl(); }
void _AppCacheLookup() { wininet.AppCacheLookup(); }
void _CommitUrlCacheEntryA() { wininet.CommitUrlCacheEntryA(); }
void _CommitUrlCacheEntryBinaryBlob() { wininet.CommitUrlCacheEntryBinaryBlob(); }
void _CommitUrlCacheEntryW() { wininet.CommitUrlCacheEntryW(); }
void _CreateMD5SSOHash() { wininet.CreateMD5SSOHash(); }
void _CreateUrlCacheContainerA() { wininet.CreateUrlCacheContainerA(); }
void _CreateUrlCacheContainerW() { wininet.CreateUrlCacheContainerW(); }
void _CreateUrlCacheEntryA() { wininet.CreateUrlCacheEntryA(); }
void _CreateUrlCacheEntryExW() { wininet.CreateUrlCacheEntryExW(); }
void _CreateUrlCacheEntryW() { wininet.CreateUrlCacheEntryW(); }
void _CreateUrlCacheGroup() { wininet.CreateUrlCacheGroup(); }
void _DeleteIE3Cache() { wininet.DeleteIE3Cache(); }
void _DeleteUrlCacheContainerA() { wininet.DeleteUrlCacheContainerA(); }
void _DeleteUrlCacheContainerW() { wininet.DeleteUrlCacheContainerW(); }
void _DeleteUrlCacheEntry() { wininet.DeleteUrlCacheEntry(); }
void _DeleteUrlCacheEntryA() { wininet.DeleteUrlCacheEntryA(); }
void _DeleteUrlCacheEntryW() { wininet.DeleteUrlCacheEntryW(); }
void _DeleteUrlCacheGroup() { wininet.DeleteUrlCacheGroup(); }
void _DeleteWpadCacheForNetworks() { wininet.DeleteWpadCacheForNetworks(); }
void _DetectAutoProxyUrl() { wininet.DetectAutoProxyUrl(); }
void _DispatchAPICall() { wininet.DispatchAPICall(); }
void _DllInstall() { wininet.DllInstall(); }
void _FindCloseUrlCache() { wininet.FindCloseUrlCache(); }
void _FindFirstUrlCacheContainerA() { wininet.FindFirstUrlCacheContainerA(); }
void _FindFirstUrlCacheContainerW() { wininet.FindFirstUrlCacheContainerW(); }
void _FindFirstUrlCacheEntryA() { wininet.FindFirstUrlCacheEntryA(); }
void _FindFirstUrlCacheEntryExA() { wininet.FindFirstUrlCacheEntryExA(); }
void _FindFirstUrlCacheEntryExW() { wininet.FindFirstUrlCacheEntryExW(); }
void _FindFirstUrlCacheEntryW() { wininet.FindFirstUrlCacheEntryW(); }
void _FindFirstUrlCacheGroup() { wininet.FindFirstUrlCacheGroup(); }
void _FindNextUrlCacheContainerA() { wininet.FindNextUrlCacheContainerA(); }
void _FindNextUrlCacheContainerW() { wininet.FindNextUrlCacheContainerW(); }
void _FindNextUrlCacheEntryA() { wininet.FindNextUrlCacheEntryA(); }
void _FindNextUrlCacheEntryExA() { wininet.FindNextUrlCacheEntryExA(); }
void _FindNextUrlCacheEntryExW() { wininet.FindNextUrlCacheEntryExW(); }
void _FindNextUrlCacheEntryW() { wininet.FindNextUrlCacheEntryW(); }
void _FindNextUrlCacheGroup() { wininet.FindNextUrlCacheGroup(); }
void _ForceNexusLookup() { wininet.ForceNexusLookup(); }
void _ForceNexusLookupExW() { wininet.ForceNexusLookupExW(); }
void _FreeUrlCacheSpaceA() { wininet.FreeUrlCacheSpaceA(); }
void _FreeUrlCacheSpaceW() { wininet.FreeUrlCacheSpaceW(); }
void _FtpCommandA() { wininet.FtpCommandA(); }
void _FtpCommandW() { wininet.FtpCommandW(); }
void _FtpCreateDirectoryA() { wininet.FtpCreateDirectoryA(); }
void _FtpCreateDirectoryW() { wininet.FtpCreateDirectoryW(); }
void _FtpDeleteFileA() { wininet.FtpDeleteFileA(); }
void _FtpDeleteFileW() { wininet.FtpDeleteFileW(); }
void _FtpFindFirstFileA() { wininet.FtpFindFirstFileA(); }
void _FtpFindFirstFileW() { wininet.FtpFindFirstFileW(); }
void _FtpGetCurrentDirectoryA() { wininet.FtpGetCurrentDirectoryA(); }
void _FtpGetCurrentDirectoryW() { wininet.FtpGetCurrentDirectoryW(); }
void _FtpGetFileA() { wininet.FtpGetFileA(); }
void _FtpGetFileEx() { wininet.FtpGetFileEx(); }
void _FtpGetFileSize() { wininet.FtpGetFileSize(); }
void _FtpGetFileW() { wininet.FtpGetFileW(); }
void _FtpOpenFileA() { wininet.FtpOpenFileA(); }
void _FtpOpenFileW() { wininet.FtpOpenFileW(); }
void _FtpPutFileA() { wininet.FtpPutFileA(); }
void _FtpPutFileEx() { wininet.FtpPutFileEx(); }
void _FtpPutFileW() { wininet.FtpPutFileW(); }
void _FtpRemoveDirectoryA() { wininet.FtpRemoveDirectoryA(); }
void _FtpRemoveDirectoryW() { wininet.FtpRemoveDirectoryW(); }
void _FtpRenameFileA() { wininet.FtpRenameFileA(); }
void _FtpRenameFileW() { wininet.FtpRenameFileW(); }
void _FtpSetCurrentDirectoryA() { wininet.FtpSetCurrentDirectoryA(); }
void _FtpSetCurrentDirectoryW() { wininet.FtpSetCurrentDirectoryW(); }
void __GetFileExtensionFromUrl() { wininet._GetFileExtensionFromUrl(); }
void _GetProxyDllInfo() { wininet.GetProxyDllInfo(); }
void _GetUrlCacheConfigInfoA() { wininet.GetUrlCacheConfigInfoA(); }
void _GetUrlCacheConfigInfoW() { wininet.GetUrlCacheConfigInfoW(); }
void _GetUrlCacheEntryBinaryBlob() { wininet.GetUrlCacheEntryBinaryBlob(); }
void _GetUrlCacheEntryInfoA() { wininet.GetUrlCacheEntryInfoA(); }
void _GetUrlCacheEntryInfoExA() { wininet.GetUrlCacheEntryInfoExA(); }
void _GetUrlCacheEntryInfoExW() { wininet.GetUrlCacheEntryInfoExW(); }
void _GetUrlCacheEntryInfoW() { wininet.GetUrlCacheEntryInfoW(); }
void _GetUrlCacheGroupAttributeA() { wininet.GetUrlCacheGroupAttributeA(); }
void _GetUrlCacheGroupAttributeW() { wininet.GetUrlCacheGroupAttributeW(); }
void _GetUrlCacheHeaderData() { wininet.GetUrlCacheHeaderData(); }
void _GopherCreateLocatorA() { wininet.GopherCreateLocatorA(); }
void _GopherCreateLocatorW() { wininet.GopherCreateLocatorW(); }
void _GopherFindFirstFileA() { wininet.GopherFindFirstFileA(); }
void _GopherFindFirstFileW() { wininet.GopherFindFirstFileW(); }
void _GopherGetAttributeA() { wininet.GopherGetAttributeA(); }
void _GopherGetAttributeW() { wininet.GopherGetAttributeW(); }
void _GopherGetLocatorTypeA() { wininet.GopherGetLocatorTypeA(); }
void _GopherGetLocatorTypeW() { wininet.GopherGetLocatorTypeW(); }
void _GopherOpenFileA() { wininet.GopherOpenFileA(); }
void _GopherOpenFileW() { wininet.GopherOpenFileW(); }
void _HttpAddRequestHeadersA() { wininet.HttpAddRequestHeadersA(); }
void _HttpAddRequestHeadersW() { wininet.HttpAddRequestHeadersW(); }
void _HttpCheckDavCompliance() { wininet.HttpCheckDavCompliance(); }
void _HttpCloseDependencyHandle() { wininet.HttpCloseDependencyHandle(); }
void _HttpDuplicateDependencyHandle() { wininet.HttpDuplicateDependencyHandle(); }
void _HttpEndRequestA() { wininet.HttpEndRequestA(); }
void _HttpEndRequestW() { wininet.HttpEndRequestW(); }
void _HttpGetServerCredentials() { wininet.HttpGetServerCredentials(); }
void _HttpGetTunnelSocket() { wininet.HttpGetTunnelSocket(); }
void _HttpIsHostHstsEnabled() { wininet.HttpIsHostHstsEnabled(); }
void _HttpOpenDependencyHandle() { wininet.HttpOpenDependencyHandle(); }
void _HttpOpenRequestA() { wininet.HttpOpenRequestA(); }
void _HttpOpenRequestW() { wininet.HttpOpenRequestW(); }
void _HttpPushClose() { wininet.HttpPushClose(); }
void _HttpPushEnable() { wininet.HttpPushEnable(); }
void _HttpPushWait() { wininet.HttpPushWait(); }
void _HttpQueryInfoA() { wininet.HttpQueryInfoA(); }
void _HttpQueryInfoW() { wininet.HttpQueryInfoW(); }
void _HttpSendRequestA() { wininet.HttpSendRequestA(); }
void _HttpSendRequestExA() { wininet.HttpSendRequestExA(); }
void _HttpSendRequestExW() { wininet.HttpSendRequestExW(); }
void _HttpSendRequestW() { wininet.HttpSendRequestW(); }
void _HttpWebSocketClose() { wininet.HttpWebSocketClose(); }
void _HttpWebSocketCompleteUpgrade() { wininet.HttpWebSocketCompleteUpgrade(); }
void _HttpWebSocketQueryCloseStatus() { wininet.HttpWebSocketQueryCloseStatus(); }
void _HttpWebSocketReceive() { wininet.HttpWebSocketReceive(); }
void _HttpWebSocketSend() { wininet.HttpWebSocketSend(); }
void _HttpWebSocketShutdown() { wininet.HttpWebSocketShutdown(); }
void _IncrementUrlCacheHeaderData() { wininet.IncrementUrlCacheHeaderData(); }
void _InternetAlgIdToStringA() { wininet.InternetAlgIdToStringA(); }
void _InternetAlgIdToStringW() { wininet.InternetAlgIdToStringW(); }
void _InternetAttemptConnect() { wininet.InternetAttemptConnect(); }
void _InternetAutodial() { wininet.InternetAutodial(); }
void _InternetAutodialCallback() { wininet.InternetAutodialCallback(); }
void _InternetAutodialHangup() { wininet.InternetAutodialHangup(); }
void _InternetCanonicalizeUrlA() { wininet.InternetCanonicalizeUrlA(); }
void _InternetCanonicalizeUrlW() { wininet.InternetCanonicalizeUrlW(); }
void _InternetCheckConnectionA() { wininet.InternetCheckConnectionA(); }
void _InternetCheckConnectionW() { wininet.InternetCheckConnectionW(); }
void _InternetClearAllPerSiteCookieDecisions() { wininet.InternetClearAllPerSiteCookieDecisions(); }
void _InternetCloseHandle() { wininet.InternetCloseHandle(); }
void _InternetCombineUrlA() { wininet.InternetCombineUrlA(); }
void _InternetCombineUrlW() { wininet.InternetCombineUrlW(); }
void _InternetConfirmZoneCrossing() { wininet.InternetConfirmZoneCrossing(); }
void _InternetConfirmZoneCrossingA() { wininet.InternetConfirmZoneCrossingA(); }
void _InternetConfirmZoneCrossingW() { wininet.InternetConfirmZoneCrossingW(); }
void _InternetConnectA() { wininet.InternetConnectA(); }
void _InternetConnectW() { wininet.InternetConnectW(); }
void _InternetConvertUrlFromWireToWideChar() { wininet.InternetConvertUrlFromWireToWideChar(); }
void _InternetCrackUrlA() { wininet.InternetCrackUrlA(); }
void _InternetCrackUrlW() { wininet.InternetCrackUrlW(); }
void _InternetCreateUrlA() { wininet.InternetCreateUrlA(); }
void _InternetCreateUrlW() { wininet.InternetCreateUrlW(); }
void _InternetDial() { wininet.InternetDial(); }
void _InternetDialA() { wininet.InternetDialA(); }
void _InternetDialW() { wininet.InternetDialW(); }
void _InternetEnumPerSiteCookieDecisionA() { wininet.InternetEnumPerSiteCookieDecisionA(); }
void _InternetEnumPerSiteCookieDecisionW() { wininet.InternetEnumPerSiteCookieDecisionW(); }
void _InternetErrorDlg() { wininet.InternetErrorDlg(); }
void _InternetFindNextFileA() { wininet.InternetFindNextFileA(); }
void _InternetFindNextFileW() { wininet.InternetFindNextFileW(); }
void _InternetFortezzaCommand() { wininet.InternetFortezzaCommand(); }
void _InternetFreeCookies() { wininet.InternetFreeCookies(); }
void _InternetFreeProxyInfoList() { wininet.InternetFreeProxyInfoList(); }
void _InternetGetCertByURL() { wininet.InternetGetCertByURL(); }
void _InternetGetCertByURLA() { wininet.InternetGetCertByURLA(); }
void _InternetGetConnectedState() { wininet.InternetGetConnectedState(); }
void _InternetGetConnectedStateEx() { wininet.InternetGetConnectedStateEx(); }
void _InternetGetConnectedStateExA() { wininet.InternetGetConnectedStateExA(); }
void _InternetGetConnectedStateExW() { wininet.InternetGetConnectedStateExW(); }
void _InternetGetCookieA() { wininet.InternetGetCookieA(); }
void _InternetGetCookieEx2() { wininet.InternetGetCookieEx2(); }
void _InternetGetCookieExA() { wininet.InternetGetCookieExA(); }
void _InternetGetCookieExW() { wininet.InternetGetCookieExW(); }
void _InternetGetCookieW() { wininet.InternetGetCookieW(); }
void _InternetGetLastResponseInfoA() { wininet.InternetGetLastResponseInfoA(); }
void _InternetGetLastResponseInfoW() { wininet.InternetGetLastResponseInfoW(); }
void _InternetGetPerSiteCookieDecisionA() { wininet.InternetGetPerSiteCookieDecisionA(); }
void _InternetGetPerSiteCookieDecisionW() { wininet.InternetGetPerSiteCookieDecisionW(); }
void _InternetGetProxyForUrl() { wininet.InternetGetProxyForUrl(); }
void _InternetGetSecurityInfoByURL() { wininet.InternetGetSecurityInfoByURL(); }
void _InternetGetSecurityInfoByURLA() { wininet.InternetGetSecurityInfoByURLA(); }
void _InternetGetSecurityInfoByURLW() { wininet.InternetGetSecurityInfoByURLW(); }
void _InternetGoOnline() { wininet.InternetGoOnline(); }
void _InternetGoOnlineA() { wininet.InternetGoOnlineA(); }
void _InternetGoOnlineW() { wininet.InternetGoOnlineW(); }
void _InternetHangUp() { wininet.InternetHangUp(); }
void _InternetInitializeAutoProxyDll() { wininet.InternetInitializeAutoProxyDll(); }
void _InternetLockRequestFile() { wininet.InternetLockRequestFile(); }
void _InternetOpenA() { wininet.InternetOpenA(); }
void _InternetOpenUrlA() { wininet.InternetOpenUrlA(); }
void _InternetOpenUrlW() { wininet.InternetOpenUrlW(); }
void _InternetOpenW() { wininet.InternetOpenW(); }
void _InternetQueryDataAvailable() { wininet.InternetQueryDataAvailable(); }
void _InternetQueryFortezzaStatus() { wininet.InternetQueryFortezzaStatus(); }
void _InternetQueryOptionA() { wininet.InternetQueryOptionA(); }
void _InternetQueryOptionW() { wininet.InternetQueryOptionW(); }
void _InternetReadFile() { wininet.InternetReadFile(); }
void _InternetReadFileExA() { wininet.InternetReadFileExA(); }
void _InternetReadFileExW() { wininet.InternetReadFileExW(); }
void _InternetSecurityProtocolToStringA() { wininet.InternetSecurityProtocolToStringA(); }
void _InternetSecurityProtocolToStringW() { wininet.InternetSecurityProtocolToStringW(); }
void _InternetSetCookieA() { wininet.InternetSetCookieA(); }
void _InternetSetCookieEx2() { wininet.InternetSetCookieEx2(); }
void _InternetSetCookieExA() { wininet.InternetSetCookieExA(); }
void _InternetSetCookieExW() { wininet.InternetSetCookieExW(); }
void _InternetSetCookieW() { wininet.InternetSetCookieW(); }
void _InternetSetDialState() { wininet.InternetSetDialState(); }
void _InternetSetDialStateA() { wininet.InternetSetDialStateA(); }
void _InternetSetDialStateW() { wininet.InternetSetDialStateW(); }
void _InternetSetFilePointer() { wininet.InternetSetFilePointer(); }
void _InternetSetOptionA() { wininet.InternetSetOptionA(); }
void _InternetSetOptionExA() { wininet.InternetSetOptionExA(); }
void _InternetSetOptionExW() { wininet.InternetSetOptionExW(); }
void _InternetSetOptionW() { wininet.InternetSetOptionW(); }
void _InternetSetPerSiteCookieDecisionA() { wininet.InternetSetPerSiteCookieDecisionA(); }
void _InternetSetPerSiteCookieDecisionW() { wininet.InternetSetPerSiteCookieDecisionW(); }
void _InternetSetStatusCallback() { wininet.InternetSetStatusCallback(); }
void _InternetSetStatusCallbackA() { wininet.InternetSetStatusCallbackA(); }
void _InternetSetStatusCallbackW() { wininet.InternetSetStatusCallbackW(); }
void _InternetShowSecurityInfoByURL() { wininet.InternetShowSecurityInfoByURL(); }
void _InternetShowSecurityInfoByURLA() { wininet.InternetShowSecurityInfoByURLA(); }
void _InternetShowSecurityInfoByURLW() { wininet.InternetShowSecurityInfoByURLW(); }
void _InternetTimeFromSystemTime() { wininet.InternetTimeFromSystemTime(); }
void _InternetTimeFromSystemTimeA() { wininet.InternetTimeFromSystemTimeA(); }
void _InternetTimeFromSystemTimeW() { wininet.InternetTimeFromSystemTimeW(); }
void _InternetTimeToSystemTime() { wininet.InternetTimeToSystemTime(); }
void _InternetTimeToSystemTimeA() { wininet.InternetTimeToSystemTimeA(); }
void _InternetTimeToSystemTimeW() { wininet.InternetTimeToSystemTimeW(); }
void _InternetUnlockRequestFile() { wininet.InternetUnlockRequestFile(); }
void _InternetWriteFile() { wininet.InternetWriteFile(); }
void _InternetWriteFileExA() { wininet.InternetWriteFileExA(); }
void _InternetWriteFileExW() { wininet.InternetWriteFileExW(); }
void _IsHostInProxyBypassList() { wininet.IsHostInProxyBypassList(); }
void _IsUrlCacheEntryExpiredA() { wininet.IsUrlCacheEntryExpiredA(); }
void _IsUrlCacheEntryExpiredW() { wininet.IsUrlCacheEntryExpiredW(); }
void _LoadUrlCacheContent() { wininet.LoadUrlCacheContent(); }
void _ParseX509EncodedCertificateForListBoxEntry() { wininet.ParseX509EncodedCertificateForListBoxEntry(); }
void _PrivacyGetZonePreferenceW() { wininet.PrivacyGetZonePreferenceW(); }
void _PrivacySetZonePreferenceW() { wininet.PrivacySetZonePreferenceW(); }
void _ReadUrlCacheEntryStream() { wininet.ReadUrlCacheEntryStream(); }
void _ReadUrlCacheEntryStreamEx() { wininet.ReadUrlCacheEntryStreamEx(); }
void _RegisterUrlCacheNotification() { wininet.RegisterUrlCacheNotification(); }
void _ResumeSuspendedDownload() { wininet.ResumeSuspendedDownload(); }
void _RetrieveUrlCacheEntryFileA() { wininet.RetrieveUrlCacheEntryFileA(); }
void _RetrieveUrlCacheEntryFileW() { wininet.RetrieveUrlCacheEntryFileW(); }
void _RetrieveUrlCacheEntryStreamA() { wininet.RetrieveUrlCacheEntryStreamA(); }
void _RetrieveUrlCacheEntryStreamW() { wininet.RetrieveUrlCacheEntryStreamW(); }
void _RunOnceUrlCache() { wininet.RunOnceUrlCache(); }
void _SetUrlCacheConfigInfoA() { wininet.SetUrlCacheConfigInfoA(); }
void _SetUrlCacheConfigInfoW() { wininet.SetUrlCacheConfigInfoW(); }
void _SetUrlCacheEntryGroup() { wininet.SetUrlCacheEntryGroup(); }
void _SetUrlCacheEntryGroupA() { wininet.SetUrlCacheEntryGroupA(); }
void _SetUrlCacheEntryGroupW() { wininet.SetUrlCacheEntryGroupW(); }
void _SetUrlCacheEntryInfoA() { wininet.SetUrlCacheEntryInfoA(); }
void _SetUrlCacheEntryInfoW() { wininet.SetUrlCacheEntryInfoW(); }
void _SetUrlCacheGroupAttributeA() { wininet.SetUrlCacheGroupAttributeA(); }
void _SetUrlCacheGroupAttributeW() { wininet.SetUrlCacheGroupAttributeW(); }
void _SetUrlCacheHeaderData() { wininet.SetUrlCacheHeaderData(); }
void _ShowCertificate() { wininet.ShowCertificate(); }
void _ShowClientAuthCerts() { wininet.ShowClientAuthCerts(); }
void _ShowSecurityInfo() { wininet.ShowSecurityInfo(); }
void _ShowX509EncodedCertificate() { wininet.ShowX509EncodedCertificate(); }
void _UnlockUrlCacheEntryFile() { wininet.UnlockUrlCacheEntryFile(); }
void _UnlockUrlCacheEntryFileA() { wininet.UnlockUrlCacheEntryFileA(); }
void _UnlockUrlCacheEntryFileW() { wininet.UnlockUrlCacheEntryFileW(); }
void _UnlockUrlCacheEntryStream() { wininet.UnlockUrlCacheEntryStream(); }
void _UpdateUrlCacheContentPath() { wininet.UpdateUrlCacheContentPath(); }
void _UrlCacheCheckEntriesExist() { wininet.UrlCacheCheckEntriesExist(); }
void _UrlCacheCloseEntryHandle() { wininet.UrlCacheCloseEntryHandle(); }
void _UrlCacheContainerSetEntryMaximumAge() { wininet.UrlCacheContainerSetEntryMaximumAge(); }
void _UrlCacheCreateContainer() { wininet.UrlCacheCreateContainer(); }
void _UrlCacheFindFirstEntry() { wininet.UrlCacheFindFirstEntry(); }
void _UrlCacheFindNextEntry() { wininet.UrlCacheFindNextEntry(); }
void _UrlCacheFreeEntryInfo() { wininet.UrlCacheFreeEntryInfo(); }
void _UrlCacheFreeGlobalSpace() { wininet.UrlCacheFreeGlobalSpace(); }
void _UrlCacheGetContentPaths() { wininet.UrlCacheGetContentPaths(); }
void _UrlCacheGetEntryInfo() { wininet.UrlCacheGetEntryInfo(); }
void _UrlCacheGetGlobalCacheSize() { wininet.UrlCacheGetGlobalCacheSize(); }
void _UrlCacheGetGlobalLimit() { wininet.UrlCacheGetGlobalLimit(); }
void _UrlCacheReadEntryStream() { wininet.UrlCacheReadEntryStream(); }
void _UrlCacheReloadSettings() { wininet.UrlCacheReloadSettings(); }
void _UrlCacheRetrieveEntryFile() { wininet.UrlCacheRetrieveEntryFile(); }
void _UrlCacheRetrieveEntryStream() { wininet.UrlCacheRetrieveEntryStream(); }
void _UrlCacheServer() { wininet.UrlCacheServer(); }
void _UrlCacheSetGlobalLimit() { wininet.UrlCacheSetGlobalLimit(); }
void _UrlCacheUpdateEntryExtraData() { wininet.UrlCacheUpdateEntryExtraData(); }
void _UrlZonesDetach() { wininet.UrlZonesDetach(); }

#pragma endregion 

#pragma region Version methods

void _GetFileVersionInfoA() { version.GetFileVersionInfoA(); }
void _GetFileVersionInfoByHandle() { version.GetFileVersionInfoByHandle(); }
void _GetFileVersionInfoExA() { version.GetFileVersionInfoExA(); }
void _GetFileVersionInfoExW() { version.GetFileVersionInfoExW(); }
void _GetFileVersionInfoSizeA() { version.GetFileVersionInfoSizeA(); }
void _GetFileVersionInfoSizeExA() { version.GetFileVersionInfoSizeExA(); }
void _GetFileVersionInfoSizeExW() { version.GetFileVersionInfoSizeExW(); }
void _GetFileVersionInfoSizeW() { version.GetFileVersionInfoSizeW(); }
void _GetFileVersionInfoW() { version.GetFileVersionInfoW(); }
void _VerFindFileA() { version.VerFindFileA(); }
void _VerFindFileW() { version.VerFindFileW(); }
void _VerInstallFileA() { version.VerInstallFileA(); }
void _VerInstallFileW() { version.VerInstallFileW(); }
void _VerLanguageNameA() { version.VerLanguageNameA(); }
void _VerLanguageNameW() { version.VerLanguageNameW(); }
void _VerQueryValueA() { version.VerQueryValueA(); }
void _VerQueryValueW() { version.VerQueryValueW(); }

#pragma endregion 

void _vc7fpo() { dbghelp.dbghelpvc7fpo(); }
void _symsrv() { dbghelp.dbghelpsymsrv(); }
void _sym() { dbghelp.dbghelpsym(); }
void _stackdbg() { dbghelp.dbghelpstackdbg(); }
void _stack_force_ebp() { dbghelp.dbghelpstack_force_ebp(); }
void _srcfiles() { dbghelp.dbghelpsrcfiles(); }
void _optdbgdumpaddr() { dbghelp.dbghelpoptdbgdumpaddr(); }
void _optdbgdump() { dbghelp.dbghelpoptdbgdump(); }
void _omap() { dbghelp.dbghelpomap(); }
void _lminfo() { dbghelp.dbghelplminfo(); }
void _lmi() { dbghelp.dbghelplmi(); }
void _itoldyouso() { dbghelp.dbghelpitoldyouso(); }
void _inlinedbg() { dbghelp.dbghelpinlinedbg(); }
void _homedir() { dbghelp.dbghelphomedir(); }
void _fptr() { dbghelp.dbghelpfptr(); }
void _dh() { dbghelp.dbghelpdh(); }
void _dbghelp() { dbghelp.dbghelpdbghelp(); }
void _chksym() { dbghelp.dbghelpchksym(); }
void _block() { dbghelp.dbghelpblock(); }
void __EFN_DumpImage() { dbghelp.dbghelp_EFN_DumpImage(); }
void _WinDbgExtensionDllInit() { dbghelp.dbghelpWinDbgExtensionDllInit(); }
void _UnDecorateSymbolNameW() { dbghelp.dbghelpUnDecorateSymbolNameW(); }
void _UnDecorateSymbolName() { dbghelp.dbghelpUnDecorateSymbolName(); }
void _SymUnloadModule64() { dbghelp.dbghelpSymUnloadModule64(); }
void _SymUnloadModule() { dbghelp.dbghelpSymUnloadModule(); }
void _SymUnDName64() { dbghelp.dbghelpSymUnDName64(); }
void _SymUnDName() { dbghelp.dbghelpSymUnDName(); }
void _SymSrvStoreSupplementW() { dbghelp.dbghelpSymSrvStoreSupplementW(); }
void _SymSrvStoreSupplement() { dbghelp.dbghelpSymSrvStoreSupplement(); }
void _SymSrvStoreFileW() { dbghelp.dbghelpSymSrvStoreFileW(); }
void _SymSrvStoreFile() { dbghelp.dbghelpSymSrvStoreFile(); }
void _SymSrvIsStoreW() { dbghelp.dbghelpSymSrvIsStoreW(); }
void _SymSrvIsStore() { dbghelp.dbghelpSymSrvIsStore(); }
void _SymSrvGetSupplementW() { dbghelp.dbghelpSymSrvGetSupplementW(); }
void _SymSrvGetSupplement() { dbghelp.dbghelpSymSrvGetSupplement(); }
void _SymSrvGetFileIndexesW() { dbghelp.dbghelpSymSrvGetFileIndexesW(); }
void _SymSrvGetFileIndexes() { dbghelp.dbghelpSymSrvGetFileIndexes(); }
void _SymSrvGetFileIndexStringW() { dbghelp.dbghelpSymSrvGetFileIndexStringW(); }
void _SymSrvGetFileIndexString() { dbghelp.dbghelpSymSrvGetFileIndexString(); }
void _SymSrvGetFileIndexInfoW() { dbghelp.dbghelpSymSrvGetFileIndexInfoW(); }
void _SymSrvGetFileIndexInfo() { dbghelp.dbghelpSymSrvGetFileIndexInfo(); }
void _SymSrvDeltaNameW() { dbghelp.dbghelpSymSrvDeltaNameW(); }
void _SymSrvDeltaName() { dbghelp.dbghelpSymSrvDeltaName(); }
void _SymSetSearchPathW() { dbghelp.dbghelpSymSetSearchPathW(); }
void _SymSetSearchPath() { dbghelp.dbghelpSymSetSearchPath(); }
void _SymSetScopeFromInlineContext() { dbghelp.dbghelpSymSetScopeFromInlineContext(); }
void _SymSetScopeFromIndex() { dbghelp.dbghelpSymSetScopeFromIndex(); }
void _SymSetScopeFromAddr() { dbghelp.dbghelpSymSetScopeFromAddr(); }
void _SymSetParentWindow() { dbghelp.dbghelpSymSetParentWindow(); }
void _SymSetOptions() { dbghelp.dbghelpSymSetOptions(); }
void _SymSetHomeDirectoryW() { dbghelp.dbghelpSymSetHomeDirectoryW(); }
void _SymSetHomeDirectory() { dbghelp.dbghelpSymSetHomeDirectory(); }
void _SymSetExtendedOption() { dbghelp.dbghelpSymSetExtendedOption(); }
void _SymSetDiaSession() { dbghelp.dbghelpSymSetDiaSession(); }
void _SymSetContext() { dbghelp.dbghelpSymSetContext(); }
void _SymSearchW() { dbghelp.dbghelpSymSearchW(); }
void _SymSearch() { dbghelp.dbghelpSymSearch(); }
void _SymRegisterFunctionEntryCallback64() { dbghelp.dbghelpSymRegisterFunctionEntryCallback64(); }
void _SymRegisterFunctionEntryCallback() { dbghelp.dbghelpSymRegisterFunctionEntryCallback(); }
void _SymRegisterCallbackW64() { dbghelp.dbghelpSymRegisterCallbackW64(); }
void _SymRegisterCallback64() { dbghelp.dbghelpSymRegisterCallback64(); }
void _SymRegisterCallback() { dbghelp.dbghelpSymRegisterCallback(); }
void _SymRefreshModuleList() { dbghelp.dbghelpSymRefreshModuleList(); }
void _SymQueryInlineTrace() { dbghelp.dbghelpSymQueryInlineTrace(); }
void _SymPrevW() { dbghelp.dbghelpSymPrevW(); }
void _SymPrev() { dbghelp.dbghelpSymPrev(); }
void _SymNextW() { dbghelp.dbghelpSymNextW(); }
void _SymNext() { dbghelp.dbghelpSymNext(); }
void _SymMatchStringW() { dbghelp.dbghelpSymMatchStringW(); }
void _SymMatchStringA() { dbghelp.dbghelpSymMatchStringA(); }
void _SymMatchString() { dbghelp.dbghelpSymMatchString(); }
void _SymMatchFileNameW() { dbghelp.dbghelpSymMatchFileNameW(); }
void _SymMatchFileName() { dbghelp.dbghelpSymMatchFileName(); }
void _SymLoadModuleExW() { dbghelp.dbghelpSymLoadModuleExW(); }
void _SymLoadModuleEx() { dbghelp.dbghelpSymLoadModuleEx(); }
void _SymLoadModule64() { dbghelp.dbghelpSymLoadModule64(); }
void _SymLoadModule() { dbghelp.dbghelpSymLoadModule(); }
void _SymInitializeW() { dbghelp.dbghelpSymInitializeW(); }
void _SymInitialize() { dbghelp.dbghelpSymInitialize(); }
void _SymGetUnwindInfo() { dbghelp.dbghelpSymGetUnwindInfo(); }
void _SymGetTypeInfoEx() { dbghelp.dbghelpSymGetTypeInfoEx(); }
void _SymGetTypeInfo() { dbghelp.dbghelpSymGetTypeInfo(); }
void _SymGetTypeFromNameW() { dbghelp.dbghelpSymGetTypeFromNameW(); }
void _SymGetTypeFromName() { dbghelp.dbghelpSymGetTypeFromName(); }
void _SymGetSymbolFileW() { dbghelp.dbghelpSymGetSymbolFileW(); }
void _SymGetSymbolFile() { dbghelp.dbghelpSymGetSymbolFile(); }
void _SymGetSymPrev64() { dbghelp.dbghelpSymGetSymPrev64(); }
void _SymGetSymPrev() { dbghelp.dbghelpSymGetSymPrev(); }
void _SymGetSymNext64() { dbghelp.dbghelpSymGetSymNext64(); }
void _SymGetSymNext() { dbghelp.dbghelpSymGetSymNext(); }
void _SymGetSymFromName64() { dbghelp.dbghelpSymGetSymFromName64(); }
void _SymGetSymFromName() { dbghelp.dbghelpSymGetSymFromName(); }
void _SymGetSymFromAddr64() { dbghelp.dbghelpSymGetSymFromAddr64(); }
void _SymGetSymFromAddr() { dbghelp.dbghelpSymGetSymFromAddr(); }
void _SymGetSourceVarFromTokenW() { dbghelp.dbghelpSymGetSourceVarFromTokenW(); }
void _SymGetSourceVarFromToken() { dbghelp.dbghelpSymGetSourceVarFromToken(); }
void _SymGetSourceFileW() { dbghelp.dbghelpSymGetSourceFileW(); }
void _SymGetSourceFileTokenW() { dbghelp.dbghelpSymGetSourceFileTokenW(); }
void _SymGetSourceFileToken() { dbghelp.dbghelpSymGetSourceFileToken(); }
void _SymGetSourceFileFromTokenW() { dbghelp.dbghelpSymGetSourceFileFromTokenW(); }
void _SymGetSourceFileFromToken() { dbghelp.dbghelpSymGetSourceFileFromToken(); }
void _SymGetSourceFileChecksumW() { dbghelp.dbghelpSymGetSourceFileChecksumW(); }
void _SymGetSourceFileChecksum() { dbghelp.dbghelpSymGetSourceFileChecksum(); }
void _SymGetSourceFile() { dbghelp.dbghelpSymGetSourceFile(); }
void _SymGetSearchPathW() { dbghelp.dbghelpSymGetSearchPathW(); }
void _SymGetSearchPath() { dbghelp.dbghelpSymGetSearchPath(); }
void _SymGetScopeW() { dbghelp.dbghelpSymGetScopeW(); }
void _SymGetScope() { dbghelp.dbghelpSymGetScope(); }
void _SymGetOptions() { dbghelp.dbghelpSymGetOptions(); }
void _SymGetOmaps() { dbghelp.dbghelpSymGetOmaps(); }
void _SymGetOmapBlockBase() { dbghelp.dbghelpSymGetOmapBlockBase(); }
void _SymGetModuleInfoW64() { dbghelp.dbghelpSymGetModuleInfoW64(); }
void _SymGetModuleInfoW() { dbghelp.dbghelpSymGetModuleInfoW(); }
void _SymGetModuleInfo64() { dbghelp.dbghelpSymGetModuleInfo64(); }
void _SymGetModuleInfo() { dbghelp.dbghelpSymGetModuleInfo(); }
void _SymGetModuleBase64() { dbghelp.dbghelpSymGetModuleBase64(); }
void _SymGetModuleBase() { dbghelp.dbghelpSymGetModuleBase(); }
void _SymGetLinePrevW64() { dbghelp.dbghelpSymGetLinePrevW64(); }
void _SymGetLinePrevEx() { dbghelp.dbghelpSymGetLinePrevEx(); }
void _SymGetLinePrev64() { dbghelp.dbghelpSymGetLinePrev64(); }
void _SymGetLinePrev() { dbghelp.dbghelpSymGetLinePrev(); }
void _SymGetLineNextW64() { dbghelp.dbghelpSymGetLineNextW64(); }
void _SymGetLineNextEx() { dbghelp.dbghelpSymGetLineNextEx(); }
void _SymGetLineNext64() { dbghelp.dbghelpSymGetLineNext64(); }
void _SymGetLineNext() { dbghelp.dbghelpSymGetLineNext(); }
void _SymGetLineFromNameW64() { dbghelp.dbghelpSymGetLineFromNameW64(); }
void _SymGetLineFromNameEx() { dbghelp.dbghelpSymGetLineFromNameEx(); }
void _SymGetLineFromName64() { dbghelp.dbghelpSymGetLineFromName64(); }
void _SymGetLineFromName() { dbghelp.dbghelpSymGetLineFromName(); }
void _SymGetLineFromInlineContextW() { dbghelp.dbghelpSymGetLineFromInlineContextW(); }
void _SymGetLineFromInlineContext() { dbghelp.dbghelpSymGetLineFromInlineContext(); }
void _SymGetLineFromAddrW64() { dbghelp.dbghelpSymGetLineFromAddrW64(); }
void _SymGetLineFromAddrEx() { dbghelp.dbghelpSymGetLineFromAddrEx(); }
void _SymGetLineFromAddr64() { dbghelp.dbghelpSymGetLineFromAddr64(); }
void _SymGetLineFromAddr() { dbghelp.dbghelpSymGetLineFromAddr(); }
void _SymGetHomeDirectoryW() { dbghelp.dbghelpSymGetHomeDirectoryW(); }
void _SymGetHomeDirectory() { dbghelp.dbghelpSymGetHomeDirectory(); }
void _SymGetFileLineOffsets64() { dbghelp.dbghelpSymGetFileLineOffsets64(); }
void _SymGetExtendedOption() { dbghelp.dbghelpSymGetExtendedOption(); }
void _SymGetDiaSession() { dbghelp.dbghelpSymGetDiaSession(); }
void _SymFunctionTableAccess64AccessRoutines() { dbghelp.dbghelpSymFunctionTableAccess64AccessRoutines(); }
void _SymFunctionTableAccess64() { dbghelp.dbghelpSymFunctionTableAccess64(); }
void _SymFunctionTableAccess() { dbghelp.dbghelpSymFunctionTableAccess(); }
void _SymFromTokenW() { dbghelp.dbghelpSymFromTokenW(); }
void _SymFromToken() { dbghelp.dbghelpSymFromToken(); }
void _SymFromNameW() { dbghelp.dbghelpSymFromNameW(); }
void _SymFromName() { dbghelp.dbghelpSymFromName(); }
void _SymFromInlineContextW() { dbghelp.dbghelpSymFromInlineContextW(); }
void _SymFromInlineContext() { dbghelp.dbghelpSymFromInlineContext(); }
void _SymFromIndexW() { dbghelp.dbghelpSymFromIndexW(); }
void _SymFromIndex() { dbghelp.dbghelpSymFromIndex(); }
void _SymFromAddrW() { dbghelp.dbghelpSymFromAddrW(); }
void _SymFromAddr() { dbghelp.dbghelpSymFromAddr(); }
void _SymFreeDiaString() { dbghelp.dbghelpSymFreeDiaString(); }
void _SymFindFileInPathW() { dbghelp.dbghelpSymFindFileInPathW(); }
void _SymFindFileInPath() { dbghelp.dbghelpSymFindFileInPath(); }
void _SymFindExecutableImageW() { dbghelp.dbghelpSymFindExecutableImageW(); }
void _SymFindExecutableImage() { dbghelp.dbghelpSymFindExecutableImage(); }
void _SymFindDebugInfoFileW() { dbghelp.dbghelpSymFindDebugInfoFileW(); }
void _SymFindDebugInfoFile() { dbghelp.dbghelpSymFindDebugInfoFile(); }
void _SymEnumerateSymbolsW64() { dbghelp.dbghelpSymEnumerateSymbolsW64(); }
void _SymEnumerateSymbolsW() { dbghelp.dbghelpSymEnumerateSymbolsW(); }
void _SymEnumerateSymbols64() { dbghelp.dbghelpSymEnumerateSymbols64(); }
void _SymEnumerateSymbols() { dbghelp.dbghelpSymEnumerateSymbols(); }
void _SymEnumerateModulesW64() { dbghelp.dbghelpSymEnumerateModulesW64(); }
void _SymEnumerateModules64() { dbghelp.dbghelpSymEnumerateModules64(); }
void _SymEnumerateModules() { dbghelp.dbghelpSymEnumerateModules(); }
void _SymEnumTypesW() { dbghelp.dbghelpSymEnumTypesW(); }
void _SymEnumTypesByNameW() { dbghelp.dbghelpSymEnumTypesByNameW(); }
void _SymEnumTypesByName() { dbghelp.dbghelpSymEnumTypesByName(); }
void _SymEnumTypes() { dbghelp.dbghelpSymEnumTypes(); }
void _SymEnumSymbolsW() { dbghelp.dbghelpSymEnumSymbolsW(); }
void _SymEnumSymbolsForAddrW() { dbghelp.dbghelpSymEnumSymbolsForAddrW(); }
void _SymEnumSymbolsForAddr() { dbghelp.dbghelpSymEnumSymbolsForAddr(); }
void _SymEnumSymbolsExW() { dbghelp.dbghelpSymEnumSymbolsExW(); }
void _SymEnumSymbolsEx() { dbghelp.dbghelpSymEnumSymbolsEx(); }
void _SymEnumSymbols() { dbghelp.dbghelpSymEnumSymbols(); }
void _SymEnumSym() { dbghelp.dbghelpSymEnumSym(); }
void _SymEnumSourceLinesW() { dbghelp.dbghelpSymEnumSourceLinesW(); }
void _SymEnumSourceLines() { dbghelp.dbghelpSymEnumSourceLines(); }
void _SymEnumSourceFilesW() { dbghelp.dbghelpSymEnumSourceFilesW(); }
void _SymEnumSourceFiles() { dbghelp.dbghelpSymEnumSourceFiles(); }
void _SymEnumSourceFileTokens() { dbghelp.dbghelpSymEnumSourceFileTokens(); }
void _SymEnumProcesses() { dbghelp.dbghelpSymEnumProcesses(); }
void _SymEnumLinesW() { dbghelp.dbghelpSymEnumLinesW(); }
void _SymEnumLines() { dbghelp.dbghelpSymEnumLines(); }
void _SymDeleteSymbolW() { dbghelp.dbghelpSymDeleteSymbolW(); }
void _SymDeleteSymbol() { dbghelp.dbghelpSymDeleteSymbol(); }
void _SymCompareInlineTrace() { dbghelp.dbghelpSymCompareInlineTrace(); }
void _SymCleanup() { dbghelp.dbghelpSymCleanup(); }
void _SymAllocDiaString() { dbghelp.dbghelpSymAllocDiaString(); }
void _SymAddrIncludeInlineTrace() { dbghelp.dbghelpSymAddrIncludeInlineTrace(); }
void _SymAddSymbolW() { dbghelp.dbghelpSymAddSymbolW(); }
void _SymAddSymbol() { dbghelp.dbghelpSymAddSymbol(); }
void _SymAddSourceStreamW() { dbghelp.dbghelpSymAddSourceStreamW(); }
void _SymAddSourceStreamA() { dbghelp.dbghelpSymAddSourceStreamA(); }
void _SymAddSourceStream() { dbghelp.dbghelpSymAddSourceStream(); }
void _StackWalkEx() { dbghelp.dbghelpStackWalkEx(); }
void _StackWalk64() { dbghelp.dbghelpStackWalk64(); }
void _StackWalk() { dbghelp.dbghelpStackWalk(); }
void _SetSymLoadError() { dbghelp.dbghelpSetSymLoadError(); }
void _SetCheckUserInterruptShared() { dbghelp.dbghelpSetCheckUserInterruptShared(); }
void _SearchTreeForFileW() { dbghelp.dbghelpSearchTreeForFileW(); }
void _SearchTreeForFile() { dbghelp.dbghelpSearchTreeForFile(); }
void _ReportSymbolLoadSummary() { dbghelp.dbghelpReportSymbolLoadSummary(); }
void _RemoveInvalidModuleList() { dbghelp.dbghelpRemoveInvalidModuleList(); }
void _RangeMapWrite() { dbghelp.dbghelpRangeMapWrite(); }
void _RangeMapRemove() { dbghelp.dbghelpRangeMapRemove(); }
void _RangeMapRead() { dbghelp.dbghelpRangeMapRead(); }
void _RangeMapFree() { dbghelp.dbghelpRangeMapFree(); }
void _RangeMapCreate() { dbghelp.dbghelpRangeMapCreate(); }
void _RangeMapAddPeImageSections() { dbghelp.dbghelpRangeMapAddPeImageSections(); }
void _MiniDumpWriteDump() { dbghelp.dbghelpMiniDumpWriteDump(); }
void _MiniDumpReadDumpStream() { dbghelp.dbghelpMiniDumpReadDumpStream(); }
void _MakeSureDirectoryPathExists() { dbghelp.dbghelpMakeSureDirectoryPathExists(); }
void _ImagehlpApiVersionEx() { dbghelp.dbghelpImagehlpApiVersionEx(); }
void _ImagehlpApiVersion() { dbghelp.dbghelpImagehlpApiVersion(); }
void _ImageRvaToVa() { dbghelp.dbghelpImageRvaToVa(); }
void _ImageRvaToSection() { dbghelp.dbghelpImageRvaToSection(); }
void _ImageNtHeader() { dbghelp.dbghelpImageNtHeader(); }
void _ImageDirectoryEntryToDataEx() { dbghelp.dbghelpImageDirectoryEntryToDataEx(); }
void _ImageDirectoryEntryToData() { dbghelp.dbghelpImageDirectoryEntryToData(); }
void _GetTimestampForLoadedLibrary() { dbghelp.dbghelpGetTimestampForLoadedLibrary(); }
void _GetSymLoadError() { dbghelp.dbghelpGetSymLoadError(); }
void _FindFileInSearchPath() { dbghelp.dbghelpFindFileInSearchPath(); }
void _FindFileInPath() { dbghelp.dbghelpFindFileInPath(); }
void _FindExecutableImageExW() { dbghelp.dbghelpFindExecutableImageExW(); }
void _FindExecutableImageEx() { dbghelp.dbghelpFindExecutableImageEx(); }
void _FindExecutableImage() { dbghelp.dbghelpFindExecutableImage(); }
void _FindDebugInfoFileExW() { dbghelp.dbghelpFindDebugInfoFileExW(); }
void _FindDebugInfoFileEx() { dbghelp.dbghelpFindDebugInfoFileEx(); }
void _FindDebugInfoFile() { dbghelp.dbghelpFindDebugInfoFile(); }
void _ExtensionApiVersion() { dbghelp.dbghelpExtensionApiVersion(); }
void _EnumerateLoadedModulesW64() { dbghelp.dbghelpEnumerateLoadedModulesW64(); }
void _EnumerateLoadedModulesExW() { dbghelp.dbghelpEnumerateLoadedModulesExW(); }
void _EnumerateLoadedModulesEx() { dbghelp.dbghelpEnumerateLoadedModulesEx(); }
void _EnumerateLoadedModules64() { dbghelp.dbghelpEnumerateLoadedModules64(); }
void _EnumerateLoadedModules() { dbghelp.dbghelpEnumerateLoadedModules(); }
void _EnumDirTreeW() { dbghelp.dbghelpEnumDirTreeW(); }
void _EnumDirTree() { dbghelp.dbghelpEnumDirTree(); }
void _DbgHelpCreateUserDumpW() { dbghelp.dbghelpDbgHelpCreateUserDumpW(); }