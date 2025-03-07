﻿#include "EditorLayer.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "entt.hpp"
#include "DyEngine/Scene/SceneSerializer.h"
#include "DyEngine/Utils/PlatformUtils.h"

#include "ImGuizmo.h"

#include "DyEngine/Math/Math.h"
#include "DyEngine/Renderer/Model.h"
#include "DyEngine/Renderer/Environment.h"

namespace DyEngine
{
	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_SquareColor({0.2f, 0.3f, 0.8f, 1.0f})
	{
		m_FlatColorShader = DyEngine::Shader::Create("assets/shaders/Default.glsl");

	}

	void EditorLayer::OnAttach()
	{
		DY_PROFILE_FUNCTION();
		//m_FlatColorShader->Bind();


		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_Environment = Environment::Create(m_Environment->GetEnvData(), env_cloudsShader);

		// 输出到editor的fbo
		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Attachments = {
			FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth
		};
		//fbSpec.Attachments.Attachments.push_back(FramebufferTextureFormat::RGBA8);
		m_Framebuffer = Framebuffer::Create(fbSpec);

		//初始化场景
		m_ActiveScene = CreateRef<Scene>();
		env_cloudsShader = Shader::Create("assets/shaders/Default.glsl");
		//相机
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

#define defaultScene 1;
#if defaultScene
		// Entitys
		//auto square = m_ActiveScene->CreateEntity("Green Square");
		//square.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
		//m_SquareEntity = square;

		//auto redSquare = m_ActiveScene->CreateEntity("Red Square");
		//redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});



		//model
		m_Scene = Model::Create("assets/models/cz/all.obj");
		m_TownEntity = m_ActiveScene->CreateEntity("town");
		m_TownEntity.AddComponent<MeshComponent>(m_Scene);
		m_TownEntity.AddComponent<MaterialComponent>(m_FlatColorShader);

		//m_TownEntity.AddComponent<TagComponent>("66");
		env_Entity = m_ActiveScene->CreateEntity("Environment");
		env_Entity.AddComponent<EnvComponent>(m_Environment->GetEnvData(),env_cloudsShader);
		auto blueSquare = m_ActiveScene->CreateEntity("blue Square");
		blueSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f });

		//m_SquareEntity = square;

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;


		class CameraController : public ScriptableEntity
		{
		public:
			virtual void OnCreate() override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				translation.x = rand() % 10 - 5.0f;
			}

			virtual void OnDestroy() override
			{
			}

			virtual void OnUpdate(Timestep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				auto& rotation = GetComponent<TransformComponent>().Rotation;
				float speed = 5.0f;

				if (Input::IsKeyPressed(Key::A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(Key::D))
					translation.x += speed * ts;
				if (Input::IsKeyPressed(Key::W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(Key::S))
					translation.y -= speed * ts;
				if (Input::IsKeyPressed(Key::Q))
					rotation.z -= speed * ts;
			}
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

		m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif
		//panel
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);



	}

	void EditorLayer::OnDetach()
	{
		DY_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		DY_PROFILE_FUNCTION();

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);

			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Update
		if (m_ViewportFocused)
			m_CameraController.OnUpdate(ts);

		m_EditorCamera.OnUpdate(ts);


		//m_FlatColorShader->SetMat4("u_ViewProjection", m_EditorCamera.GetViewProjection());


		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		RenderCommand::Clear();


		Renderer2D::BeginScene(m_CameraController.GetCamera());

		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);

		// Update scene
		//当我们打开一个场景的时候，会把场景文件中所有的entity都反序列化并加载到sharedPtr scene的m_ActiveScene中
		//然后我们用OnUpdateEditor把场景画出来
		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

		//得到鼠标在viewport中的的xy坐标(像素级就行了
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		//std::cout << mouseX<< std::endl;
		//通过
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			//通过readpixel把framebuffer中的entityID读出来
			int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
			//pixelData =-1 说明鼠标现在没有放在任何像素上
			//我们把pixelData的值直接给m_HoveredEntity
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
		}


		Renderer2D::EndScene();

		m_Framebuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		DY_PROFILE_FUNCTION();

		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		//panel
		m_SceneHierarchyPanel.OnImGuiRender();


		ImGui::Begin("Stats");

		std::string name = "None";
		if (m_HoveredEntity)
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("Hovered Entity: %s", name.c_str());

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("PixelData: %i", m_HoveredEntity);



		ImGui::End();

	
		ImGui::Begin("Environment");
		ImGui::Text("Save:");
		if (ImGui::Button("Save_1")) cloudStats_01 = cloudStatsDefault;ImGui::SameLine();
		if (ImGui::Button("Save_2")) cloudStats_02 = cloudStatsDefault;ImGui::SameLine();
		if (ImGui::Button("Save_3")) cloudStats_03 = cloudStatsDefault;
		ImGui::Text("Load:");
		if (ImGui::Button("Load_1")) cloudStatsDefault = cloudStats_01; ImGui::SameLine();
		if (ImGui::Button("Load_2")) cloudStatsDefault = cloudStats_02; ImGui::SameLine();
		if (ImGui::Button("Load_3")) cloudStatsDefault = cloudStats_03;
		ImGui::SliderFloat("windStrength", &cloudStatsDefault.windStrength, 0.000f, 0.001f, "%.5f");
		ImGui::InputFloat3("windDirection", glm::value_ptr(cloudStatsDefault.windDirection));

		ImGui::InputFloat("Sun Energy", &sunStats.sunEnergy, 0.0f, 10.0f);
		ImGui::ColorEdit3("Sun Color", glm::value_ptr(sunStats.sunColor));
		ImGui::ColorEdit3("CloudBaseColor", glm::value_ptr(cloudStatsDefault.CloudBaseColor));
		ImGui::ColorEdit3("CloudTopColor", glm::value_ptr(cloudStatsDefault.CloudTopColor));
		ImGui::SliderFloat("NoiseThreshold", &cloudStatsDefault.NoiseThreshold, 0.0f, 1.0f);
		ImGui::SliderFloat("NoiseMax", &cloudStatsDefault.NoiseMax, 0.0f, 1.0f);

		ImGui::Text("Global Clouds Settings");
		ImGui::InputFloat("CloudVolumeStartHeight", &cloudStatsDefault.cloudVolumeStartHeight, 0.0f, 10000.0f);
		ImGui::InputFloat("CloudVolumeHeight", &cloudStatsDefault.cloudVolumeHeight, 0.0f, 10000.0f);
		ImGui::InputFloat("groundRadius", &cloudStatsDefault.groundRadius, 0.0f, 10000.0f);


		ImGui::SliderFloat("cloudTopOffset", &cloudStatsDefault.cloudTopOffset, -10.0f, 500.0f);

		ImGui::InputFloat3("weatherTexMod", glm::value_ptr(cloudStatsDefault.weatherTexMod));

		ImGui::Text("Cloud Shape Settings");
		ImGui::SliderFloat("precipiFactor", &cloudStatsDefault.precipiFactor, 0, 1.0f, "%.3f");
		ImGui::SliderFloat("coverageFactor", &cloudStatsDefault.coverageFactor, 0, 1.0f, "%.3f");


		ImGui::Text("Cloud Detail Settings");
		ImGui::SliderFloat("detailScale", &cloudStatsDefault.detailScale, 0.0001, 10.0f, "%.4f");
		ImGui::SliderFloat("curlNoiseMultiple", &cloudStatsDefault.curlNoiseMultiple, 0, 10.0f, "%.3f");

		ImGui::InputFloat3("detailwindDirection", (float*)&cloudStatsDefault.detailwindDirection);
		ImGui::SliderFloat("Wind Cloud Speed", &cloudStatsDefault.cloudSpeed, 0.0f, 100.0f);

		ImGui::End();



		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
		m_ViewportBounds[1] = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};

		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{m_ViewportSize.x, m_ViewportSize.y}, ImVec2{0, 1},
		             ImVec2{1, 0});


		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
			                  m_ViewportBounds[1].x - m_ViewportBounds[0].x,
			                  m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera

			// Runtime camera from entity
			// auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			// const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			// const glm::mat4& cameraProjection = camera.GetProjection();
			// glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();


			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = {snapValue, snapValue, snapValue};

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			                     (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
			                     nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}


		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(DY_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(DY_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;
		//m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode())
		{
		case Key::N:
			{
				if (control)
					NewScene();

				break;
			}
		case Key::O:
			{
				if (control)
					OpenScene();

				break;
			}
		case Key::S:
			{
				if (control && shift)
					SaveSceneAs();

				break;
			}


		// Gizmos
		case Key::Q:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			}
		case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}

		case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
		case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
		}
		return true;
	}

	//选择
	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
				//第一次点击默认就显示translate

				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("DY Scene (*.dy)\0*.dy\0");
		if (!filepath.empty())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filepath);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("DY Scene (*.dy)\0*.dy\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}
}
