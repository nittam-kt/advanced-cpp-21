#pragma once

#include <memory>
#include <SimpleMath.h>

#include "UniDxDefine.h"
#include "Component.h"
#include "GameObject.h"

using namespace DirectX::SimpleMath;

namespace UniDx {


// --------------------
// Transformクラス
// --------------------
class Transform : public Component
{
public:
    // ローカルの姿勢
    Property<Vector3> localPosition;
    Property<Quaternion> localRotation;
    Property<Vector3> localScale;

    // ワールド空間のプロパティ
    Property<Vector3> position;
    Property<Quaternion> rotation;

    Transform* parent = nullptr;

    const GameObjectContainer& getChildGameObjects() { return children; }

    // 親の変更
    GameObject* SetParent(Transform* newParent);

    // 親のいないTransformを持つGameObjectに親を設定
    static void SetParent(unique_ptr<GameObject> gameObjectPtr, Transform* newParent);

    // 子の数を取得
    size_t childCount() const { return children.size(); }

    // 子を取得
    Transform* GetChild(size_t index) const;

    // ローカル行列
    const Matrix& GetLocalMatrix() const {
        if (m_dirty) {
            m_localMatrix = Matrix::CreateScale(_localScale)
                * Matrix::CreateFromQuaternion(_localRotation)
                * Matrix::CreateTranslation(_localPosition);
        }
        return m_localMatrix;
    }

    // ワールド行列
    const Matrix& getLocalToWorldMatrix() const {
        updateMatrices();
        return m_worldMatrix;
    }

    Transform()
        : localPosition(
            // getter
            [this]() { return _localPosition; },
            // setter
            [this](Vector3 v) { _localPosition = v; m_dirty = true; }
        ),
        localRotation(
            [this]() { return _localRotation; },
            [this](Quaternion q) { _localRotation = q; m_dirty = true; }
        ),
        localScale(
            [this]() { return _localScale; },
            [this](Vector3 v) { _localScale = v; m_dirty = true; }
        ),
        position(
            // getter: グローバル座標
            [this]() {
                updateMatrices();
                return m_worldMatrix.Translation();
            },
            // setter: グローバル座標からlocalPositionを逆算
            [this](Vector3 worldPos) {
                if (parent) {
                    parent->updateMatrices();
                    Matrix invParent = parent->m_worldMatrix.Invert();
                    _localPosition = Vector3::Transform(worldPos, invParent);
                } else {
                    _localPosition = worldPos;
                }
                m_dirty = true;
            }
        ),
        rotation(
            [this]() {
                updateMatrices();
                // ワールド行列からクォータニオンを取得
                Vector3 s, t;
                Quaternion q;
                m_worldMatrix.Decompose(s, q, t);
                return q;
            },
            [this](Quaternion worldRot) {
                if (parent) {
                    parent->updateMatrices();
                    // 親のワールド回転の逆を掛けてローカル回転を算出
                    Quaternion parentWorldRot, parentWorldRotInv;
                    Vector3 s, t;
                    parent->m_worldMatrix.Decompose(s, parentWorldRot, t);
                    parentWorldRot.Inverse(parentWorldRotInv);
                    _localRotation = Quaternion::Concatenate(worldRot, parentWorldRotInv);
                }
                else {
                    _localRotation = worldRot;
                }
                m_dirty = true;
            }
        )
    {
    }

    virtual ~Transform();

    // ローカル空間の方向ベクトルをワールド空間の方向ベクトルに変換
    Vector3 TransformDirection(Vector3 localDirection) const {
        // 平行移動成分を除外した回転・スケールのみ適用
        Matrix m = getLocalToWorldMatrix();
        m.Translation(Vector3(0, 0, 0)); // 平行移動を除外
        return Vector3::TransformNormal(localDirection, m);
    }

    // ローカル空間のベクトルをワールド空間のベクトルに変換（スケール・回転のみ、平行移動なし）
    Vector3 TransformVector(Vector3 localVector) const {
        // TransformDirectionと同じ実装（Unityと同様の意味）
        return TransformDirection(localVector);
    }

    // ローカル空間の座標をワールド空間の座標に変換
    Vector3 TransformPoint(Vector3 localPoint) const {
        return Vector3::Transform(localPoint, getLocalToWorldMatrix());
    }

private:
    // ダーティフラグと行列
    mutable bool m_dirty = true;
    mutable Matrix m_localMatrix = Matrix::Identity;
    mutable Matrix m_worldMatrix = Matrix::Identity;

    bool dirtyInHierarchy() const { return m_dirty || parent && parent->dirtyInHierarchy(); }

    Vector3 _localPosition{ 0,0,0 };
    Quaternion _localRotation = Quaternion::Identity;
    Vector3 _localScale{ 1,1,1 };

    // 子GameObject
    // トップ以外のGameObjectはTransformによって保持される
    GameObjectContainer children;

    // 行列の更新
    void updateMatrices() const;
};

} // namespace UniDx
