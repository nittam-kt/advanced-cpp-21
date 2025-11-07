#pragma once

#include <memory>
#include <map>

#include <SimpleMath.h>

#include "Component.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

namespace UniDx {

class Camera;
class Texture;

// --------------------
// Materialクラス
// --------------------
class Material : public Object
{
public:
    Shader shader;
    ReadOnlyProperty<Texture*> mainTexture;
    bool zTest;
    D3D11_DEPTH_WRITE_MASK depthWrite;
    D3D11_COMPARISON_FUNC ztest;

    // コンストラクタ
    Material();

    // マテリアル情報設定。Render()内で呼び出す
    void setForRender() const;

    // テクスチャ追加
    void AddTexture(std::shared_ptr<Texture> tex);

    // 有効化
    virtual void OnEnable();

    std::span<std::shared_ptr<Texture>> getTextures() { return textures; }

protected:
    ComPtr<ID3D11Buffer> constantBuffer;
    ComPtr<ID3D11DepthStencilState> depthStencilState;

    std::vector<std::shared_ptr<Texture>> textures;
};


} // namespace UniDx
