﻿#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "DyEngine/Renderer/Model.h"
#include "DyEngine/Renderer/Environment.h"

/**
 * \brief
 * if add any component:
 * 1.add OnComponentAdded<>() function  in scene.cpp
 */
namespace DyEngine
{
	struct TagComponent
	{
		//这就是这个Entity的名字
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;

		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}
	};
	struct EnvComponent
	{
		Environment::EnvData env_data;
		Ref<Shader> env_shader;  

		EnvComponent() = default;
		EnvComponent(const EnvComponent&) = default;

		EnvComponent(const Environment::EnvData& env_data,const Ref<Shader>& envShader)
			:env_data(env_data), env_shader(envShader)
		{
		}
		void EnvDraw(Environment::EnvData& env_data,Ref<Shader>& env_shader )
		{
			auto Env = Environment::Create(env_data, env_shader);
			Env->draw();
		}

	};

	struct TransformComponent
	{
		glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(const glm::vec3& translation)
			: Translation(translation)
		{
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, {1, 0, 0})
				* glm::rotate(glm::mat4(1.0f), Rotation.y, {0, 1, 0})
				* glm::rotate(glm::mat4(1.0f), Rotation.z, {0, 0, 1});

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;

		SpriteRendererComponent(const glm::vec4& color)
			: Color(color)
		{
		}
	};

	struct MeshComponent
	{

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
#ifdef USEOPENGL
		MeshComponent(const Ref<Model> mesh) :m_model(mesh) {};
		//MeshComponent(const Ref<Model> mesh) :m_model(mesh) {};
		Ref<Model> m_model;
#else
		DY_CORE_ERROR("None API")
#endif

	};
	//TODO finish materialComp
	struct MaterialComponent
	{
		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(const Ref<Shader> shader) :m_shader(shader) {};
		Ref<Shader>m_shader;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; // TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc)
			{
				delete nsc->Instance;
				nsc->Instance = nullptr;
			};
		}
	};




}
