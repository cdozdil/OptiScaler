#pragma once
#include "framework.h"

//#define LOGGING_ACTIVE 
//#define D3D11on12

typedef enum _log_level_t
{
	LEVEL_NONE = -1,
	LEVEL_DEBUG = 0,
	LEVEL_INFO = 1,
	LEVEL_WARNING = 2,
	LEVEL_ERROR = 3
} log_level_t;

#define SAFE_RELEASE(p) \
  do                    \
  {                     \
	if(p)               \
	{                   \
	  (p)->Release();   \
	  (p) = NULL;       \
	}                   \
  } while((void)0, 0)

#ifdef LOGGING_ACTIVE
#define LOG(string, ...) logprintf(string, __VA_ARGS__)
#else
#define LOG(string, level)
#endif

#ifdef LOGGING_ACTIVE

#include <string>
#include <fstream>
#include "Config.h"

void logprintf(std::string logMsg, log_level_t level = LEVEL_DEBUG);

void prepareOfs(std::string fileName, log_level_t level);

void closeOfs();

static inline int64_t GetTicks()
{
	LARGE_INTEGER ticks;
	if (!QueryPerformanceCounter(&ticks))
		return 0;

	return ticks.QuadPart;
}

template< typename T >
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

#endif