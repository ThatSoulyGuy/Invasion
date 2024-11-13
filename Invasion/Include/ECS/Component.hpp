#pragma once

#include "Util/Typedefs.hpp"

using namespace Invasion::Util;

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
		virtual void Render() { }
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