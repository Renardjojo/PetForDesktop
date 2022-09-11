/* =================== Constructors =================== */
inline constexpr Vector2::Vector2(const float k) noexcept : x{k}, y{k}
{
}

inline constexpr Vector2::Vector2(const float x_, const float y_) noexcept : x{x_}, y{y_}
{
}

inline constexpr Vector2::Vector2(const float coef[2]) noexcept : e{coef[0], coef[1]}
{
}

/* =================== Static methods (pseudo-constructors) =================== */
inline constexpr Vector2 Vector2::zero() noexcept
{
    return {.0f, .0f};
}

inline constexpr Vector2 one() noexcept
{
    return {1.f, 1.f};
}

inline constexpr Vector2 Vector2::right() noexcept
{
    return {1.f, .0f};
}

inline constexpr Vector2 Vector2::up() noexcept
{
    return {.0f, 1.f};
}

inline constexpr Vector2 Vector2::left() noexcept
{
    return {-1.f, .0f};
}

inline constexpr Vector2 Vector2::down() noexcept
{
    return {.0f, -1.f};
}

/* =================== Methods =================== */
inline constexpr float Vector2::sqrLength() const noexcept
{
    return (x * x) + (y * y);
}

inline float Vector2::length() const noexcept
{
    return sqrtf(sqrLength());
}

inline constexpr float Vector2::dot(const Vector2& v) const noexcept
{
    return (x * v.x) + (y * v.y);
}

inline constexpr float Vector2::cross(const Vector2& v) const noexcept
{
    return (x * v.y) - (y * v.x);
}

inline constexpr bool Vector2::isNull() const noexcept
{
    return !x && !y;
}

inline constexpr bool Vector2::isOrthogonalTo(const Vector2& v) const noexcept
{
    return !dot(v);
}

inline constexpr bool Vector2::isNormalized() const noexcept
{
    return !(sqrLength() - 1.f);
}

inline constexpr bool Vector2::isOrthonormalTo(const Vector2& v) const noexcept
{
    return !dot(v) && isNormalized() && v.isNormalized();
}

inline constexpr bool Vector2::isColinearTo(const Vector2& v) const noexcept
{
    return !cross(v);
}

inline bool Vector2::isEqualTo(const Vector2& v, const float eps) const noexcept
{
    return fabs(x - v.x) <= eps && fabs(y - v.y) <= eps;
}

inline bool Vector2::isNotEqualTo(const Vector2& v, const float eps) const noexcept
{
    return fabs(x - v.x) > eps || fabs(y - v.y) > eps;
}

inline void Vector2::normalize() noexcept
{
    *this /= length();
}

inline void Vector2::safelyNormalize() noexcept
{
    const float sqrLen{sqrLength()};

    if (sqrLen)
        *this /= sqrtf(sqrLen);
}

inline constexpr float Vector2::sqrDistanceTo(const Vector2& v) const noexcept
{
    return (*this - v).sqrLength();
}

inline float Vector2::distanceTo(const Vector2& v) const noexcept
{
    return (*this - v).length();
}

inline float Vector2::angleWithUnitary(const Vector2& v) const noexcept
{
    return acosf(dot(v) / length());
}

inline float Vector2::angleWith(const Vector2& v) const noexcept
{
    return acosf(dot(v) / sqrtf(sqrLength() * v.sqrLength()));
}

inline constexpr float Vector2::triangleArea(const Vector2& v) const noexcept
{
    return cross(v) * .5f;
}

inline constexpr Vector2 Vector2::projectedOnUnitary(const Vector2& v) const noexcept
{
    return v * dot(v);
}

inline constexpr Vector2 Vector2::projectedOn(const Vector2& v) const noexcept
{
    return v * (dot(v) / v.sqrLength());
}

inline Vector2 Vector2::normalized() const noexcept
{
    return *this / length();
}

