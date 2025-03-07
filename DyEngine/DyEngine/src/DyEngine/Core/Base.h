﻿#pragma once
#include <memory>
#include <iostream>
#include "DyEngine/Core/PlatformDetection.h"

#ifdef DY_DEBUG
	#if defined(DY_PLATFORM_WINDOWS)
	)
	#elif defined(DY_PLATFORM_LINUX)
		#include <signal.h>
		#define DY_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
		#define DY_ENABLE_ASSERTS
#else
	#define DY_DEBUGBREAK()
#endif
#define DY_EXPAND_MACRO(x) x
#define DY_STRINGIFY_MACRO(x) #x
#define USEOPENGL true


/**
 * \brief
 * 用二进制左移操作<<
 * example:
 * 1 << 0 = `0000 0001`
 * 1 << 1 = `0000 0010`
 * 1 << 2 = `0000 0100`
 * 让enum的值不只等于赋给他的值，还等于小一位的值
 *	比如
 *	EventCategorMouse			= BIT(3),
 *	EventCategorMoustButton	= BIT(4)
 *	那么MoustButton既是4又是3
 *
 * \param x 
 */
#define BIT(x) (1 << x)

namespace DyEngine
{
	template<typename T>
	/// <summary>
	/// scope 是unique ptr
	/// </summary>
	/// <typeparam name="T"></typeparam>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	/// <summary>
	/// 用来一次性修改项目中的智能指针类型，此时ref是shared ptr
	/// </summary>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


}

/**
 * \brief 用来把实现的函数绑定到事件上
 * \param x 发生的各种事件，鼠标点击、按键、etc.
 */
#define DY_BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

#include "DyEngine/Core/Log.h"
#include "DyEngine/Core/Assert.h"