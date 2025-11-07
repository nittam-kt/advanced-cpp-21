// CreateDefaultScene.cpp
// デフォルトのシーンを生成します

#include <numbers>

#include <UniDx.h>
#include <UniDx/Scene.h>
#include <UniDx/Camera.h>
#include <UniDx/PrimitiveRenderer.h>
#include <UniDx/GltfModel.h>
#include <UniDx/Rigidbody.h>
#include <UniDx/Random.h>
#include <UniDx/Collider.h>
#include <UniDx/Light.h>
#include <UniDx/Canvas.h>
#include <UniDx/TextMesh.h>
#include <UniDx/Font.h>
#include <UniDx/Image.h>

#include "CameraBehaviour.h"
#include "Player.h"
#include "MapData.h"

using namespace std;
using namespace UniDx;

// VertexPNTにウェイトWを追加した頂点情報
struct VertexPNTW
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv0;
    float   weight;

    void setPosition(Vector3 v) { position = v; }
    void setNormal(Vector3 v) { normal = v; }
    void setColor(Color c) {}
    void setUV(Vector2 v) { uv0 = v; }
    void setUV2(Vector2 v) {}
    void setUV3(Vector2 v) {}
    void setUV4(Vector2 v) {}
    void setWeight(float w) { weight = w; }

    static const std::array< D3D11_INPUT_ELEMENT_DESC, 4> layout;
};
const std::array< D3D11_INPUT_ELEMENT_DESC, 4> VertexPNTW::layout =
{
    D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "TEXUV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    D3D11_INPUT_ELEMENT_DESC{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


unique_ptr<GameObject> createMap(GameObject* player)
{
    // マップデータ作成
    MapData::create();

    // マテリアルの作成
    auto material = std::make_shared<Material>();

    // シェーダを指定してコンパイル
    material->shader.compile<VertexPNT>(L"Resource/AlbedoShade.hlsl");

    // 壁のテクスチャ作成
    auto texture = std::make_shared<Texture>();
    texture->Load(L"Resource/wall-1.png");
    material->AddTexture(std::move(texture));

    // マップ作成
    auto map = make_unique<GameObject>();

    // 各ブロック作成
    {
        auto rb = make_unique<Rigidbody>();
        rb->gravityScale = 0;
        rb->mass = numeric_limits<float>::infinity();

        // 壁オブジェクトを作成
        auto wall = make_unique<GameObject>(L"壁",
            CubeRenderer::create<VertexPNT>(material),
            move(rb),
            make_unique<AABBCollider>());
        wall->transform->localScale = Vector3(2, 2, 2);
        wall->transform->localPosition = Vector3(0, 0, 2);

        // 壁の親をマップにする
        Transform::SetParent(move(wall), map->transform);
    }

    return move(map);
}


unique_ptr<Scene> CreateDefaultScene()
{    
    // -- プレイヤー --
    auto playerObj = make_unique<GameObject>(L"プレイヤー",
        make_unique<GltfModel>(),
        make_unique<Player>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>(Vector3(0, 0.25f, 0))
    );
    auto model = playerObj->GetComponent<GltfModel>(true);
    model->Load<VertexPNT>(
        L"Resource/ModularCharacterPBR.glb",
        L"Resource/AlbedoShade.hlsl",
        L"Resource/Albedo.png");
    playerObj->transform->localPosition = Vector3(0, -1, 0);
    playerObj->transform->localRotation = Quaternion::CreateFromYawPitchRoll(XM_PI, 0, 0);

    // -- 床 --
    // キューブレンダラを作ってサイズを調整
    auto rb = make_unique<Rigidbody>();
    rb->gravityScale = 0;
    rb->mass = numeric_limits<float>::infinity();
    auto floor = make_unique<GameObject>(L"床",
        CubeRenderer::create<VertexPNT>(L"Resource/AlbedoShade.hlsl", L"Resource/brick-1.png"),
        move(rb),
        make_unique<AABBCollider>());
    floor->transform->localPosition = Vector3(0.0f, -1.5f, 0.0f);
    floor->transform->localScale = Vector3(6, 1, 6);

    // -- カメラ --
    auto cameraBehaviour = make_unique<CameraBehaviour>();
    cameraBehaviour->player = playerObj->GetComponent<Player>(true);

    // -- ライト --
    auto light = make_unique<GameObject>(L"ライト",
        make_unique<Light>());
    light->transform->localRotation = Quaternion::CreateFromYawPitchRoll(0.2f, 0.9f, 0);

    // -- UI --
    auto font = make_shared<Font>();
    font->Load(L"Resource/M PLUS 1.spritefont");
    auto textMesh = make_unique<TextMesh>();
    textMesh->font = font;
    textMesh->text = L"-";
    auto textObj = make_unique<GameObject>(L"テキスト", textMesh);
    textObj->transform->localPosition = Vector3(100, 20, 0);

    auto canvas = make_unique<Canvas>();
    canvas->LoadDefaultMaterial(L"Resource");


    // -- マップデータ --
    auto map = createMap(playerObj.get());


    // シーンを作って戻す
    return make_unique<Scene>(

        make_unique<GameObject>(L"オブジェクトルート",
            move(playerObj),
            move(map),
            move(floor)
        ),

        move(light),

        make_unique<GameObject>(L"カメラルート", Vector3(0, 3, -5),
            make_unique<Camera>(),
            move(cameraBehaviour)
        ),

        make_unique<GameObject>(L"UI",
            move(canvas),
            move(textObj)
        )
    );
}
