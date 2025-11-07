#pragma once

#include <UniDx.h>
#include <UniDx/Behaviour.h>


class Player;
class CameraBehaviour : public UniDx::Behaviour
{
public:
    const Player* player = nullptr;

    virtual void OnEnable() override;
    virtual void Update() override;
};