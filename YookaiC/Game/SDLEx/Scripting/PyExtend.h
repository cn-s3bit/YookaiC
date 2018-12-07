#ifndef SDLEX_PY_EXTEND_H
#define SDLEX_PY_EXTEND_H
#include "../LazyPython.h"
void extend_py();

inline void extended_py_initialize(char ** argv) {
	if (argv == NULL) {
		Py_SetProgramName(L"YookaiC-SDLEx Program");
	}
	else {
		wchar_t * program = Py_DecodeLocale(argv[0], NULL);
		if (program == NULL) {
			Py_SetProgramName(L"YookaiC-SDLEx Program");
		}
		else {
			Py_SetProgramName(program);
		}
	}
	extend_py();
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
