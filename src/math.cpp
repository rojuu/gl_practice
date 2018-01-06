internal inline f32
clamp(f32 value, f32 min, f32 max) {
	if(value < min) value = min;
	if(value > max) value = max;
	return value;
}

// Normalize or zero
internal inline v3
noz(v3 input) {
	v3 result = input.x||input.y||input.z ? glm::normalize(input) : v3(0,0,0);
	return result;
}

internal inline v2
noz(v2 input) {
	v2 result = input.x||input.y ? glm::normalize(input) : v2(0,0);
	return result;
}

internal v3
rotate(v3 in, v3 axis, f32 theta) {
	f32 cosTheta = glm::cos(theta);
	f32 sinTheta = glm::sin(theta);
	return (in * cosTheta) + (glm::cross(axis, in) * sinTheta) + (axis * glm::dot(axis, in)) * (1 - cosTheta);
}

inline f32
angleBetween(v3 a, v3 b) {
	v3 da=noz(a);
	v3 db=noz(b);
	return glm::acos(glm::dot(da, db));
}

inline f32
angleBetween(v3 a, v3 b, v3 origin) {
	v3 da=noz(a-origin);
	v3 db=noz(b-origin);
	return glm::acos(glm::dot(da, db));
}

internal inline v3
sphericalToCartesian(f32 radius, f32 longtitude, f32 latitude) {
	f32 x = radius * glm::cos(latitude) * glm::sin(longtitude);
	f32 y = radius * glm::sin(latitude);
	f32 z = radius * glm::cos(latitude) * glm::cos(longtitude);
	return v3(x,y,z);
}
