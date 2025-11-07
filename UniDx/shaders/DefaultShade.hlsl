// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
cbuffer VSConstants : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float3 nrm : NORMAL;
};

// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 pos : SV_Position;   // 頂点の座標(射影座標系)
    float3 nrm : NORMAL;        // 法線
};


// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;

    float4 p = float4(vin.pos.xyz, 1);
    p = mul(world, p);
    p = mul(view, p);
    p = mul(projection, p);
    Out.pos = p;

    float3x3 world3x3 = (float3x3) world;
    Out.nrm = mul(world3x3, vin.nrm);
    return Out;
}

 
// ----------------------------------------------------------
// ピクセル
// ----------------------------------------------------------
struct GPULight
{
    float3 posOrDirWS;
    float rangeOrInvCos;
    float4 color; // rgb=Color, a=Intensity
    float3 spotDirWS;
    float spotOuterCos;
    uint type;
    uint pad0;
    uint pad1;
    uint pad2;
};

// ライト（UNIDX_PS_SLOT_ALL_LIGHTS)
StructuredBuffer<GPULight> allLights : register(t0);


//  ランバート拡散
float4 EvaluateLight(in GPULight L, in float3 posW, in float3 nrmW)
{
    float3 Ldir; // from point to light (方向ライトなら -dir)

    Ldir = -L.posOrDirWS; // store as -dir in posOrDirWS

    float NdotL = dot(Ldir, nrmW);
    return L.color * saturate(NdotL);
}


// ピクセルシェーダー
float4 PS(PSInput In) : SV_Target0
{
    // 明示的に法線を正規化（モデルスケール非均等だと崩れるため）
    float3 N = normalize(In.nrm);

    // ライトループ
    float4 diffAccum = 0;
    [loop]
    for (uint k = 0; k < 1; ++k)
    {
        GPULight L = allLights[k];
        diffAccum += EvaluateLight(L, In.pos.xyz, N);
    }

    // 環境光。ここでは固定
    const float4 Ambient = float4(0.1f, 0.1f, 0.1f, 1);

    // 最終の色
    float4 color = (diffAccum + Ambient);

    // テクスチャの色を出力
    return color;
}
