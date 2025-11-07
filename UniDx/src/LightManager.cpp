#include "pch.h"
#include <UniDx/LightManager.h>

#include <UniDx/Light.h>
#include <UniDx/D3DManager.h>


namespace UniDx
{

using namespace std;

// Lightを登録
bool LightManager::registerLight(Light* light)
{
    for (size_t i = 0; i < lights_.size(); ++i)
    {
        if (lights_[i] == nullptr)
        {
            lights_[i] = light;
            return true;
        }
        if (lights_[i] == light)
        {
            return true; // 登録済み
        }
    }

    // 無効化されたものがなければ追加
    if (lights_.size() < LightCountMax)
    {
        lights_.push_back(light);
        return true;
    }
    return false;
}


// Lightの登録を解除
void LightManager::unregisterLight(Light* light)
{
    for (size_t i = 0; i < lights_.size(); ++i)
    {
        if (lights_[i] == light)
        {
            lights_[i] = light;
            return;
        }
    }
}


// ライト情報を定数バッファに反映
void LightManager::updateLightCBuffer()
{
    // 無効になっているものをvectorから削除
    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end();)
    {
        if (*it == nullptr)
        {
            it = lights_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // --- GPULight作成 ---
    gpuLights_.clear();
    gpuLights_.reserve(lights_.size());
    for (Light* l : lights_)
    {
        GPULight g{};
        g.color = l->color;
        g.color.A(l->intensity);
        g.type = static_cast<uint32_t>(l->type);

        auto world = l->transform->getLocalToWorldMatrix();       // ワールド行列取得

        switch (l->type)
        {
        case LightType_Directional:
            g.positionOrDirWS = -world.Forward(); // 方向
            break;
        case LightType_Point:
            g.positionOrDirWS = world.Translation();
            g.rangeOrInvCos = 1.0f / l->range;
            break;
        case LightType_Spot:
            g.positionOrDirWS = world.Translation();
            g.spotDirWS = -world.Forward();
            g.rangeOrInvCos = 1.0f / l->range;
            g.spotOuterCos = cosf(DirectX::XMConvertToRadians(l->spotAngle * 0.5f));
            break;
        }
        gpuLights_.push_back(g);
    }

    // --- バッファ容量を確保 ---
    if (capacity_ < gpuLights_.size() || lightBuf_ == nullptr)
    {
        capacity_ = gpuLights_.size();
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = UINT(sizeof(GPULight) * capacity_);
        bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bd.StructureByteStride = sizeof(GPULight);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        lightBuf_.Reset();
        D3DManager::getInstance()->GetDevice()->CreateBuffer(&bd, nullptr, lightBuf_.GetAddressOf());

        // SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = DXGI_FORMAT_UNKNOWN;  // Structured
        sd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        sd.Buffer.NumElements = UINT(capacity_);
        D3DManager::getInstance()->GetDevice()->CreateShaderResourceView(lightBuf_.Get(), &sd, lightSRV_.GetAddressOf());
    }

    // --- Map & Copy ---
    if (!gpuLights_.empty())
    {
        D3D11_MAPPED_SUBRESOURCE ms{};
        D3DManager::getInstance()->GetContext()->Map(lightBuf_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
        memcpy(ms.pData, gpuLights_.data(), sizeof(GPULight) * gpuLights_.size());
        D3DManager::getInstance()->GetContext()->Unmap(lightBuf_.Get(), 0);
    }
    /*
    // --- LightCount CB 更新 ---
    struct { uint32_t Count; float pad[3]; } meta = { uint32_t(gpuLights_.size()) };
    if (!metaCB_)
    {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = 16;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3DManager::instance->GetDevice()->CreateBuffer(&bd, nullptr, metaCB_.GetAddressOf());
    }
    D3DManager::instance->GetContext()->UpdateSubresource(metaCB_.Get(), 0, nullptr, &meta, 0, 0);
*/
    // ライト
    ID3D11ShaderResourceView* srv = lightSRV_.Get();
    D3DManager::getInstance()->GetContext()->PSSetShaderResources(UNIDX_PS_SLOT_LIGHTS, 1, &srv);
}


} // UniDx
