#include "pch.h"
#include <iostream>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include "Config.h"

HMODULE dllModule;

static bool InitializeConsole()
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

static void PrepareLogger()
{
	try
	{
		if (Config::Instance()->LoggingEnabled.value_or(true))
		{
			if (Config::Instance()->OpenConsole.value_or(false))
				InitializeConsole();

			std::shared_ptr<spdlog::logger> shared_logger = nullptr;

			if (Config::Instance()->LogToConsole.value_or(true) && Config::Instance()->LogToFile.value_or(false))
			{
				auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				console_sink->set_level(spdlog::level::level_enum::info);
				console_sink->set_pattern("[XeSS] [%H:%M:%S.%f] [%L] %v");

				auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(Config::Instance()->LogFileName.value(), true);
				file_sink->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));
				file_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

				spdlog::logger logger("multi_sink", { console_sink, file_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}
			else if (Config::Instance()->LogToFile.value_or(false))
			{
				auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(Config::Instance()->LogFileName.value(), true);
				file_sink->set_level((spdlog::level::level_enum)Config::Instance()->LogLevel.value_or(2));
				file_sink->set_pattern("[%H:%M:%S.%f] [%L] %v");

				spdlog::logger logger("file_sink", { file_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}
			else if (Config::Instance()->LogToConsole.value_or(true))
			{
				auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
				console_sink->set_level(spdlog::level::level_enum::info);
				console_sink->set_pattern("[XeSS] [%H:%M:%S.%f] [%L] %v");

				spdlog::logger logger("console_sink", { console_sink });
				shared_logger = std::make_shared<spdlog::logger>(logger);
			}

			if (shared_logger)
			{
				shared_logger->set_level(spdlog::level::level_enum::trace);

#if _DEBUG
				shared_logger->flush_on(spdlog::level::debug);
#else
				shared_logger->flush_on(spdlog::level::err);
#endif // _DEBUG

				spdlog::set_default_logger(shared_logger);
			}
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

static void CloseLogger()
{
	spdlog::default_logger()->flush();
	spdlog::shutdown();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		dllModule = hModule;
		PrepareLogger();
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		CloseLogger();
		break;
	}

	return TRUE;
}

