#include "pch.h"
#include "spdlog/sinks/basic_file_sink.h"

std::shared_ptr<spdlog::logger> logger = nullptr;


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

		if (config.XeSSLogging.value_or(false))
		{
			logger = spdlog::basic_logger_mt("xess_logger", config.LogFile.value());
			logger->set_pattern("[%H:%M:%S.%f] [%L] %v");
			logger->set_level((spdlog::level::level_enum)config.LogLevel.value_or(1));
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

