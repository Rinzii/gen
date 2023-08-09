// Copyright (c) 2023-present Genesis Engine contributors (see LICENSE.txt)

#include "windowing/window.hpp"

#include <format>
#include "logger/log.hpp"

#include <GLFW/glfw3.h>

#include <utility>

namespace gen
{

	Window::Window(int width, int height, const std::string& title)
		: m_extent{width, height} // NOLINT(cppcoreguidelines-pro-type-member-init)
	{
		if (!glfwInit()) // NOLINT(readability-implicit-bool-conversion)
		{
			gen::logger::error("windowing", "Failed to initialize GLFW");
			throw std::runtime_error("Failed to initialize GLFW");
		}

		// Set window hints
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // TODO: Later add support for resizing of windows.

		// Create window
		GLFWwindow * window_ = glfwCreateWindow(static_cast<int>(m_extent.x),
												static_cast<int>(m_extent.y),
												title.c_str(),
												nullptr,
												nullptr);

		m_window = std::unique_ptr<GLFWwindow, Deleter>(window_);

		// Check if window was created
		if (!m_window)
		{
			gen::logger::error("windowing", "Failed to create GLFW window");
			throw std::runtime_error("Failed to create GLFW window");
		}

		// Set modes
		glfwSetInputMode(m_window.get(), GLFW_CURSOR, static_cast<int>(m_currentCursorMode));

		// Set callbacks
		glfwSetErrorCallback(callback_error);
		glfwSetCursorPosCallback(m_window.get(), callback_cursor_position);

		// Report successful window creation
		gen::logger::info("windowing", "Window instance constructed");
	}

	Window::~Window()
	{
		gen::logger::info("windowing", "Window instance destructed");
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(m_window.get()) != 0;
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

	/// Setters

	void Window::setTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_window.get(), title.c_str());
	}

	void Window::setCursorMode(Window::CursorMode mode)
	{
		m_currentCursorMode = mode;
		glfwSetInputMode(m_window.get(), GLFW_CURSOR, static_cast<int>(m_currentCursorMode));
	}

	/// Callbacks

	void Window::callback_error(int error, const char * description)
	{
		gen::logger::warn("windowing", std::format("GLFW Error: {} - {}", error, description));
	}

	void Window::callback_cursor_position(GLFWwindow * window, double xPos, double yPos)
	{
	}


	void Window::Deleter::operator()(GLFWwindow * ptr) const
	{
		glfwDestroyWindow(ptr);
		glfwTerminate();
	}
} // namespace gen
