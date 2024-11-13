#pragma once

#include <d3d11_4.h>
#include <DirectXTex/DirectXTex.h>
#include "Render/Renderer.hpp"
#include "Util/IO/AssetPath.hpp"
#include "Util/IO/FileSystem.hpp"
#include "Util/Typedefs.hpp"

using namespace Invasion::Util;
using namespace Invasion::Util::IO;

namespace Invasion::Render
{
	class Texture
	{
		
	public:

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		void Bind(uint32_t slot = 0) const
		{
			Renderer::GetInstance().GetContext()->PSSetShaderResources(slot, 1, shaderResourceView.GetAddressOf());
			Renderer::GetInstance().GetContext()->PSSetSamplers(slot, 1, samplerState.GetAddressOf());
		}

		NarrowString GetName() const
		{
			return name;
		}

		AssetPath GetPath() const
		{
			return path;
		}

		D3D11_SAMPLER_DESC GetSamplerDescription() const
		{
			return samplerDescription;
		}

		void Uninitialize_NoOverride()
		{
			shaderResourceView.Reset();
			samplerState.Reset();
		}

		static Shared<Texture> Create(const NarrowString& name, const AssetPath& path, const D3D11_SAMPLER_DESC& samplerDescription)
		{
			Shared<Texture> result(new Texture());

			result->name = name;
			result->path = path;
			result->fullPath = path.GetFullPath();
			result->samplerDescription = samplerDescription;

			result->Generate();

			return result;
		}

	private:

		Texture() = default;

        void Generate()
        {
            DirectX::TexMetadata metadata;
            DirectX::ScratchImage scratchImage;

            std::wstring wfullPath = fullPath;

            if (FAILED(DirectX::LoadFromDDSFile(wfullPath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage)))
                throw std::runtime_error(NarrowString("Failed to load texture: ") + fullPath);
            
            if (FAILED(DirectX::CreateShaderResourceView(Renderer::GetInstance().GetDevice().Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, shaderResourceView.GetAddressOf())))
                throw std::runtime_error(NarrowString("Failed to create shader resource view for texture: ") + fullPath);
            
            if (FAILED(Renderer::GetInstance().GetDevice()->CreateSamplerState(&samplerDescription, samplerState.GetAddressOf())))
                throw std::runtime_error(NarrowString("Failed to create sampler state for texture: ") + fullPath);
        }

		NarrowString name;

		AssetPath path;

		NarrowString fullPath;

		D3D11_SAMPLER_DESC samplerDescription = {};

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> shaderResourceView;
		ComPtr<ID3D11SamplerState> samplerState;

	};
}