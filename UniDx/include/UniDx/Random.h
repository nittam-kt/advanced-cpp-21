#pragma once

#include <chrono>
#include "UniDxDefine.h"


namespace UniDx
{

// Random
class Random
{
public:
    // シングルトン的に使う場合のグローバルインスタンス
    static Random& global()
    {
        static Random inst;
        return inst;
    }

    explicit Random(uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count())
    {
        InitState(seed);
    }

    // シード設定 (Unity互換: InitState)
    void InitState(uint64_t seed)
    {
        state = seed ? seed : 88172645463325252ull;
    }

    // シード値の取得
    uint64_t getState() const { return state; }

    // 0.0～1.0の乱数（1.0を含む、Unity互換）
    float value()
    {
        // 24bit乱数を[0,1]に正規化、1.0を含む
        uint32_t v = static_cast<uint32_t>(nextUInt64() >> 40);
        return static_cast<float>(v) / static_cast<float>(0xFFFFFF); // 0xFFFFFF == 2^24-1
    }

    // [min, max) のfloat乱数（最大値含まない、UnityのRangeと同じ挙動）
    float Range(float min, float max)
    {
        return min + (max - min) * value();
    }

    // [min, max) のint乱数（最大値含まない、UnityのRangeと同じ挙動）
    int Range(int min, int max)
    {
        return min + static_cast<int>(nextUInt64() % static_cast<uint64_t>(max - min));
    }

    // [min, max] のfloat乱数（最大値含む）
    float RangeInclusive(float min, float max)
    {
        // value()は1.0を含むのでそのまま
        return min + (max - min) * value();
    }

    // [min, max] のint乱数（最大値含む）
    int RangeInclusive(int min, int max)
    {
        return min + static_cast<int>(nextUInt64() % (static_cast<uint64_t>(max - min + 1)));
    }

    // -1.0～1.0のfloat乱数
    float symmetricValue()
    {
        return value() * 2.0f - 1.0f;
    }

    // 単位球内のランダムな点
    Vector3 insideUnitSphere()
    {
        while (true)
        {
            float x = symmetricValue();
            float y = symmetricValue();
            float z = symmetricValue();
            float len2 = x * x + y * y + z * z;
            if (len2 <= 1.0f)
                return Vector3(x, y, z);
        }
    }

    // 単位円内のランダムな点
    Vector2 insideUnitCircle()
    {
        while (true)
        {
            float x = symmetricValue();
            float y = symmetricValue();
            float len2 = x * x + y * y;
            if (len2 <= 1.0f)
                return Vector2(x, y);
        }
    }

private:
    uint64_t state = 88172645463325252ull; // デフォルトシード

    // 64bit XorShift
    uint64_t nextUInt64()
    {
        uint64_t x = state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        state = x;
        return x;
    }
};


}
