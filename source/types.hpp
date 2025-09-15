#pragma once

#include <cstdint>
#include <cstdio>
#include <cmath>

#define COL_RESET "\033[0m"
#define COL_INFO "\033[32m"          // Green
#define COL_WARN "\033[33m"          // Yellow
#define COL_ERROR "\033[31m"         // Red
#define COL_FATAL "\033[41m\033[97m" // White on Red background

typedef const char *cstr;

#define LIST_VAR "\n\t> "

#define INFO(msg, ...) \
    printf(COL_INFO "[INFO]" COL_RESET "  [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define WARN(msg, ...) \
    printf(COL_WARN "[WARN]" COL_RESET "  [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define ERR(msg, ...) \
    printf(COL_ERROR "[ERROR]" COL_RESET " [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define FATAL(msg, ...)                                            \
    do                                                             \
    {                                                              \
        printf(COL_FATAL "[FATAL]" COL_RESET " [%s:%d] " msg "\n", \
               __func__,                                           \
               __LINE__,                                           \
               ##__VA_ARGS__);                                     \
        abort();                                                   \
    } while (0);

#define global static
#define persist static
#define internal static

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float_t;
using f64 = double_t;

// Strip name from variable in macro.
// @example StripName("Mem->foo.bar") == "bar"
// @example StripName("Mem->foo") == "foo"
cstr StripName(cstr var)
{
    cstr strippedName = var;
    u64 len = strlen(var);
    for (int i = len - 1; i >= 0; i--)
    {
        if (var[i] == '>' || var[i] == '.')
        {
            strippedName = &var[i + 1];
            break;
        }
    }

    return strippedName;
}

template <typename T>
constexpr bool IsZero(T value, T epsilon = 0.001f)
{
    return fabs(value) < epsilon;
}

namespace Rand
{
    u32 Init(u32 seed = 123456789u)
    {
        srand(seed);
        INFO("Initialized random seed:\t%d", seed);
        return seed;
    }
} // namespace Rand
struct v2
{
    f32 x, y;

    f32 LenSq() { return x * x + y * y; }
    f32 Len() { return sqrt(this->LenSq()); }

    f32 Cross(v2 b) { return x * b.y - y * b.x; }

    constexpr v2 operator+() const { return *this; }
    constexpr v2 operator-() const { return {-x, -y}; }

    constexpr v2 operator+(const v2 &rhs) const { return {x + rhs.x, y + rhs.y}; }
    constexpr v2 operator-(const v2 &rhs) const { return {x - rhs.x, y - rhs.y}; }
    constexpr v2 operator*(const v2 &rhs) const { return {x * rhs.x, y * rhs.y}; }
    constexpr v2 operator/(const v2 &rhs) const { return {x / rhs.x, y / rhs.y}; }

    constexpr v2 operator+(f32 s) const { return {x + s, y + s}; }
    constexpr v2 operator-(f32 s) const { return {x - s, y - s}; }
    constexpr v2 operator*(f32 s) const { return {x * s, y * s}; }
    constexpr v2 operator/(f32 s) const { return {x / s, y / s}; }

    constexpr v2 &operator+=(const v2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    constexpr v2 &operator-=(const v2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    constexpr v2 &operator*=(const v2 &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }
    constexpr v2 &operator/=(const v2 &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    constexpr v2 &operator+=(f32 s)
    {
        x += s;
        y += s;
        return *this;
    }
    constexpr v2 &operator-=(f32 s)
    {
        x -= s;
        y -= s;
        return *this;
    }
    constexpr v2 &operator*=(f32 s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    constexpr v2 &operator/=(f32 s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    constexpr friend v2 operator*(v2 v, float s) { return {v.x * s, v.y * s}; }
    constexpr friend v2 operator*(float s, v2 v) { return {v.x * s, v.y * s}; }
};

struct v3
{
    f32 x, y, z;

    f32 LenSq() { return x * x + y * y + z * z; }
    f32 Len() { return sqrt(this->LenSq()); }

    v3 Cross(v3 b) { return v3{y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x}; }

    constexpr v3 operator+() const { return *this; }
    constexpr v3 operator-() const { return {-x, -y, -z}; }

    constexpr v3 operator+(const v3 &rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    constexpr v3 operator-(const v3 &rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    constexpr v3 operator*(const v3 &rhs) const { return {x * rhs.x, y * rhs.y, z * rhs.z}; }
    constexpr v3 operator/(const v3 &rhs) const { return {x / rhs.x, y / rhs.y, z / rhs.z}; }

    constexpr v3 operator+(f32 s) const { return {x + s, y + s, z + s}; }
    constexpr v3 operator-(f32 s) const { return {x - s, y - s, z - s}; }
    constexpr v3 operator*(f32 s) const { return {x * s, y * s, z * s}; }
    constexpr v3 operator/(f32 s) const { return {x / s, y / s, z / s}; }

    constexpr v3 &operator+=(const v3 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    constexpr v3 &operator-=(const v3 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    constexpr v3 &operator*=(const v3 &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    constexpr v3 &operator/=(const v3 &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    constexpr v3 &operator+=(f32 s)
    {
        x += s;
        y += s;
        z += s;
        return *this;
    }
    constexpr v3 &operator-=(f32 s)
    {
        x -= s;
        y -= s;
        z -= s;
        return *this;
    }
    constexpr v3 &operator*=(f32 s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr v3 &operator/=(f32 s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    constexpr friend v3 operator*(v3 v, float s) { return {v.x * s, v.y * s, v.z * s}; }
    constexpr friend v3 operator*(float s, v3 v) { return {v.x * s, v.y * s, v.z * s}; }
};

struct v4
{
    f32 x, y, z, w;

    f32 LenSq() { return x * x + y * y + z * z + w * w; }
    f32 Len() { return sqrt(this->LenSq()); }

    constexpr v4 operator+() const { return *this; }
    constexpr v4 operator-() const { return {-x, -y, -z, -w}; }

    constexpr v4 operator+(const v4 &rhs) const
    {
        return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
    }
    constexpr v4 operator-(const v4 &rhs) const
    {
        return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
    }
    constexpr v4 operator*(const v4 &rhs) const
    {
        return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
    }
    constexpr v4 operator/(const v4 &rhs) const
    {
        return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
    }

    constexpr v4 operator+(f32 s) const { return {x + s, y + s, z + s, w + s}; }
    constexpr v4 operator-(f32 s) const { return {x - s, y - s, z - s, w - s}; }
    constexpr v4 operator*(f32 s) const { return {x * s, y * s, z * s, w * s}; }
    constexpr v4 operator/(f32 s) const { return {x / s, y / s, z / s, w / s}; }

    constexpr v4 &operator+=(const v4 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    constexpr v4 &operator-=(const v4 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    constexpr v4 &operator*=(const v4 &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }
    constexpr v4 &operator/=(const v4 &rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    constexpr v4 &operator+=(f32 s)
    {
        x += s;
        y += s;
        z += s;
        w += s;
        return *this;
    }
    constexpr v4 &operator-=(f32 s)
    {
        x -= s;
        y -= s;
        z -= s;
        w -= s;
        return *this;
    }
    constexpr v4 &operator*=(f32 s)
    {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    constexpr v4 &operator/=(f32 s)
    {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    constexpr friend v4 operator*(v4 v, float s) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }
    constexpr friend v4 operator*(float s, v4 v) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }
};

template <typename T>
struct Damped
{
    T y, yd;

    Damped(T initial = {}) : y{initial}, yd{0} {}

    operator T() { return y; }
};

using f32d = Damped<f32>;
using v2d = Damped<v2>;
using v3d = Damped<v3>;
using v4d = Damped<v4>;

#if defined(__GNUC__) || defined(__clang__)
#define DEFER(func) __attribute__((cleanup(func)))
#else
#define DEFER(func)
#endif

static constexpr f32 PI = 3.14159265358979323846;
static constexpr f32 TAU = PI * 2;

struct Deg;

struct Rad
{
    f32 value;

    constexpr Rad(f32 v = 0) : value(v) {}
    constexpr Rad(Deg deg); // defined after Deg
    constexpr Rad &operator=(f32 rhs)
    {
        value = rhs;
        return *this;
    }

    constexpr explicit operator f32() const { return value; }

    constexpr Rad operator+() const { return *this; }
    constexpr Rad operator-() const { return Rad{-value}; }

    constexpr Rad operator+(Rad rhs) const { return Rad{value + rhs.value}; }
    constexpr Rad operator-(Rad rhs) const { return Rad{value - rhs.value}; }
    constexpr Rad operator*(f32 s) const { return Rad{value * s}; }
    constexpr Rad operator/(f32 s) const { return Rad{value / s}; }

    constexpr Rad &operator+=(Rad rhs)
    {
        value += rhs.value;
        return *this;
    }
    constexpr Rad &operator-=(Rad rhs)
    {
        value -= rhs.value;
        return *this;
    }
    constexpr Rad &operator*=(f32 s)
    {
        value *= s;
        return *this;
    }
    constexpr Rad &operator/=(f32 s)
    {
        value /= s;
        return *this;
    }

    constexpr bool operator<(Rad rhs) const { return value < rhs.value; }
    constexpr bool operator<=(Rad rhs) const { return value <= rhs.value; }
    constexpr bool operator>(Rad rhs) const { return value > rhs.value; }
    constexpr bool operator>=(Rad rhs) const { return value >= rhs.value; }
};

struct Deg
{
    f32 value;

    constexpr Deg(f32 v = 0) : value(v) {}
    constexpr Deg(Rad rad) : value(rad.value * (180.0f / PI)) {}
    constexpr Deg operator=(f32 rhs)
    {
        value = rhs;
        return *this;
    }

    constexpr Deg operator+() const { return *this; }
    constexpr Deg operator-() const { return Deg{-value}; }

    constexpr Deg operator+(Deg rhs) const { return Deg{value + rhs.value}; }
    constexpr Deg operator-(Deg rhs) const { return Deg{value - rhs.value}; }
    constexpr Deg operator*(f32 s) const { return Deg{value * s}; }
    constexpr Deg operator/(f32 s) const { return Deg{value / s}; }

    constexpr Deg &operator+=(Deg rhs)
    {
        value += rhs.value;
        return *this;
    }
    constexpr Deg &operator-=(Deg rhs)
    {
        value -= rhs.value;
        return *this;
    }
    constexpr Deg &operator*=(f32 s)
    {
        value *= s;
        return *this;
    }
    constexpr Deg &operator/=(f32 s)
    {
        value /= s;
        return *this;
    }

    constexpr bool operator<(Deg rhs) const { return value < rhs.value; }
    constexpr bool operator<=(Deg rhs) const { return value <= rhs.value; }
    constexpr bool operator>(Deg rhs) const { return value > rhs.value; }
    constexpr bool operator>=(Deg rhs) const { return value >= rhs.value; }
};

inline constexpr Rad::Rad(Deg deg) : value{deg.value / (180.0f / static_cast<f32>(PI))}
{
}

static Deg bla = 32.0f;
static Rad ble = bla;

template <typename T>
constexpr auto KB(T val)
{
    return val * 1024;
}

template <typename T>
constexpr auto MB(T val)
{
    return KB(val) * 1024;
}

template <typename T>
constexpr auto GB(T val)
{
    return MB(val) * 1024;
}

template <typename T>
constexpr auto TB(T val)
{
    return GB(val) * 1024;
}