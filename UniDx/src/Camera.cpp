#include "pch.h"
#include <UniDx/Camera.h>

namespace UniDx{

// static なメインカメラ
Camera* Camera::main;

Matrix Camera::GetViewMatrix() const
{
    // ワールド行列を分解
    Vector3 scale, translation;
    Quaternion rotation;
    Matrix mtx = transform->getLocalToWorldMatrix();
    mtx.Decompose(scale, rotation, translation);

    // スケールを(1,1,1)にして再構成
    Matrix worldNoScale = Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(translation);

    // 逆行列を返す（ビュー行列）
    return worldNoScale.Invert();
}


void Camera::OnEnable()
{
    if (main == nullptr)
    {
        main = this;
    }
}


void Camera::OnDisable()
{
    if (main == this)
    {
        main = nullptr;
    }
}

}
