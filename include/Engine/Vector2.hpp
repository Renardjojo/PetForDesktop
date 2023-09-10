/*
 * Copyright (C) 2021 Amara Sami, Dallard Thomas, Nardone William, Six Jonathan
 * This file is subject to the LGNU license terms in the LICENSE file
 * found in the top-level directory of this distribution.
 */

#pragma once

#include <cfloat>
#include <cmath>
#include "yaml-cpp/node/convert.h"

template <typename T>
union Vector2 {
    // Data members
    struct
    {
        T x;
        T y;
    };
    T e[2];

    Vector2() = default;
    constexpr Vector2(const T k) noexcept : x{k}, y{k}
    {
    }

    constexpr Vector2(const T x_, const T y_) noexcept : x{x_}, y{y_}
    {
    }

    constexpr Vector2(const T coef[2]) noexcept : e{coef[0], coef[1]}
    {
    }

    // Static methods (pseudo-constructors)
    static constexpr Vector2 zero() noexcept
    {
        return {static_cast<T>(0), static_cast<T>(0)};
    }

    static constexpr Vector2 one() noexcept
    {
        return {static_cast<T>(1), static_cast<T>(1)};
    }

    static constexpr Vector2 right() noexcept
    {
        return {static_cast<T>(1), static_cast<T>(0)};
    }

    static constexpr Vector2 up() noexcept
    {
        return {static_cast<T>(0), static_cast<T>(1)};
    }

    static constexpr Vector2 left() noexcept
    {
        return {-static_cast<T>(1), static_cast<T>(0)};
    }

    static constexpr Vector2 down() noexcept
    {
        return {static_cast<T>(0), -static_cast<T>(1)};
    }

    // Member methods
    constexpr T sqrLength() const noexcept
    {
        return (x * x) + (y * y);
    }

    T length() const noexcept
    {
        return sqrtf(sqrLength());
    }

    constexpr T dot(const Vector2& v) const noexcept
    {
        return (x * v.x) + (y * v.y);
    }

    constexpr T cross(const Vector2& v) const noexcept
    {
        return (x * v.y) - (y * v.x);
    }

    constexpr bool isNull() const noexcept
    {
        return !x && !y;
    }

    constexpr bool isOrthogonalTo(const Vector2& v) const noexcept
    {
        return !dot(v);
    }

    constexpr bool isNormalized() const noexcept
    {
        return !(sqrLength() - static_cast<T>(1));
    }

    constexpr bool isOrthonormalTo(const Vector2& v) const noexcept
    {
        return !dot(v) && isNormalized() && v.isNormalized();
    }

    constexpr bool isColinearTo(const Vector2& v) const noexcept
    {
        return !cross(v);
    }

    bool isEqualTo(const Vector2& v, const T eps = FLT_EPSILON) const noexcept
    {
        return fabs(x - v.x) <= eps && fabs(y - v.y) <= eps;
    }

    bool isNotEqualTo(const Vector2& v, const T eps = FLT_EPSILON) const noexcept
    {
        return fabs(x - v.x) > eps || fabs(y - v.y) > eps;
    }

    void normalize() noexcept
    {
        *this /= length();
    }

    void safelyNormalize() noexcept
    {
        const float sqrLen{sqrLength()};

        if (sqrLen)
            *this /= sqrtf(sqrLen);
    }

    T angleWithUnitary(const Vector2& v) const noexcept
    {
        return acosf(dot(v) / length());
    }

    T angleWith(const Vector2& v) const noexcept
    {
        return acosf(dot(v) / sqrtf(sqrLength() * v.sqrLength()));
    }

    constexpr T triangleArea(const Vector2& v) const noexcept
    {
        return cross(v) * .5f;
    }

    Vector2 normalized() const noexcept
    {
        return *this / length();
    }

    Vector2 safelyNormalized() const noexcept
    {
        const float sqrLen{sqrLength()};

        return sqrLen ? (*this / sqrtf(sqrLen)) : *this;
    }

    constexpr Vector2 projectedOnUnitary(const Vector2& v) const noexcept
    {
        return v * dot(v);
    }

    constexpr Vector2 projectedOn(const Vector2& v) const noexcept
    {
        return v * (dot(v) / v.sqrLength());
    }

    Vector2 rotated(const T angle) const noexcept
    {
        const float cosAngle{cosf(angle)}, sinAngle{sinf(angle)};

        return {(cosAngle * x) - (sinAngle * y), (sinAngle * x) + (cosAngle * y)};
    }

    constexpr Vector2 rotated90() const noexcept
    {
        return {y, -x};
    }

    constexpr Vector2 lerp(const Vector2& v, const T t) const noexcept
    {
        const float tmp{static_cast<T>(1) - t};

        return {(x * tmp) + (v.x * t), (y * tmp) + (v.y * t)};
    }

