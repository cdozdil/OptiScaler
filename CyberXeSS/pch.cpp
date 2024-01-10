#include "pch.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

std::shared_ptr<spdlog::logger> logger = nullptr;

bool InitializeConsole()
{
	// Allocate a console for this app
	if (!AllocConsole()) {
		return false;
	}

	FILE* pFile;

	// Redirect STDIN if the console has an input handle
	if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE) {
		if (freopen_s(&pFile, "CONIN$", "r", stdin) != 0) {
			return false;
		}
	}

	// Redirect STDOUT if the console has an output handle
	if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) {
		if (freopen_s(&pFile, "CONOUT$", "w", stdout) != 0) {
			return false;
		}
	}

	// Redirect STDERR if the console has an error handle
	if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
		if (freopen_s(&pFile, "CONOUT$", "w", stderr) != 0) {
			return false;
		}
	}

	// Clear the error state for each of the C++ standard streams
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::wcout.clear();
	std::wcerr.clear();

	// Make C++ standard streams point to console as well.
	std::ios::sync_with_stdio();

	return true;
}


void AddLog(const std::string logMsg, spdlog::level::level_enum level)
{

	if (logger == nullptr)
		return;

	logger->log(level, logMsg);
}

void PrepareLogger()
{
	try
	{
		auto config = Config(L"nvngx.ini");

		if (config.LoggingEnabled.value_or(false))
		{
			if (config.LogToConsole.value_or(false) && InitializeConsole())
				logger = spdlog::stdout_logger_mt("xess");
			else
				logger = spdlog::basic_logger_mt("xess", config.LogFileName.value());

			logger->set_pattern("[%H:%M:%S.%f] [%L] %v");
			logger->set_level((spdlog::level::level_enum)config.LogLevel.value_or(2));
		}
	}
	catch (const spdlog::spdlog_ex& ex)
	{

	}
}

void CloseLogger()
{
	spdlog::shutdown();
}

