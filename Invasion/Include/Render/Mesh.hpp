#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include "ECS/GameObject.hpp"
#include "Render/Renderer.hpp"
#include "Render/Shader.hpp"
#include "Render/Texture.hpp"
#include "Render/Vertex.hpp"
#include "Util/Typedefs.hpp"

using namespace Invasion::ECS;
using namespace Invasion::Util;

namespace Invasion::Render
{
	struct DefaultMatrixBuffer
	{
		DirectX::XMMATRIX modelMatrix;
	};

	class Mesh : public Component
	{

	public:

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		void Generate()
		{
			std::shared_lock lock(*mutex);

			ComPtr<ID3D11Device5> device = Renderer::GetInstance().GetDevice();
			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			if (!vertexBuffer)
			{
				D3D11_BUFFER_DESC bufferDescription = {};

				bufferDescription.Usage = D3D11_USAGE_DEFAULT;
				bufferDescription.ByteWidth = sizeof(Vertex) * (UINT)vertices.Length();
				bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA subresourceData = {};
				subresourceData.pSysMem = vertices;

				device->CreateBuffer(&bufferDescription, &subresourceData, vertexBuffer.GetAddressOf());
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;

				context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				memcpy(mappedResource.pData, vertices, sizeof(Vertex) * vertices.Length());
				context->Unmap(vertexBuffer.Get(), 0);
			}

			if (!indexBuffer)
			{
				D3D11_BUFFER_DESC bufferDescription = {};

				bufferDescription.Usage = D3D11_USAGE_DEFAULT;
				bufferDescription.ByteWidth = sizeof(uint32_t) * (UINT)indices.Length();
				bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDescription.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA subresourceData = {};
				subresourceData.pSysMem = indices;

				device->CreateBuffer(&bufferDescription, &subresourceData, indexBuffer.GetAddressOf());
			}
			else
			{
				D3D11_MAPPED_SUBRESOURCE mappedResource;

				context->Map(indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				memcpy(mappedResource.pData, indices, sizeof(uint32_t) * indices.Length());
				context->Unmap(indexBuffer.Get(), 0);
			}
		}

		void Render() override
		{
			std::shared_lock lock(*mutex);

			ComPtr<ID3D11DeviceContext4> context = Renderer::GetInstance().GetContext();

			Shared<Shader> shader = GetGameObject()->GetComponent<Shader>();
			Shared<Texture> texture = GetGameObject()->GetComponent<Texture>();
			Shared<Transform> transform = GetGameObject()->GetComponent<Transform>();

			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
			context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			shader->SetConstantBuffer(SubShaderType::VERTEX, 0, DefaultMatrixBuffer{ Matrix<float, 4, 4>::Transpose(transform->GetModelMatrix()) });

			shader->Bind();
			texture->Bind();

			context->DrawIndexed((UINT)indices.Length(), 0, 0);
		}

		void SetVertices(const MutableArray<Vertex>& vertices)
		{
			std::unique_lock lock(*mutex);

			this->vertices = vertices;
		}

		void SetIndices(const MutableArray<uint32_t>& indices)
		{
			std::unique_lock lock(*mutex);

			this->indices = indices;
		}

		MutableArray<Vertex> GetVertices() const
		{
			return vertices;
		}

		MutableArray<uint32_t> GetIndices() const
		{
			return indices;
		}

		void Uninitialize() override
		{
			std::unique_lock lock(*mutex);

			vertexBuffer.Reset();
			indexBuffer.Reset();
		}

		static Shared<Mesh> Create(const MutableArray<Vertex>& vertices, const MutableArray<uint32_t>& indices)
		{
			Shared<Mesh> result(new Mesh());

			result->vertices = vertices;
			result->indices = indices;

			return result;
		}

	private:
		
		Mesh() = default;

		Shared<std::shared_mutex> mutex = std::make_shared<std::shared_mutex>();

		MutableArray<Vertex> vertices;
		MutableArray<uint32_t> indices;

		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

	};
}