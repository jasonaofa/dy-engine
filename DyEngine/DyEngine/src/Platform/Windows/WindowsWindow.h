﻿#pragma once

#include "DyEngine/Core/Window.h"
#include <GLFW/glfw3.h>
#include "DyEngine/Renderer/GraphicsContext.h"

namespace DyEngine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// 属性
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		void DisableMouse();

		virtual void* GetNativeWindow() const { return m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		//window
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		//只要传这个结构体给GLFW就行了。不用传递整个类
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}
