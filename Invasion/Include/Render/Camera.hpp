#pragma once

#include "pch.h"
#include "ECS/GameObject.hpp"

using namespace winrt::Windows::UI::Core;

using namespace Invasion::ECS;

namespace Invasion::Render
{
	class Camera : public Component
	{

	public:

		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;

		Matrix<float, 4, 4> GetProjectionMatrix() const
		{
			CoreWindow window = CoreWindow::GetForCurrentThread();

			return Matrix<float, 4, 4>::Projection(DirectX::XMConvertToRadians(45.0f), window.Bounds().Width / window.Bounds().Height, 0.01f, 1000.0f);
		}

		Matrix<float, 4, 4> GetViewMatrix() const
		{
			Vector<float, 3> position = GetGameObject()->GetTransform()->GetWorldPosition();
			Vector<float, 3> forward = GetGameObject()->GetTransform()->GetForward();
			Vector<float, 3> up = { 0.0f, 1.0f, 0.0f };

			return Matrix<float, 4, 4>::LookAt(position, position + forward, up);
		}

		static Shared<Camera> Create(float fieldOfView, float nearPlane, float farPlane)
		{
			Shared<Camera> camera(new Camera());

			camera->fieldOfView = fieldOfView;
			camera->nearPlane = nearPlane;
			camera->farPlane = farPlane;

			return camera;
		}

	private:

		Camera() = default;

		float fieldOfView;
		float nearPlane;
		float farPlane;

	};
}