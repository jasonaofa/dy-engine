﻿#pragma once

#include "Base.h"
#include <sstream>
#include "DyEngine/Events/Event.h"
#include "Log.h"

namespace DyEngine
{

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		//Tags 窗口尺寸
		WindowProps(const std::string& title = "DyEngine",
			uint32_t width = 1280,
			uint32_t height = 720)
						:Title(title),Width(width),Height(height)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;
		//这里用uniqueptr 
		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};








}
