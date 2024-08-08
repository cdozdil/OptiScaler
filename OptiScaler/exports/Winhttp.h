#pragma once
#include "shared.h"

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
    }
} winhttp;

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