#include "Collider.h"

#include <limits>

#include "pch.h"
#include <UniDx/Rigidbody.h>
#include <UniDx/Collider.h>

namespace
{

using namespace UniDx;
using namespace std;

constexpr float infinity = numeric_limits<float>::infinity();


// トリガーチェック
bool checkTrigger_(SphereCollider* sphere, AABBCollider* aabb)
{
    // 球の中心（ワールド座標）
    Vector3 sphereCenter = sphere->transform->TransformPoint(sphere->center);
    float sphereRadius = sphere->radius;

    // AABBのBounds
    Bounds aabbBounds = aabb->getBounds();

    // AABB上で球中心に最も近い点
    Vector3 closest = aabbBounds.ClosestPoint(sphereCenter);

    // 最近点と球中心のベクトル
    Vector3 normal = sphereCenter - closest;
    float distSqr = normal.LengthSquared();

    // 衝突していない
    return distSqr <= sphereRadius * sphereRadius;
}

// 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
bool checkIntersect_(SphereCollider* sphere, AABBCollider* aabb, PhysicsActor* sphereActor, PhysicsActor* aabbActor)
{
    // 球の中心（ワールド座標）
    Vector3 sphereCenter = sphere->transform->TransformPoint(sphere->center);
    float sphereRadius = sphere->radius;

    // AABBのBounds
    Bounds aabbBounds = aabb->getBounds();

    // AABB上で球中心に最も近い点
    Vector3 closest = aabbBounds.ClosestPoint(sphereCenter);

    // 最近点と球中心のベクトル
    Vector3 normal = sphereCenter - closest;
    float distSqr = normal.LengthSquared();

    // 衝突していない
    if (distSqr > sphereRadius * sphereRadius)
        return false;

    // Rigidbody取得
    Rigidbody* rbA = sphere->attachedRigidbody;
    Rigidbody* rbB = aabb->attachedRigidbody;

    // 相対速度
    Vector3 velA = rbA ? rbA->linearVelocity : Vector3::Zero;
    Vector3 velB = rbB ? rbB->linearVelocity : Vector3::Zero;
    Vector3 relVel = velA - velB;

    // 相対速度が法線方向（離れようとしている）場合は無視
    if (relVel.Dot(normal) > 0)
        return false;

    float dist = std::sqrt(distSqr);
    // 法線（dist==0のときは適当な軸にする）
    Vector3 contactNormal = (dist > 1e-6f) ? (normal / dist) : Vector3(1, 0, 0);

    // penetration（めり込み量）
    float penetration = sphereRadius - dist;

    // 質量取得（0以下は1.0f扱い）
    float massA = (rbA && !rbA->isKinematic) ? (rbA->mass > 0.0f ? rbA->mass : 1.0f) : infinity;
    float massB = (rbB && !rbB->isKinematic) ? (rbB->mass > 0.0f ? rbB->mass : 1.0f) : infinity;
    float totalMass = massA + massB;

    float massAPerTotal = massA != infinity ? massA / totalMass : 1;
    float massBPerTotal = massB != infinity ? massB / totalMass : 1;

    // 補正ベクトル
    Vector3 correctionA = contactNormal * (penetration * massBPerTotal);
    Vector3 correctionB = -contactNormal * (penetration * massAPerTotal);

    // 位置補正
    if (rbA && !rbA->isKinematic && massA != infinity) sphereActor->addCorrectPosition(correctionA);
    if (rbB && !rbB->isKinematic && massB != infinity) aabbActor->addCorrectPosition(correctionB);

    // 跳ね返り係数
    float bounce = sphere->bounciness * aabb->bounciness;

    // 法線方向の速度成分
    float relVelN = relVel.Dot(contactNormal);

    // 反射させる
    Vector3 impulse = -(1.0f + bounce) * relVelN * contactNormal;

    if (rbA && !rbA->isKinematic && massA != infinity) sphereActor->addCorrectVelocity(impulse * massBPerTotal);
    if (rbB && !rbB->isKinematic && massB != infinity) aabbActor->addCorrectVelocity(-impulse * massAPerTotal);

    return true;
}

}


