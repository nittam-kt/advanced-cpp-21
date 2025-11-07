// ----------------------------------------------------------
// 頂点
// ----------------------------------------------------------
cbuffer ParentConstants : register(b0)
{
    float4x4 world;     // 親ノードのワールド変換行列
    float4x4 view;
    float4x4 projection;
};
cbuffer LocalConstants : register(b1)
{
    float4x4 local;     // 子ノードのローカル変換行列
}

// 頂点シェーダーへ入力するデータ
struct VSInput
{
    float3 pos : POSITION;
    float3 nrm : NORMAL;
    float2 uv : TEXUV;
    float  weight : BLENDWEIGHT;
};

// 頂点シェーダーから出力するデータ
struct PSInput
{
    float4 pos : SV_Position;   // 頂点の座標(射影座標系)
    float3 nrm : NORMAL;        // 法線
    float2 uv : TEXCOORD0;      // UV座標
};


// 頂点シェーダー
PSInput VS(VSInput vin)
{
    PSInput Out;

    float4x4 I = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
    float t = saturate(vin.weight);
    float4x4 blendLocal = I + t * (local - I);

    float4 p = float4(vin.pos.xyz, 1);
    p = mul(blendLocal, p); // 親座標へ
    p = mul(world, p);
    p = mul(view, p);
    p = mul(projection, p);
    Out.pos = p;

    float3x3 world3x3 = (float3x3) world;
    Out.nrm = mul(world3x3, vin.nrm);

    Out.uv = vin.uv;

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

// テクスチャとサンプラ。4番のテクスチャスロットとサンプラスロットを使用（UNIDX_PS_SLOT_ALBEDO）
Texture2D texture0 : register(t4);
SamplerState sampler0 : register(s4);


//  ランバート拡散＋アンビエント  (Directional / Point / Spot)
float4 EvaluateLight(in GPULight L, in float3 posW, in float3 nrmW)
{
    float3 Ldir; // from point to light (方向ライトなら -dir)
    float atten;

    if (L.type == 0 /*Spot*/)
    {
        Ldir = L.posOrDirWS - posW;
        float dist = length(Ldir);
        Ldir /= dist;
        float spotCos = dot(-Ldir, L.spotDirWS);
        if (spotCos < L.spotOuterCos)
            return 0;
        atten = saturate(1 - dist * L.rangeOrInvCos) * saturate((spotCos - L.spotOuterCos) / (1 - L.spotOuterCos));
    }
    else if (L.type == 1 /*Directional*/)
    {
        Ldir = -L.posOrDirWS; // store as -dir in posOrDirWS
        atten = 1;
    }
    else /*Point*/
    {
        Ldir = L.posOrDirWS - posW;
        float dist = length(Ldir);
        Ldir /= dist;
        atten = saturate(1 - dist * L.rangeOrInvCos);
    }

    float NdotL = dot(nrmW, Ldir);
    return L.color * saturate(atten * NdotL);
}


// ピクセルシェーダー
float4 PS(PSInput In) : SV_Target0
{
    // テクスチャから色を取得
    float4 albedo = texture0.Sample(sampler0, In.uv);

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

    const float4 Ambient = float4(0.1f, 0.1f, 0.1f, 1);    // 環境光。ここでは固定
    float4 color = (diffAccum + Ambient) * albedo;

    // テクスチャの色を出力
    return color;
}
