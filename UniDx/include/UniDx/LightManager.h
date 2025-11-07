#pragma once

#include <vector>
#include <array>
#include <SimpleMath.h>

#include "UniDxDefine.h"
#include "Singleton.h"

namespace UniDx
{

class Light;

struct GPULight // 16byte aligned
{
    Vector3 positionOrDirWS;
    float rangeOrInvCos;
    Color color;
    Vector3 spotDirWS;
    float spotOuterCos;
    uint32_t  type;
    uint32_t shadowIndex;
    uint32_t  padding[2];
};


// --------------------
// LightManager
// --------------------
class LightManager : public Singleton<LightManager>
{
public:
    static constexpr int LightCountMax = 8;

    bool registerLight(Light* light);
    void unregisterLight(Light* light);

    // ライト情報を定数バッファに反映
    virtual void updateLightCBuffer();

private:
    std::vector<Light*> lights_;
    size_t              capacity_ = 0;

    std::vector<GPULight> gpuLights_;
    Microsoft::WRL::ComPtr<ID3D11Buffer>           lightBuf_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>lightSRV_;
//    Microsoft::WRL::ComPtr<ID3D11Buffer>           metaCB_;
};

}