namespace UniDx
{


// TransformをたどってRigidbodyを探す
Rigidbody* Collider::findNearestRigidbody(Transform* t) const
{
    // 同じGameObjectにRigidbodyがあればそれを登録
    Rigidbody* rb = gameObject->GetComponent<Rigidbody>();
    if (rb != nullptr)
    {
        return rb;
    }

    // なければ親をたどって再帰呼び出し
    if (t->parent != nullptr)
    {
        return findNearestRigidbody(t->parent);
    }

    // ない。このときは動かないCollilderになる
    return nullptr;
}


// ワールド空間における空間境界を取得
Bounds SphereCollider::getBounds() const
{
    return Bounds(transform->position + transform->TransformVector(center), Vector3(radius, radius, radius));
}


// ワールド空間における空間境界を取得
Bounds AABBCollider::getBounds() const
{
    return Bounds(transform->position + transform->TransformVector(center), transform->TransformVector(size));
}


// トリガーチェック
bool AABBCollider::checkTrigger(AABBCollider* other)
{
    return getBounds().Intersects(other->getBounds());
}


// トリガーチェック
bool AABBCollider::checkTrigger(SphereCollider* other)
{
    return checkTrigger_(other, this);
}


// 衝突チェック
// 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
bool AABBCollider::checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
{
    return false;
}


// 衝突チェック
// 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
bool AABBCollider::checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
{
    return checkIntersect_(other, this, otherActor, myActor);
}


// トリガーチェック
bool SphereCollider::checkTrigger(AABBCollider* other)
{
    return checkTrigger_(this, other);
}


// トリガーチェック
bool SphereCollider::checkTrigger(SphereCollider* other)
{
    Vector3 centerA = transform->TransformPoint(center);
    Vector3 centerB = other->transform->TransformPoint(other->center);
    float radiusAB = radius + other->radius;

    // 中心距離が半径の合計より離れていれば当たっていない
    return Vector3::DistanceSquared(centerA, centerB) <= radiusAB * radiusAB;
}


// 衝突チェック
// 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
bool SphereCollider::checkIntersect(AABBCollider* other, PhysicsActor* myActor, PhysicsActor* otherActor)
{
    return checkIntersect_(this, other, myActor, otherActor);
}


// 衝突チェック
// 衝突していれば attachedRigidbody に addCorrectPosition(), addCorrectVelocity() で補正する
bool SphereCollider::checkIntersect(SphereCollider* other, PhysicsActor* myActor, PhysicsActor* otherShap)
{
    Vector3 centerA = transform->TransformPoint(center);
    float radiusA = radius;
    Vector3 centerB = other->transform->TransformPoint(other->center);
    float radiusB = other->radius;
    
    // 中心距離が半径の合計より離れていれば当たっていない
    if (Vector3::Distance(centerA, centerB) > radiusA + radiusB)
        return false;

    // めり込みの深さ
    float penetration = radiusA + radiusB - Vector3::Distance(centerA, centerB);

    // 中心の差
    Vector3 sub = centerB - centerA;

    // それぞれの位置補正
    Vector3 addB = sub;
    addB.Normalize();
    addB *= penetration * 0.5f;

    otherShap->addCorrectPosition(addB);

    Vector3 addA = -sub;
    addA.Normalize();
    addA *= penetration * 0.5f;

    myActor->addCorrectPosition(addA);

    // 跳ね返り計算
    Vector3 va = attachedRigidbody->linearVelocity;
    Vector3 vb = other->attachedRigidbody->linearVelocity;

    // 相対速度
    Vector3 relV = va - vb;

    Vector3 normal = sub;
    normal.Normalize();
    if (relV.Dot(normal) < 0)
    {
        return false;
    }

    // 跳ね返り係数
    float bounce = bounciness * other->bounciness;

    Vector3 relVNormal = normal * relV.Dot(normal);
    myActor->addCorrectVelocity(relVNormal * -bounce);
    otherShap->addCorrectVelocity(relVNormal * bounce);

    return false;
}




}
