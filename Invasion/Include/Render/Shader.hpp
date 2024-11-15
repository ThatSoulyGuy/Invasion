#pragma once

#include <stdexcept>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "ECS/Component.hpp"
#include "Render/Renderer.hpp"
#include "Render/Vertex.hpp"
#include "Util/IO/AssetPath.hpp"
#include "Util/IO/FileSystem.hpp"
#include "Util/Typedefs.hpp"

using namespace Invasion::ECS;
using namespace Invasion::Util;
using namespace Invasion::Util::IO;

namespace Invasion::Render
{
	enum class SubShaderType
	{
		VERTEX,
		PIXEL,
		DOMAIN,
		HULL,
		GEOMETRY,
		COMPUTE	
	};

	class Shader : public Component
	{

	public:

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		void Bind()
		{
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			context->IASetInputLayout(inputLayout.Get());

			constantBuffers.ForEach([&](const Tuple<SubShaderType, uint32_t>& key, const ComPtr<ID3D11Buffer>& value)
			{
				if (key.Get<0>() == SubShaderType::VERTEX)
					context->VSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::PIXEL)
					context->PSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::DOMAIN)
					context->DSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::HULL)
					context->HSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::GEOMETRY)
					context->GSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::COMPUTE)
					context->CSSetConstantBuffers(key.Get<1>(), 1, value.GetAddressOf());
			});

			shaderResourceViews.ForEach([&](const Tuple<SubShaderType, uint32_t>& key, const ComPtr<ID3D11ShaderResourceView>& value)
			{
				if (key.Get<0>() == SubShaderType::VERTEX)
					context->VSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::PIXEL)
					context->PSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::DOMAIN)
					context->DSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::HULL)
					context->HSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::GEOMETRY)
					context->GSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::COMPUTE)
					context->CSSetShaderResources(key.Get<1>(), 1, value.GetAddressOf());
			});

			samplerStates.ForEach([&](const Tuple<SubShaderType, uint32_t>& key, const ComPtr<ID3D11SamplerState>& value)
			{
				if (key.Get<0>() == SubShaderType::VERTEX)
					context->VSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::PIXEL)
					context->PSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::DOMAIN)
					context->DSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::HULL)
					context->HSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::GEOMETRY)
					context->GSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
				else if (key.Get<0>() == SubShaderType::COMPUTE)
					context->CSSetSamplers(key.Get<1>(), 1, value.GetAddressOf());
			});

			context->VSSetShader(vertexShader.Get(), nullptr, 0);
			context->PSSetShader(pixelShader.Get(), nullptr, 0);
			context->DSSetShader(domainShader.Get(), nullptr, 0);
			context->HSSetShader(hullShader.Get(), nullptr, 0);
			context->GSSetShader(geometryShader.Get(), nullptr, 0);
			context->CSSetShader(computeShader.Get(), nullptr, 0);
		}

		void SetConstantBuffer(SubShaderType type, uint32_t slot, const ComPtr<ID3D11Buffer>& buffer)
		{
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			constantBuffers[{type, slot}] = buffer;
		}

		template <typename T>
		void SetConstantBuffer(SubShaderType type, uint32_t slot, const T& data)
		{
			ComPtr<ID3D11Device5> device = Renderer::GetInstance().GetDevice();
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			if (!constantBuffers.Contains({ type, slot }))
			{
				D3D11_BUFFER_DESC bufferDescription = {};

				bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
				bufferDescription.ByteWidth = sizeof(T);
				bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				ComPtr<ID3D11Buffer> buffer;

				D3D11_SUBRESOURCE_DATA subresourceData = {};

				subresourceData.pSysMem = &data;

				device->CreateBuffer(&bufferDescription, &subresourceData, buffer.GetAddressOf());

				constantBuffers[{type, slot}] = buffer;
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;

				context->Map(constantBuffers[{type, slot}].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				memcpy(mappedResource.pData, &data, sizeof(T));
				context->Unmap(constantBuffers[{type, slot}].Get(), 0);
			}
		}

		void SetShaderResourceView(SubShaderType type, uint32_t slot, const ComPtr<ID3D11ShaderResourceView>& srv)
		{
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			shaderResourceViews[{type, slot}] = srv;
		}

		void SetSamplerState(SubShaderType type, uint32_t slot, const ComPtr<ID3D11SamplerState>& sampler)
		{
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			samplerStates[{type, slot}] = sampler;
		}
		
		NarrowString GetName() const
		{
			return name;
		}

		AssetPath GetPath() const
		{
			return path;
		}

		NarrowString GetVertexShaderPath() const
		{
			return vertexShaderPath;
		}

		NarrowString GetPixelShaderPath() const
		{
			return pixelShaderPath;
		}

		NarrowString GetDomainShaderPath() const
		{
			return domainShaderPath;
		}

		NarrowString GetHullShaderPath() const
		{
			return hullShaderPath;
		}

		NarrowString GetGeometryShaderPath() const
		{
			return geometryShaderPath;
		}

		NarrowString GetComputeShaderPath() const
		{
			return computeShaderPath;
		}

		void Uninitialize_NoOverride()
		{
			vertexShader.Reset();
			pixelShader.Reset();
			domainShader.Reset();
			hullShader.Reset();
			geometryShader.Reset();
			computeShader.Reset();
			inputLayout.Reset();
		}

		static Shared<Shader> Create(const NarrowString& name, const AssetPath& path)
		{
			Shared<Shader> result(new Shader());

			result->name = name;
			result->path = path;
			result->vertexShaderPath = path.GetFullPath() + "Vertex.hlsl";
			result->pixelShaderPath = path.GetFullPath() + "Pixel.hlsl";
			result->domainShaderPath = path.GetFullPath() + "Domain.hlsl";
			result->hullShaderPath = path.GetFullPath() + "Hull.hlsl";
			result->geometryShaderPath = path.GetFullPath() + "Geometry.hlsl";
			result->computeShaderPath = path.GetFullPath() + "Compute.hlsl";

			result->Generate();

			return result;
		}

	private:

		Shader() = default;

		void Generate()
		{
			ComPtr<ID3D11Device5> device = Renderer::GetInstance().GetDevice();

			ComPtr<ID3DBlob> vertexShaderBlob;
			ComPtr<ID3DBlob> pixelShaderBlob;
			ComPtr<ID3DBlob> domainShaderBlob;
			ComPtr<ID3DBlob> hullShaderBlob;
			ComPtr<ID3DBlob> geometryShaderBlob;
			ComPtr<ID3DBlob> computeShaderBlob;

			HRESULT result = S_OK;

			if (!vertexShader)
				result = CompileShader<SubShaderType::VERTEX>(vertexShaderPath, "Main", device, vertexShaderBlob);

			if (!pixelShader)
				result = CompileShader<SubShaderType::PIXEL>(pixelShaderPath, "Main", device, pixelShaderBlob);

			if (FAILED(result))
			{
				OutputDebugStringA("Failed to compile required shaders");
				throw std::runtime_error("Failed to compile required shaders");
				return;
			}

			if (!domainShader)
				CompileShader<SubShaderType::DOMAIN>(domainShaderPath, "Main", device, domainShaderBlob);

			if (!hullShader)
				CompileShader<SubShaderType::HULL>(hullShaderPath, "Main", device, hullShaderBlob);

			if (!geometryShader)
				CompileShader<SubShaderType::GEOMETRY>(geometryShaderPath, "Main", device, geometryShaderBlob);

			if (!computeShader)
				CompileShader<SubShaderType::COMPUTE>(computeShaderPath, "Main", device, computeShaderBlob);

			if (!vertexShaderBlob || !pixelShaderBlob)
				return;

			if (!inputLayout)
			{
				ImmutableArray<D3D11_INPUT_ELEMENT_DESC, 4> inputElementDescription = Vertex::GetInputLayout();

				device->CreateInputLayout(inputElementDescription, (UINT)inputElementDescription.Length(), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
			}
		}

		template <SubShaderType T>
		HRESULT CompileShader(const NarrowString& path, const NarrowString& entryPoint, const ComPtr<ID3D11Device>& device, ComPtr<ID3DBlob>& shaderBlobOut)
		{
			if (!FileSystem::FileExists(path))
				return E_FAIL;

			ComPtr<ID3DBlob> errorBlob;

			const char* shaderModel =
				(T == SubShaderType::VERTEX) ? "vs_5_0" :
				(T == SubShaderType::PIXEL) ? "ps_5_0" :
				(T == SubShaderType::DOMAIN) ? "ds_5_0" :
				(T == SubShaderType::HULL) ? "hs_5_0" :
				(T == SubShaderType::GEOMETRY) ? "gs_5_0" :
				(T == SubShaderType::COMPUTE) ? "cs_5_0" : "";

			std::wstring wpath = path;
			std::string nentryPoint = entryPoint;

			HRESULT result = D3DCompileFromFile(
				wpath.c_str(),
				nullptr,
				D3D_COMPILE_STANDARD_FILE_INCLUDE,
				nentryPoint.c_str(),
				shaderModel,
				0,
				0,
				shaderBlobOut.GetAddressOf(),
				errorBlob.GetAddressOf()
			);

			if (FAILED(result))
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());

				return result;
			}

			if constexpr (T == SubShaderType::VERTEX)
				return device->CreateVertexShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
			else if constexpr (T == SubShaderType::PIXEL)
				return device->CreatePixelShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
			else if constexpr (T == SubShaderType::DOMAIN)
				return device->CreateDomainShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, domainShader.GetAddressOf());
			else if constexpr (T == SubShaderType::HULL)
				return device->CreateHullShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, hullShader.GetAddressOf());
			else if constexpr (T == SubShaderType::GEOMETRY)
				return device->CreateGeometryShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, geometryShader.GetAddressOf());
			else if constexpr (T == SubShaderType::COMPUTE)
				return device->CreateComputeShader(shaderBlobOut->GetBufferPointer(), shaderBlobOut->GetBufferSize(), nullptr, computeShader.GetAddressOf());
			else
				return E_FAIL;
		}

		NarrowString name;

		AssetPath path;  

		NarrowString vertexShaderPath;
		NarrowString pixelShaderPath;
		NarrowString domainShaderPath;
		NarrowString hullShaderPath;
		NarrowString geometryShaderPath;
		NarrowString computeShaderPath;

		ComPtr<ID3D11VertexShader> vertexShader;
		ComPtr<ID3D11PixelShader> pixelShader;
		ComPtr<ID3D11DomainShader> domainShader;
		ComPtr<ID3D11HullShader> hullShader;
		ComPtr<ID3D11GeometryShader> geometryShader;
		ComPtr<ID3D11ComputeShader> computeShader;

		ComPtr<ID3D11InputLayout> inputLayout;

		UnorderedMap<Tuple<SubShaderType, uint32_t>, ComPtr<ID3D11Buffer>> constantBuffers;
		UnorderedMap<Tuple<SubShaderType, uint32_t>, ComPtr<ID3D11ShaderResourceView>> shaderResourceViews;
		UnorderedMap<Tuple<SubShaderType, uint32_t>, ComPtr<ID3D11SamplerState>> samplerStates;
	};
}