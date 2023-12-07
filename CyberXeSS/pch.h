#pragma once
#include "framework.h"

//#define LOGGING_ACTIVE 

typedef enum _log_level_t
{
	LEVEL_NONE = -1,
	LEVEL_DEBUG = 0,
	LEVEL_INFO = 1,
	LEVEL_WARNING = 2,
	LEVEL_ERROR = 3
} log_level_t;


#ifdef LOGGING_ACTIVE
#define LOG(string, level) logprintf(string, level)
#else
#define LOG(string, level)
#endif

#ifdef LOGGING_ACTIVE

#include <string>
#include <fstream>
#include "Config.h"

void logprintf(std::string logMsg, log_level_t level);

void prepareOfs(std::string fileName, log_level_t level);

void closeOfs();

static inline int64_t GetTicks()
{
	LARGE_INTEGER ticks;
	if (!QueryPerformanceCounter(&ticks))
		return 0;

	return ticks.QuadPart;
}

#endif