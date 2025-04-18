#pragma once

#include <pch.h>

#include "shared.h"

#include <proxies/KernelBase_Proxy.h>

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
        
        Private1 = KernelBaseProxy::GetProcAddress_()(dll, "Private1");
        SvchostPushServiceGlobals = KernelBaseProxy::GetProcAddress_()(dll, "SvchostPushServiceGlobals");
        WinHttpAddRequestHeaders = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpAddRequestHeaders");
        WinHttpAddRequestHeadersEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpAddRequestHeadersEx");
        WinHttpAutoProxySvcMain = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpAutoProxySvcMain");
        WinHttpCheckPlatform = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpCheckPlatform");
        WinHttpCloseHandle = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpCloseHandle");
        WinHttpConnect = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnect");
        WinHttpConnectionDeletePolicyEntries = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionDeletePolicyEntries");
        WinHttpConnectionDeleteProxyInfo = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionDeleteProxyInfo");
        WinHttpConnectionFreeNameList = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionFreeNameList");
        WinHttpConnectionFreeProxyInfo = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionFreeProxyInfo");
        WinHttpConnectionFreeProxyList = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionFreeProxyList");
        WinHttpConnectionGetNameList = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionGetNameList");
        WinHttpConnectionGetProxyInfo = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionGetProxyInfo");
        WinHttpConnectionGetProxyList = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionGetProxyList");
        WinHttpConnectionOnlyConvert = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionOnlyConvert");
        WinHttpConnectionOnlyReceive = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionOnlyReceive");
        WinHttpConnectionOnlySend = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionOnlySend");
        WinHttpConnectionSetPolicyEntries = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionSetPolicyEntries");
        WinHttpConnectionSetProxyInfo = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionSetProxyInfo");
        WinHttpConnectionUpdateIfIndexTable = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpConnectionUpdateIfIndexTable");
        WinHttpCrackUrl = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpCrackUrl");
        WinHttpCreateProxyResolver = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpCreateProxyResolver");
        WinHttpCreateUrl = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpCreateUrl");
        WinHttpDetectAutoProxyConfigUrl = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpDetectAutoProxyConfigUrl");
        WinHttpFreeProxyResult = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpFreeProxyResult");
        WinHttpFreeProxyResultEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpFreeProxyResultEx");
        WinHttpFreeProxySettings = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpFreeProxySettings");
        WinHttpFreeProxySettingsEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpFreeProxySettingsEx");
        WinHttpFreeQueryConnectionGroupResult = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpFreeQueryConnectionGroupResult");
        WinHttpGetDefaultProxyConfiguration = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetDefaultProxyConfiguration");
        WinHttpGetIEProxyConfigForCurrentUser = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetIEProxyConfigForCurrentUser");
        WinHttpGetProxyForUrl = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyForUrl");
        WinHttpGetProxyForUrlEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyForUrlEx");
        WinHttpGetProxyForUrlEx2 = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyForUrlEx2");
        WinHttpGetProxyForUrlHvsi = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyForUrlHvsi");
        WinHttpGetProxyResult = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyResult");
        WinHttpGetProxyResultEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxyResultEx");
        WinHttpGetProxySettingsEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxySettingsEx");
        WinHttpGetProxySettingsResultEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxySettingsResultEx");
        WinHttpGetProxySettingsVersion = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetProxySettingsVersion");
        WinHttpGetTunnelSocket = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpGetTunnelSocket");
        WinHttpOpen = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpOpen");
        WinHttpOpenRequest = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpOpenRequest");
        WinHttpPacJsWorkerMain = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpPacJsWorkerMain");
        WinHttpProbeConnectivity = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpProbeConnectivity");
        WinHttpQueryAuthSchemes = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryAuthSchemes");
        WinHttpQueryConnectionGroup = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryConnectionGroup");
        WinHttpQueryDataAvailable = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryDataAvailable");
        WinHttpQueryHeaders = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryHeaders");
        WinHttpQueryHeadersEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryHeadersEx");
        WinHttpQueryOption = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpQueryOption");
        WinHttpReadData = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpReadData");
        WinHttpReadDataEx = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpReadDataEx");
        WinHttpReadProxySettings = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpReadProxySettings");
        WinHttpReadProxySettingsHvsi = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpReadProxySettingsHvsi");
        WinHttpReceiveResponse = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpReceiveResponse");
        WinHttpRegisterProxyChangeNotification = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpRegisterProxyChangeNotification");
        WinHttpResetAutoProxy = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpResetAutoProxy");
        WinHttpSaveProxyCredentials = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSaveProxyCredentials");
        WinHttpSendRequest = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSendRequest");
        WinHttpSetCredentials = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetCredentials");
        WinHttpSetDefaultProxyConfiguration = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetDefaultProxyConfiguration");
        WinHttpSetOption = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetOption");
        WinHttpSetProxySettingsPerUser = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetProxySettingsPerUser");
        WinHttpSetSecureLegacyServersAppCompat = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetSecureLegacyServersAppCompat");
        WinHttpSetStatusCallback = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetStatusCallback");
        WinHttpSetTimeouts = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpSetTimeouts");
        WinHttpTimeFromSystemTime = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpTimeFromSystemTime");
        WinHttpTimeToSystemTime = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpTimeToSystemTime");
        WinHttpUnregisterProxyChangeNotification = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpUnregisterProxyChangeNotification");
        WinHttpWebSocketClose = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketClose");
        WinHttpWebSocketCompleteUpgrade = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketCompleteUpgrade");
        WinHttpWebSocketQueryCloseStatus = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketQueryCloseStatus");
        WinHttpWebSocketReceive = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketReceive");
        WinHttpWebSocketSend = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketSend");
        WinHttpWebSocketShutdown = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWebSocketShutdown");
        WinHttpWriteData = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWriteData");
        WinHttpWriteProxySettings = KernelBaseProxy::GetProcAddress_()(dll, "WinHttpWriteProxySettings");
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
