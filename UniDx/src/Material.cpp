#include "pch.h"
#include <UniDx/Material.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>
#include <UniDx/ConstantBuffer.h>


namespace UniDx{

// -----------------------------------------------------------------------------
// コンストラクタ
// -----------------------------------------------------------------------------
Material::Material() :
    Object([this]() { return shader.name; }),
    color(1, 1, 1, 1),
    mainTexture(
        [this]() { return textures.size() > 0 ? textures.front().get() : nullptr; }
    ),
    depthWrite(D3D11_DEPTH_WRITE_MASK_ALL), // デフォルトは書き込み有効
    ztest(D3D11_COMPARISON_LESS), // デフォルトは小さい値が手前
    renderingMode(RenderingMode_Opaque)
{
}


// -----------------------------------------------------------------------------
// レンダリング用にデバイスへ設定
// -----------------------------------------------------------------------------
bool Material::setForRender() const
{
    if (renderingMode != D3DManager::getInstance()->getCurrentRenderingMode())
    {
        return false;
    }

    shader.setToContext();
    for (auto& tex : textures)
    {
        tex->setForRender();
    }

    // デプス
    D3DManager::getInstance()->GetContext()->OMSetDepthStencilState(depthStencilState.Get(), 1);

    // ブレンド
    D3DManager::getInstance()->GetContext()->OMSetBlendState(blendState.Get(), NULL, 0xffffffff);

    // 定数バッファ更新
    VSConstantBuffer1 cb{};
    cb.baseColor = color;

    ID3D11Buffer* cbs[1] = { constantBuffer1.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(1, 1, cbs);
    D3DManager::getInstance()->GetContext()->PSSetConstantBuffers(1, 1, cbs);
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBuffer1.Get(), 0, nullptr, &cb, 0, 0);

    return true;
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
    // デプスステート作成
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE; // 深度テスト有効
    dsDesc.DepthWriteMask = depthWrite; // 書き込み有効
    dsDesc.DepthFunc = ztest; // 小さい値が手前

    dsDesc.StencilEnable = FALSE;

    D3DManager::getInstance()->GetDevice()->CreateDepthStencilState(&dsDesc, &depthStencilState);

    // ブレンドステート作成
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;

    D3D11_RENDER_TARGET_BLEND_DESC& rt = blendDesc.RenderTarget[0];
    rt.BlendEnable = TRUE;
    rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp = D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt.DestBlendAlpha = D3D11_BLEND_ZERO;
    rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3DManager::getInstance()->GetDevice()->CreateBlendState(&blendDesc, &blendState);

    // マテリアル用の定数バッファ生成
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(VSConstantBuffer1);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBuffer1.GetAddressOf());
}


}
