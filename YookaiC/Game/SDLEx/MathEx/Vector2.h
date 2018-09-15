#ifndef SDLEX_MATHEX_VECTOR2_H
#define SDLEX_MATHEX_VECTOR2_H

typedef struct Vector2 {
	float X, Y;
} Vector2;

inline Vector2 vector2_zero() {
	Vector2 res = { 0.0f, 0.0f };
	return res;
}

inline Vector2 vector2_one() {
	Vector2 res = { 1.0f, 1.0f };
	return res;
}

inline Vector2 vector2_unitX() {
	Vector2 res = { 1.0f, 0.0f };
	return res;
}

inline Vector2 vector2_unitY() {
	Vector2 res = { 0.0f, 1.0f };
	return res;
}

typedef struct Vector2Operations {
	Vector2 Current;
	struct Vector2Operations * (*Set)(Vector2);
	struct Vector2Operations * (*Add)(Vector2);
} Vector2Operations;

extern Vector2Operations * Vct2Ops;

inline Vector2Operations * get_vct2op_context() {
	return Vct2Ops;
}


#endif
