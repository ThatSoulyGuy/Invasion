#pragma once

#include <typeindex>
#include "ECS/Component.hpp"

namespace Invasion::ECS
{
	class GameObject : public std::enable_shared_from_this<GameObject>
	{

	public:

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;

		template <typename T>
		Shared<T> AddComponent(Shared<T> component)
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

			std::unique_lock lock(*mutex);

			component->gameObject = shared_from_this();
			component->Initialize();

			components |= { std::type_index(typeid(T)), component };

			return std::static_pointer_cast<T>(components[std::type_index(typeid(T))]);
		}

		template <typename T>
		Shared<T> GetComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

			std::shared_lock lock(*mutex);

			return std::static_pointer_cast<T>(components[std::type_index(typeid(T))]);
		}

		template <typename T>
		bool HasComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

			std::shared_lock lock(*mutex);

			return components.Contains(std::type_index(typeid(T)));
		}

		template <typename T>
		void RemoveComponent()
		{
			static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

			std::shared_lock lock(*mutex);

			components[std::type_index(typeid(T))]->Uninitialize();

			components -= std::type_index(typeid(T));
		}

		void Update()
		{
			MutableArray<Shared<Component>> componentsCopy;

			{
				std::shared_lock lock(*mutex);

				components.ForEach([&](const Shared<Component>& component)
				{
					componentsCopy += component;
				});
			}

			for (const auto& component : componentsCopy)
				component->Update();
		}

		void Render()
		{
			MutableArray<Shared<Component>> componentsCopy;

			{
				std::shared_lock lock(*mutex);

				components.ForEach([&](const Shared<Component>& component)
				{
					componentsCopy += component;
				});
			}

			for (const auto& component : componentsCopy)
				component->Render();
		}

		NarrowString GetName() const
		{
			return name;
		}

		void Uninitialize()
		{
			MutableArray<Shared<Component>> componentsCopy;

			{
				std::shared_lock lock(*mutex);

				components.ForEach([&](const Shared<Component>& component)
				{
					componentsCopy += component;
				});

				components.Clear();
			}

			for (const auto& component : componentsCopy)
				component->Uninitialize();
		}

		static Shared<GameObject> Create(const NarrowString& name)
		{
			Shared<GameObject> result(new GameObject());

			result->name = name;

			return result;
		}

	private:

		GameObject() = default;

		NarrowString name;

		Shared<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

		UnorderedMap<std::type_index, Shared<Component>> components;
	};
}