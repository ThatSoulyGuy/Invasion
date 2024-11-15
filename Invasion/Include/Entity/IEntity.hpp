#pragma once

#include "ECS/Component.hpp"
#include "Util/Helpers/Builder.hpp"
#include "Util/Typedefs.hpp"

using namespace Invasion::ECS;
using namespace Invasion::Util;

namespace Invasion::Entity
{
	class IEntity : public Component
	{

	public:

		IEntity(const IEntity&) = delete;
		IEntity& operator=(const IEntity&) = delete;

		NarrowString GetRegistryName() const
		{
			return RegistryName;
		}

		NarrowString GetDisplayName() const
		{
			return DisplayName;
		}

		float GetCurrentHealth() const
		{
			return CurrentHealth;
		}

		float GetMaxHealth() const
		{
			return MaxHealth;
		}

		float GetMovementSpeed() const
		{
			return MovementSpeed;
		}

		float GetRunningAccelerator() const
		{
			return RunningAccelerator;
		}

		float GetJumpHeight() const
		{
			return JumpHeight;
		}

		bool CouldJump() const
		{
			return CanJump;
		}

		template <typename T>
		static Shared<T> Create()
		{
			static_assert(std::is_base_of<IEntity, T>::value, "T must derive from IEntity");

			return Shared<T>(new T());
		}

	protected:

		IEntity() = default;

		virtual ~IEntity() = default;
		
	private:

		friend class Builder<IEntity>;

		template <typename Class, typename MemberType, MemberType Class::* MemberPtr>
		friend struct Setter;

		BUILDABLE_PROPERTY(RegistryName, NarrowString, IEntity)
		BUILDABLE_PROPERTY(DisplayName, NarrowString, IEntity)

		BUILDABLE_PROPERTY(CurrentHealth, float, IEntity)
		BUILDABLE_PROPERTY(MaxHealth, float, IEntity)

		BUILDABLE_PROPERTY(MovementSpeed, float, IEntity)
		BUILDABLE_PROPERTY(RunningAccelerator, float, IEntity)

		BUILDABLE_PROPERTY(JumpHeight, float, IEntity)
		BUILDABLE_PROPERTY(CanJump, bool, IEntity)
	};
}