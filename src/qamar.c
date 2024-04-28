#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "interpreter.h"
#include "types.h"



static PyModuleDef qamar_module = {
	PyModuleDef_HEAD_INIT,
	"qamar",
	"A Python module that provides a Lua interpreter",
	-1,
};


PyMODINIT_FUNC
PyInit_qamar(void) {
	PyObject *m;

	if (PyType_Ready(&LuaInterpreterType) < 0) {
		return NULL;
	}

	m = PyModule_Create(&qamar_module);
	if (m == NULL) {
		return NULL;
	}

	Py_INCREF(&LuaInterpreterType);
	if (PyModule_AddObject(m, "lua", (PyObject*)&LuaInterpreterType) < 0) {
		Py_DECREF(&LuaInterpreterType);
		Py_DECREF(m);
		return NULL;
	}

	return m;
}
