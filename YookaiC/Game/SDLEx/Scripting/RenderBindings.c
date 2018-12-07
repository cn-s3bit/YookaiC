#include "../LazyPython.h"
#include "RenderBindings.h"
#include "../Vulkan/SDLExVulkan.h"

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

void extend_py_render_bindings(ArrayList * methods) {
	PyMethodDef beginFrame =
	{ "begin_frame", _sdlex_binding_begin_frame, METH_VARARGS, "Start a new Frame and return the ID of the Frame for later usage.\nArgs: None" };
	PyMethodDef endFrame =
	{ "end_frame", _sdlex_binding_end_frame, METH_VARARGS, "End a Frame represented by the ID and represent the rendered contents.\nArgs: imageID - integer" };
	PyMethodDef renderTexture =
	{ "render_texture", _sdlex_binding_render_texture, METH_VARARGS, "Render a texture to the Frame.\nArgs: imageID, x, y, w, h - integer" };
	append_array_list(methods, &beginFrame);
	append_array_list(methods, &endFrame);
	append_array_list(methods, &renderTexture);
}
