/*
 * Copyright (C) 2021 Amara Sami, Dallard Thomas, Nardone William, Six Jonathan
 * This file is subject to the LGNU license terms in the LICENSE file
 * found in the top-level directory of this distribution.
 */

#pragma once

#include <cfloat>
#include <cmath>

union Vector2 {
    // Data members
    struct
    {
        float x;
        float y;
    };
    float e[2];

    Vector2() = default;
    constexpr Vector2(const float k) noexcept;
    constexpr Vector2(const float x, const float y = .0f) noexcept;
    constexpr Vector2(const float coef[2]) noexcept;

    // Static methods (pseudo-constructors)
    static constexpr Vector2 zero() noexcept;
    static constexpr Vector2 one() noexcept;
    static constexpr Vector2 right() noexcept;
    static constexpr Vector2 up() noexcept;
    static constexpr Vector2 left() noexcept;
    static constexpr Vector2 down() noexcept;

    // Member methods
    constexpr float   sqrLength() const noexcept;
    float             length() const noexcept;
    constexpr float   dot(const Vector2& v) const noexcept;
    constexpr float   cross(const Vector2& v) const noexcept;
    constexpr bool    isNull() const noexcept;
    constexpr bool    isOrthogonalTo(const Vector2& v) const noexcept;
    constexpr bool    isNormalized() const noexcept;
    constexpr bool    isOrthonormalTo(const Vector2& v) const noexcept;
    constexpr bool    isColinearTo(const Vector2& v) const noexcept;
    bool              isEqualTo(const Vector2& v, const float eps = FLT_EPSILON) const noexcept;
    bool              isNotEqualTo(const Vector2& v, const float eps = FLT_EPSILON) const noexcept;
    void              normalize() noexcept;
    void              safelyNormalize() noexcept;
    constexpr float   sqrDistanceTo(const Vector2& v) const noexcept;
    float             distanceTo(const Vector2& v) const noexcept;
    float             angleWithUnitary(const Vector2& v) const noexcept;
    float             angleWith(const Vector2& v) const noexcept;
    constexpr float   triangleArea(const Vector2& v) const noexcept;
    Vector2           normalized() const noexcept;
    Vector2           safelyNormalized() const noexcept;
    constexpr Vector2 projectedOnUnitary(const Vector2& v) const noexcept;
    constexpr Vector2 projectedOn(const Vector2& v) const noexcept;
    Vector2           rotated(const float angle) const noexcept;
    constexpr Vector2 rotated90() const noexcept;
    constexpr Vector2 lerp(const Vector2& v, const float f) const noexcept;
    constexpr Vector2 reflect(const Vector2& n) const noexcept;

    // Operator overloads
    constexpr Vector2& operator+=(const Vector2& v) noexcept;
    constexpr Vector2& operator+=(const Vector2&& v) noexcept;
    constexpr Vector2& operator-=(const Vector2& v) noexcept;
    constexpr Vector2& operator-=(const Vector2&& v) noexcept;
    constexpr Vector2& operator*=(const Vector2& v) noexcept;
    constexpr Vector2& operator*=(const Vector2&& v) noexcept;
    constexpr Vector2& operator/=(const Vector2& v) noexcept;
    constexpr Vector2& operator/=(const Vector2&& v) noexcept;
    constexpr Vector2& operator*=(const float k) noexcept;
    constexpr Vector2& operator/=(const float k) noexcept;
    constexpr Vector2  operator+(const Vector2& v) const noexcept;
    constexpr Vector2  operator+(const Vector2&& v) const noexcept;
    constexpr Vector2  operator-(const Vector2& v) const noexcept;
    constexpr Vector2  operator-(const Vector2&& v) const noexcept;
    constexpr Vector2  operator*(const Vector2& v) const noexcept;
    constexpr Vector2  operator*(const Vector2&& v) const noexcept;
    constexpr Vector2  operator/(const Vector2& v) const noexcept;
    constexpr Vector2  operator/(const Vector2&& v) const noexcept;
    constexpr Vector2  operator-() const noexcept;
    constexpr Vector2  operator*(const float k) const noexcept;
    constexpr Vector2  operator/(const float k) const noexcept;
};

using Vec2 = Vector2;
using vec2 = Vector2;

#include "Vector2.inl"