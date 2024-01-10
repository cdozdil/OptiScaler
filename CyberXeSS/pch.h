#pragma once
#include "framework.h"
#include "Config.h"

#define SAFE_RELEASE(p)		\
do {						\
	if(p && p != nullptr)	\
	{						\
		(p)->Release();		\
		(p) = nullptr;		\
	}						\
} while((void)0, 0);		

#define LOG(string, ...) AddLog(string, __VA_ARGS__)

void AddLog(std::string logMsg, spdlog::level::level_enum level = spdlog::level::debug);

void PrepareLogger();

void CloseLogger();


static inline int64_t GetTicks()
{
	LARGE_INTEGER ticks;
	if (!QueryPerformanceCounter(&ticks))
		return 0;

	return ticks.QuadPart;
}

template<typename T>
static inline std::string int_to_hex(T i)
{
	std::stringstream stream;
	stream << "0x"
		<< std::setfill('0')
		<< std::setw(sizeof(T) * 2)
		<< std::hex << i;
	return stream.str();
}

static inline std::string ToString(REFIID guid)
{
	char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator

	snprintf(
		guid_string, sizeof(guid_string),
		"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return guid_string;
}
