#pragma once

#include <tiny_gltf.h>

#include "Renderer.h"


namespace UniDx {

// --------------------
// GltfModelクラス
// --------------------
class GltfModel : public Component
{
public:
    // glTF形式のモデルファイルを読み込む
    template<typename TVertex>
    bool Load(const std::wstring& modelPath, const std::wstring& shaderPath, const std::wstring& texturePath)
    {
        // モデル
        if (!Load<TVertex>(modelPath)) return false;

        // マテリアルとシェーダー
        auto material = std::make_shared<Material>();
        if(! material->shader.compile<TVertex>(shaderPath)) return false;

        // テクスチャ
        auto tex = std::make_unique<Texture>();
        SetAddressModeUV(tex.get(), 0);     // モデルで指定されたラップモード
        if(! tex->Load(texturePath)) return false;
        material->AddTexture(move(tex));

        AddMaterial(material);
        return true;
    }

    // glTF形式のモデルファイルを読み込む
    template<typename TVertex>
    bool Load(const std::wstring& filePath)
    {
        if (load_(filePath))
        {
            for (auto& sub : submesh)
            {
                sub->createBuffer<TVertex>();
            }
            return true;
        }
        return false;
    }

    // 生成した全ての Renderer にマテリアルを追加
    void AddMaterial(std::shared_ptr<Material> material)
    {
        for (auto& r : renderer)
        {
            r->AddMaterial(material);
        }
    }

    // Textureのラップモードをこのモデルの指定インデクスのテクスチャ設定に合わせる
    void SetAddressModeUV(Texture* texture, int texIndex) const;

protected:
    std::vector<MeshRenderer*> renderer;
    std::unique_ptr< tinygltf::Model> model;
    std::vector< std::shared_ptr<SubMesh> > submesh;

    bool load_(const std::wstring& filePath);
    void createNodeRecursive(const tinygltf::Model& model, int nodeIndex, GameObject* parentGO);
};


} // namespace UniDx
