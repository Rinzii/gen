#pragma once
#include <logger/level.hpp>
#include <chrono>
#include <source_location>
#include <string_view>

namespace gen::logger
{
	using Clock	 = std::chrono::system_clock;
	using SrcLoc = std::source_location;

	///
	/// \brief Strongly typed integer representing logging thread ID.
	///
	enum struct ThreadId : int
	{
	};

	///
	/// \brief Log context.
	///
	/// Expected to be built at the call site and passed around.
	///
	struct Context
	{
		std::string_view category{};
		Clock::time_point timestamp{};
		std::source_location location{};
		ThreadId thread{};
		Level level{};

		///
		/// \brief Obtain this thread's ID.
		/// \returns Monotonically increasing IDs per thread, in order of being called for the first time.
		///
		static ThreadId getThreadId();
		static Context make(std::string_view category, Level level, SrcLoc const & location = SrcLoc::current());
	};
} // namespace gen::logger
