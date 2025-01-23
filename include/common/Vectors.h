#pragma once
#include <math.h>

struct Vec2 {
    double x;
    double y;

    inline void operator+=(const Vec2& rhs) { x += rhs.x; y += rhs.y; }
    inline void operator-=(const Vec2& rhs) { x -= rhs.x; y -= rhs.y; }
    inline void operator*=(double scalar) { x *= scalar; y *= scalar; }
    inline void operator/=(double scalar) { x /= scalar; y /= scalar; }
};

inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
inline Vec2 operator*(const Vec2& lhs, double scalar) { return {lhs.x * scalar, lhs.y * scalar}; }
inline Vec2 operator*(double scalar, const Vec2& rhs) { return {rhs.x * scalar, rhs.y * scalar}; }
inline Vec2 operator/(const Vec2& lhs, double scalar) { return {lhs.x / scalar, lhs.y / scalar}; }
inline Vec2 operator/(double scalar, const Vec2& rhs) { return {rhs.x / scalar, rhs.y / scalar}; }

inline double norm(Vec2 v) {
    return sqrt(v.x*v.x + v.y * v.y);
}

inline double dist(Vec2 a, Vec2 b) {
    return norm(a - b);
}

struct Vec2i {
    int x;
    int y;

    inline void operator+=(const Vec2i& rhs) { x += rhs.x; y += rhs.y; }
    inline void operator-=(const Vec2i& rhs) { x -= rhs.x; y -= rhs.y; }
    inline void operator*=(int scalar) { x *= scalar; y *= scalar; }
};

inline Vec2i operator+(const Vec2i& lhs, const Vec2i& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y}; }
inline Vec2i operator-(const Vec2i& lhs, const Vec2i& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y}; }
inline Vec2i operator*(const Vec2i& lhs, int scalar) { return {lhs.x * scalar, lhs.y * scalar}; }
inline Vec2i operator*(int scalar, const Vec2i& rhs) { return {rhs.x * scalar, rhs.y * scalar}; }

struct Vec3 {
    double x;
    double y;
    double z;

    inline void operator+=(const Vec3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }
    inline void operator-=(const Vec3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }
    inline void operator*=(double scalar) { x *= scalar; y *= scalar; z *= scalar; }
    inline void operator/=(double scalar) { x /= scalar; y /= scalar; z /= scalar; }

    inline Vec2 xy() const {
        return {x,y};
    }
};

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) { return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z}; }
inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) { return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z}; }
inline Vec3 operator*(const Vec3& lhs, double scalar) { return {lhs.x * scalar, lhs.y * scalar, lhs.z * scalar}; }
inline Vec3 operator*(double scalar, const Vec3& rhs) { return {rhs.x * scalar, rhs.y * scalar, rhs.z * scalar}; }
inline Vec3 operator/(const Vec3& lhs, double scalar) { return {lhs.x / scalar, lhs.y / scalar, lhs.z / scalar}; }
inline Vec3 operator/(double scalar, const Vec3& rhs) { return {rhs.x / scalar, rhs.y / scalar, rhs.z / scalar}; }

inline double norm(Vec3 v) {
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

inline double dist(Vec3 a, Vec3 b) {
    return norm(a - b);
}

inline double dist_sqr(Vec3 a, Vec3 b) {
    Vec3 v = a - b;
    return v.x*v.x + v.y*v.y + v.z*v.z;
}
