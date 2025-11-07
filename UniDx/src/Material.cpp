#include "pch.h"
#include <UniDx/Material.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>

namespace UniDx{

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


// -----------------------------------------------------------------------------
// コンストラクタ
// -----------------------------------------------------------------------------
Material::Material() :
    Object([this]() { return shader.name; }),
    mainTexture(
        [this]() { return textures.size() > 0 ? textures.front().get() : nullptr; }
    ),
    depthWrite(D3D11_DEPTH_WRITE_MASK_ALL), // デフォルトは書き込み有効
    ztest(D3D11_COMPARISON_LESS) // デフォルトは小さい値が手前
{
}


// -----------------------------------------------------------------------------
// レンダリング用にデバイスへ設定
// -----------------------------------------------------------------------------
void Material::setForRender() const
{
    shader.setToContext();
    for (auto& tex : textures)
    {
        tex->setForRender();
    }

    // デプス
    D3DManager::getInstance()->GetContext()->OMSetDepthStencilState(depthStencilState.Get(), 1);
}


// -----------------------------------------------------------------------------
// テクスチャ追加
// -----------------------------------------------------------------------------
void Material::AddTexture(std::shared_ptr<Texture> tex)
{
    textures.push_back(tex);
}


// -----------------------------------------------------------------------------
// 有効化
// -----------------------------------------------------------------------------
void Material::OnEnable()
{
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE; // 深度テスト有効
    dsDesc.DepthWriteMask = depthWrite; // 書き込み有効
    dsDesc.DepthFunc = ztest; // 小さい値が手前

    dsDesc.StencilEnable = FALSE;

    D3DManager::getInstance()->GetDevice()->CreateDepthStencilState(&dsDesc, &depthStencilState);
}


}
