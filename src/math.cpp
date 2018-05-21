static inline f32
min(f32 a, f32 b) {
    return a < b ? a : b;
}

static inline f32
max(f32 a, f32 b) {
    return a > b ? a : b;
}

static inline f32
clamp(f32 value, f32 _min, f32 _max) {
    f32 result = min(_max, max(value, _min));
    return value;
}

// Normalize or zero
static inline Vec3
noz(Vec3 input) {
    Vec3 result = input.x || input.y || input.z ? glm::normalize(input) : Vec3(0, 0, 0);
    return result;
}

static inline Vec2
noz(Vec2 input) {
    Vec2 result = input.x || input.y ? glm::normalize(input) : Vec2(0, 0);
    return result;
}

static Vec3
rotate(Vec3 in, Vec3 axis, f32 theta) {
    f32 cosTheta = glm::cos(theta);
    f32 sinTheta = glm::sin(theta);
    return (in * cosTheta) + (glm::cross(axis, in) * sinTheta) + (axis * glm::dot(axis, in)) * (1 - cosTheta);
}

inline f32
angleBetween(Vec3 a, Vec3 b) {
    Vec3 da = noz(a);
    Vec3 db = noz(b);
    return glm::acos(glm::dot(da, db));
}

inline f32
angleBetween(Vec3 a, Vec3 b, Vec3 origin) {
    Vec3 da = noz(a - origin);
    Vec3 db = noz(b - origin);
    return glm::acos(glm::dot(da, db));
}

static inline Vec3
sphericalToCartesian(f32 radius, f32 longtitude, f32 latitude) {
    f32 x = radius * glm::cos(latitude) * glm::sin(longtitude);
    f32 y = radius * glm::sin(latitude);
    f32 z = radius * glm::cos(latitude) * glm::cos(longtitude);
    return Vec3(x, y, z);
}
