#include "../LazyPython.h"
#include "../SDLWithPlugins.h"
#include "InputEventBindings.h"

static PyObject * _sdlex_binding_poll_event(PyObject * self, PyObject * args) {
	char * path;
	if (!PyArg_ParseTuple(args, ":poll_event", &path))
		return NULL;
	SDL_Event sevent;
	if (SDL_PollEvent(&sevent)) {
		switch (sevent.type)
		{
			case SDL_KEYDOWN:
				if (sevent.key.repeat)
					break;
				return Py_BuildValue("ii", 1, sevent.key.keysym.sym);
			case SDL_KEYUP:
				return Py_BuildValue("ii", 2, sevent.key.keysym.sym);
			case SDL_QUIT:
				return Py_BuildValue("iO", 32767, Py_None);
			// TODO: More Types of Input
		}
	}
	Py_RETURN_NONE;
}

void extend_py_input_event_bindings(ArrayList * methods) {
	PyMethodDef pollEvent =
	{ "poll_event", _sdlex_binding_poll_event, METH_VARARGS, "Poll the queue of Input Events. Returns None if there are no events currently. Otherwise a tuple (type, data1, data2, ...) is returned." };
	append_array_list(methods, &pollEvent);
}
