#pragma once

#include <UniDx/Renderer.h>



// --------------------
// SquareThrustRendererクラス
// --------------------
class SquareThrustRenderer : public UniDx::MeshRenderer
{
public:
    template<typename TVertex>
    static std::unique_ptr<SquareThrustRenderer> create(const std::wstring& shaderPath, const std::wstring& texturePath)
    {
        auto ptr = std::unique_ptr<SquareThrustRenderer>(new SquareThrustRenderer());
        ptr->AddMaterial<TVertex>(shaderPath, texturePath);
        return ptr;
    }

protected:
    virtual void OnEnable() override;
};

