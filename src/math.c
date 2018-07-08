/*
  TODO:
    - Should we have wrappers for all the external math functions?
*/

static inline Vec2
vec2(f32 x, f32 y) { 
    Vec2 result = {.x = x, .y = y};
    return result;
}

static inline Vec3
vec3(f32 x, f32 y, f32 z) {
    Vec3 result = {.x = x, .y = y, .z = z};
    return result;
}

static inline Vec4
vec4(f32 x, f32 y, f32 z, f32 w) {
    Vec4 result = {.x = x, .y = y, .z = z, .w = w};
    return result;
}

#undef min
#undef max
#define min HMM_MIN
#define max HMM_MAX

static inline f32
radians(f32 degrees) {
    return degrees*0.0174533;
}

static inline f32
clamp(f32 value, f32 _min, f32 _max) {
    f32 result = min(_max, max(value, _min));
    return result;
}

// Normalize or zero
static inline Vec3
noz_vec3(Vec3 input) {
    //HMM normalize returns a zero vector if input is zero
    Vec3 result = HMM_NormalizeVec3(input);
    return result;
}

static inline Vec2
noz_vec2(Vec2 input) {
    //HMM normalize returns a zero vector if input is zero
    Vec2 result = HMM_NormalizeVec2(input);
    return result;
}

static inline Vec3
rotate_vec3(Vec3 in, Vec3 axis, f32 theta) {
    f32 cosTheta = HMM_CosF(theta);
    f32 sinTheta = HMM_SinF(theta);
    Vec3 result = 
        HMM_AddVec3(
            HMM_AddVec3(
                HMM_MultiplyVec3f(in, cosTheta), 
                HMM_MultiplyVec3f(HMM_Cross(axis, in), sinTheta)),
            HMM_MultiplyVec3f(
                HMM_MultiplyVec3f(axis, HMM_DotVec3(axis, in)),
                1 - cosTheta));
    return result;
}

static inline f32
angle_between_vec3(Vec3 a, Vec3 b) {
    Vec3 da = noz_vec3(a);
    Vec3 db = noz_vec3(b);
    return HMM_ACosF(HMM_DotVec3(da, db));
}

static inline f32
angle_between_origin_vec3(Vec3 a, Vec3 b, Vec3 origin) {
    Vec3 da = noz_vec3(HMM_SubtractVec3(a, origin));
    Vec3 db = noz_vec3(HMM_SubtractVec3(b, origin));
    return HMM_ACosF(HMM_DotVec3(da, db));
}

static inline Vec3
spherical_to_cartesian_vec3(f32 radius, f32 longtitude, f32 latitude) {
    f32 x = radius * HMM_CosF(latitude) * HMM_SinF(longtitude);
    f32 y = radius * HMM_SinF(latitude);
    f32 z = radius * HMM_CosF(latitude) * HMM_CosF(longtitude);
    Vec3 result = {.x = x, .y = y, .z = z};
    return result;
 }
