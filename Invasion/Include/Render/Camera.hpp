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

			return Matrix<float, 4, 4>::Projection(DirectX::XMConvertToRadians(fieldOfView), window.Bounds().Width / (float)window.Bounds().Height, nearPlane, farPlane);
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

		float fieldOfView = 0;
		float nearPlane = 0;
		float farPlane = 0;

	};
}