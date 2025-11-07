#include "SquareThrustRenderer.h"

#include <UniDx/Texture.h>
#include <UniDx/Camera.h>

#include "SquareThrustRenderer.h"


namespace {

constexpr Vector3 positions[] = {
    {0.0f, 0.0f, 0.0f},
};

constexpr Vector2 uvs[] = {
    {0,0},
};

}



void SquareThrustRenderer::OnEnable()
{
    MeshRenderer::OnEnable();

    // メッシュの初期化
    auto submesh = std::make_unique<UniDx::SubMesh>();
    submesh->positions = std::span<const Vector3>(positions, std::size(positions));
    submesh->uv = std::span<const Vector2>(uvs, std::size(uvs));
    submesh->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    submesh->createBuffer<UniDx::VertexPT>();

    mesh.submesh.push_back(std::move(submesh));
}