    constexpr Vector2 reflect(const Vector2& n) const noexcept
    {
        return *this - n * (2.f * dot(n));
    }

    constexpr Vector2 abs() const noexcept
    {
        return {abs(x), abbs(y)};
    }

    static constexpr Vector2 remap(const Vector2 value, const Vector2& from1, const Vector2& to1, const Vector2& from2,
                                   const Vector2& to2)
    {
        return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    }

    // Operator overloads
    constexpr bool operator==(const Vector2& v) const noexcept
    {
        return x == v.x && y == v.y;
    }

    // TODO:
    //constexpr bool operator==(const Vector2<float>& v) noexcept
    //{
    //    return std::fabs(x - v.x) <= std::numeric_limits<double>::epsilon() &&
    //           std::fabs(y - v.y) <= std::numeric_limits<double>::epsilon();
    //}

    constexpr Vector2& operator+=(const Vector2& v) noexcept
    {
        x += v.x;
        y += v.y;

        return *this;
    }

    constexpr Vector2& operator+=(const Vector2&& v) noexcept
    {
        x += v.x;
        y += v.y;

        return *this;
    }

    constexpr Vector2& operator-=(const Vector2& v) noexcept
    {
        x -= v.x;
        y -= v.y;

        return *this;
    }

    constexpr Vector2& operator-=(const Vector2&& v) noexcept
    {
        x -= v.x;
        y -= v.y;

        return *this;
    }

    constexpr Vector2& operator*=(const Vector2& v) noexcept
    {
        x *= v.x;
        y *= v.y;

        return *this;
    }

    constexpr Vector2& operator*=(const Vector2&& v) noexcept
    {
        x *= v.x;
        y *= v.y;

        return *this;
    }

    constexpr Vector2& operator/=(const Vector2& v) noexcept
    {
        const float reciprocal{static_cast<T>(1) / (v.x * v.y)};

        x *= v.y * reciprocal;
        y *= v.x * reciprocal;

        return *this;
    }

    constexpr Vector2& operator/=(const Vector2&& v) noexcept
    {
        const float reciprocal{static_cast<T>(1) / (v.x * v.y)};
        x *= v.y * reciprocal;
        y *= v.x * reciprocal;

        return *this;
    }

    constexpr Vector2& operator*=(const T k) noexcept
    {
        x *= k;
        y *= k;

        return *this;
    }

    constexpr Vector2& operator/=(const T k) noexcept
    {
        const float reciprocal{static_cast<T>(1) / k};

        x *= reciprocal;
        y *= reciprocal;

        return *this;
    }

    constexpr Vector2 operator+(const Vector2& v) const noexcept
    {
        return {x + v.x, y + v.y};
    }

    constexpr Vector2 operator+(const Vector2&& v) const noexcept
    {
        return {x + v.x, y + v.y};
    }

    constexpr Vector2 operator-(const Vector2& v) const noexcept
    {
        return {x - v.x, y - v.y};
    }

    constexpr Vector2 operator-(const Vector2&& v) const noexcept
    {
        return {x - v.x, y - v.y};
    }

    constexpr Vector2 operator*(const Vector2& v) const noexcept
    {
        return {x * v.x, y * v.y};
    }

    constexpr Vector2 operator*(const Vector2&& v) const noexcept
    {
        return {x * v.x, y * v.y};
    }

    constexpr Vector2 operator/(const Vector2& v) const noexcept
    {
        const float reciprocal{static_cast<T>(1) / (v.x * v.y)};

        return {x * v.y * reciprocal, y * v.x * reciprocal};
    }

    constexpr Vector2 operator/(const Vector2&& v) const noexcept
    {
        const float reciprocal{static_cast<T>(1) / (v.x * v.y)};

        return {x * v.y * reciprocal, y * v.x * reciprocal};
    }

    constexpr Vector2 operator-() const noexcept
    {
        return {-x, -y};
    }

    constexpr Vector2 operator-(const T k) const noexcept
    {
        return {x - k, y - k};
    }

    constexpr Vector2 operator*(const T k) const noexcept
    {
        return {x * k, y * k};
    }

    constexpr Vector2 operator/(const T k) const noexcept
    {
        return {x / k, y / k};
    }

    template <typename U>
    constexpr operator Vector2<U>() noexcept
    {
        return Vector2<U>{static_cast<U>(x), static_cast<U>(y)};
    }
};

namespace YAML
{
template <typename T>
struct convert<Vector2<T>>
{
    static Node encode(const Vector2<T>& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node& node, Vector2<T>& rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
        {
            return false;
        }

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
    }
};
} // namespace YAML

using Vec2  = Vector2<float>;
using vec2  = Vector2<float>;
using Vec2i = Vector2<int>;
using vec2i = Vector2<int>;