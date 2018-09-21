#ifndef SDLEX_MATHEX_VECTOR2_H
#define SDLEX_MATHEX_VECTOR2_H
#include <stdio.h>
#include "../SDLWithPlugins.h"

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

inline Vector2 vector2_add(Vector2 a, Vector2 b) {
	a.X += b.X;
	a.Y += b.Y;
	return a;
}

#define vector2_sprintf(v, target) sprintf(target, "(%f, %f)", v.X, v.Y)
#define vector2_sprintf_s(v, target, maxlen) sprintf_s(target, maxlen, "(%f, %f)", v.X, v.Y)
#define vector2_sprintf_append(v, target) sprintf(target, "%s(%f, %f)", target, v.X, v.Y)
#define vector2_sprintf_append_s(v, target, maxlen) sprintf_s(target, maxlen, "%s(%f, %f)", target, v.X, v.Y)

inline void vector2_log_output(Vector2 v, const char * name) {
	char buffer[32];
#ifdef _MSC_VER
	vector2_sprintf_s(v, buffer, 32);
#else
	vector2_sprintf(v, buffer);
#endif
	SDL_Log("Vector2 %s = %s", name, buffer);
}

#endif