inline Vector2 Vector2::safelyNormalized() const noexcept
{
    const float sqrLen{sqrLength()};

    return sqrLen ? (*this / sqrtf(sqrLen)) : *this;
}

inline Vector2 Vector2::rotated(const float angle) const noexcept
{
    const float cosAngle{cosf(angle)}, sinAngle{sinf(angle)};

    return {(cosAngle * x) - (sinAngle * y), (sinAngle * x) + (cosAngle * y)};
}

inline constexpr Vector2 Vector2::rotated90() const noexcept
{
    return {y, -x};
}

inline constexpr Vector2 Vector2::lerp(const Vector2& v, const float t) const noexcept
{
    const float tmp{1.f - t};

    return {(x * tmp) + (v.x * t), (y * tmp) + (v.y * t)};
}

inline constexpr Vector2 Vector2::reflect(const Vector2& n) const noexcept
{
    return *this - n * (2.f * dot(n));
}

inline constexpr Vector2& Vector2::operator+=(const Vector2& v) noexcept
{
    x += v.x;
    y += v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator+=(const Vector2&& v) noexcept
{
    x += v.x;
    y += v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator-=(const Vector2& v) noexcept
{
    x -= v.x;
    y -= v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator-=(const Vector2&& v) noexcept
{
    x -= v.x;
    y -= v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator*=(const Vector2& v) noexcept
{
    x *= v.x;
    y *= v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator*=(const Vector2&& v) noexcept
{
    x *= v.x;
    y *= v.y;

    return *this;
}

inline constexpr Vector2& Vector2::operator/=(const Vector2& v) noexcept
{
    const float reciprocal{1.f / (v.x * v.y)};

    x *= v.y * reciprocal;
    y *= v.x * reciprocal;

    return *this;
}

inline constexpr Vector2& Vector2::operator/=(const Vector2&& v) noexcept
{
    const float reciprocal{1.f / (v.x * v.y)};
    x *= v.y * reciprocal;
    y *= v.x * reciprocal;

    return *this;
}

inline constexpr Vector2& Vector2::operator*=(const float k) noexcept
{
    x *= k;
    y *= k;

    return *this;
}

inline constexpr Vector2& Vector2::operator/=(const float k) noexcept
{
    const float reciprocal{1.f / k};

    x *= reciprocal;
    y *= reciprocal;

    return *this;
}

inline constexpr Vector2 Vector2::operator+(const Vector2& v) const noexcept
{
    return {x + v.x, y + v.y};
}

inline constexpr Vector2 Vector2::operator+(const Vector2&& v) const noexcept
{
    return {x + v.x, y + v.y};
}

inline constexpr Vector2 Vector2::operator*(const Vector2& v) const noexcept
{
    return {x * v.x, y * v.y};
}

inline constexpr Vector2 Vector2::operator*(const Vector2&& v) const noexcept
{
    return {x * v.x, y * v.y};
}

inline constexpr Vector2 Vector2::operator/(const Vector2& v) const noexcept
{
    const float reciprocal{1.f / (v.x * v.y)};

    return {x * v.y * reciprocal, y * v.x * reciprocal};
}

inline constexpr Vector2 Vector2::operator/(const Vector2&& v) const noexcept
{
    const float reciprocal{1.f / (v.x * v.y)};

    return {x * v.y * reciprocal, y * v.x * reciprocal};
}

inline constexpr Vector2 Vector2::operator-() const noexcept
{
    return {-x, -y};
}

inline constexpr Vector2 Vector2::operator-(const Vector2& v) const noexcept
{
    return {x - v.x, y - v.y};
}

inline constexpr Vector2 Vector2::operator-(const Vector2&& v) const noexcept
{
    return {x - v.x, y - v.y};
}

inline constexpr Vector2 Vector2::operator*(const float k) const noexcept
{
    return {x * k, y * k};
}

inline constexpr Vector2 Vector2::operator/(const float k) const noexcept
{
    const float reciprocal{1.f / k};

    return {x * reciprocal, y * reciprocal};
}