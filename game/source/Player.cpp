#pragma once

#include "Player.h"

#include <UniDx/Input.h>
#include <UniDx/Collider.h>
#include <UniDx/Time.h>

using namespace DirectX;
using namespace UniDx;

namespace
{
    void debugNode(GameObject* p, wstring head)
    {
        wstring str = head;
        str.append(p->name);
        Debug::Log(str);
        str = head;
        str.append( std::to_wstring(p->transform->position.get().y));
        Debug::Log(str);
        str = head + L" ";
        for (int i = 0; i < p->transform->childCount(); ++i)
        {
            debugNode(p->transform->GetChild(i)->gameObject, str);
        }
    }
}


void Player::OnEnable()
{
    rb = GetComponent<Rigidbody>(true);
    assert(rb != nullptr);

    rb->gravityScale = 1.5f;
    GetComponent<Collider>(true)->bounciness = 0.0f;
}


void Player::Update()
{
    const float moveSpeed = 4;
    const float jumpSpeed = 6;

    Vector3 velocity = rb->linearVelocity;
    Quaternion rotation = rb->rotation;
    if (Input::GetKey(Keyboard::A))
    {
        velocity.x = -moveSpeed;
        rotation = Quaternion::CreateFromYawPitchRoll(XM_PI * -0.5f, 0, 0);
    }
    else if (Input::GetKey(Keyboard::D))
    {
        velocity.x = moveSpeed;
        rotation = Quaternion::CreateFromYawPitchRoll(XM_PI * 0.5f, 0, 0);
    }
    else
    {
        velocity.x = 0.0f;
    }

    if (Input::GetKey(Keyboard::S))
    {
        velocity.z = -moveSpeed;
        rotation = Quaternion::CreateFromYawPitchRoll(XM_PI, 0, 0);
    }
    else if (Input::GetKey(Keyboard::W))
    {
        velocity.z = moveSpeed;
        rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
    }
    else
    {
        velocity.z = 0.0f;
    }

    rb->linearVelocity = velocity;
    rb->rotation = rotation;
}


void Player::OnTriggerEnter(Collider* other)
{
}


void Player::OnTriggerStay(Collider* other)
{
}


void Player::OnTriggerExit(Collider* other)
{
}


void Player::OnCollisionEnter(const Collision& collision)
{
}


void Player::OnCollisionStay(const Collision& collision)
{
}

void Player::OnCollisionExit(const Collision& collision)
{
}

