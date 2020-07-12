#pragma once

#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
//#define LOG std::cout

namespace AIR
{
	class Log
	{
	public:
		static void Init()
		{
			Logger();
		}
		template<typename FormatString, typename... Args>
		static void Info(const FormatString& fmt, const Args&... args)
		{
			logger->info(fmt, args...);
		}

		template<typename FormatString, typename... Args>
		static void Error(const FormatString& fmt, const Args&... args)
		{
			logger->error(fmt, args...);
		}

		template<typename FormatString, typename... Args>
		static void Warn(const FormatString& fmt, const Args&... args)
		{
			logger->warn(fmt, args...);
		}
	private:
		static std::shared_ptr<spdlog::logger> Logger()
		{
			if (logger == nullptr)
			{
				logger = spdlog::stdout_color_mt("console");
			}
			return logger;
		}

		static std::shared_ptr<spdlog::logger> logger;
	};
}

