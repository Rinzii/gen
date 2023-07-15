#pragma once
#include <logger/target.hpp>
#include <source_location>
#include <string_view>

namespace gen::refactor::logger {
using SrcLoc = std::source_location;

void error(std::string_view category, std::string_view message, SrcLoc const& location = SrcLoc::current());
void warn(std::string_view category, std::string_view message, SrcLoc const& location = SrcLoc::current());
void info(std::string_view category, std::string_view message, SrcLoc const& location = SrcLoc::current());
void debug(std::string_view category, std::string_view message, SrcLoc const& location = SrcLoc::current());

inline void error(std::string_view message, SrcLoc const& location = SrcLoc::current()) { error("general", message, location); }
inline void warn(std::string_view message, SrcLoc const& location = SrcLoc::current()) { warn("general", message, location); }
inline void info(std::string_view message, SrcLoc const& location = SrcLoc::current()) { info("general", message, location); }
inline void debug(std::string_view message, SrcLoc const& location = SrcLoc::current()) { debug("general", message, location); }
} // namespace gen::refactor::logger
