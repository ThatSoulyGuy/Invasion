#pragma once

#include "pch.h"
#include "Math/Vector.hpp"
#include "Util/Typedefs.hpp"

using namespace winrt;

using namespace winrt::Windows::System;
using namespace winrt::Windows::Devices::Input;
using namespace winrt::Windows::UI::Core;

using namespace Invasion::Math;
using namespace Invasion::Util;

namespace Invasion::Core
{
	class InputManager
	{

	public:

		InputManager(const InputManager&) = delete;
		InputManager(InputManager&&) = delete;
		InputManager& operator=(const InputManager&) = delete;
		InputManager& operator=(InputManager&&) = delete;

		void Initialize(const CoreWindow& window)
		{
			this->window = window;

			this->window.Activated({ this, &InputManager::OnWindowActivated });

			mouseDevice = MouseDevice::GetForCurrentView();
			mouseMovedToken = mouseDevice.MouseMoved({ this, &InputManager::OnMouseMoved });
		}

		void Update()
		{
			if (!cursorCanMove && isWindowActive)
			{
				auto bounds = window.Bounds();

				winrt::Windows::Foundation::Point point = { bounds.X + bounds.Width / 2, bounds.Y + bounds.Height / 2 };

				window.PointerPosition(point);
			}
		}

		bool IsKeyOrMouseButtonAtState(VirtualKey key, CoreVirtualKeyStates state)
		{
			return window.GetAsyncKeyState(key) == state;
		}

		Vector<float, 2> GetMousePosition()
		{
			auto point = window.PointerPosition();

			return { point.X, point.Y };
		}

		Vector<float, 2> GetMouseDelta()
		{
			Vector<float, 2> delta = { static_cast<float>(deltaX), static_cast<float>(deltaY) };

			deltaX = 0;
			deltaY = 0;

			return delta;
		}

		void SetCursorIcon(CoreCursorType type)  
		{  
			window.PointerCursor(CoreCursor(type, 1)); 
		}

		void SetCursorVisibility(bool visible)
		{
			window.PointerCursor(visible ? CoreCursor(CoreCursorType::Arrow, 1) : nullptr);
		}

		void SetCursorCanMove(bool canMove)
		{
			cursorCanMove = canMove;
		}

		bool GetCursorCanMove() const
		{
			return cursorCanMove;
		}

		static InputManager& GetInstance()
		{
			std::call_once(initFlag, []()
			{
				instance.reset(new InputManager);
			});

			return *instance;
		}

	private:

		InputManager() = default;

		void OnWindowActivated(CoreWindow const&, WindowActivatedEventArgs const& args)
		{
			if (args.WindowActivationState() == CoreWindowActivationState::CodeActivated || args.WindowActivationState() == CoreWindowActivationState::PointerActivated)
				isWindowActive = true;
			else if (args.WindowActivationState() == CoreWindowActivationState::Deactivated)
				isWindowActive = false;
		}

		void OnMouseMoved(MouseDevice const&, MouseEventArgs const& args)
		{
			if (isWindowActive)
			{
				auto delta = args.MouseDelta();
				deltaX += delta.X;
				deltaY += delta.Y;
			}
		}

		int deltaX = 0;
		int deltaY = 0;

		bool cursorCanMove = true;
		bool isWindowActive = true;

		MouseDevice mouseDevice = nullptr;
		event_token mouseMovedToken{};

		CoreWindow window = nullptr;

		static Unique<InputManager> instance;
		static std::once_flag initFlag;
	};

	Unique<InputManager> InputManager::instance;
	std::once_flag InputManager::initFlag;
}