﻿#include "DyPch.h"
#include "Scene.h"


#include "Components.h"
#include "DyEngine/Renderer/Renderer2D.h"

#include <glm/glm.hpp>
#include "Entity.h"
namespace DyEngine {

	static void DoMath(const glm::mat4& transform)
	{

	}

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{

	}

	Scene::Scene()
	{
#if ENTT_EXAMPLE_CODE
		entt::entity entity = m_Registry.create();
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();


		if (m_Registry.has<TransformComponent>(entity))
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);


		auto view = m_Registry.view<TransformComponent>();
		for (auto entity : view)
		{
			TransformComponent& transform = view.get<TransformComponent>(entity);
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto entity : group)
		{
			auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
		}
#endif
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}
	//放到应用/编辑器的OnUpdate中
	void Scene::OnUpdateRuntime(Timestep ts)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this };

						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
				});
		}


		// Render 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}

			Renderer2D::EndScene();
		}
	}
	

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);



		//auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		//for (auto entity : group)
		//{
		//	auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
		//	Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		//}
		//auto EnvGroup = m_Registry.get<EnvComponent>();
		//auto total_entity = m_Registry.create();
		//auto ent_entity = m_Registry.get<EnvComponent>(total_entity);
		auto env_view = m_Registry.view<EnvComponent>();
		for (auto entity : env_view)
		{
			auto trans = m_Registry.get<TransformComponent>(entity);
			EnvComponent& Env = env_view.get<EnvComponent>(entity);
			Env.env_shader->Bind();
			Env.env_shader->SetInt("a_EntityID", (int)entity);
			Env.env_shader->SetMat4("u_ViewProjection", camera.GetViewProjection() * (trans.GetTransform()));
			Env.EnvDraw(Env.env_data, Env.env_shader);
		}

		auto meshGroup = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto m_entity:meshGroup)
		{
			auto trans = m_Registry.get<TransformComponent>(m_entity);
			if (m_Registry.has<MaterialComponent>(m_entity)&&m_Registry.has<MeshComponent>(m_entity))
			{
				auto allMesh = m_Registry.get<MeshComponent>(m_entity);
				auto allMaterial = m_Registry.get<MaterialComponent>(m_entity);
				allMaterial.m_shader->Bind();
				allMaterial.m_shader->SetInt("a_EntityID", (int)m_entity);
				allMaterial.m_shader->SetMat4("u_ViewProjection", camera.GetViewProjection() * (trans.GetTransform()));
				allMesh.m_model->Draw(allMaterial.m_shader);
			}
	


			//m_FlatColorShader->SetInt("a_EntityID", (int)m_entity);
			//std::cout<<"Entity id is :" << ((int)m_entity) << std::endl;//id is 1
			//todo add the entity ID to the colorattachment id in fbo 

			//auto [transform, sprite] = group.get<TransformComponent, MeshComponent>(m_entity);
			//std::cout << transform.Scale.x;
		}
		Renderer2D::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}

	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(false);
	}
	//TODO DO SOMETHING
	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}
	template<>
	void Scene::OnComponentAdded<EnvComponent>(Entity entity, EnvComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}


}