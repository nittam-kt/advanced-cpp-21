#include "pch.h"

#include <UniDx/Image.h>
#include <UniDx/D3DManager.h>
#include <UniDx/Canvas.h>
#include <UniDx/Material.h>
#include <UniDx/Shader.h>

using namespace DirectX;

namespace {

constexpr Vector3 image_positions[] = {
    {-0.5f, -0.5f, 0}, {-0.5f,  0.5f, -0.5f}, {0.5f, -0.5f, 0}, {0.5f, 0.5f, -0.5f}
};

constexpr Vector2 image_uvs[] = {
	{0,1}, {0,0}, {1,1}, {1,0}
};

}

namespace UniDx {

// -----------------------------------------------------------------------------
// 頂点シェーダー側と共有する、モデルビュー行列の定数バッファ
//     UniDxではすべてのシェーダーでスロット0番に共通で指定する
// -----------------------------------------------------------------------------
struct VSConstantBuffer0
{
    Matrix world;
    Matrix view;
    Matrix projection;
};


// コンストラクタ
Image::Image()
{
	mesh = make_unique<SubMesh>();
	mesh->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	colors.resize(4, Color(1, 1, 1, 1));
}


void Image::OnEnable()
{
	UIBehaviour::OnEnable();

	// 行列用の定数バッファ生成
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = sizeof(VSConstantBuffer0);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBuffer0.GetAddressOf());

	mesh->positions = std::span<const Vector3>(image_positions, std::size(image_positions));
	mesh->uv = std::span<const Vector2>(image_uvs, std::size(image_uvs));
	mesh->colors = colors;
}


void Image::Render(const Matrix& proj) const
{
	UIBehaviour::Render(proj);

	if (texture == nullptr)
	{
		if (mesh->vertexBuffer == nullptr)
		{
			mesh->createBuffer<VertexPC>();
		}
		owner->getDefaultMaterial()->setForRender();
	}
	else
	{
		if (mesh->vertexBuffer == nullptr)
		{
			mesh->createBuffer<VertexPTC>();
		}
		owner->getDefaultTextureMaterial()->setForRender();
	}

	// 定数バッファ
	ID3D11Buffer* cbs[1] = { constantBuffer0.Get() };
	D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(0, 1, cbs);

	// ─ ワールド行列を位置に合わせて作成
	VSConstantBuffer0 cb{};
	cb.world = transform->getLocalToWorldMatrix();
	cb.view = Matrix::Identity;
	cb.projection = proj;

	// 定数バッファ更新
	D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBuffer0.Get(), 0, nullptr, &cb, 0, 0);

	mesh->Render();
}

}
