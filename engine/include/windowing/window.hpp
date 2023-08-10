// Copyright (c) 2023-present Genesis Engine contributors (see LICENSE.txt)

#pragma once

#include "core.hpp"

#include <mim/vec2.hpp>

#include <memory>
#include <string>

// TODO: Replace this with a proper implementation

// Start Forward declarations
struct GLFWwindow;
// End Forward declarations

namespace gen
{

	class Window
	{
	public:

		// Values mirrored from GLFW
		// https://www.glfw.org/docs/3.3/glfw3_8h.html#a2315b99a329ce53e6a13a9d46fd5ca88
		enum class CursorMode : int
		{
			Normal = 0x00034001,
			Hidden = 0x00034002,
			Disabled = 0x00034003
		};

		Window(int width, int height, const char* title);
		~Window();

		Window(const Window &)			   = delete;
		Window & operator=(const Window &) = delete;
		Window(Window &&)				   = default;
		Window & operator=(Window &&)	   = default;

		// Member functions

		GEN_NODISCARD bool shouldClose() const;
		static void pollEvents();

		// Getters

		GEN_NODISCARD mim::vec2i getExtent();
		GEN_NODISCARD int getWidth();
		GEN_NODISCARD int getHeight();
		GEN_NODISCARD CursorMode getCursorMode() const;
		GEN_NODISCARD GLFWwindow * getHandle() const;

		// TODO: Currently these functions will break the window. Have not yet implemented window resizing.

		// Setters

		void setWidth(int width);
		void setHeight(int height);
		void setTitle(const char* title);
		void setCursorMode(CursorMode mode);

	private:

		// Callbacks
		static void callback_error(int error, const char * description);
		static void callback_cursor_position(GLFWwindow * window, double xPos, double yPos);
		static void callback_window_focus(GLFWwindow * window, int focused);
		static void callback_window_close(GLFWwindow * window);

		mim::vec2i m_extent;

		CursorMode m_currentCursorMode { Window::CursorMode::Normal };


		struct Deleter
		{
			void operator()(GLFWwindow* ptr) const;
		};

		std::unique_ptr<GLFWwindow, Deleter> m_window;

	};

} // namespace gen
