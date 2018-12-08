#include "../LazyPython.h"
#include "RenderBindings.h"
#include "../Vulkan/SDLExVulkan.h"

static PyObject * _sdlex_binding_load_texture2d(PyObject * self, PyObject * args) {
	char * path;
	if (!PyArg_ParseTuple(args, "s:load_texture2d", &path))
		return NULL;
	return PyLong_FromLong(load_texture2d(path));
}

static PyObject * _sdlex_binding_dispose_texture2d(PyObject * self, PyObject * args) {
	int textureID;
	if (!PyArg_ParseTuple(args, "i:dispose_texture2d", &textureID))
		return NULL;
	dispose_texture2d(textureID);
	Py_RETURN_NONE;
}

static PyObject * _sdlex_binding_bind_texture2d(PyObject * self, PyObject * args) {
	int textureID;
	unsigned imageID;
	if (!PyArg_ParseTuple(args, "Ii:bind_texture2d", &imageID, &textureID))
		return NULL;
	bind_texture2d(imageID, textureID);
	Py_RETURN_NONE;
}

static PyObject * _sdlex_binding_begin_frame(PyObject * self, PyObject * args) {
	if (!PyArg_ParseTuple(args, ":begin_frame"))
		return NULL;
	return PyLong_FromUnsignedLong(sdlex_begin_frame());
}

static PyObject * _sdlex_binding_end_frame(PyObject * self, PyObject * args) {
	unsigned imageID;
	if (!PyArg_ParseTuple(args, "I:end_frame", &imageID))
		return NULL;
	sdlex_end_frame(imageID);
	Py_RETURN_NONE;
}

static PyObject * _sdlex_binding_render_texture(PyObject * self, PyObject * args) {
	SDL_Rect drawRect;
	unsigned imageID;
	if (!PyArg_ParseTuple(args,
		"Iiiii:render_texture",
		&imageID,
		&drawRect.x, &drawRect.y, &drawRect.w, &drawRect.h))
		return NULL;
	sdlex_render_texture(imageID, drawRect);
	Py_RETURN_NONE;
}

static PyObject * _sdlex_binding_render_texture_ex(PyObject * self, PyObject * args) {
	Vector2 pos, origin, scale;
	float rotation;
	unsigned imageID;
	if (!PyArg_ParseTuple(args,
		"Ifffffff:render_texture_ex",
		&imageID,
		&pos.X, &pos.Y,
		&origin.X, &origin.Y,
		&rotation,
		&scale.X, &scale.Y))
		return NULL;
	sdlex_render_texture_ex(imageID, pos, origin, rotation, scale);
	Py_RETURN_NONE;
}

void extend_py_render_bindings(ArrayList * methods) {
	PyMethodDef beginFrame =
	{ "begin_frame", _sdlex_binding_begin_frame, METH_VARARGS, "Start a new Frame and return the ID of the Frame for later usage.\nArgs: None" };
	PyMethodDef endFrame =
	{ "end_frame", _sdlex_binding_end_frame, METH_VARARGS, "End a Frame represented by the ID and represent the rendered contents.\nArgs: frameID - integer" };
	PyMethodDef renderTexture =
	{ "render_texture", _sdlex_binding_render_texture, METH_VARARGS, "Render a texture to the Frame.\nArgs: frameID, x, y, w, h - integer" };
	PyMethodDef renderTextureEx =
	{ "render_texture_ex", _sdlex_binding_render_texture_ex, METH_VARARGS, "Render a texture to the Frame.\nArgs: frameID - integer, x, y, origX, origY, rotation, scaleX, scaleY - float" };
	PyMethodDef loadTexture2D =
	{ "load_texture2d", _sdlex_binding_load_texture2d, METH_VARARGS, "Load a texture and return the ID of the texture for later usage.\nArgs: textureFilePath - string" };
	PyMethodDef bindTexture2D =
	{ "bind_texture2d", _sdlex_binding_bind_texture2d, METH_VARARGS, "Bind a texture to a frame for rendering to follow.\nArgs: textureID - integer" };
	PyMethodDef disposeTexture2D =
	{ "dispose_texture2d", _sdlex_binding_dispose_texture2d, METH_VARARGS, "Release the resource a texture currently using.\nArgs: textureID - integer" };
	append_array_list(methods, &beginFrame);
	append_array_list(methods, &endFrame);
	append_array_list(methods, &renderTexture);
	append_array_list(methods, &renderTextureEx);
	append_array_list(methods, &loadTexture2D);
	append_array_list(methods, &bindTexture2D);
	append_array_list(methods, &disposeTexture2D);
}
