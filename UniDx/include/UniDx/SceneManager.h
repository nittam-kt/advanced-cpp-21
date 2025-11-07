#pragma once

#include <memory>

#include "Singleton.h"
#include "Scene.h"
#include "Material.h"


std::unique_ptr<UniDx::Scene> CreateDefaultScene();


namespace UniDx
{

class Scene;

// シーンマネージャ
class SceneManager : public Singleton<SceneManager>
{
public:
    void createScene();

    Scene* GetActiveScene() { return activeScene.get(); }

protected:
    std::unique_ptr<Scene> activeScene;
//    std::unique_ptr<Material> defaultMaterial;
};

}