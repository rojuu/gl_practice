/*
  TODO:
    - Should we have wrappers for all the external math functions?
*/

static inline Vec2
vec3(f32 x, f32 y) {
    Vec2 result {x, y};
    return result;
}

static inline Vec3
vec3(f32 x, f32 y, f32 z) {
    Vec3 result {x, y, z};
    return result;
}

static inline Vec4
vec3(f32 x, f32 y, f32 z, f32 w) {
    Vec4 result {x, y, z, w};
    return result;
}

#define min HMM_MIN
#define max HMM_MAX

static inline f32
radians(f32 degrees) {
    return degrees*0.0174533;
}

static inline f32
clamp(f32 value, f32 _min, f32 _max) {
    f32 result = min(_max, max(value, _min));
    return value;
}

// Normalize or zero
static inline Vec3
noz(Vec3 input) {
    Vec3 zero = {0};
    Vec3 result = input.x || input.y || input.z ? HMM_NormalizeVec3(input) : zero;
    return result;
}

static inline Vec2
noz(Vec2 input) {
    Vec2 zero = {0};
    Vec2 result = input.x || input.y ? HMM_NormalizeVec2(input) : zero;
    return result;
}

static Vec3
rotate(Vec3 in, Vec3 axis, f32 theta) {
    f32 cosTheta = HMM_CosF(theta);
    f32 sinTheta = HMM_SinF(theta);
    return (in * cosTheta) + (HMM_Cross(axis, in) * sinTheta) + (axis * HMM_DotVec3(axis, in)) * (1 - cosTheta);
}

inline f32
angle_between(Vec3 a, Vec3 b) {
    Vec3 da = noz(a);
    Vec3 db = noz(b);
    return HMM_ACosF(HMM_DotVec3(da, db));
}

inline f32
angle_between(Vec3 a, Vec3 b, Vec3 origin) {
    Vec3 da = noz(a - origin);
    Vec3 db = noz(b - origin);
    return HMM_ACosF(HMM_DotVec3(da, db));
}

static inline Vec3
spherical_to_cartesian(f32 radius, f32 longtitude, f32 latitude) {
    f32 x = radius * HMM_CosF(latitude) * HMM_SinF(longtitude);
    f32 y = radius * HMM_SinF(latitude);
    f32 z = radius * HMM_CosF(latitude) * HMM_CosF(longtitude);
    Vec3 result = {x,y,z};
    return result;
}
