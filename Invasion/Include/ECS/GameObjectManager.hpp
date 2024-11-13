#pragma once

#include "ECS/GameObject.hpp"

namespace Invasion::ECS
{
	class GameObjectManager
	{

	public:

		GameObjectManager(const GameObjectManager&) = delete;
		GameObjectManager& operator=(const GameObjectManager&) = delete;

		Shared<GameObject> Register(const Shared<GameObject>& gameObject)
		{
			std::unique_lock lock(*mutex);

			NarrowString name = gameObject->GetName();

			gameObjects |= { name, gameObject };

			return gameObjects[name];
		}

		Shared<GameObject> Get(const NarrowString& name)
		{
			std::shared_lock lock(*mutex);

			return gameObjects[name];
		}

		void Update()
		{
			MutableArray<Shared<GameObject>> gameObjectsCopy;

			{
				std::shared_lock lock(*mutex);

				gameObjects.ForEach([&](const Shared<GameObject>& gameObject)
				{
					gameObjectsCopy += gameObject;
				});
			}

			for (const auto& gameObject : gameObjectsCopy)
				gameObject->Update();
		}

		void Render()
		{
			MutableArray<Shared<GameObject>> gameObjectsCopy;

			{
				std::shared_lock lock(*mutex);

				gameObjects.ForEach([&](const Shared<GameObject>& gameObject)
				{
					gameObjectsCopy += gameObject;
				});
			}

			for (const auto& gameObject : gameObjectsCopy)
				gameObject->Render();
		}

		void Unregister(const NarrowString& name)
		{
			std::unique_lock lock(*mutex);

			gameObjects[name]->Uninitialize();
			gameObjects -= name;
		}

		void Uninitialize()
		{
			MutableArray<Shared<GameObject>> gameObjectsCopy;

			{
				std::unique_lock lock(*mutex);

				gameObjects.ForEach([&](const Shared<GameObject>& gameObject)
				{
					gameObjectsCopy += gameObject;
				});

				gameObjects.Clear();
			}

			for (const auto& gameObject : gameObjectsCopy)
				gameObject->Uninitialize();
		}

		static GameObjectManager& GetInstance()
		{
			std::call_once(initFlag, []()
			{
				instance.reset(new GameObjectManager);
			});

			return *instance;
		}

	private:

		GameObjectManager() = default;

		Shared<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

		UnorderedMap<NarrowString, Shared<GameObject>> gameObjects;

		static Unique<GameObjectManager> instance;
		static std::once_flag initFlag;
	};

	Unique<GameObjectManager> GameObjectManager::instance;
	std::once_flag GameObjectManager::initFlag;
}