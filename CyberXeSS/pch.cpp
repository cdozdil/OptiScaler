#include "pch.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

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

void PrepareLogger()
{
	try
	{
		auto config = Config(L"nvngx.ini");

		if (config.LoggingEnabled.value_or(false))
		{
			if (config.OpenConsole.value_or(false))
				InitializeConsole();

			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			console_sink->set_level(spdlog::level::level_enum::info);
			console_sink->set_pattern("[XeSS] [%H:%M:%S.%f] [%L] %v");

			auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.LogFileName.value(), true);
			file_sink->set_level((spdlog::level::level_enum)config.LogLevel.value_or(2));
			file_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

			std::shared_ptr<spdlog::logger> shared_logger = nullptr;

			if (config.LogToConsole.value_or(true) && config.LogToFile.value_or(false))
			{
				spdlog::logger logger("multi_sink", { console_sink, file_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}
			else if (config.LogToFile.value_or(false))
			{
				spdlog::logger logger("file_sink", { file_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}
			else if (config.LogToConsole.value_or(true))
			{
				spdlog::logger logger("console_sink", { console_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}

			shared_logger->set_level(spdlog::level::level_enum::trace);
			spdlog::set_default_logger(shared_logger);
		}
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cerr << ex.what() << std::endl;

		auto logger = spdlog::stdout_color_mt("xess");
		logger->set_pattern("[%H:%M:%S.%f] [%L] %v");
		logger->set_level((spdlog::level::level_enum)2);
		spdlog::set_default_logger(logger);
	}
}

void CloseLogger()
{
	spdlog::shutdown();
}

