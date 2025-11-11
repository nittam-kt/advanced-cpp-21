// CreateDefaultScene.cpp
// デフォルトのシーンを生成します

#include <numbers>

#include <UniDx.h>
#include <UniDx/Scene.h>
#include <UniDx/PrimitiveRenderer.h>
#include <UniDx/GltfModel.h>
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
    MapData::getInstance()->load("Resource/map_data.txt");

    // マテリアルの作成
    auto wallMat = std::make_shared<Material>();

    // シェーダを指定してコンパイル
    wallMat->shader.compile<VertexPNT>(L"Resource/AlbedoShade.hlsl");

    // 壁テクスチャ作成
    auto wallTex = std::make_shared<Texture>();
    wallTex->Load(L"Resource/wall-1.png");
    wallMat->AddTexture(std::move(wallTex));

    // 敵テクスチャ作成
    auto enemyTex = std::make_shared<Texture>();
    enemyTex->Load(L"Resource/Universal_A_Alb.png");

    // マップ作成
    auto map = make_unique<GameObject>();

    // 各ブロック作成
    for (int i = 0; i < MapData::getInstance()->getWidth(); i++)
    {
        for (int j = 0; j < MapData::getInstance()->getHeight(); j++)
        {
            switch (MapData::getInstance()->getData(i, j))
            {
            case '#':
            {
                auto rb = make_unique<Rigidbody>();
                rb->gravityScale = 0;
                rb->mass = numeric_limits<float>::infinity();

                // 壁オブジェクトを作成
                auto wall = make_unique<GameObject>(L"壁",
                    CubeRenderer::create<VertexPNT>(wallMat),
                    move(rb),
                    make_unique<AABBCollider>());
                wall->transform->localScale = Vector3(2, 2, 2);
                wall->transform->localPosition = Vector3(
                    i * 2 - float(MapData::getInstance()->getWidth() / 2) * 2,
                    0,
                    j * -2 + float(MapData::getInstance()->getHeight() / 2) * 2
                );

                // 壁の親をマップにする
                Transform::SetParent(move(wall), map->transform);
            }
            break;

            case 'E':
            {
                // 敵オブジェクトを作成
                auto enemy = make_unique<GameObject>(L"敵",
                    make_unique<GltfModel>(),
                    make_unique<Rigidbody>(),
                    make_unique<SphereCollider>(Vector3(0, 0.25f, 0), 1.5f)
                    );
                auto model = enemy->GetComponent<GltfModel>(true);
                model->Load<VertexPNT>(
                    L"Resource/Pumpkin-carved-lit-a.glb",
                    L"Resource/AlbedoShade.hlsl",
                    enemyTex);

                // マテリアルカラーを半透明にする
                for (auto& mat : model->GetMaterials())
                {
                    mat->color = Color(1, 1, 1, 0.3f);
                }

                enemy->transform->localPosition = Vector3(
                    i * 2 - float(MapData::getInstance()->getWidth() / 2) * 2,
                    -0.5f,
                    j * -2 + float(MapData::getInstance()->getHeight() / 2) * 2
                );
                enemy->transform->localScale = Vector3(3, 3, 3);

                // 敵の親をマップにする
                Transform::SetParent(move(enemy), map->transform);
            }
            break;

            default:
                break;
            }
        }
    }

    return move(map);
}


unique_ptr<Scene> CreateDefaultScene()
{
    // -- プレイヤー --
    auto playerObj = make_unique<GameObject>(L"プレイヤー",
        make_unique<GltfModel>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>(Vector3(0, 0.25f, 0)),
        make_unique<Player>()
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
    floor->transform->localScale = Vector3(26, 1, 26);

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
