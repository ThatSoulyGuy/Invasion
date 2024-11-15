#pragma once

#include "Render/Shader.hpp"

namespace Invasion::Render
{
	class ShaderManager
	{

	public:

		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;

		void Register(const Shared<Shader>& shader)
		{
			shaders |= { shader->GetName(), shader };
		}

		Shared<Shader> Get(const NarrowString& name)
		{
			return shaders[name];
		}

		void Unregister(const NarrowString& name)
		{
			shaders[name]->Uninitialize_NoOverride();
			shaders -= name;
		}

		void Uninitialize()
		{
			shaders.ForEach([](const Shared<Shader>& shader)
			{
				shader->Uninitialize_NoOverride();
			});
		}
		
		static ShaderManager& GetInstance()
		{
			std::call_once(initFlag, []()
			{
				instance.reset(new ShaderManager);
			});

			return *instance;
		}

	private:

		ShaderManager() = default;

		UnorderedMap<NarrowString, Shared<Shader>> shaders;

		static Unique<ShaderManager> instance;
		static std::once_flag initFlag;
	};

	Unique<ShaderManager> ShaderManager::instance;
	std::once_flag ShaderManager::initFlag;
}