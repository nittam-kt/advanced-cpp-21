#pragma once

#include <Keyboard.h>
#include "UniDxDefine.h"


namespace UniDx
{

// InputèÓïÒ
class Input
{
public:
    static void initialize()
    {
        keyboard = std::make_unique< DirectX::Keyboard>();
    }

    static void update()
    {
        prevKeyState = nowKeyState;
        nowKeyState = keyboard->GetState();
    }

    static bool GetKey(DirectX::Keyboard::Keys key)
    {
        return nowKeyState.IsKeyDown(key);
    }

    static bool GetKeyDown(DirectX::Keyboard::Keys key)
    {
        return prevKeyState.IsKeyUp(key) && nowKeyState.IsKeyDown(key);
    }

    static bool GetKeyUp(DirectX::Keyboard::Keys key)
    {
        return prevKeyState.IsKeyDown(key) && nowKeyState.IsKeyUp(key);
    }

protected:
    static std::unique_ptr<DirectX::Keyboard> keyboard;
    static DirectX::Keyboard::State nowKeyState;
    static DirectX::Keyboard::State prevKeyState;
};

}
