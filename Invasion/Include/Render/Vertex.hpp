#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include "Math/Vector.hpp"
#include "Util/Typedefs.hpp"

using namespace Invasion::Math;
using namespace Invasion::Util;

namespace Invasion::Render
{
	struct Vertex
	{
		Vertex() = default;

		Vertex(const Vector<float, 3>& position, const Vector<float, 3>& color, const Vector<float, 3>& normal, const Vector<float, 2>& uvs)
		{
			this->position = position;
			this->color = color;
			this->normal = normal;
			this->uvs = uvs;
		}

		static ImmutableArray<D3D11_INPUT_ELEMENT_DESC, 4> GetInputLayout()
		{
			return
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "UVS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
		}
		
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 uvs;
	};
}