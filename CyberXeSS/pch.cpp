#include "pch.h"

#ifdef LOGGING_ACTIVE

std::ofstream ofs;
log_level_t logLevel = LEVEL_NONE;

void logprintf(std::string logMsg, log_level_t level)
{
	if (logLevel == LEVEL_NONE || level < logLevel)
		return;

	ofs << GetTicks() << ": " << logMsg << '\n';
	ofs.flush();
}

void prepareOfs(std::string fileName, log_level_t level)
{
	logLevel = level;
	ofs = std::ofstream(fileName, std::ios_base::out | std::ios_base::app);
}

void closeOfs()
{
	ofs.close();
}

#endif