#pragma once

#include "Render/Texture.hpp"

namespace Invasion::Render
{
	class TextureManager
	{

	public:

		TextureManager(const TextureManager&) = delete;
		TextureManager& operator=(const TextureManager&) = delete;

		void Register(const Shared<Texture>& texture)
		{
			textures |= { texture->GetName(), texture };
		}

		Shared<Texture> Get(const NarrowString& name)
		{
			return textures[name];
		}

		void Unregister(const NarrowString& name)
		{
			textures[name]->Uninitialize_NoOverride();
			textures -= name;
		}

		void Uninitialize()
		{
			textures.ForEach([](const Shared<Texture>& texture)
			{
				texture->Uninitialize_NoOverride();
			});
		}
		
		static TextureManager& GetInstance()
		{
			std::call_once(initFlag, []()
			{
				instance.reset(new TextureManager);
			});

			return *instance;
		}

	private:

		TextureManager() = default;

		UnorderedMap<NarrowString, Shared<Texture>> textures;

		static Unique<TextureManager> instance;
		static std::once_flag initFlag;
	};

	Unique<TextureManager> TextureManager::instance;
	std::once_flag TextureManager::initFlag;
}