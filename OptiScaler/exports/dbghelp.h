#pragma once

#include <pch.h>

#include "shared.h"

#include <proxies/KernelBase_Proxy.h>

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
        
        dbghelpvc7fpo = KernelBaseProxy::GetProcAddress_()(dll, "vc7fpo");
        dbghelpsymsrv = KernelBaseProxy::GetProcAddress_()(dll, "symsrv");
        dbghelpsym = KernelBaseProxy::GetProcAddress_()(dll, "sym");
        dbghelpstackdbg = KernelBaseProxy::GetProcAddress_()(dll, "stackdbg");
        dbghelpstack_force_ebp = KernelBaseProxy::GetProcAddress_()(dll, "stack_force_ebp");
        dbghelpsrcfiles = KernelBaseProxy::GetProcAddress_()(dll, "srcfiles");
        dbghelpoptdbgdumpaddr = KernelBaseProxy::GetProcAddress_()(dll, "optdbgdumpaddr");
        dbghelpoptdbgdump = KernelBaseProxy::GetProcAddress_()(dll, "optdbgdump");
        dbghelpomap = KernelBaseProxy::GetProcAddress_()(dll, "omap");
        dbghelplminfo = KernelBaseProxy::GetProcAddress_()(dll, "lminfo");
        dbghelplmi = KernelBaseProxy::GetProcAddress_()(dll, "lmi");
        dbghelpitoldyouso = KernelBaseProxy::GetProcAddress_()(dll, "itoldyouso");
        dbghelpinlinedbg = KernelBaseProxy::GetProcAddress_()(dll, "inlinedbg");
        dbghelphomedir = KernelBaseProxy::GetProcAddress_()(dll, "homedir");
        dbghelpfptr = KernelBaseProxy::GetProcAddress_()(dll, "fptr");
        dbghelpdh = KernelBaseProxy::GetProcAddress_()(dll, "dh");
        dbghelpdbghelp = KernelBaseProxy::GetProcAddress_()(dll, "dbghelp");
        dbghelpchksym = KernelBaseProxy::GetProcAddress_()(dll, "chksym");
        dbghelpblock = KernelBaseProxy::GetProcAddress_()(dll, "block");
        dbghelp_EFN_DumpImage = KernelBaseProxy::GetProcAddress_()(dll, "_EFN_DumpImage");
        dbghelpWinDbgExtensionDllInit = KernelBaseProxy::GetProcAddress_()(dll, "WinDbgExtensionDllInit");
        dbghelpUnDecorateSymbolNameW = KernelBaseProxy::GetProcAddress_()(dll, "UnDecorateSymbolNameW");
        dbghelpUnDecorateSymbolName = KernelBaseProxy::GetProcAddress_()(dll, "UnDecorateSymbolName");
        dbghelpSymUnloadModule64 = KernelBaseProxy::GetProcAddress_()(dll, "SymUnloadModule64");
        dbghelpSymUnloadModule = KernelBaseProxy::GetProcAddress_()(dll, "SymUnloadModule");
        dbghelpSymUnDName64 = KernelBaseProxy::GetProcAddress_()(dll, "SymUnDName64");
        dbghelpSymUnDName = KernelBaseProxy::GetProcAddress_()(dll, "SymUnDName");
        dbghelpSymSrvStoreSupplementW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvStoreSupplementW");
        dbghelpSymSrvStoreSupplement = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvStoreSupplement");
        dbghelpSymSrvStoreFileW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvStoreFileW");
        dbghelpSymSrvStoreFile = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvStoreFile");
        dbghelpSymSrvIsStoreW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvIsStoreW");
        dbghelpSymSrvIsStore = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvIsStore");
        dbghelpSymSrvGetSupplementW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetSupplementW");
        dbghelpSymSrvGetSupplement = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetSupplement");
        dbghelpSymSrvGetFileIndexesW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexesW");
        dbghelpSymSrvGetFileIndexes = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexes");
        dbghelpSymSrvGetFileIndexStringW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexStringW");
        dbghelpSymSrvGetFileIndexString = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexString");
        dbghelpSymSrvGetFileIndexInfoW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexInfoW");
        dbghelpSymSrvGetFileIndexInfo = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvGetFileIndexInfo");
        dbghelpSymSrvDeltaNameW = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvDeltaNameW");
        dbghelpSymSrvDeltaName = KernelBaseProxy::GetProcAddress_()(dll, "SymSrvDeltaName");
        dbghelpSymSetSearchPathW = KernelBaseProxy::GetProcAddress_()(dll, "SymSetSearchPathW");
        dbghelpSymSetSearchPath = KernelBaseProxy::GetProcAddress_()(dll, "SymSetSearchPath");
        dbghelpSymSetScopeFromInlineContext = KernelBaseProxy::GetProcAddress_()(dll, "SymSetScopeFromInlineContext");
        dbghelpSymSetScopeFromIndex = KernelBaseProxy::GetProcAddress_()(dll, "SymSetScopeFromIndex");
        dbghelpSymSetScopeFromAddr = KernelBaseProxy::GetProcAddress_()(dll, "SymSetScopeFromAddr");
        dbghelpSymSetParentWindow = KernelBaseProxy::GetProcAddress_()(dll, "SymSetParentWindow");
        dbghelpSymSetOptions = KernelBaseProxy::GetProcAddress_()(dll, "SymSetOptions");
        dbghelpSymSetHomeDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "SymSetHomeDirectoryW");
        dbghelpSymSetHomeDirectory = KernelBaseProxy::GetProcAddress_()(dll, "SymSetHomeDirectory");
        dbghelpSymSetExtendedOption = KernelBaseProxy::GetProcAddress_()(dll, "SymSetExtendedOption");
        dbghelpSymSetDiaSession = KernelBaseProxy::GetProcAddress_()(dll, "SymSetDiaSession");
        dbghelpSymSetContext = KernelBaseProxy::GetProcAddress_()(dll, "SymSetContext");
        dbghelpSymSearchW = KernelBaseProxy::GetProcAddress_()(dll, "SymSearchW");
        dbghelpSymSearch = KernelBaseProxy::GetProcAddress_()(dll, "SymSearch");
        dbghelpSymRegisterFunctionEntryCallback64 = KernelBaseProxy::GetProcAddress_()(dll, "SymRegisterFunctionEntryCallback64");
        dbghelpSymRegisterFunctionEntryCallback = KernelBaseProxy::GetProcAddress_()(dll, "SymRegisterFunctionEntryCallback");
        dbghelpSymRegisterCallbackW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymRegisterCallbackW64");
        dbghelpSymRegisterCallback64 = KernelBaseProxy::GetProcAddress_()(dll, "SymRegisterCallback64");
        dbghelpSymRegisterCallback = KernelBaseProxy::GetProcAddress_()(dll, "SymRegisterCallback");
        dbghelpSymRefreshModuleList = KernelBaseProxy::GetProcAddress_()(dll, "SymRefreshModuleList");
        dbghelpSymQueryInlineTrace = KernelBaseProxy::GetProcAddress_()(dll, "SymQueryInlineTrace");
        dbghelpSymPrevW = KernelBaseProxy::GetProcAddress_()(dll, "SymPrevW");
        dbghelpSymPrev = KernelBaseProxy::GetProcAddress_()(dll, "SymPrev");
        dbghelpSymNextW = KernelBaseProxy::GetProcAddress_()(dll, "SymNextW");
        dbghelpSymNext = KernelBaseProxy::GetProcAddress_()(dll, "SymNext");
        dbghelpSymMatchStringW = KernelBaseProxy::GetProcAddress_()(dll, "SymMatchStringW");
        dbghelpSymMatchStringA = KernelBaseProxy::GetProcAddress_()(dll, "SymMatchStringA");
        dbghelpSymMatchString = KernelBaseProxy::GetProcAddress_()(dll, "SymMatchString");
        dbghelpSymMatchFileNameW = KernelBaseProxy::GetProcAddress_()(dll, "SymMatchFileNameW");
        dbghelpSymMatchFileName = KernelBaseProxy::GetProcAddress_()(dll, "SymMatchFileName");
        dbghelpSymLoadModuleExW = KernelBaseProxy::GetProcAddress_()(dll, "SymLoadModuleExW");
        dbghelpSymLoadModuleEx = KernelBaseProxy::GetProcAddress_()(dll, "SymLoadModuleEx");
        dbghelpSymLoadModule64 = KernelBaseProxy::GetProcAddress_()(dll, "SymLoadModule64");
        dbghelpSymLoadModule = KernelBaseProxy::GetProcAddress_()(dll, "SymLoadModule");
        dbghelpSymInitializeW = KernelBaseProxy::GetProcAddress_()(dll, "SymInitializeW");
        dbghelpSymInitialize = KernelBaseProxy::GetProcAddress_()(dll, "SymInitialize");
        dbghelpSymGetUnwindInfo = KernelBaseProxy::GetProcAddress_()(dll, "SymGetUnwindInfo");
        dbghelpSymGetTypeInfoEx = KernelBaseProxy::GetProcAddress_()(dll, "SymGetTypeInfoEx");
        dbghelpSymGetTypeInfo = KernelBaseProxy::GetProcAddress_()(dll, "SymGetTypeInfo");
        dbghelpSymGetTypeFromNameW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetTypeFromNameW");
        dbghelpSymGetTypeFromName = KernelBaseProxy::GetProcAddress_()(dll, "SymGetTypeFromName");
        dbghelpSymGetSymbolFileW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymbolFileW");
        dbghelpSymGetSymbolFile = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymbolFile");
        dbghelpSymGetSymPrev64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymPrev64");
        dbghelpSymGetSymPrev = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymPrev");
        dbghelpSymGetSymNext64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymNext64");
        dbghelpSymGetSymNext = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymNext");
        dbghelpSymGetSymFromName64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymFromName64");
        dbghelpSymGetSymFromName = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymFromName");
        dbghelpSymGetSymFromAddr64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymFromAddr64");
        dbghelpSymGetSymFromAddr = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSymFromAddr");
        dbghelpSymGetSourceVarFromTokenW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceVarFromTokenW");
        dbghelpSymGetSourceVarFromToken = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceVarFromToken");
        dbghelpSymGetSourceFileW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileW");
        dbghelpSymGetSourceFileTokenW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileTokenW");
        dbghelpSymGetSourceFileToken = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileToken");
        dbghelpSymGetSourceFileFromTokenW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileFromTokenW");
        dbghelpSymGetSourceFileFromToken = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileFromToken");
        dbghelpSymGetSourceFileChecksumW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileChecksumW");
        dbghelpSymGetSourceFileChecksum = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFileChecksum");
        dbghelpSymGetSourceFile = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSourceFile");
        dbghelpSymGetSearchPathW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSearchPathW");
        dbghelpSymGetSearchPath = KernelBaseProxy::GetProcAddress_()(dll, "SymGetSearchPath");
        dbghelpSymGetScopeW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetScopeW");
        dbghelpSymGetScope = KernelBaseProxy::GetProcAddress_()(dll, "SymGetScope");
        dbghelpSymGetOptions = KernelBaseProxy::GetProcAddress_()(dll, "SymGetOptions");
        dbghelpSymGetOmaps = KernelBaseProxy::GetProcAddress_()(dll, "SymGetOmaps");
        dbghelpSymGetOmapBlockBase = KernelBaseProxy::GetProcAddress_()(dll, "SymGetOmapBlockBase");
        dbghelpSymGetModuleInfoW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleInfoW64");
        dbghelpSymGetModuleInfoW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleInfoW");
        dbghelpSymGetModuleInfo64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleInfo64");
        dbghelpSymGetModuleInfo = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleInfo");
        dbghelpSymGetModuleBase64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleBase64");
        dbghelpSymGetModuleBase = KernelBaseProxy::GetProcAddress_()(dll, "SymGetModuleBase");
        dbghelpSymGetLinePrevW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLinePrevW64");
        dbghelpSymGetLinePrevEx = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLinePrevEx");
        dbghelpSymGetLinePrev64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLinePrev64");
        dbghelpSymGetLinePrev = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLinePrev");
        dbghelpSymGetLineNextW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineNextW64");
        dbghelpSymGetLineNextEx = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineNextEx");
        dbghelpSymGetLineNext64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineNext64");
        dbghelpSymGetLineNext = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineNext");
        dbghelpSymGetLineFromNameW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromNameW64");
        dbghelpSymGetLineFromNameEx = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromNameEx");
        dbghelpSymGetLineFromName64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromName64");
        dbghelpSymGetLineFromName = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromName");
        dbghelpSymGetLineFromInlineContextW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromInlineContextW");
        dbghelpSymGetLineFromInlineContext = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromInlineContext");
        dbghelpSymGetLineFromAddrW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromAddrW64");
        dbghelpSymGetLineFromAddrEx = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromAddrEx");
        dbghelpSymGetLineFromAddr64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromAddr64");
        dbghelpSymGetLineFromAddr = KernelBaseProxy::GetProcAddress_()(dll, "SymGetLineFromAddr");
        dbghelpSymGetHomeDirectoryW = KernelBaseProxy::GetProcAddress_()(dll, "SymGetHomeDirectoryW");
        dbghelpSymGetHomeDirectory = KernelBaseProxy::GetProcAddress_()(dll, "SymGetHomeDirectory");
        dbghelpSymGetFileLineOffsets64 = KernelBaseProxy::GetProcAddress_()(dll, "SymGetFileLineOffsets64");
        dbghelpSymGetExtendedOption = KernelBaseProxy::GetProcAddress_()(dll, "SymGetExtendedOption");
        dbghelpSymGetDiaSession = KernelBaseProxy::GetProcAddress_()(dll, "SymGetDiaSession");
        dbghelpSymFunctionTableAccess64AccessRoutines = KernelBaseProxy::GetProcAddress_()(dll, "SymFunctionTableAccess64AccessRoutines");
        dbghelpSymFunctionTableAccess64 = KernelBaseProxy::GetProcAddress_()(dll, "SymFunctionTableAccess64");
        dbghelpSymFunctionTableAccess = KernelBaseProxy::GetProcAddress_()(dll, "SymFunctionTableAccess");
        dbghelpSymFromTokenW = KernelBaseProxy::GetProcAddress_()(dll, "SymFromTokenW");
        dbghelpSymFromToken = KernelBaseProxy::GetProcAddress_()(dll, "SymFromToken");
        dbghelpSymFromNameW = KernelBaseProxy::GetProcAddress_()(dll, "SymFromNameW");
        dbghelpSymFromName = KernelBaseProxy::GetProcAddress_()(dll, "SymFromName");
        dbghelpSymFromInlineContextW = KernelBaseProxy::GetProcAddress_()(dll, "SymFromInlineContextW");
        dbghelpSymFromInlineContext = KernelBaseProxy::GetProcAddress_()(dll, "SymFromInlineContext");
        dbghelpSymFromIndexW = KernelBaseProxy::GetProcAddress_()(dll, "SymFromIndexW");
        dbghelpSymFromIndex = KernelBaseProxy::GetProcAddress_()(dll, "SymFromIndex");
        dbghelpSymFromAddrW = KernelBaseProxy::GetProcAddress_()(dll, "SymFromAddrW");
        dbghelpSymFromAddr = KernelBaseProxy::GetProcAddress_()(dll, "SymFromAddr");
        dbghelpSymFreeDiaString = KernelBaseProxy::GetProcAddress_()(dll, "SymFreeDiaString");
        dbghelpSymFindFileInPathW = KernelBaseProxy::GetProcAddress_()(dll, "SymFindFileInPathW");
        dbghelpSymFindFileInPath = KernelBaseProxy::GetProcAddress_()(dll, "SymFindFileInPath");
        dbghelpSymFindExecutableImageW = KernelBaseProxy::GetProcAddress_()(dll, "SymFindExecutableImageW");
        dbghelpSymFindExecutableImage = KernelBaseProxy::GetProcAddress_()(dll, "SymFindExecutableImage");
        dbghelpSymFindDebugInfoFileW = KernelBaseProxy::GetProcAddress_()(dll, "SymFindDebugInfoFileW");
        dbghelpSymFindDebugInfoFile = KernelBaseProxy::GetProcAddress_()(dll, "SymFindDebugInfoFile");
        dbghelpSymEnumerateSymbolsW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateSymbolsW64");
        dbghelpSymEnumerateSymbolsW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateSymbolsW");
        dbghelpSymEnumerateSymbols64 = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateSymbols64");
        dbghelpSymEnumerateSymbols = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateSymbols");
        dbghelpSymEnumerateModulesW64 = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateModulesW64");
        dbghelpSymEnumerateModules64 = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateModules64");
        dbghelpSymEnumerateModules = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumerateModules");
        dbghelpSymEnumTypesW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumTypesW");
        dbghelpSymEnumTypesByNameW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumTypesByNameW");
        dbghelpSymEnumTypesByName = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumTypesByName");
        dbghelpSymEnumTypes = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumTypes");
        dbghelpSymEnumSymbolsW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbolsW");
        dbghelpSymEnumSymbolsForAddrW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbolsForAddrW");
        dbghelpSymEnumSymbolsForAddr = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbolsForAddr");
        dbghelpSymEnumSymbolsExW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbolsExW");
        dbghelpSymEnumSymbolsEx = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbolsEx");
        dbghelpSymEnumSymbols = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSymbols");
        dbghelpSymEnumSym = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSym");
        dbghelpSymEnumSourceLinesW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSourceLinesW");
        dbghelpSymEnumSourceLines = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSourceLines");
        dbghelpSymEnumSourceFilesW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSourceFilesW");
        dbghelpSymEnumSourceFiles = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSourceFiles");
        dbghelpSymEnumSourceFileTokens = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumSourceFileTokens");
        dbghelpSymEnumProcesses = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumProcesses");
        dbghelpSymEnumLinesW = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumLinesW");
        dbghelpSymEnumLines = KernelBaseProxy::GetProcAddress_()(dll, "SymEnumLines");
        dbghelpSymDeleteSymbolW = KernelBaseProxy::GetProcAddress_()(dll, "SymDeleteSymbolW");
        dbghelpSymDeleteSymbol = KernelBaseProxy::GetProcAddress_()(dll, "SymDeleteSymbol");
        dbghelpSymCompareInlineTrace = KernelBaseProxy::GetProcAddress_()(dll, "SymCompareInlineTrace");
        dbghelpSymCleanup = KernelBaseProxy::GetProcAddress_()(dll, "SymCleanup");
        dbghelpSymAllocDiaString = KernelBaseProxy::GetProcAddress_()(dll, "SymAllocDiaString");
        dbghelpSymAddrIncludeInlineTrace = KernelBaseProxy::GetProcAddress_()(dll, "SymAddrIncludeInlineTrace");
        dbghelpSymAddSymbolW = KernelBaseProxy::GetProcAddress_()(dll, "SymAddSymbolW");
        dbghelpSymAddSymbol = KernelBaseProxy::GetProcAddress_()(dll, "SymAddSymbol");
        dbghelpSymAddSourceStreamW = KernelBaseProxy::GetProcAddress_()(dll, "SymAddSourceStreamW");
        dbghelpSymAddSourceStreamA = KernelBaseProxy::GetProcAddress_()(dll, "SymAddSourceStreamA");
        dbghelpSymAddSourceStream = KernelBaseProxy::GetProcAddress_()(dll, "SymAddSourceStream");
        dbghelpStackWalkEx = KernelBaseProxy::GetProcAddress_()(dll, "StackWalkEx");
        dbghelpStackWalk64 = KernelBaseProxy::GetProcAddress_()(dll, "StackWalk64");
        dbghelpStackWalk = KernelBaseProxy::GetProcAddress_()(dll, "StackWalk");
        dbghelpSetSymLoadError = KernelBaseProxy::GetProcAddress_()(dll, "SetSymLoadError");
        dbghelpSetCheckUserInterruptShared = KernelBaseProxy::GetProcAddress_()(dll, "SetCheckUserInterruptShared");
        dbghelpSearchTreeForFileW = KernelBaseProxy::GetProcAddress_()(dll, "SearchTreeForFileW");
        dbghelpSearchTreeForFile = KernelBaseProxy::GetProcAddress_()(dll, "SearchTreeForFile");
        dbghelpReportSymbolLoadSummary = KernelBaseProxy::GetProcAddress_()(dll, "ReportSymbolLoadSummary");
        dbghelpRemoveInvalidModuleList = KernelBaseProxy::GetProcAddress_()(dll, "RemoveInvalidModuleList");
        dbghelpRangeMapWrite = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapWrite");
        dbghelpRangeMapRemove = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapRemove");
        dbghelpRangeMapRead = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapRead");
        dbghelpRangeMapFree = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapFree");
        dbghelpRangeMapCreate = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapCreate");
        dbghelpRangeMapAddPeImageSections = KernelBaseProxy::GetProcAddress_()(dll, "RangeMapAddPeImageSections");
        dbghelpMiniDumpWriteDump = KernelBaseProxy::GetProcAddress_()(dll, "MiniDumpWriteDump");
        dbghelpMiniDumpReadDumpStream = KernelBaseProxy::GetProcAddress_()(dll, "MiniDumpReadDumpStream");
        dbghelpMakeSureDirectoryPathExists = KernelBaseProxy::GetProcAddress_()(dll, "MakeSureDirectoryPathExists");
        dbghelpImagehlpApiVersionEx = KernelBaseProxy::GetProcAddress_()(dll, "ImagehlpApiVersionEx");
        dbghelpImagehlpApiVersion = KernelBaseProxy::GetProcAddress_()(dll, "ImagehlpApiVersion");
        dbghelpImageRvaToVa = KernelBaseProxy::GetProcAddress_()(dll, "ImageRvaToVa");
        dbghelpImageRvaToSection = KernelBaseProxy::GetProcAddress_()(dll, "ImageRvaToSection");
        dbghelpImageNtHeader = KernelBaseProxy::GetProcAddress_()(dll, "ImageNtHeader");
        dbghelpImageDirectoryEntryToDataEx = KernelBaseProxy::GetProcAddress_()(dll, "ImageDirectoryEntryToDataEx");
        dbghelpImageDirectoryEntryToData = KernelBaseProxy::GetProcAddress_()(dll, "ImageDirectoryEntryToData");
        dbghelpGetTimestampForLoadedLibrary = KernelBaseProxy::GetProcAddress_()(dll, "GetTimestampForLoadedLibrary");
        dbghelpGetSymLoadError = KernelBaseProxy::GetProcAddress_()(dll, "GetSymLoadError");
        dbghelpFindFileInSearchPath = KernelBaseProxy::GetProcAddress_()(dll, "FindFileInSearchPath");
        dbghelpFindFileInPath = KernelBaseProxy::GetProcAddress_()(dll, "FindFileInPath");
        dbghelpFindExecutableImageExW = KernelBaseProxy::GetProcAddress_()(dll, "FindExecutableImageExW");
        dbghelpFindExecutableImageEx = KernelBaseProxy::GetProcAddress_()(dll, "FindExecutableImageEx");
        dbghelpFindExecutableImage = KernelBaseProxy::GetProcAddress_()(dll, "FindExecutableImage");
        dbghelpFindDebugInfoFileExW = KernelBaseProxy::GetProcAddress_()(dll, "FindDebugInfoFileExW");
        dbghelpFindDebugInfoFileEx = KernelBaseProxy::GetProcAddress_()(dll, "FindDebugInfoFileEx");
        dbghelpFindDebugInfoFile = KernelBaseProxy::GetProcAddress_()(dll, "FindDebugInfoFile");
        dbghelpExtensionApiVersion = KernelBaseProxy::GetProcAddress_()(dll, "ExtensionApiVersion");
        dbghelpEnumerateLoadedModulesW64 = KernelBaseProxy::GetProcAddress_()(dll, "EnumerateLoadedModulesW64");
        dbghelpEnumerateLoadedModulesExW = KernelBaseProxy::GetProcAddress_()(dll, "EnumerateLoadedModulesExW");
        dbghelpEnumerateLoadedModulesEx = KernelBaseProxy::GetProcAddress_()(dll, "EnumerateLoadedModulesEx");
        dbghelpEnumerateLoadedModules64 = KernelBaseProxy::GetProcAddress_()(dll, "EnumerateLoadedModules64");
        dbghelpEnumerateLoadedModules = KernelBaseProxy::GetProcAddress_()(dll, "EnumerateLoadedModules");
        dbghelpEnumDirTreeW = KernelBaseProxy::GetProcAddress_()(dll, "EnumDirTreeW");
        dbghelpEnumDirTree = KernelBaseProxy::GetProcAddress_()(dll, "EnumDirTree");
        dbghelpDbgHelpCreateUserDumpW = KernelBaseProxy::GetProcAddress_()(dll, "DbgHelpCreateUserDumpW");
    }
} dbghelp;


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
