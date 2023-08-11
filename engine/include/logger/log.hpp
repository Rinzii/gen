#pragma once
#include <logger/level.hpp>
#include <logger/target.hpp>
#include <format>
#include <string_view>

namespace gen
{
	namespace logger
	{
		void print(Level level, std::string_view category, std::string_view message);
	} // namespace logger

	class Logger
	{
	public:
		using Level = logger::Level;

		explicit Logger(std::string_view category);

		template <typename... Args>
		void error(std::format_string<Args...> fmt, Args &&... args) const
		{
			logger::print(Level::eError, m_category, std::format(fmt, std::forward<Args>(args)...));
		}

		template <typename... Args>
		void warn(std::format_string<Args...> fmt, Args &&... args) const
		{
			logger::print(Level::eWarn, m_category, std::format(fmt, std::forward<Args>(args)...));
		}

		template <typename... Args>
		void info(std::format_string<Args...> fmt, Args &&... args) const
		{
			logger::print(Level::eInfo, m_category, std::format(fmt, std::forward<Args>(args)...));
		}

		template <typename... Args>
		void log(std::format_string<Args...> fmt, Args &&... args) const
		{
			info(fmt, std::forward<Args>(args)...);
		}

		template <typename... Args>
		void debug(std::format_string<Args...> fmt, Args &&... args) const
		{
			logger::print(Level::eDebug, m_category, std::format(fmt, std::forward<Args>(args)...));
		}

	private:
		std::string_view m_category{};
	};

	namespace logger
	{
		inline Logger const general{"general"};
	}
} // namespace gen
