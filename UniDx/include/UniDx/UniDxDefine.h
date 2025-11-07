#pragma once

#include <vector>
#include <memory>
#include <string>

#include <d3d11.h>
#include <wrl/client.h>
#include <SimpleMath.h>

namespace UniDx
{

using std::wstring;
using std::wstring_view;
using std::unique_ptr;
using std::shared_ptr;
using std::make_unique;
using std::make_shared;
using Microsoft::WRL::ComPtr;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Color;
using DirectX::XM_PI;
using DirectX::XM_2PI;

class Object;
class GameObject;
class Component;
class Transform;

typedef std::vector<unique_ptr<GameObject>> GameObjectContainer;

}
