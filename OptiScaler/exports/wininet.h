#pragma once

#include <pch.h>

#include "shared.h"

#include <proxies/KernelBase_Proxy.h>

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

        AppCacheCheckManifest = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheCheckManifest");
        AppCacheCloseHandle = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheCloseHandle");
        AppCacheCreateAndCommitFile = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheCreateAndCommitFile");
        AppCacheDeleteGroup = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheDeleteGroup");
        AppCacheDeleteIEGroup = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheDeleteIEGroup");
        AppCacheDuplicateHandle = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheDuplicateHandle");
        AppCacheFinalize = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheFinalize");
        AppCacheFreeDownloadList = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheFreeDownloadList");
        AppCacheFreeGroupList = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheFreeGroupList");
        AppCacheFreeIESpace = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheFreeIESpace");
        AppCacheFreeSpace = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheFreeSpace");
        AppCacheGetDownloadList = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetDownloadList");
        AppCacheGetFallbackUrl = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetFallbackUrl");
        AppCacheGetGroupList = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetGroupList");
        AppCacheGetIEGroupList = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetIEGroupList");
        AppCacheGetInfo = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetInfo");
        AppCacheGetManifestUrl = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheGetManifestUrl");
        AppCacheLookup = KernelBaseProxy::GetProcAddress_()(dll, "AppCacheLookup");
        CommitUrlCacheEntryA = KernelBaseProxy::GetProcAddress_()(dll, "CommitUrlCacheEntryA");
        CommitUrlCacheEntryBinaryBlob = KernelBaseProxy::GetProcAddress_()(dll, "CommitUrlCacheEntryBinaryBlob");
        CommitUrlCacheEntryW = KernelBaseProxy::GetProcAddress_()(dll, "CommitUrlCacheEntryW");
        CreateMD5SSOHash = KernelBaseProxy::GetProcAddress_()(dll, "CreateMD5SSOHash");
        CreateUrlCacheContainerA = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheContainerA");
        CreateUrlCacheContainerW = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheContainerW");
        CreateUrlCacheEntryA = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheEntryA");
        CreateUrlCacheEntryExW = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheEntryExW");
        CreateUrlCacheEntryW = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheEntryW");
        CreateUrlCacheGroup = KernelBaseProxy::GetProcAddress_()(dll, "CreateUrlCacheGroup");
        DeleteIE3Cache = KernelBaseProxy::GetProcAddress_()(dll, "DeleteIE3Cache");
        DeleteUrlCacheContainerA = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheContainerA");
        DeleteUrlCacheContainerW = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheContainerW");
        DeleteUrlCacheEntry = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheEntry");
        DeleteUrlCacheEntryA = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheEntryA");
        DeleteUrlCacheEntryW = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheEntryW");
        DeleteUrlCacheGroup = KernelBaseProxy::GetProcAddress_()(dll, "DeleteUrlCacheGroup");
        DeleteWpadCacheForNetworks = KernelBaseProxy::GetProcAddress_()(dll, "DeleteWpadCacheForNetworks");
        DetectAutoProxyUrl = KernelBaseProxy::GetProcAddress_()(dll, "DetectAutoProxyUrl");
        DispatchAPICall = KernelBaseProxy::GetProcAddress_()(dll, "DispatchAPICall");
        DllInstall = KernelBaseProxy::GetProcAddress_()(dll, "DllInstall");
        FindCloseUrlCache = KernelBaseProxy::GetProcAddress_()(dll, "FindCloseUrlCache");
        FindFirstUrlCacheContainerA = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheContainerA");
        FindFirstUrlCacheContainerW = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheContainerW");
        FindFirstUrlCacheEntryA = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheEntryA");
        FindFirstUrlCacheEntryExA = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheEntryExA");
        FindFirstUrlCacheEntryExW = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheEntryExW");
        FindFirstUrlCacheEntryW = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheEntryW");
        FindFirstUrlCacheGroup = KernelBaseProxy::GetProcAddress_()(dll, "FindFirstUrlCacheGroup");
        FindNextUrlCacheContainerA = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheContainerA");
        FindNextUrlCacheContainerW = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheContainerW");
        FindNextUrlCacheEntryA = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheEntryA");
        FindNextUrlCacheEntryExA = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheEntryExA");
        FindNextUrlCacheEntryExW = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheEntryExW");
        FindNextUrlCacheEntryW = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheEntryW");
        FindNextUrlCacheGroup = KernelBaseProxy::GetProcAddress_()(dll, "FindNextUrlCacheGroup");
        ForceNexusLookup = KernelBaseProxy::GetProcAddress_()(dll, "ForceNexusLookup");
        ForceNexusLookupExW = KernelBaseProxy::GetProcAddress_()(dll, "ForceNexusLookupExW");
        FreeUrlCacheSpaceA = KernelBaseProxy::GetProcAddress_()(dll, "FreeUrlCacheSpaceA");
        FreeUrlCacheSpaceW = KernelBaseProxy::GetProcAddress_()(dll, "FreeUrlCacheSpaceW");
        FtpCommandA = KernelBaseProxy::GetProcAddress_()(dll, "FtpCommandA");
        FtpCommandW = KernelBaseProxy::GetProcAddress_()(dll, "FtpCommandW");
        FtpCreateDirectoryA = KernelBaseProxy::GetProcAddress_()(dll, "FtpCreateDirectoryA");
        FtpCreateDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "FtpCreateDirectoryW");
        FtpDeleteFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpDeleteFileA");
        FtpDeleteFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpDeleteFileW");
        FtpFindFirstFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpFindFirstFileA");
        FtpFindFirstFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpFindFirstFileW");
        FtpGetCurrentDirectoryA = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetCurrentDirectoryA");
        FtpGetCurrentDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetCurrentDirectoryW");
        FtpGetFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetFileA");
        FtpGetFileEx = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetFileEx");
        FtpGetFileSize = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetFileSize");
        FtpGetFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpGetFileW");
        FtpOpenFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpOpenFileA");
        FtpOpenFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpOpenFileW");
        FtpPutFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpPutFileA");
        FtpPutFileEx = KernelBaseProxy::GetProcAddress_()(dll, "FtpPutFileEx");
        FtpPutFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpPutFileW");
        FtpRemoveDirectoryA = KernelBaseProxy::GetProcAddress_()(dll, "FtpRemoveDirectoryA");
        FtpRemoveDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "FtpRemoveDirectoryW");
        FtpRenameFileA = KernelBaseProxy::GetProcAddress_()(dll, "FtpRenameFileA");
        FtpRenameFileW = KernelBaseProxy::GetProcAddress_()(dll, "FtpRenameFileW");
        FtpSetCurrentDirectoryA = KernelBaseProxy::GetProcAddress_()(dll, "FtpSetCurrentDirectoryA");
        FtpSetCurrentDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "FtpSetCurrentDirectoryW");
        _GetFileExtensionFromUrl = KernelBaseProxy::GetProcAddress_()(dll, "_GetFileExtensionFromUrl");
        GetProxyDllInfo = KernelBaseProxy::GetProcAddress_()(dll, "GetProxyDllInfo");
        GetUrlCacheConfigInfoA = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheConfigInfoA");
        GetUrlCacheConfigInfoW = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheConfigInfoW");
        GetUrlCacheEntryBinaryBlob = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheEntryBinaryBlob");
        GetUrlCacheEntryInfoA = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheEntryInfoA");
        GetUrlCacheEntryInfoExA = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheEntryInfoExA");
        GetUrlCacheEntryInfoExW = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheEntryInfoExW");
        GetUrlCacheEntryInfoW = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheEntryInfoW");
        GetUrlCacheGroupAttributeA = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheGroupAttributeA");
        GetUrlCacheGroupAttributeW = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheGroupAttributeW");
        GetUrlCacheHeaderData = KernelBaseProxy::GetProcAddress_()(dll, "GetUrlCacheHeaderData");
        GopherCreateLocatorA = KernelBaseProxy::GetProcAddress_()(dll, "GopherCreateLocatorA");
        GopherCreateLocatorW = KernelBaseProxy::GetProcAddress_()(dll, "GopherCreateLocatorW");
        GopherFindFirstFileA = KernelBaseProxy::GetProcAddress_()(dll, "GopherFindFirstFileA");
        GopherFindFirstFileW = KernelBaseProxy::GetProcAddress_()(dll, "GopherFindFirstFileW");
        GopherGetAttributeA = KernelBaseProxy::GetProcAddress_()(dll, "GopherGetAttributeA");
        GopherGetAttributeW = KernelBaseProxy::GetProcAddress_()(dll, "GopherGetAttributeW");
        GopherGetLocatorTypeA = KernelBaseProxy::GetProcAddress_()(dll, "GopherGetLocatorTypeA");
        GopherGetLocatorTypeW = KernelBaseProxy::GetProcAddress_()(dll, "GopherGetLocatorTypeW");
        GopherOpenFileA = KernelBaseProxy::GetProcAddress_()(dll, "GopherOpenFileA");
        GopherOpenFileW = KernelBaseProxy::GetProcAddress_()(dll, "GopherOpenFileW");
        HttpAddRequestHeadersA = KernelBaseProxy::GetProcAddress_()(dll, "HttpAddRequestHeadersA");
        HttpAddRequestHeadersW = KernelBaseProxy::GetProcAddress_()(dll, "HttpAddRequestHeadersW");
        HttpCheckDavCompliance = KernelBaseProxy::GetProcAddress_()(dll, "HttpCheckDavCompliance");
        HttpCloseDependencyHandle = KernelBaseProxy::GetProcAddress_()(dll, "HttpCloseDependencyHandle");
        HttpDuplicateDependencyHandle = KernelBaseProxy::GetProcAddress_()(dll, "HttpDuplicateDependencyHandle");
        HttpEndRequestA = KernelBaseProxy::GetProcAddress_()(dll, "HttpEndRequestA");
        HttpEndRequestW = KernelBaseProxy::GetProcAddress_()(dll, "HttpEndRequestW");
        HttpGetServerCredentials = KernelBaseProxy::GetProcAddress_()(dll, "HttpGetServerCredentials");
        HttpGetTunnelSocket = KernelBaseProxy::GetProcAddress_()(dll, "HttpGetTunnelSocket");
        HttpIsHostHstsEnabled = KernelBaseProxy::GetProcAddress_()(dll, "HttpIsHostHstsEnabled");
        HttpOpenDependencyHandle = KernelBaseProxy::GetProcAddress_()(dll, "HttpOpenDependencyHandle");
        HttpOpenRequestA = KernelBaseProxy::GetProcAddress_()(dll, "HttpOpenRequestA");
        HttpOpenRequestW = KernelBaseProxy::GetProcAddress_()(dll, "HttpOpenRequestW");
        HttpPushClose = KernelBaseProxy::GetProcAddress_()(dll, "HttpPushClose");
        HttpPushEnable = KernelBaseProxy::GetProcAddress_()(dll, "HttpPushEnable");
        HttpPushWait = KernelBaseProxy::GetProcAddress_()(dll, "HttpPushWait");
        HttpQueryInfoA = KernelBaseProxy::GetProcAddress_()(dll, "HttpQueryInfoA");
        HttpQueryInfoW = KernelBaseProxy::GetProcAddress_()(dll, "HttpQueryInfoW");
        HttpSendRequestA = KernelBaseProxy::GetProcAddress_()(dll, "HttpSendRequestA");
        HttpSendRequestExA = KernelBaseProxy::GetProcAddress_()(dll, "HttpSendRequestExA");
        HttpSendRequestExW = KernelBaseProxy::GetProcAddress_()(dll, "HttpSendRequestExW");
        HttpSendRequestW = KernelBaseProxy::GetProcAddress_()(dll, "HttpSendRequestW");
        HttpWebSocketClose = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketClose");
        HttpWebSocketCompleteUpgrade = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketCompleteUpgrade");
        HttpWebSocketQueryCloseStatus = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketQueryCloseStatus");
        HttpWebSocketReceive = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketReceive");
        HttpWebSocketSend = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketSend");
        HttpWebSocketShutdown = KernelBaseProxy::GetProcAddress_()(dll, "HttpWebSocketShutdown");
        IncrementUrlCacheHeaderData = KernelBaseProxy::GetProcAddress_()(dll, "IncrementUrlCacheHeaderData");
        InternetAlgIdToStringA = KernelBaseProxy::GetProcAddress_()(dll, "InternetAlgIdToStringA");
        InternetAlgIdToStringW = KernelBaseProxy::GetProcAddress_()(dll, "InternetAlgIdToStringW");
        InternetAttemptConnect = KernelBaseProxy::GetProcAddress_()(dll, "InternetAttemptConnect");
        InternetAutodial = KernelBaseProxy::GetProcAddress_()(dll, "InternetAutodial");
        InternetAutodialCallback = KernelBaseProxy::GetProcAddress_()(dll, "InternetAutodialCallback");
        InternetAutodialHangup = KernelBaseProxy::GetProcAddress_()(dll, "InternetAutodialHangup");
        InternetCanonicalizeUrlA = KernelBaseProxy::GetProcAddress_()(dll, "InternetCanonicalizeUrlA");
        InternetCanonicalizeUrlW = KernelBaseProxy::GetProcAddress_()(dll, "InternetCanonicalizeUrlW");
        InternetCheckConnectionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetCheckConnectionA");
        InternetCheckConnectionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetCheckConnectionW");
        InternetClearAllPerSiteCookieDecisions = KernelBaseProxy::GetProcAddress_()(dll, "InternetClearAllPerSiteCookieDecisions");
        InternetCloseHandle = KernelBaseProxy::GetProcAddress_()(dll, "InternetCloseHandle");
        InternetCombineUrlA = KernelBaseProxy::GetProcAddress_()(dll, "InternetCombineUrlA");
        InternetCombineUrlW = KernelBaseProxy::GetProcAddress_()(dll, "InternetCombineUrlW");
        InternetConfirmZoneCrossing = KernelBaseProxy::GetProcAddress_()(dll, "InternetConfirmZoneCrossing");
        InternetConfirmZoneCrossingA = KernelBaseProxy::GetProcAddress_()(dll, "InternetConfirmZoneCrossingA");
        InternetConfirmZoneCrossingW = KernelBaseProxy::GetProcAddress_()(dll, "InternetConfirmZoneCrossingW");
        InternetConnectA = KernelBaseProxy::GetProcAddress_()(dll, "InternetConnectA");
        InternetConnectW = KernelBaseProxy::GetProcAddress_()(dll, "InternetConnectW");
        InternetConvertUrlFromWireToWideChar = KernelBaseProxy::GetProcAddress_()(dll, "InternetConvertUrlFromWireToWideChar");
        InternetCrackUrlA = KernelBaseProxy::GetProcAddress_()(dll, "InternetCrackUrlA");
        InternetCrackUrlW = KernelBaseProxy::GetProcAddress_()(dll, "InternetCrackUrlW");
        InternetCreateUrlA = KernelBaseProxy::GetProcAddress_()(dll, "InternetCreateUrlA");
        InternetCreateUrlW = KernelBaseProxy::GetProcAddress_()(dll, "InternetCreateUrlW");
        InternetDial = KernelBaseProxy::GetProcAddress_()(dll, "InternetDial");
        InternetDialA = KernelBaseProxy::GetProcAddress_()(dll, "InternetDialA");
        InternetDialW = KernelBaseProxy::GetProcAddress_()(dll, "InternetDialW");
        InternetEnumPerSiteCookieDecisionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetEnumPerSiteCookieDecisionA");
        InternetEnumPerSiteCookieDecisionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetEnumPerSiteCookieDecisionW");
        InternetErrorDlg = KernelBaseProxy::GetProcAddress_()(dll, "InternetErrorDlg");
        InternetFindNextFileA = KernelBaseProxy::GetProcAddress_()(dll, "InternetFindNextFileA");
        InternetFindNextFileW = KernelBaseProxy::GetProcAddress_()(dll, "InternetFindNextFileW");
        InternetFortezzaCommand = KernelBaseProxy::GetProcAddress_()(dll, "InternetFortezzaCommand");
        InternetFreeCookies = KernelBaseProxy::GetProcAddress_()(dll, "InternetFreeCookies");
        InternetFreeProxyInfoList = KernelBaseProxy::GetProcAddress_()(dll, "InternetFreeProxyInfoList");
        InternetGetCertByURL = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCertByURL");
        InternetGetCertByURLA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCertByURLA");
        InternetGetConnectedState = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetConnectedState");
        InternetGetConnectedStateEx = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetConnectedStateEx");
        InternetGetConnectedStateExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetConnectedStateExA");
        InternetGetConnectedStateExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetConnectedStateExW");
        InternetGetCookieA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCookieA");
        InternetGetCookieEx2 = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCookieEx2");
        InternetGetCookieExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCookieExA");
        InternetGetCookieExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCookieExW");
        InternetGetCookieW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetCookieW");
        InternetGetLastResponseInfoA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetLastResponseInfoA");
        InternetGetLastResponseInfoW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetLastResponseInfoW");
        InternetGetPerSiteCookieDecisionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetPerSiteCookieDecisionA");
        InternetGetPerSiteCookieDecisionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetPerSiteCookieDecisionW");
        InternetGetProxyForUrl = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetProxyForUrl");
        InternetGetSecurityInfoByURL = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetSecurityInfoByURL");
        InternetGetSecurityInfoByURLA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetSecurityInfoByURLA");
        InternetGetSecurityInfoByURLW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGetSecurityInfoByURLW");
        InternetGoOnline = KernelBaseProxy::GetProcAddress_()(dll, "InternetGoOnline");
        InternetGoOnlineA = KernelBaseProxy::GetProcAddress_()(dll, "InternetGoOnlineA");
        InternetGoOnlineW = KernelBaseProxy::GetProcAddress_()(dll, "InternetGoOnlineW");
        InternetHangUp = KernelBaseProxy::GetProcAddress_()(dll, "InternetHangUp");
        InternetInitializeAutoProxyDll = KernelBaseProxy::GetProcAddress_()(dll, "InternetInitializeAutoProxyDll");
        InternetLockRequestFile = KernelBaseProxy::GetProcAddress_()(dll, "InternetLockRequestFile");
        InternetOpenA = KernelBaseProxy::GetProcAddress_()(dll, "InternetOpenA");
        InternetOpenUrlA = KernelBaseProxy::GetProcAddress_()(dll, "InternetOpenUrlA");
        InternetOpenUrlW = KernelBaseProxy::GetProcAddress_()(dll, "InternetOpenUrlW");
        InternetOpenW = KernelBaseProxy::GetProcAddress_()(dll, "InternetOpenW");
        InternetQueryDataAvailable = KernelBaseProxy::GetProcAddress_()(dll, "InternetQueryDataAvailable");
        InternetQueryFortezzaStatus = KernelBaseProxy::GetProcAddress_()(dll, "InternetQueryFortezzaStatus");
        InternetQueryOptionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetQueryOptionA");
        InternetQueryOptionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetQueryOptionW");
        InternetReadFile = KernelBaseProxy::GetProcAddress_()(dll, "InternetReadFile");
        InternetReadFileExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetReadFileExA");
        InternetReadFileExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetReadFileExW");
        InternetSecurityProtocolToStringA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSecurityProtocolToStringA");
        InternetSecurityProtocolToStringW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSecurityProtocolToStringW");
        InternetSetCookieA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetCookieA");
        InternetSetCookieEx2 = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetCookieEx2");
        InternetSetCookieExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetCookieExA");
        InternetSetCookieExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetCookieExW");
        InternetSetCookieW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetCookieW");
        InternetSetDialState = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetDialState");
        InternetSetDialStateA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetDialStateA");
        InternetSetDialStateW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetDialStateW");
        InternetSetFilePointer = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetFilePointer");
        InternetSetOptionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetOptionA");
        InternetSetOptionExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetOptionExA");
        InternetSetOptionExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetOptionExW");
        InternetSetOptionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetOptionW");
        InternetSetPerSiteCookieDecisionA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetPerSiteCookieDecisionA");
        InternetSetPerSiteCookieDecisionW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetPerSiteCookieDecisionW");
        InternetSetStatusCallback = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetStatusCallback");
        InternetSetStatusCallbackA = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetStatusCallbackA");
        InternetSetStatusCallbackW = KernelBaseProxy::GetProcAddress_()(dll, "InternetSetStatusCallbackW");
        InternetShowSecurityInfoByURL = KernelBaseProxy::GetProcAddress_()(dll, "InternetShowSecurityInfoByURL");
        InternetShowSecurityInfoByURLA = KernelBaseProxy::GetProcAddress_()(dll, "InternetShowSecurityInfoByURLA");
        InternetShowSecurityInfoByURLW = KernelBaseProxy::GetProcAddress_()(dll, "InternetShowSecurityInfoByURLW");
        InternetTimeFromSystemTime = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeFromSystemTime");
        InternetTimeFromSystemTimeA = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeFromSystemTimeA");
        InternetTimeFromSystemTimeW = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeFromSystemTimeW");
        InternetTimeToSystemTime = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeToSystemTime");
        InternetTimeToSystemTimeA = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeToSystemTimeA");
        InternetTimeToSystemTimeW = KernelBaseProxy::GetProcAddress_()(dll, "InternetTimeToSystemTimeW");
        InternetUnlockRequestFile = KernelBaseProxy::GetProcAddress_()(dll, "InternetUnlockRequestFile");
        InternetWriteFile = KernelBaseProxy::GetProcAddress_()(dll, "InternetWriteFile");
        InternetWriteFileExA = KernelBaseProxy::GetProcAddress_()(dll, "InternetWriteFileExA");
        InternetWriteFileExW = KernelBaseProxy::GetProcAddress_()(dll, "InternetWriteFileExW");
        IsHostInProxyBypassList = KernelBaseProxy::GetProcAddress_()(dll, "IsHostInProxyBypassList");
        IsUrlCacheEntryExpiredA = KernelBaseProxy::GetProcAddress_()(dll, "IsUrlCacheEntryExpiredA");
        IsUrlCacheEntryExpiredW = KernelBaseProxy::GetProcAddress_()(dll, "IsUrlCacheEntryExpiredW");
        LoadUrlCacheContent = KernelBaseProxy::GetProcAddress_()(dll, "LoadUrlCacheContent");
        ParseX509EncodedCertificateForListBoxEntry = KernelBaseProxy::GetProcAddress_()(dll, "ParseX509EncodedCertificateForListBoxEntry");
        PrivacyGetZonePreferenceW = KernelBaseProxy::GetProcAddress_()(dll, "PrivacyGetZonePreferenceW");
        PrivacySetZonePreferenceW = KernelBaseProxy::GetProcAddress_()(dll, "PrivacySetZonePreferenceW");
        ReadUrlCacheEntryStream = KernelBaseProxy::GetProcAddress_()(dll, "ReadUrlCacheEntryStream");
        ReadUrlCacheEntryStreamEx = KernelBaseProxy::GetProcAddress_()(dll, "ReadUrlCacheEntryStreamEx");
        RegisterUrlCacheNotification = KernelBaseProxy::GetProcAddress_()(dll, "RegisterUrlCacheNotification");
        ResumeSuspendedDownload = KernelBaseProxy::GetProcAddress_()(dll, "ResumeSuspendedDownload");
        RetrieveUrlCacheEntryFileA = KernelBaseProxy::GetProcAddress_()(dll, "RetrieveUrlCacheEntryFileA");
        RetrieveUrlCacheEntryFileW = KernelBaseProxy::GetProcAddress_()(dll, "RetrieveUrlCacheEntryFileW");
        RetrieveUrlCacheEntryStreamA = KernelBaseProxy::GetProcAddress_()(dll, "RetrieveUrlCacheEntryStreamA");
        RetrieveUrlCacheEntryStreamW = KernelBaseProxy::GetProcAddress_()(dll, "RetrieveUrlCacheEntryStreamW");
        RunOnceUrlCache = KernelBaseProxy::GetProcAddress_()(dll, "RunOnceUrlCache");
        SetUrlCacheConfigInfoA = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheConfigInfoA");
        SetUrlCacheConfigInfoW = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheConfigInfoW");
        SetUrlCacheEntryGroup = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheEntryGroup");
        SetUrlCacheEntryGroupA = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheEntryGroupA");
        SetUrlCacheEntryGroupW = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheEntryGroupW");
        SetUrlCacheEntryInfoA = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheEntryInfoA");
        SetUrlCacheEntryInfoW = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheEntryInfoW");
        SetUrlCacheGroupAttributeA = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheGroupAttributeA");
        SetUrlCacheGroupAttributeW = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheGroupAttributeW");
        SetUrlCacheHeaderData = KernelBaseProxy::GetProcAddress_()(dll, "SetUrlCacheHeaderData");
        ShowCertificate = KernelBaseProxy::GetProcAddress_()(dll, "ShowCertificate");
        ShowClientAuthCerts = KernelBaseProxy::GetProcAddress_()(dll, "ShowClientAuthCerts");
        ShowSecurityInfo = KernelBaseProxy::GetProcAddress_()(dll, "ShowSecurityInfo");
        ShowX509EncodedCertificate = KernelBaseProxy::GetProcAddress_()(dll, "ShowX509EncodedCertificate");
        UnlockUrlCacheEntryFile = KernelBaseProxy::GetProcAddress_()(dll, "UnlockUrlCacheEntryFile");
        UnlockUrlCacheEntryFileA = KernelBaseProxy::GetProcAddress_()(dll, "UnlockUrlCacheEntryFileA");
        UnlockUrlCacheEntryFileW = KernelBaseProxy::GetProcAddress_()(dll, "UnlockUrlCacheEntryFileW");
        UnlockUrlCacheEntryStream = KernelBaseProxy::GetProcAddress_()(dll, "UnlockUrlCacheEntryStream");
        UpdateUrlCacheContentPath = KernelBaseProxy::GetProcAddress_()(dll, "UpdateUrlCacheContentPath");
        UrlCacheCheckEntriesExist = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheCheckEntriesExist");
        UrlCacheCloseEntryHandle = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheCloseEntryHandle");
        UrlCacheContainerSetEntryMaximumAge = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheContainerSetEntryMaximumAge");
        UrlCacheCreateContainer = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheCreateContainer");
        UrlCacheFindFirstEntry = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheFindFirstEntry");
        UrlCacheFindNextEntry = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheFindNextEntry");
        UrlCacheFreeEntryInfo = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheFreeEntryInfo");
        UrlCacheFreeGlobalSpace = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheFreeGlobalSpace");
        UrlCacheGetContentPaths = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheGetContentPaths");
        UrlCacheGetEntryInfo = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheGetEntryInfo");
        UrlCacheGetGlobalCacheSize = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheGetGlobalCacheSize");
        UrlCacheGetGlobalLimit = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheGetGlobalLimit");
        UrlCacheReadEntryStream = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheReadEntryStream");
        UrlCacheReloadSettings = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheReloadSettings");
        UrlCacheRetrieveEntryFile = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheRetrieveEntryFile");
        UrlCacheRetrieveEntryStream = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheRetrieveEntryStream");
        UrlCacheServer = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheServer");
        UrlCacheSetGlobalLimit = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheSetGlobalLimit");
        UrlCacheUpdateEntryExtraData = KernelBaseProxy::GetProcAddress_()(dll, "UrlCacheUpdateEntryExtraData");
        UrlZonesDetach = KernelBaseProxy::GetProcAddress_()(dll, "UrlZonesDetach");
    }
} wininet;


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