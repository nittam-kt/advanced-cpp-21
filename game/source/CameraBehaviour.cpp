#include "CameraBehaviour.h"

#include <UniDx/Input.h>
#include <UniDx/Time.h>

#include "Player.h"


using namespace DirectX;
using namespace UniDx;


void CameraBehaviour::OnEnable()
{
    Quaternion rot = Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), 0.6f);
    transform->localRotation = rot;
}


void CameraBehaviour::Update()
{
    transform->position = player->transform->position + Vector3(0, 6, -6);

    const float speed = 1;

    const float angleSpeed = 1.0f;
    Quaternion localRot = transform->localRotation;
    Quaternion rot;
    if (Input::GetKey(Keyboard::J))
    {
        rot = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), -angleSpeed * Time::deltaTime);
    }
    if (Input::GetKey(Keyboard::L))
    {
        rot = Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), angleSpeed * Time::deltaTime);
    }
    if (Input::GetKey(Keyboard::I))
    {
        rot = Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), -angleSpeed * Time::deltaTime);
    }
    if (Input::GetKey(Keyboard::K))
    {
        rot = Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), angleSpeed * Time::deltaTime);
    }
    transform->localRotation = localRot * rot;
}
