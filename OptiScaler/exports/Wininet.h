#pragma once
#include "shared.h"

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
    FARPROC GetFileExtensionFromUrl;
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
        GetFileExtensionFromUrl = GetProcAddress(dll, "GetFileExtensionFromUrl");
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
void __GetFileExtensionFromUrl() { wininet.GetFileExtensionFromUrl(); }
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