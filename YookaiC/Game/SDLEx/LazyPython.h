#ifndef SDLEX_LAZY_PYTHON_H
#define SDLEX_LAZY_PYTHON_H
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
#include "Python.h"
#endif
#include "Scripting/PyExtend.h"

inline void extended_py_initialize(char ** argv) {
	if (argv == NULL) {
		Py_SetProgramName(L"YookaiC-SDLEx Program");
	}
	else {
		wchar_t * program = Py_DecodeLocale(argv[0], NULL);
		if (program == NULL) {
			Py_SetProgramName(L"YookaiC-SDLEx Program");
		} else {
			Py_SetProgramName(program);
		}
	}
	Py_Initialize();
}

inline int extended_py_dispose() {
	int ret;
	if ((ret = Py_FinalizeEx()) < 0) {
		return ret;
	}
	PyMem_RawFree(Py_GetProgramName());
	return 0;
}
#endif
