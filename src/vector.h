#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

    Vec3 operator + (const Vec3& r) const { return Vec3(x + r.x, y + r.y, z + r.z); }
    Vec3 operator - (const Vec3& r) const { return Vec3(x - r.x, y - r.y, z - r.z); }
    Vec3 operator * (float s) const { return Vec3(x * s, y * s, z * s); }
    friend Vec3 operator * (float s, const Vec3& v) {
        return Vec3(v.x * s, v.y * s, v.z * s);
    }

    float length() const { return std::sqrt(x * x + y * y + z * z); }

    Vec3 normalized() const {
        float len = length();
        return (len > 0) ? (*this) * (1.0f / len) : *this;
    }
};

float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Vec4 {
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float xx, float yy, float zz, float ww) : x(xx), y(yy), z(zz), w(ww) {}
};
