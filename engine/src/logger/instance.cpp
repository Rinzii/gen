#include <logger/instance.hpp>
#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#if defined(_WIN32)
	#include <Windows.h>
#endif

namespace gen::refactor::logger {
namespace {
namespace fs = std::filesystem;

auto append_timestamp(std::string& out, Clock::time_point const& timestamp, Timestamp const mode) {
	static auto s_mutex{std::mutex{}};
	static constexpr std::size_t buf_size_v{64};
	auto buffer = std::array<char, buf_size_v>{};
	auto const time = Clock::to_time_t(timestamp);
	// std::localtime / std::gmtime are not thread safe. cppref:
	//  pointer to a static internal std::tm object on success, or null pointer otherwise.
	// The structure may be shared between std::gmtime, std::localtime, and std::ctime,
	// and may be overwritten on each invocation.
	auto lock = std::unique_lock{s_mutex};
	auto const* tm_struct = mode == Timestamp::eGm ? std::gmtime(&time) : std::localtime(&time);
	std::strftime(buffer.data(), buffer.size(), "%F %T", tm_struct);
	lock.unlock();
	out.append(buffer.data());
}

auto append_location(std::string& out, std::source_location const& location, Location const mode) -> void {
	auto const path = [&] {
		auto ret = fs::path{location.file_name()};
		if (mode == Location::eFilename) { ret = ret.filename(); }
		return ret.generic_string();
	}();
	out.append(path);
}

struct Formatter {
	struct Data {
		std::string format{};
		Location location{};
		Timestamp timestamp{};
	};

	std::string_view message;
	Data data;
	Context const& context;

	std::string out{};
	std::string_view format{data.format};

	char current{};

	[[nodiscard]] auto at_end() const -> bool { return format.empty(); }

	auto advance() -> bool {
		if (at_end()) {
			current = {};
			return false;
		}
		current = format.front();
		format = format.substr(1);
		return true;
	}

	auto try_keyword() -> bool {
		assert(current == '{');
		auto const close = format.find_first_of('}');
		if (close == std::string_view::npos) { return false; }

		auto const key = format.substr(0, close);
		if (!keyword(key)) { return false; }

		format = format.substr(close + 1);
		return true;
	}

	auto keyword(std::string_view key) -> bool {
		if (key == "level") {
			out += levelChar(context.level);
			return true;
		}

		if (key == "thread") {
			std::format_to(std::back_inserter(out), "{}", static_cast<int>(context.thread));
			return true;
		}

		if (key == "message") {
			out.append(message);
			return true;
		}

		if (key == "timestamp") {
			append_timestamp(out, context.timestamp, data.timestamp);
			return true;
		}

		if (key == "location") {
			append_location(out, context.location, data.location);
			return true;
		}

		return false;
	}

	auto operator()() -> std::string {
		static constexpr std::size_t reserve_v{128};
		out.reserve(message.size() + reserve_v);
		while (advance()) {
			if (current == '{' && try_keyword()) { continue; }
			out.append({current});
		}
		out.append("\n");
		return std::move(out);
	}
};
} // namespace

namespace {
struct ConsoleSink : Sink {
	auto handle(std::string_view const formatted, Context const& context) -> void final {
		// pick stdout / stderr based on level
		auto& stream = context.level == Level::eError ? std::cerr : std::cout;
		// thread-safe: operator<< is atomic
		stream << formatted;
#if defined(_WIN32)
		// assuming null terminated string (which it is)
		OutputDebugStringA(formatted.data());
#endif
	}
};

struct FileSink : Sink {
	std::string path{};
	std::mutex mutex{};
	std::string buffer{};

	// cv must outlive thread (so it receives the stop signal)
	std::condition_variable_any cv{};
	// thread must be destroyed before cv (so it can signal stop)
	std::jthread thread{};

	FileSink(std::string file_path) : path(std::move(file_path)), thread(&FileSink::run, this) {}

	auto run(std::stop_token const& stop) -> void {
		// remove existing log file
		if (fs::exists(path)) { fs::remove(path); }
		// loop until stopped
		while (!stop.stop_requested()) {
			auto lock = std::unique_lock{mutex};
			// sleep until buffer is not empty (or stopped, in which case return)
			if (!cv.wait(lock, stop, [this] { return !buffer.empty(); })) { return; }
			// empty buffer into file
			if (auto file = std::ofstream{path, std::ios::binary | std::ios::app}) {
				file << buffer;
				buffer.clear();
			}
		}
	}

	auto handle(std::string_view const formatted, [[maybe_unused]] Context const& context) -> void final {
		auto lock = std::unique_lock{mutex};
		buffer.append(formatted);
		lock.unlock();
		cv.notify_one();
	}
};
} // namespace

struct Instance::Impl {
	std::vector<std::unique_ptr<Sink>> sinks{};
	Config config{};
	std::mutex mutex{};

	ConsoleSink console{};
	FileSink file;

	Impl(char const* filePath) : file(filePath) {}

	auto print(std::string_view const message, Context const& context) -> void {
		auto lock = std::unique_lock{mutex};
		if (auto const itr = config.categoryMaxLevels.find(context.category); itr != config.categoryMaxLevels.end()) {
			if (context.level > itr->second) { return; }
		} else if (context.level > config.maxLevel) {
			return;
		}
		auto const target = [&] {
			if (auto const itr = config.levelTargets.find(context.level); itr != config.levelTargets.end()) { return itr->second; }
			return all_v;
		}();

		auto const data = Formatter::Data{.format = config.format, .location = config.location, .timestamp = config.timestamp};
		lock.unlock();

		auto const formatted = Formatter{.message = message, .data = data, .context = context}();

		if ((target & console_v) == console_v) { console.handle(formatted, context); }
		if ((target & file_v) == file_v) { file.handle(formatted, context); }
		if ((target & sinks_v) == sinks_v) {
			for (auto const& sink : sinks) { sink->handle(formatted, context); }
		}
	}
};

auto Instance::Deleter::operator()(Impl const* ptr) const -> void { delete ptr; }

Instance::Instance(char const* filePath, Config config) : m_impl(new Impl{filePath}) {
	if (s_instance != nullptr) { throw DuplicateError{"Duplicate logger Instance"}; }
	s_instance = m_impl.get();

	m_impl->config = std::move(config);

	m_impl->print(std::format("logging to file: {}", filePath), Context::make("logger", Level::eInfo));
}

Instance::~Instance() { s_instance = {}; }

auto Instance::getConfig() const -> Config {
	assert(m_impl);
	auto lock = std::scoped_lock{m_impl->mutex};
	return m_impl->config;
}

auto Instance::setConfig(Config config) -> void {
	assert(m_impl);
	auto lock = std::scoped_lock{m_impl->mutex};
	m_impl->config = std::move(config);
}

auto Instance::addSink(std::unique_ptr<Sink> sink) -> void {
	if (!sink) { return; }
	assert(m_impl != nullptr);
	m_impl->sinks.push_back(std::move(sink));
}

auto Instance::print(std::string_view const message, Context const& context) -> void {
	if (s_instance == nullptr) { return; }
	s_instance->print(message, context);
}
} // namespace gen::refactor::logger
