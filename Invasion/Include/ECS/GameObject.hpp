#pragma once

#include <typeindex>
#include "ECS/Component.hpp"
#include "Math/Transform.hpp"

using namespace Invasion::Math;

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
        Shared<T> GetComponent() const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            std::shared_lock lock(*mutex);

            return std::static_pointer_cast<T>(components[std::type_index(typeid(T))]);
        }

        template <typename T>
        bool HasComponent() const
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
            std::shared_lock lock(*mutex);
            return name;
        }

        Shared<Transform> GetTransform() const
        {
            return GetComponent<Transform>();
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

        void SetParent(const Shared<GameObject>& parent)
        {
            std::unique_lock lock(*mutex);

            if (this->parent.lock() != parent)
            {
                if (auto currentParent = this->parent.lock())
                    currentParent->RemoveChild(shared_from_this());
                
                this->parent = parent;

                if (parent)
                    parent->AddChild(shared_from_this());
                
                if (HasComponent<Transform>())
                {
                    auto transform = GetComponent<Transform>();

                    if (parent && parent->HasComponent<Transform>())
                        transform->SetParent(parent->GetComponent<Transform>());
                    else
                        transform->SetParent(nullptr); 
                }
            }
        }
        
        void AddChild(const Shared<GameObject>& child)
        {
            std::unique_lock lock(*mutex);

            children += child;
        }

        void RemoveChild(const Shared<GameObject>& child)
        {
            std::unique_lock lock(*mutex);
            
			if (children.Contains(child))
				children -= child;
        }

        Shared<GameObject> GetParent() const
        {
            std::shared_lock lock(*mutex);

            return parent.lock();
        }

        MutableArray<Shared<GameObject>> GetChildren() const
        {
            std::shared_lock lock(*mutex);

            return children;
        }

        static Shared<GameObject> Create(const NarrowString& name)
        {
            Shared<GameObject> result(new GameObject());

            result->name = name;

            result->AddComponent(Transform::Create());

            return result;
        }

    private:

		GameObject() : mutex(std::make_shared<std::shared_mutex>()) { }

        NarrowString name;
        
        Weak<GameObject> parent;
        MutableArray<Shared<GameObject>> children;

        Shared<std::shared_mutex> mutex;

        UnorderedMap<std::type_index, Shared<Component>> components;
    };
}