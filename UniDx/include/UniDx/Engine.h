#pragma once

#include <windows.h>
#include <Keyboard.h>

#include "Singleton.h"

namespace UniDx
{

class GameObject;
class Camera;
class Canvas;

// エンジンのメイン
class Engine : public Singleton<Engine>
{
public:
    virtual void Initialize(HWND hWnd);
    virtual int PlayerLoop();

    void ProcessKeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
    }

    void registerCanvas(Canvas* c);
    void unregisterCanvas(Canvas* c);

protected:
    virtual void fixedUpdate();
    virtual void physics();
    virtual void input();
    virtual void update();
    virtual void lateUpdate();
    virtual void render();
    virtual void finalize();

    void awake(GameObject* object);
    void fixedUpdate(GameObject* object);
    void checkStart(GameObject* object);
    void update(GameObject* object);
    void lateUpdate(GameObject* object);
    void render(GameObject* object, const Camera& camera);

private:
    std::vector<Canvas*> canvas_;

    void createScene();
};

}