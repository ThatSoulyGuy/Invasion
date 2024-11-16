#pragma once

#include "Core/InputManager.hpp"
#include "ECS/GameObject.hpp"
#include "Entity/IEntity.hpp"
#include "Render/Camera.hpp"

using namespace Invasion::Core;
using namespace Invasion::Entity;
using namespace Invasion::Render;

namespace Invasion::Entity::Entities
{
	class EntityPlayer : public IEntity
	{
		
	public:

		EntityPlayer(const EntityPlayer&) = delete;
		EntityPlayer(EntityPlayer&&) = delete;
		EntityPlayer& operator=(const EntityPlayer&) = delete;
		EntityPlayer& operator=(EntityPlayer&&) = delete;

		EntityPlayer()
		{
			Builder<IEntity>::New()
				.Set(IEntity::RegistryNameSetter{ "entity_player" })
				.Set(IEntity::DisplayNameSetter{ "Player** Unknwn" })
				.Set(IEntity::CurrentHealthSetter{ 100.0f })
				.Set(IEntity::MaxHealthSetter{ 100.0f })
				.Set(IEntity::MovementSpeedSetter{ 0.1f })
				.Set(IEntity::RunningAcceleratorSetter{ 1.0f })
				.Set(IEntity::JumpHeightSetter{ 5.0f })
				.Set(IEntity::CanJumpSetter{ true })
				.Build(static_cast<IEntity&>(*this));

			Builder<EntityPlayer>::New()
				.Set(EntityPlayer::MouseSensitivitySetter{ 0.01f })
				.Build(static_cast<EntityPlayer&>(*this));
		}

		~EntityPlayer() = default;

		void Initialize() override
		{
			IEntity::Initialize();

			//InputManager::GetInstance().SetCursorVisibility(false);
			InputManager::GetInstance().SetCursorCanMove(false);

			cameraObject = GameObject::Create("camera");

			cameraObject->GetTransform()->SetLocalPosition({ 0.0f, 0.0f, 0.0f });
			cameraObject->AddComponent(Camera::Create(45.0f, 0.01f, 1000.0f));

			GetGameObject()->AddChild(cameraObject);
		}

		void Update() override
		{
			IEntity::Update();

			UpdateMouselook();
			UpdateMovement();
		}

		void Uninitialize() override
		{
			IEntity::Uninitialize();
		}

		Shared<GameObject> GetCameraObject() const
		{
			return cameraObject;
		}

	private:

		void UpdateMouselook()
		{
			Vector<float, 2> mouseDelta = InputManager::GetInstance().GetMouseDelta();

			Vector<float, 3> rotation = cameraObject->GetTransform()->GetLocalRotation();

			rotation[0] += mouseDelta[0] * MouseSensitivity;
			rotation[1] += mouseDelta[1] * MouseSensitivity;

			rotation[0] = std::fmod(rotation[0], 360.0f);
			rotation[1] = std::clamp(rotation[1], -89.0f, 89.0f);

			cameraObject->GetTransform()->SetLocalRotation(rotation);

			//OutputDebugString(String("Mouse Delta: ") + "{ " + mouseDelta.x + ", " + mouseDelta.y + " }" + "\n");
			//OutputDebugString(String("Player Rotation: ") + "{ " + cameraObject->GetTransform()->GetLocalRotation().x + ", " + cameraObject->GetTransform()->GetLocalRotation().y + ", " + cameraObject->GetTransform()->GetLocalRotation().z + " }" + "\n");
		}

		void UpdateMovement()
		{
			Vector<float, 3> forward = GetGameObject()->GetTransform()->GetForward();
			Vector<float, 3> right = GetGameObject()->GetTransform()->GetRight();

			forward = forward.Normalize();

			right = right.Normalize();

			Vector<float, 3> movement = { 0.0f, 0.0f, 0.0f };

			if (InputManager::GetInstance().IsKeyOrMouseButtonAtState(VirtualKey::W, CoreVirtualKeyStates::Down))
				movement += forward;

			if (InputManager::GetInstance().IsKeyOrMouseButtonAtState(VirtualKey::S, CoreVirtualKeyStates::Down))
				movement -= forward;

			if (InputManager::GetInstance().IsKeyOrMouseButtonAtState(VirtualKey::A, CoreVirtualKeyStates::Down))
				movement -= right;

			if (InputManager::GetInstance().IsKeyOrMouseButtonAtState(VirtualKey::D, CoreVirtualKeyStates::Down))
				movement += right;

			if (movement != Vector<float, 3>(0))
			{
				movement = movement.Normalize();

				movement *= GetMovementSpeed();

				if (InputManager::GetInstance().IsKeyOrMouseButtonAtState(VirtualKey::Shift, CoreVirtualKeyStates::Down))
					movement *= GetRunningAccelerator();
			}

			GetGameObject()->GetTransform()->Translate(movement);

			//OutputDebugString(String("Player Movement: ") + "{ " + movement.x + ", " + movement.y + ", " + movement.z + " }" + "\n");
		}

		BUILDABLE_PROPERTY(MouseSensitivity, float, EntityPlayer)

		Shared<GameObject> cameraObject;
	};
}