#include "Vector2.h"

Vector2Operations * Vector2Operations_Set(Vector2 val) {
	Vector2Operations * ctx = get_vct2op_context();
	ctx->Current = val;
	return ctx;
}

Vector2Operations * Vector2Operations_Add(Vector2 val) {
	Vector2Operations * ctx = get_vct2op_context();
	ctx->Current.X += val.X;
	ctx->Current.Y += val.Y;
	return ctx;
}

Vector2Operations _SDLEx_MathEx_Vector2Ops0 = {
	.Set = &Vector2Operations_Set,
	.Add = &Vector2Operations_Add
};
Vector2Operations * Vct2Ops = &_SDLEx_MathEx_Vector2Ops0;

