#include "logger/context.hpp"
#include <atomic>

namespace gen::refactor::logger {
auto Context::getThreadId() -> ThreadId {
	auto const get_next_id = [] {
		static auto s_prevId{std::atomic<int>{}};
		return s_prevId++;
	};
	thread_local auto const s_thisThreadId{get_next_id()};
	return ThreadId{s_thisThreadId};
}

auto Context::make(std::string_view category, Level level, SrcLoc const& location) -> Context {
	return Context{
		.category = category,
		.timestamp = Clock::now(),
		.location = location,
		.thread = getThreadId(),
		.level = level,
	};
}
} // namespace gen::refactor::logger
