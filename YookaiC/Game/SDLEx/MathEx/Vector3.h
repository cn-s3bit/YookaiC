#ifndef SDLEX_MATHEX_VECTOR3_H
#define SDLEX_MATHEX_VECTOR3_H

typedef struct Vector3 {
	float X, Y, Z;
} Vector3;

inline Vector3 vector3_zero() {
	Vector3 res = { 0.0f, 0.0f, 0.0f };
	return res;
}

inline Vector3 vector3_one() {
	Vector3 res = { 1.0f, 1.0f, 1.0f };
	return res;
}

inline Vector3 vector3_unitX() {
	Vector3 res = { 1.0f, 0.0f, 0.0f };
	return res;
}

inline Vector3 vector3_unitY() {
	Vector3 res = { 0.0f, 1.0f, 0.0f };
	return res;
}

inline Vector3 vector3_unitZ() {
	Vector3 res = { 0.0f, 0.0f, 1.0f };
	return res;
}


#endif
