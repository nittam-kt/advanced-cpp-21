#include "pch.h"
#include <UniDx/Physics.h>

#include <algorithm>

#include <UniDx/Collider.h>
#include <UniDx/Rigidbody.h>


namespace UniDx
{

using namespace std;

// 初期化
void PhysicsShape::initialize(Collider* collider)
{
    collider_ = collider;
    // moveBounds
}

// 衝突対象の新旧を調べて OnTrigger～, OnCollidion～ を呼ぶ
void PhysicsShape::collideCallback()
{
    // トリガーコールバック
    for (auto other : othersNew_)
    {
        auto inOld = std::ranges::find(others_, other);
        if (inOld == others_.end())
        {
            // 以前のリストに含まれていない＝新規
            getCollider()->gameObject->onTriggerEnter(other);
        }
        else
        {
            // 以前のリストに含まれていれば、一旦削除
            others_.erase(inOld);
        }

        // 新しいほうに含まれているので、Stay
        getCollider()->gameObject->onTriggerStay(other);
    }

    // 新しいリストになくて古いほうに残っている=離れた
    for (auto other : others_)
    {
        getCollider()->gameObject->onTriggerExit(other);
    }

    // 古いほうを削除して新しいほうを古いほうに
    others_.clear();
    std::swap(others_, othersNew_);

    // 衝突コールバック
    for (const auto& collision : collisionsNew_)
    {
        const auto& inOld = std::ranges::find_if(collisions_, [collision](auto i) {return i.collider == collision.collider; });
        if (inOld == collisions_.end())
        {
        // 以前のリストに含まれていない＝新規
            getCollider()->gameObject->onCollisionEnter(collision);
        }
        else
        {
        // 以前のリストに含まれていれば、一旦削除
            collisions_.erase(inOld);
        }

        // 新しいほうに含まれているので、Stay
        getCollider()->gameObject->onCollisionStay(collision);
    }

    // 新しいリストになくて古いほうに残っている=離れた
    for (auto col : collisions_)
    {
        getCollider()->gameObject->onCollisionExit(col);
    }

    // 古いほうを削除して新しいほうを古いほうに
    collisions_.clear();
    std::swap(collisions_, collisionsNew_);
}


// Rigidbodyを登録
void Physics::registerRigidbody(Rigidbody* rigidbody)
{
    PhysicsActor temp(rigidbody);
    physicsActors.insert(std::make_pair(rigidbody, temp));
}


// 3D形状を持ったコライダーの登録を解除
void Physics::unregisterRigidbody(Rigidbody* rigidbody)
{
    physicsActors.erase(rigidbody);
}


// 3D形状を持ったコライダーを登録
void Physics::register3d(Collider* collider)
{
    for(size_t i  = 0; i < physicsShapes.size(); ++i)
    {
        if(!physicsShapes[i].isValid())
        {
            physicsShapes[i].initialize(collider);
            return;
        }
        if (physicsShapes[i].getCollider() == collider)
        {
            return; // 登録済み
        }
    }

    // 無効化されたものがなければ追加
    physicsShapes.push_back(PhysicsShape());
    physicsShapes.back().initialize(collider);
}


// 3D形状を持ったコライダーの登録を解除
void Physics::unregister3d(Collider* collider)
{
    for (size_t i = 0; i < physicsShapes.size(); ++i)
    {
        if (physicsShapes[i].getCollider() == collider)
        {
            physicsShapes[i].setInvalid();
            return;
        }
    }
}


// 物理計算準備
void Physics::initializeSimulate(float step)
{
    // 無効になっているものをvectorから削除
    for (auto it = physicsActors.begin(); it != physicsActors.end();)
    {
        if (!it->second.isValid())
        {
            it = physicsActors.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 無効になっているシェイプを削除
    for (vector<PhysicsShape>::iterator it = physicsShapes.begin(); it != physicsShapes.end();)
    {
        if (!it->isValid())
        {
            it = physicsShapes.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // Rigidbodyの更新
    for (auto& act : physicsActors)
    {
        act.second.getRigidbody()->physicsUpdate();
        act.second.initCorrectBounds();
    }

    // Shapeの移動Boundsと次に当たるコライダーを初期化を更新
    for (auto& shape : physicsShapes)
    {
        shape.initOtherNew();

        Bounds bounds = shape.getCollider()->getBounds();
        auto rb = shape.getCollider()->attachedRigidbody;
        if (rb != nullptr)
        {
            bounds.Encapsulate(bounds.min() + rb->getMoveVector(step));
            bounds.Encapsulate(bounds.max() + rb->getMoveVector(step));
        }
        shape.moveBounds = bounds;
        Rigidbody* r = shape.getCollider()->attachedRigidbody;
        if (r != nullptr)
        {
            shape.actor = &physicsActors.at(r);
        }
        else
        {
            shape.actor = nullptr;
        }
    }
}


// 位置補正法（射影法）による物理計算のシミュレート
void Physics::simulatePositionCorrection(float step)
{
    initializeSimulate(step);

    // まずは当たりそうなペアをAABBで判定して抽出
    potentialPairs.clear();
    potentialPairsTrigger.clear();
    for (size_t i = 0; i < physicsShapes.size(); ++i)
    {
        for (size_t j = i + 1; j < physicsShapes.size(); ++j)
        {
            if (physicsShapes[i].moveBounds.Intersects(physicsShapes[j].moveBounds))
            {
                // 同じ Rigidbody に属しているコンパウンド同士は自己衝突なのでスキップ
                auto rbA = physicsShapes[i].getCollider()->attachedRigidbody;
                auto rbB = physicsShapes[j].getCollider()->attachedRigidbody;
                if (rbA && rbA == rbB) continue;

                // ペアを記憶
                if (physicsShapes[i].getCollider()->isTrigger || physicsShapes[j].getCollider()->isTrigger)
                {
                    // トリガー
                    potentialPairsTrigger.push_back({ &physicsShapes[i], &physicsShapes[j] });
                }
                else
                {
                    // コリジョン
                    potentialPairs.push_back({ &physicsShapes[i], &physicsShapes[j] });
                }
            }
        }
    }

    // 先に位置を更新する
    for (auto& act : physicsActors)
    {
        act.second.getRigidbody()->applyMove(step);
    }

    // トリガーチェックする
    for (auto& pair : potentialPairsTrigger)
    {
        if (pair.a->getCollider()->checkTrigger(pair.b->getCollider()))
        {
            pair.a->addTrigger(pair.b->getCollider());
            pair.b->addTrigger(pair.a->getCollider());
        }
    }

    // 衝突をチェックする
    for (auto& pair : potentialPairs)
    {
        if(pair.a->getCollider()->checkIntersect(pair.b->getCollider(), pair.a->actor, pair.b->actor))
        {
            Collision ca;
            ca.collider = pair.b->getCollider();
            pair.a->addCollide(ca);

            Collision cb;
            cb.collider = pair.a->getCollider();
            pair.b->addCollide(cb);
        }
    }

    // 衝突で生じた補正を含めて位置と速度を解決する
    for (auto& act : physicsActors)
    {
        act.second.getRigidbody()->solveCorrection(act.second.getCorrectPositionBounds(), act.second.getCorrectVelocityBounds());
    }

    // OnTrigger～, OnCollision～等のコールバックを呼び出す
    // TODO: 当たったRigidbodyがついているGameObjectでも呼び出す
    for (auto& shape : physicsShapes)
    {
        shape.collideCallback();
    }
}


// 物理計算のシミュレート
void Physics::simulate(float step)
{
    initializeSimulate(step);

    // まずは当たりそうなペアをAABBで判定して抽出
    potentialPairs.clear();
    for (size_t i = 0; i < physicsShapes.size(); ++i)
    {
        for (size_t j = i + 1; j < physicsShapes.size(); ++j)
        {
            if (physicsShapes[i].moveBounds.Intersects(physicsShapes[j].moveBounds))
            {
                // 同じ Rigidbody に属しているコンパウンド同士は自己衝突なのでスキップ
                auto rbA = physicsShapes[i].getCollider()->attachedRigidbody;
                auto rbB = physicsShapes[j].getCollider()->attachedRigidbody;
                if (rbA && rbA == rbB) continue;

                // ペアを記憶。ここでは詳細判定しない
                potentialPairs.push_back({ &physicsShapes[i], &physicsShapes[j] });
            }
        }
    }

    // 形状ごとに実衝突を確定する
    manifolds.clear();
    for (auto& pair : potentialPairs)
    {
        ContactManifold m;
//        if (intersectShapes(*pair.a, *pair.b, &m)) {  // ← ここが Narrow-phase
//            manifolds.push_back(m);
//        }
    }

    // ソルバ
    for (auto& m : manifolds)
    {
        Rigidbody* rbA = m.a->getCollider()->attachedRigidbody;
        Rigidbody* rbB = m.b->getCollider()->attachedRigidbody;

        // 速度レベルの反発インパルス (Impulses) をかける
        solveVelocityConstraint(rbA, rbB, m);

        // 位置のめり込みを少し戻す (Baumgarte / Position correction)
        solvePositionConstraint(rbA, rbB, m);
    }
}


void Physics::solveVelocityConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m)
{
    const float restitution = 0.2f;   // 反発係数

    for (int i = 0; i < m.numContacts; ++i)
    {
    }
}


void Physics::solvePositionConstraint(Rigidbody* A, Rigidbody* B, const ContactManifold& m)
{

}

// Raycast
bool Physics::Raycast(const Vector3& origin, const Vector3& direction, float maxDistance,
    RaycastHit* hitInfo, std::function<bool(const Collider*)> filter)
{
    // 無効な方向や負の距離はヒットしない
    const float eps = 1e-6f;
    if (maxDistance <= 0.0f) return false;
    if (fabs(direction.x) < eps && fabs(direction.y) < eps && fabs(direction.z) < eps) return false;

    bool hitAny = false;
    float bestT = std::numeric_limits<float>::infinity();

    // スラブ法で各 AABB とレイの交差を調べる
    for (const auto& shape : physicsShapes)
    {
        if (!shape.isValid()) continue;
        Collider* col = shape.getCollider();
        if (!col) continue;

        if (filter && !filter(col)) continue; // フィルタで除外

        Bounds b = col->getBounds();
        Vector3 bmin = b.min();
        Vector3 bmax = b.max();

        float tmin = 0.0f;
        float tmax = maxDistance;

        // X axis
        if (fabs(direction.x) < eps)
        {
            if (origin.x < bmin.x || origin.x > bmax.x) continue;
        }
        else
        {
            float inv = 1.0f / direction.x;
            float t1 = (bmin.x - origin.x) * inv;
            float t2 = (bmax.x - origin.x) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) continue;
        }

        // Y axis
        if (fabs(direction.y) < eps)
        {
            if (origin.y < bmin.y || origin.y > bmax.y) continue;
        }
        else
        {
            float inv = 1.0f / direction.y;
            float t1 = (bmin.y - origin.y) * inv;
            float t2 = (bmax.y - origin.y) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) continue;
        }

        // Z axis
        if (fabs(direction.z) < eps)
        {
            if (origin.z < bmin.z || origin.z > bmax.z) continue;
        }
        else
        {
            float inv = 1.0f / direction.z;
            float t1 = (bmin.z - origin.z) * inv;
            float t2 = (bmax.z - origin.z) * inv;
            float tn = std::min(t1, t2);
            float tf = std::max(t1, t2);
            tmin = std::max(tmin, tn);
            tmax = std::min(tmax, tf);
            if (tmin > tmax) continue;
        }

        // ここで交差。tmin が最初に交差するパラメータ
        float tHit = tmin;
        if (tHit < 0.0f)
        {
            // レイ始点がボックス内部にいる場合、tmin は負になりうる -> その場合は 0 を使う（始点でヒット）
            tHit = 0.0f;
        }

        if (tHit <= maxDistance && tHit < bestT)
        {
            // ヒット点と法線を求める（近似）
            Vector3 hitPoint = origin + direction * tHit;

            Vector3 normal = Vector3::Zero;
            const float normEps = 1e-3f;
            if (fabs(hitPoint.x - bmin.x) < normEps) normal = Vector3(-1, 0, 0);
            else if (fabs(hitPoint.x - bmax.x) < normEps) normal = Vector3(1, 0, 0);
            else if (fabs(hitPoint.y - bmin.y) < normEps) normal = Vector3(0, -1, 0);
            else if (fabs(hitPoint.y - bmax.y) < normEps) normal = Vector3(0, 1, 0);
            else if (fabs(hitPoint.z - bmin.z) < normEps) normal = Vector3(0, 0, -1);
            else if (fabs(hitPoint.z - bmax.z) < normEps) normal = Vector3(0, 0, 1);
            else
            {
                // 近似で決められなければ、法線を direction の逆方向に設定
                Vector3 invDir = -direction;
                // 正規化が可能なら正規化（Vector3::Normalize などがあればそちらを使ってください）
                float len = std::sqrt(invDir.x * invDir.x + invDir.y * invDir.y + invDir.z * invDir.z);
                if (len > eps) normal = invDir / len;
            }

            bestT = tHit;
            if (hitInfo != nullptr)
            {
                hitInfo->collider = col;
                hitInfo->point = hitPoint;
                hitInfo->normal = normal;
                hitInfo->distance = bestT;
            }
            hitAny = true;
        }
    }

    return hitAny;
}

} // UniDx
