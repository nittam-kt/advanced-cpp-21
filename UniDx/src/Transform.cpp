#include "pch.h"


namespace UniDx
{

Transform::~Transform() 
{
    for (auto& child : children)
    {
        if (child) child->transform->parent = nullptr;
    }
}


// 親の変更
GameObject* Transform::SetParent(Transform * newParent)
{
    // 親のTransformから自分を外す
    if (parent == nullptr)
    {
        // 新規Transformに親を設定する場合はsmart_ptrを渡すstatic版を使ってください
        abort();
        return nullptr;
    }
    auto& siblings = parent->children;

    // 以前の親からGameObjectのスマートポインタを所有権ごと移動
    auto it = std::find_if(
        siblings.begin(), siblings.end(),
        [this](const unique_ptr<GameObject>& ptr) { return ptr->transform == this; });
    assert(it != siblings.end());

    GameObject* gameObject_ptr = it->get();
    assert(gameObject_ptr != nullptr);

    // 元の親から削除
    siblings.erase(it);

    // 新しい親を設定
    parent = newParent;

    if (parent)
    {
        // 新しい親に自分を持つGameObjectを追加
        parent->children.push_back(std::move(*it));
    }
    m_dirty = true;

    return gameObject_ptr;
}


void Transform::SetParent(unique_ptr<GameObject> gameObjectPtr, Transform* newParent)
{
    // 親のTransformから自分を外す
    if (gameObjectPtr->transform->parent != nullptr)
    {
        // すでに親がある場合はメンバ変数版を使ってください
        abort();
        return;
    }

    // 新しい親を設定
    gameObjectPtr->transform->parent = newParent;
    gameObjectPtr->transform->m_dirty = true;
    if (newParent)
    {
        // 新しい親に自分を持つGameObjectを追加
        newParent->children.push_back(std::move(gameObjectPtr));
    }
}


// 子を取得
Transform* Transform::GetChild(size_t index) const
{
    if (index < children.size() && children[index]) {
        return children[index]->transform;
    }
    return nullptr;
}


// 行列を更新
void Transform::updateMatrices() const
{
    if (parent) {
        parent->updateMatrices();
    }

    if (m_dirty)
    {
        m_localMatrix = Matrix::CreateScale(_localScale)
            * Matrix::CreateFromQuaternion(_localRotation)
            * Matrix::CreateTranslation(_localPosition);
        if (parent) {
            parent->updateMatrices();
            m_worldMatrix = m_localMatrix * parent->m_worldMatrix;
        }
        else {
            m_worldMatrix = m_localMatrix;
        }
        m_dirty = false;

        // 親の行列が変わったので、子も変わるように
        for (auto& c : children)
        {
            c->transform->m_dirty = true;
        }
    }
}


}
