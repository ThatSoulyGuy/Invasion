#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include "pch.h"
#include "Math/Vector.hpp"
#include "Util/Typedefs.hpp"

using namespace winrt::Windows::UI::Core;

using namespace Invasion::Math;
using namespace Invasion::Util;

namespace Invasion::Render
{
	class Renderer
	{

	public:
		
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void Initialize(const CoreWindow& window)
		{
			CreateDeviceAndSwapChain(window);
			CreateRenderTargetView();
			SetViewport({ (int)window.Bounds().Width, (int)window.Bounds().Height });
		}

		void Clear(const Vector<float, 4>& color)
		{
			float clearColor[] = { color[0], color[1], color[2], color[3] };

			context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
			context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
		}

		void Present()
		{
			swapChain->Present(1, 0);
		}

		void Resize(const Vector<int, 2>& dimensions)
		{
			renderTargetView.Reset();

			swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

			CreateRenderTargetView();
			SetViewport(dimensions);
		}

		ComPtr<ID3D11Device5> GetDevice() const
		{
			return device;
		}

		ComPtr<ID3D11DeviceContext4> GetContext() const
		{
			return context;
		}

		ComPtr<IDXGISwapChain4> GetSwapChain() const
		{
			return swapChain;
		}

		ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const
		{
			return renderTargetView;
		}

		void Uninitialize()
		{
			renderTargetView.Reset();
			swapChain.Reset();
			context.Reset();
			device.Reset();
		}

		static Renderer& GetInstance()
		{
			std::call_once(initFlag, []() 
			{
				instance.reset(new Renderer);
			});

			return *instance;
		}

	private:
		
		Renderer() = default;
		
		void CreateDeviceAndSwapChain(const CoreWindow& window)
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};

			swapChainDescription.Width = 0;
			swapChainDescription.Height = 0;
			swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			swapChainDescription.Stereo = false;
			swapChainDescription.SampleDesc.Count = 1;
			swapChainDescription.SampleDesc.Quality = 0;
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.BufferCount = 2;
			swapChainDescription.Scaling = DXGI_SCALING_NONE;
			swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapChainDescription.Flags = 0;

			ComPtr<IDXGIDevice4> dxgiDevice;
			ComPtr<IDXGIAdapter> dxgiAdapter;
			ComPtr<IDXGIFactory2> dxgiFactory;
			ComPtr<IDXGISwapChain1> swapChain1;

			ComPtr<ID3D11Device> deviceOut;
			ComPtr<ID3D11DeviceContext> contextOut;

			D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
			UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
			
			creationFlags |= D3D11_CREATE_DEVICE_DEBUG;

#endif // _DEBUG

			D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, 1, D3D11_SDK_VERSION, deviceOut.GetAddressOf(), nullptr, contextOut.GetAddressOf());

			deviceOut.As(&device);
			contextOut.As(&context);

			device.As(&dxgiDevice);

			dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

			dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

			dxgiFactory->CreateSwapChainForCoreWindow(device.Get(), winrt::get_unknown(window), &swapChainDescription, nullptr, swapChain1.GetAddressOf());

			swapChain1.As(&swapChain);
		}

		void CreateRenderTargetView()
		{
			if (renderTargetView != nullptr)
				renderTargetView.Reset();
			
			ComPtr<ID3D11Texture2D> backBuffer;
			swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);

			device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
		}

		void SetViewport(const Vector<int, 2>& dimensions)
		{
			D3D11_VIEWPORT viewport = {};

			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.Width = static_cast<float>(dimensions[0]);
			viewport.Height = static_cast<float>(dimensions[1]);
			viewport.MinDepth = 0;
			viewport.MaxDepth = 1;

			context->RSSetViewports(1, &viewport);
		}

		ComPtr<ID3D11Device5> device;
		ComPtr<ID3D11DeviceContext4> context;
		ComPtr<IDXGISwapChain4> swapChain;
		ComPtr<ID3D11RenderTargetView> renderTargetView;

		static Unique<Renderer> instance;
		static std::once_flag initFlag;
	};

	Unique<Renderer> Renderer::instance;
	std::once_flag Renderer::initFlag;
}