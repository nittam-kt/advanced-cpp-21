#pragma once

#include "UniDxDefine.h"


namespace UniDx
{

class Collider;
class Component;


// --------------------
// ContactPoint情報
// --------------------
struct ContactPoint
{
    Vector3 point;
    Vector3 normal;
};


// --------------------
// Collision情報
// --------------------
struct Collision
{
public:
    Collider* collider;                 // 衝突した相手側のコライダー
    std::vector<ContactPoint> contacts;
};


} // namespace UniDx
