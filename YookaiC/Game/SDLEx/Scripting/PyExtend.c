#include "PyExtend.h"
#include "../Utils/ArrayList.h"

#include "RenderBindings.h"

static ArrayList * methods = NULL;
static PyModuleDef module = {
	PyModuleDef_HEAD_INIT, "yookaic", NULL, -1, NULL,
	NULL, NULL, NULL, NULL
};

static PyObject * create_py_module(void) {
	module.m_methods = (PyMethodDef *)methods->_data;
	return PyModule_Create(&module);
}

void extend_py() {
	if (methods != NULL)
		destroy_array_list(methods);
	methods = create_array_list(sizeof(PyMethodDef), 8);

	extend_py_render_bindings(methods);

	PyMethodDef end_marker = {
		NULL, NULL, 0, NULL
	};
	append_array_list(methods, &end_marker);
	PyImport_AppendInittab("yookaic", &create_py_module);
}
