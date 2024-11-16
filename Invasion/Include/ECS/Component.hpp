#pragma once

#include "Util/Typedefs.hpp"

using namespace Invasion::Util;

namespace Invasion::Render
{
	class Camera;
}

namespace Invasion::ECS
{
	class GameObject;

	class Component
	{

	public:

		Component() = default;
		virtual ~Component() = default;

		virtual void Initialize() { }
		virtual void Update() { }
		virtual void Render(const Shared<Invasion::Render::Camera>&) { }
		virtual void Resize() { }
		virtual void Uninitialize() { }

		Shared<GameObject> GetGameObject() const
		{
			return gameObject.lock();
		}

	private:
		
		friend class GameObject;

		Weak<GameObject> gameObject;
		
	};
}