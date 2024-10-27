#pragma once

#include "../detours/detours.h"
#include "NvApiTypes.h"

class ReflexHooks {
	inline static bool _inited = false;
	inline static uint32_t _minimumIntervalUs = 0;
	inline static NV_SET_SLEEP_MODE_PARAMS _lastSleepParams{};
	inline static IUnknown* _lastSleepDev = nullptr;
	inline static bool _markersPresent = false;

	inline static decltype(&NvAPI_D3D_SetSleepMode) o_NvAPI_D3D_SetSleepMode = nullptr;
	inline static decltype(&NvAPI_D3D_Sleep) o_NvAPI_D3D_Sleep = nullptr;
	inline static decltype(&NvAPI_D3D_GetLatency) o_NvAPI_D3D_GetLatency = nullptr;
	inline static decltype(&NvAPI_D3D_SetLatencyMarker) o_NvAPI_D3D_SetLatencyMarker = nullptr;
	inline static decltype(&NvAPI_D3D12_SetAsyncFrameMarker) o_NvAPI_D3D12_SetAsyncFrameMarker = nullptr;

	inline static NvAPI_Status hNvAPI_D3D_SetSleepMode(IUnknown* pDev, NV_SET_SLEEP_MODE_PARAMS* pSetSleepModeParams) {
		LOG_FUNC();

		// Store for later so we can adjust the fps whenever we want
		memcpy(&_lastSleepParams, pSetSleepModeParams, sizeof(NV_SET_SLEEP_MODE_PARAMS));
		_lastSleepDev = pDev;

		if (_minimumIntervalUs != 0)
			pSetSleepModeParams->minimumIntervalUs = _minimumIntervalUs;

		return o_NvAPI_D3D_SetSleepMode(pDev, pSetSleepModeParams);
	}

	inline static NvAPI_Status hNvAPI_D3D_Sleep(IUnknown* pDev) {
		LOG_FUNC();
		return o_NvAPI_D3D_Sleep(pDev);
	}

	inline static NvAPI_Status hNvAPI_D3D_GetLatency(IUnknown* pDev, NV_LATENCY_RESULT_PARAMS* pGetLatencyParams) {
		LOG_FUNC();
		return o_NvAPI_D3D_GetLatency(pDev, pGetLatencyParams);
	}

	inline static NvAPI_Status hNvAPI_D3D_SetLatencyMarker(IUnknown* pDev, NV_LATENCY_MARKER_PARAMS* pSetLatencyMarkerParams) {
		LOG_FUNC();
		_markersPresent = true;
		return o_NvAPI_D3D_SetLatencyMarker(pDev, pSetLatencyMarkerParams);
	}

	inline static NvAPI_Status hNvAPI_D3D12_SetAsyncFrameMarker(ID3D12CommandQueue* pCommandQueue, NV_ASYNC_FRAME_MARKER_PARAMS* pSetAsyncFrameMarkerParams) {
		LOG_FUNC();
		return o_NvAPI_D3D12_SetAsyncFrameMarker(pCommandQueue, pSetAsyncFrameMarkerParams);
	}

public:
	inline static void* hookReflex(NvApiTypes::PFN_NvApi_QueryInterface &queryInterface, NvApiTypes::NV_INTERFACE InterfaceId) {
		LOG_FUNC();

		if (o_NvAPI_D3D_SetSleepMode == nullptr || o_NvAPI_D3D_Sleep == nullptr || o_NvAPI_D3D_GetLatency == nullptr || o_NvAPI_D3D_SetLatencyMarker == nullptr || o_NvAPI_D3D12_SetAsyncFrameMarker == nullptr) {
			o_NvAPI_D3D_SetSleepMode = static_cast<decltype(&NvAPI_D3D_SetSleepMode)>(queryInterface(NvApiTypes::NV_INTERFACE::D3D_SetSleepMode));
			o_NvAPI_D3D_Sleep = static_cast<decltype(&NvAPI_D3D_Sleep)>(queryInterface(NvApiTypes::NV_INTERFACE::D3D_Sleep));
			o_NvAPI_D3D_GetLatency = static_cast<decltype(&NvAPI_D3D_GetLatency)>(queryInterface(NvApiTypes::NV_INTERFACE::D3D_GetLatency));
			o_NvAPI_D3D_SetLatencyMarker = static_cast<decltype(&NvAPI_D3D_SetLatencyMarker)>(queryInterface(NvApiTypes::NV_INTERFACE::D3D_SetLatencyMarker));
			o_NvAPI_D3D12_SetAsyncFrameMarker = static_cast<decltype(&NvAPI_D3D12_SetAsyncFrameMarker)>(queryInterface(NvApiTypes::NV_INTERFACE::D3D12_SetAsyncFrameMarker));
		}

		switch (InterfaceId) {
		case NvApiTypes::NV_INTERFACE::D3D_SetSleepMode:
			if (o_NvAPI_D3D_SetSleepMode != nullptr)
				return &hNvAPI_D3D_SetSleepMode;
			break;
		case NvApiTypes::NV_INTERFACE::D3D_Sleep:
			if (o_NvAPI_D3D_Sleep != nullptr)
				return &hNvAPI_D3D_Sleep;
			break;
		case NvApiTypes::NV_INTERFACE::D3D_GetLatency:
			if (o_NvAPI_D3D_GetLatency != nullptr)
				return &hNvAPI_D3D_GetLatency;
			break;
		case NvApiTypes::NV_INTERFACE::D3D_SetLatencyMarker:
			if (o_NvAPI_D3D_SetLatencyMarker != nullptr)
				return &hNvAPI_D3D_SetLatencyMarker;
			break;
		case NvApiTypes::NV_INTERFACE::D3D12_SetAsyncFrameMarker:
			if (o_NvAPI_D3D12_SetAsyncFrameMarker != nullptr)
				return &hNvAPI_D3D12_SetAsyncFrameMarker;
			break;
		}

		return nullptr;
	}

	// For updating information about Reflex hooks
	inline static void update(bool fgState) {
		// Not a perfect check, doesn't confirm that Reflex is actually currently enabled
		// This is intentional as fakenvapi might override what the game sends
		Config::Instance()->ReflexAvailable = _markersPresent;

		static float lastFps = 0;
		float currentFps = Config::Instance()->FramerateLimit.value_or(200);

		if (fgState)
			currentFps = currentFps / 2;

		if (Config::Instance()->FramerateLimit.has_value() && (currentFps != lastFps)) {
			setFPSLimit(currentFps);
			lastFps = currentFps;
		}
	}

	// 0 - disables the fps cap
	inline static void setFPSLimit(float fps) {
		if (fps == 0.0)
			_minimumIntervalUs = 0;
		else
			_minimumIntervalUs = 1'000'000 / fps;
		if (_lastSleepDev != nullptr)
			hNvAPI_D3D_SetSleepMode(_lastSleepDev, &_lastSleepParams);
	}
};