#include "pch.h"
#include <UniDx/Renderer.h>

#include <UniDx/D3DManager.h>
#include <UniDx/Texture.h>
#include <UniDx/Camera.h>
#include <UniDx/Material.h>
#include <UniDx/SceneManager.h>

namespace UniDx{


// -----------------------------------------------------------------------------
// 有効化
// -----------------------------------------------------------------------------
void Renderer::OnEnable()
{
    // マテリアル有効化
    for (auto& material : materials)
    {
        material->OnEnable();
    }

    // 行列用の定数バッファ生成
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = sizeof(VSConstantBuffer0);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    D3DManager::getInstance()->GetDevice()->CreateBuffer(&desc, nullptr, constantBuffer0.GetAddressOf());
}


// -----------------------------------------------------------------------------
// 現在の姿勢とカメラをシェーダーの定数バッファに転送
// -----------------------------------------------------------------------------
void Renderer::updatePositionCameraCBuffer(const UniDx::Camera& camera) const
{
    // ワールド行列を transform から合わせて作成
    VSConstantBuffer0 cb{};
    cb.world = transform->getLocalToWorldMatrix();
    cb.view = camera.GetViewMatrix();
    cb.projection = camera.GetProjectionMatrix(16.0f/9.0f);

    // 定数バッファ更新
    ID3D11Buffer* cbs[1] = { constantBuffer0.Get() };
    D3DManager::getInstance()->GetContext()->VSSetConstantBuffers(0, 1, cbs);
    D3DManager::getInstance()->GetContext()->UpdateSubresource(constantBuffer0.Get(), 0, nullptr, &cb, 0, 0);
}


// -----------------------------------------------------------------------------
// 現在の姿勢とカメラをシェーダーの定数バッファに転送
// -----------------------------------------------------------------------------
void Renderer::setShaderForRender() const
{
    if (materials.size() == 0)
    {
//        SceneManager::getInstance()->GetDefaultMaterial()->setForRender();
    }
    for(auto& material : materials)
    {
        material->setForRender();
    }
}


// -----------------------------------------------------------------------------
// メッシュを使って描画
// -----------------------------------------------------------------------------
void MeshRenderer::Render(const Camera& camera) const
{
    //-----------------------------
    // シェーダーをセット
    //-----------------------------
    setShaderForRender();

    // 現在のTransformとカメラの情報をシェーダーのConstantBufferに転送
    updatePositionCameraCBuffer(camera);

    //-----------------------------
    // 描画実行
    //-----------------------------
    mesh.Render();
}

}
