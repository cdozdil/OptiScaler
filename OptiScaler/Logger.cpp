#include "Logger.h"

#include <iostream>

#include "Config.h"
#include "Util.h"

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/callback_sink.h"

static bool InitializeConsole()
{
	// Allocate a console for this app
	if (!AllocConsole()) 
		return false;
	
	FILE* pFile;

	// Redirect STDIN if the console has an input handle
	if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE) 
	{
		if (freopen_s(&pFile, "CONIN$", "r", stdin) != 0) 
			return false;
	}

	// Redirect STDOUT if the console has an output handle
	if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) 
	{
		if (freopen_s(&pFile, "CONOUT$", "w", stdout) != 0) 
			return false;
	}

	// Redirect STDERR if the console has an error handle
	if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) 
	{
		if (freopen_s(&pFile, "CONOUT$", "w", stderr) != 0) 
			return false;
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

	if (Config::Instance()->DebugWait.value_or(false))
	{
		std::cout << "Press ENTER to continue..." << std::endl;
		std::cin.get();
	}

	return true;
}

void PrepareLogger()
{
	try
	{
		if (spdlog::default_logger() != nullptr)
			spdlog::default_logger().reset();

		if (Config::Instance()->LogToConsole.value_or(false) || Config::Instance()->LogToFile.value_or(false) || Config::Instance()->LogToNGX.value_or(false))
		{
			if (Config::Instance()->OpenConsole.value_or(false))
				InitializeConsole();

			std::shared_ptr<spdlog::logger> shared_logger = nullptr;

			std::vector<spdlog::sink_ptr> sinks;

			if (Config::Instance()->LogToConsole.value_or(false))
			{
				auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				console_sink->set_level(spdlog::level::level_enum::info);
				console_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

				sinks.push_back(console_sink);
			}

			if (Config::Instance()->LogToFile.value_or(false))
			{
				auto logFile = Util::DllPath().parent_path() / "OptiScaler.log";
				auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(Config::Instance()->LogFileName.value_or(logFile.wstring()), true);
				file_sink->set_level(spdlog::level::level_enum::trace);
				file_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

				sinks.push_back(file_sink);
			}

			auto callback_sink = std::make_shared<spdlog::sinks::callback_sink_mt>([](const spdlog::details::log_msg& msg)
				{
					if (Config::Instance()->LogToNGX.value_or(false) &&
						Config::Instance()->NVNGX_Logger.LoggingCallback != nullptr &&
						Config::Instance()->NVNGX_Logger.MinimumLoggingLevel != NVSDK_NGX_LOGGING_LEVEL_OFF &&
						(Config::Instance()->NVNGX_Logger.MinimumLoggingLevel == NVSDK_NGX_LOGGING_LEVEL_VERBOSE || msg.level >= spdlog::level::info))
					{
						auto message = (char*)msg.payload.data();
						Config::Instance()->NVNGX_Logger.LoggingCallback(message, NVSDK_NGX_LOGGING_LEVEL_ON, NVSDK_NGX_Feature_SuperSampling);
					}
				});

			callback_sink->set_level(spdlog::level::level_enum::trace);
			callback_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

			sinks.push_back(callback_sink);

			spdlog::logger logger("multi_sink", sinks.begin(), sinks.end());
			shared_logger = std::make_shared<spdlog::logger>(logger);

			shared_logger->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));

			shared_logger->flush_on(spdlog::level::trace);

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
	spdlog::default_logger()->flush();
	spdlog::shutdown();
}

