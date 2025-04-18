#pragma once

#include <pch.h>

#include "shared.h"

#include <proxies/KernelBase_Proxy.h>

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
        
        CloseDriver = KernelBaseProxy::GetProcAddress_()(dll, "CloseDriver");
        DefDriverProc = KernelBaseProxy::GetProcAddress_()(dll, "DefDriverProc");
        DriverCallback = KernelBaseProxy::GetProcAddress_()(dll, "DriverCallback");
        DrvGetModuleHandle = KernelBaseProxy::GetProcAddress_()(dll, "DrvGetModuleHandle");
        GetDriverModuleHandle = KernelBaseProxy::GetProcAddress_()(dll, "GetDriverModuleHandle");
        NotifyCallbackData = KernelBaseProxy::GetProcAddress_()(dll, "NotifyCallbackData");
        OpenDriver = KernelBaseProxy::GetProcAddress_()(dll, "OpenDriver");
        PlaySound = KernelBaseProxy::GetProcAddress_()(dll, "PlaySound");
        PlaySoundA = KernelBaseProxy::GetProcAddress_()(dll, "PlaySoundA");
        PlaySoundW = KernelBaseProxy::GetProcAddress_()(dll, "PlaySoundW");
        SendDriverMessage = KernelBaseProxy::GetProcAddress_()(dll, "SendDriverMessage");
        WOW32DriverCallback = KernelBaseProxy::GetProcAddress_()(dll, "WOW32DriverCallback");
        WOW32ResolveMultiMediaHandle = KernelBaseProxy::GetProcAddress_()(dll, "WOW32ResolveMultiMediaHandle");
        WOWAppExit = KernelBaseProxy::GetProcAddress_()(dll, "WOWAppExit");
        aux32Message = KernelBaseProxy::GetProcAddress_()(dll, "aux32Message");
        auxGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "auxGetDevCapsA");
        auxGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "auxGetDevCapsW");
        auxGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "auxGetNumDevs");
        auxGetVolume = KernelBaseProxy::GetProcAddress_()(dll, "auxGetVolume");
        auxOutMessage = KernelBaseProxy::GetProcAddress_()(dll, "auxOutMessage");
        auxSetVolume = KernelBaseProxy::GetProcAddress_()(dll, "auxSetVolume");
        joy32Message = KernelBaseProxy::GetProcAddress_()(dll, "joy32Message");
        joyConfigChanged = KernelBaseProxy::GetProcAddress_()(dll, "joyConfigChanged");
        joyGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "joyGetDevCapsA");
        joyGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "joyGetDevCapsW");
        joyGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "joyGetNumDevs");
        joyGetPos = KernelBaseProxy::GetProcAddress_()(dll, "joyGetPos");
        joyGetPosEx = KernelBaseProxy::GetProcAddress_()(dll, "joyGetPosEx");
        joyGetThreshold = KernelBaseProxy::GetProcAddress_()(dll, "joyGetThreshold");
        joyReleaseCapture = KernelBaseProxy::GetProcAddress_()(dll, "joyReleaseCapture");
        joySetCapture = KernelBaseProxy::GetProcAddress_()(dll, "joySetCapture");
        joySetThreshold = KernelBaseProxy::GetProcAddress_()(dll, "joySetThreshold");
        mci32Message = KernelBaseProxy::GetProcAddress_()(dll, "mci32Message");
        mciDriverNotify = KernelBaseProxy::GetProcAddress_()(dll, "mciDriverNotify");
        mciDriverYield = KernelBaseProxy::GetProcAddress_()(dll, "mciDriverYield");
        mciExecute = KernelBaseProxy::GetProcAddress_()(dll, "mciExecute");
        mciFreeCommandResource = KernelBaseProxy::GetProcAddress_()(dll, "mciFreeCommandResource");
        mciGetCreatorTask = KernelBaseProxy::GetProcAddress_()(dll, "mciGetCreatorTask");
        mciGetDeviceIDA = KernelBaseProxy::GetProcAddress_()(dll, "mciGetDeviceIDA");
        mciGetDeviceIDFromElementIDA = KernelBaseProxy::GetProcAddress_()(dll, "mciGetDeviceIDFromElementIDA");
        mciGetDeviceIDFromElementIDW = KernelBaseProxy::GetProcAddress_()(dll, "mciGetDeviceIDFromElementIDW");
        mciGetDeviceIDW = KernelBaseProxy::GetProcAddress_()(dll, "mciGetDeviceIDW");
        mciGetDriverData = KernelBaseProxy::GetProcAddress_()(dll, "mciGetDriverData");
        mciGetErrorStringA = KernelBaseProxy::GetProcAddress_()(dll, "mciGetErrorStringA");
        mciGetErrorStringW = KernelBaseProxy::GetProcAddress_()(dll, "mciGetErrorStringW");
        mciGetYieldProc = KernelBaseProxy::GetProcAddress_()(dll, "mciGetYieldProc");
        mciLoadCommandResource = KernelBaseProxy::GetProcAddress_()(dll, "mciLoadCommandResource");
        mciSendCommandA = KernelBaseProxy::GetProcAddress_()(dll, "mciSendCommandA");
        mciSendCommandW = KernelBaseProxy::GetProcAddress_()(dll, "mciSendCommandW");
        mciSendStringA = KernelBaseProxy::GetProcAddress_()(dll, "mciSendStringA");
        mciSendStringW = KernelBaseProxy::GetProcAddress_()(dll, "mciSendStringW");
        mciSetDriverData = KernelBaseProxy::GetProcAddress_()(dll, "mciSetDriverData");
        mciSetYieldProc = KernelBaseProxy::GetProcAddress_()(dll, "mciSetYieldProc");
        mid32Message = KernelBaseProxy::GetProcAddress_()(dll, "mid32Message");
        midiConnect = KernelBaseProxy::GetProcAddress_()(dll, "midiConnect");
        midiDisconnect = KernelBaseProxy::GetProcAddress_()(dll, "midiDisconnect");
        midiInAddBuffer = KernelBaseProxy::GetProcAddress_()(dll, "midiInAddBuffer");
        midiInClose = KernelBaseProxy::GetProcAddress_()(dll, "midiInClose");
        midiInGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetDevCapsA");
        midiInGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetDevCapsW");
        midiInGetErrorTextA = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetErrorTextA");
        midiInGetErrorTextW = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetErrorTextW");
        midiInGetID = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetID");
        midiInGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "midiInGetNumDevs");
        midiInMessage = KernelBaseProxy::GetProcAddress_()(dll, "midiInMessage");
        midiInOpen = KernelBaseProxy::GetProcAddress_()(dll, "midiInOpen");
        midiInPrepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "midiInPrepareHeader");
        midiInReset = KernelBaseProxy::GetProcAddress_()(dll, "midiInReset");
        midiInStart = KernelBaseProxy::GetProcAddress_()(dll, "midiInStart");
        midiInStop = KernelBaseProxy::GetProcAddress_()(dll, "midiInStop");
        midiInUnprepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "midiInUnprepareHeader");
        midiOutCacheDrumPatches = KernelBaseProxy::GetProcAddress_()(dll, "midiOutCacheDrumPatches");
        midiOutCachePatches = KernelBaseProxy::GetProcAddress_()(dll, "midiOutCachePatches");
        midiOutClose = KernelBaseProxy::GetProcAddress_()(dll, "midiOutClose");
        midiOutGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetDevCapsA");
        midiOutGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetDevCapsW");
        midiOutGetErrorTextA = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetErrorTextA");
        midiOutGetErrorTextW = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetErrorTextW");
        midiOutGetID = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetID");
        midiOutGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetNumDevs");
        midiOutGetVolume = KernelBaseProxy::GetProcAddress_()(dll, "midiOutGetVolume");
        midiOutLongMsg = KernelBaseProxy::GetProcAddress_()(dll, "midiOutLongMsg");
        midiOutMessage = KernelBaseProxy::GetProcAddress_()(dll, "midiOutMessage");
        midiOutOpen = KernelBaseProxy::GetProcAddress_()(dll, "midiOutOpen");
        midiOutPrepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "midiOutPrepareHeader");
        midiOutReset = KernelBaseProxy::GetProcAddress_()(dll, "midiOutReset");
        midiOutSetVolume = KernelBaseProxy::GetProcAddress_()(dll, "midiOutSetVolume");
        midiOutShortMsg = KernelBaseProxy::GetProcAddress_()(dll, "midiOutShortMsg");
        midiOutUnprepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "midiOutUnprepareHeader");
        midiStreamClose = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamClose");
        midiStreamOpen = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamOpen");
        midiStreamOut = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamOut");
        midiStreamPause = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamPause");
        midiStreamPosition = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamPosition");
        midiStreamProperty = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamProperty");
        midiStreamRestart = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamRestart");
        midiStreamStop = KernelBaseProxy::GetProcAddress_()(dll, "midiStreamStop");
        mixerClose = KernelBaseProxy::GetProcAddress_()(dll, "mixerClose");
        mixerGetControlDetailsA = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetControlDetailsA");
        mixerGetControlDetailsW = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetControlDetailsW");
        mixerGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetDevCapsA");
        mixerGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetDevCapsW");
        mixerGetID = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetID");
        mixerGetLineControlsA = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetLineControlsA");
        mixerGetLineControlsW = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetLineControlsW");
        mixerGetLineInfoA = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetLineInfoA");
        mixerGetLineInfoW = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetLineInfoW");
        mixerGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "mixerGetNumDevs");
        mixerMessage = KernelBaseProxy::GetProcAddress_()(dll, "mixerMessage");
        mixerOpen = KernelBaseProxy::GetProcAddress_()(dll, "mixerOpen");
        mixerSetControlDetails = KernelBaseProxy::GetProcAddress_()(dll, "mixerSetControlDetails");
        mmDrvInstall = KernelBaseProxy::GetProcAddress_()(dll, "mmDrvInstall");
        mmGetCurrentTask = KernelBaseProxy::GetProcAddress_()(dll, "mmGetCurrentTask");
        mmTaskBlock = KernelBaseProxy::GetProcAddress_()(dll, "mmTaskBlock");
        mmTaskCreate = KernelBaseProxy::GetProcAddress_()(dll, "mmTaskCreate");
        mmTaskSignal = KernelBaseProxy::GetProcAddress_()(dll, "mmTaskSignal");
        mmTaskYield = KernelBaseProxy::GetProcAddress_()(dll, "mmTaskYield");
        mmioAdvance = KernelBaseProxy::GetProcAddress_()(dll, "mmioAdvance");
        mmioAscend = KernelBaseProxy::GetProcAddress_()(dll, "mmioAscend");
        mmioClose = KernelBaseProxy::GetProcAddress_()(dll, "mmioClose");
        mmioCreateChunk = KernelBaseProxy::GetProcAddress_()(dll, "mmioCreateChunk");
        mmioDescend = KernelBaseProxy::GetProcAddress_()(dll, "mmioDescend");
        mmioFlush = KernelBaseProxy::GetProcAddress_()(dll, "mmioFlush");
        mmioGetInfo = KernelBaseProxy::GetProcAddress_()(dll, "mmioGetInfo");
        mmioInstallIOProcA = KernelBaseProxy::GetProcAddress_()(dll, "mmioInstallIOProcA");
        mmioInstallIOProcW = KernelBaseProxy::GetProcAddress_()(dll, "mmioInstallIOProcW");
        mmioOpenA = KernelBaseProxy::GetProcAddress_()(dll, "mmioOpenA");
        mmioOpenW = KernelBaseProxy::GetProcAddress_()(dll, "mmioOpenW");
        mmioRead = KernelBaseProxy::GetProcAddress_()(dll, "mmioRead");
        mmioRenameA = KernelBaseProxy::GetProcAddress_()(dll, "mmioRenameA");
        mmioRenameW = KernelBaseProxy::GetProcAddress_()(dll, "mmioRenameW");
        mmioSeek = KernelBaseProxy::GetProcAddress_()(dll, "mmioSeek");
        mmioSendMessage = KernelBaseProxy::GetProcAddress_()(dll, "mmioSendMessage");
        mmioSetBuffer = KernelBaseProxy::GetProcAddress_()(dll, "mmioSetBuffer");
        mmioSetInfo = KernelBaseProxy::GetProcAddress_()(dll, "mmioSetInfo");
        mmioStringToFOURCCA = KernelBaseProxy::GetProcAddress_()(dll, "mmioStringToFOURCCA");
        mmioStringToFOURCCW = KernelBaseProxy::GetProcAddress_()(dll, "mmioStringToFOURCCW");
        mmioWrite = KernelBaseProxy::GetProcAddress_()(dll, "mmioWrite");
        mmsystemGetVersion = KernelBaseProxy::GetProcAddress_()(dll, "mmsystemGetVersion");
        mod32Message = KernelBaseProxy::GetProcAddress_()(dll, "mod32Message");
        mxd32Message = KernelBaseProxy::GetProcAddress_()(dll, "mxd32Message");
        sndPlaySoundA = KernelBaseProxy::GetProcAddress_()(dll, "sndPlaySoundA");
        sndPlaySoundW = KernelBaseProxy::GetProcAddress_()(dll, "sndPlaySoundW");
        tid32Message = KernelBaseProxy::GetProcAddress_()(dll, "tid32Message");
        timeBeginPeriod = KernelBaseProxy::GetProcAddress_()(dll, "timeBeginPeriod");
        timeEndPeriod = KernelBaseProxy::GetProcAddress_()(dll, "timeEndPeriod");
        timeGetDevCaps = KernelBaseProxy::GetProcAddress_()(dll, "timeGetDevCaps");
        timeGetSystemTime = KernelBaseProxy::GetProcAddress_()(dll, "timeGetSystemTime");
        timeGetTime = KernelBaseProxy::GetProcAddress_()(dll, "timeGetTime");
        timeKillEvent = KernelBaseProxy::GetProcAddress_()(dll, "timeKillEvent");
        timeSetEvent = KernelBaseProxy::GetProcAddress_()(dll, "timeSetEvent");
        waveInAddBuffer = KernelBaseProxy::GetProcAddress_()(dll, "waveInAddBuffer");
        waveInClose = KernelBaseProxy::GetProcAddress_()(dll, "waveInClose");
        waveInGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetDevCapsA");
        waveInGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetDevCapsW");
        waveInGetErrorTextA = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetErrorTextA");
        waveInGetErrorTextW = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetErrorTextW");
        waveInGetID = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetID");
        waveInGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetNumDevs");
        waveInGetPosition = KernelBaseProxy::GetProcAddress_()(dll, "waveInGetPosition");
        waveInMessage = KernelBaseProxy::GetProcAddress_()(dll, "waveInMessage");
        waveInOpen = KernelBaseProxy::GetProcAddress_()(dll, "waveInOpen");
        waveInPrepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "waveInPrepareHeader");
        waveInReset = KernelBaseProxy::GetProcAddress_()(dll, "waveInReset");
        waveInStart = KernelBaseProxy::GetProcAddress_()(dll, "waveInStart");
        waveInStop = KernelBaseProxy::GetProcAddress_()(dll, "waveInStop");
        waveInUnprepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "waveInUnprepareHeader");
        waveOutBreakLoop = KernelBaseProxy::GetProcAddress_()(dll, "waveOutBreakLoop");
        waveOutClose = KernelBaseProxy::GetProcAddress_()(dll, "waveOutClose");
        waveOutGetDevCapsA = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetDevCapsA");
        waveOutGetDevCapsW = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetDevCapsW");
        waveOutGetErrorTextA = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetErrorTextA");
        waveOutGetErrorTextW = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetErrorTextW");
        waveOutGetID = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetID");
        waveOutGetNumDevs = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetNumDevs");
        waveOutGetPitch = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetPitch");
        waveOutGetPlaybackRate = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetPlaybackRate");
        waveOutGetPosition = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetPosition");
        waveOutGetVolume = KernelBaseProxy::GetProcAddress_()(dll, "waveOutGetVolume");
        waveOutMessage = KernelBaseProxy::GetProcAddress_()(dll, "waveOutMessage");
        waveOutOpen = KernelBaseProxy::GetProcAddress_()(dll, "waveOutOpen");
        waveOutPause = KernelBaseProxy::GetProcAddress_()(dll, "waveOutPause");
        waveOutPrepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "waveOutPrepareHeader");
        waveOutReset = KernelBaseProxy::GetProcAddress_()(dll, "waveOutReset");
        waveOutRestart = KernelBaseProxy::GetProcAddress_()(dll, "waveOutRestart");
        waveOutSetPitch = KernelBaseProxy::GetProcAddress_()(dll, "waveOutSetPitch");
        waveOutSetPlaybackRate = KernelBaseProxy::GetProcAddress_()(dll, "waveOutSetPlaybackRate");
        waveOutSetVolume = KernelBaseProxy::GetProcAddress_()(dll, "waveOutSetVolume");
        waveOutUnprepareHeader = KernelBaseProxy::GetProcAddress_()(dll, "waveOutUnprepareHeader");
        waveOutWrite = KernelBaseProxy::GetProcAddress_()(dll, "waveOutWrite");
        wid32Message = KernelBaseProxy::GetProcAddress_()(dll, "wid32Message");
        wod32Message = KernelBaseProxy::GetProcAddress_()(dll, "wod32Message");
    }
} winmm; 

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
