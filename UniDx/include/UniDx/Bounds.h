#pragma once
#include <string>

#include "UniDxDefine.h"
#include "Property.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace UniDx {

// --------------------
// Boundsクラス
// --------------------
struct Bounds : public DirectX::BoundingBox
{
public:
    Bounds() = default;

    Bounds(Vector3 center, Vector3 extents) : DirectX::BoundingBox(center, extents) {}

    // サイズ（全長）
    Vector3 size() const { return Vector3(Extents.x * 2, Extents.y * 2, Extents.z * 2); }

    // 最小座標
    Vector3 min() const { return Vector3(Center) - Extents; }

    // 最大座標
    Vector3 max() const { return Vector3(Center) + Extents; }

    // 指定点に最も近い点
    Vector3 ClosestPoint(Vector3 point) const {
        Vector3 mn = min();
        Vector3 mx = max();
        return Vector3(
            std::max(mn.x, std::min(point.x, mx.x)),
            std::max(mn.y, std::min(point.y, mx.y)),
            std::max(mn.z, std::min(point.z, mx.z))
        );
    }

    // Boundsを拡張（全体サイズを増やす）
    void Expand(float amount) {
        Vector3 delta(amount, amount, amount);
        Extents = Extents + delta * 0.5f;
    }
    void Expand(Vector3 amount) {
        Extents = Extents + amount * 0.5f;
    }

    // 指定点を含むように拡張
    void Encapsulate(Vector3 point) {
        Vector3 mn = min();
        Vector3 mx = max();
        mn.x = std::min(mn.x, point.x);
        mn.y = std::min(mn.y, point.y);
        mn.z = std::min(mn.z, point.z);
        mx.x = std::max(mx.x, point.x);
        mx.y = std::max(mx.y, point.y);
        mx.z = std::max(mx.z, point.z);
        SetMinMax(mn, mx);
    }
    // 指定Boundsを含むように拡張
    void Encapsulate(const Bounds& bounds) {
        Encapsulate(bounds.min());
        Encapsulate(bounds.max());
    }

    // min/maxからBoundsを再設定
    void SetMinMax(Vector3 min, Vector3 max) {
        Center = (min + max) * 0.5f;
        Extents = (max - min) * 0.5f;
    }

    // 他のBoundsと交差しているか
    bool Intersects(const Bounds& bounds) const {
        if(std::abs(Center.x - bounds.Center.x) > Extents.x + bounds.Extents.x) return false;
        if(std::abs(Center.y - bounds.Center.y) > Extents.y + bounds.Extents.y) return false;
        if(std::abs(Center.z - bounds.Center.z) > Extents.z + bounds.Extents.z) return false;
        return true;
    }

    // 指定点までの二乗距離
    float SqrDistance(Vector3 point) const {
        Vector3 cp = ClosestPoint(point);
        Vector3 d = cp - point;
        return d.LengthSquared();
    }
};

} // namespace UniDx
