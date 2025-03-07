﻿#pragma once
#include "Base.h"
#include "Window.h"
#include "DyEngine/Events/Event.h"
#include "LayerStack.h"
#include "DyEngine/Events/ApplicationEvent.h"
#include "DyEngine/ImGui/ImGuiLayer.h"

#include "DyEngine/Renderer/Shader.h"
#include "DyEngine/Renderer/Buffer.h"
#include "DyEngine/Renderer/VertexArray.h"
#include "DyEngine/Renderer/OrthographicCamera.h"

#include "DyEngine/Core/Timestep.h"

namespace DyEngine
{
	class Application
	{
	public:
		Application(const std::string& name = "DY App");
		virtual ~Application();

		void OnEvent(Event& e);

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get(){return *s_Instance;}

		Window& GetWindow() { return *m_Window; }

		void Close();
		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		Scope<Window> m_Window;

		bool m_Running = true;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Minimized = false;
		Ref<Shader> m_Shader;
		//Ref<VertexArray>  m_VertexArray;
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		
		Ref<VertexArray> m_SquareVA;

		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	Application* CreateApplication();
	
}


