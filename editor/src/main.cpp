// Copyright (c) 2023-present Genesis Engine contributors (see LICENSE.txt)

#include <gen/logger/instance.hpp>
#include <gen/logger/log.hpp>
#include "game.hpp"

// TODO: Replace this with a config file. At least for the startup window size.
static constexpr const char * appName{"Genesis Game"};
static constexpr mim::vec2i startingWindowSize{800, 600};

int main()
{
	try
	{
		auto logger = gen::logger::Instance{}; // Required to initialize the logger

		gen::Game app{appName, startingWindowSize};
		app.run();
	}
	catch (std::exception const & e)
	{
		gen::logger::general.error("{}", e.what());
		return 1;
	}

	return 0;
}
