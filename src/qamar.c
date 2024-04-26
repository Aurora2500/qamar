#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "interpreter.h"
#include "types.h"


// typedef struct {
// 	PyObject_HEAD
// 	PyObject *interpreter;
// 	int func_ref;
// } LuaFunction;

// int
// qamar_lua_func_init()
// {}

// static PyMethodDef LuaFunctionMethods[] = {
// 	{NULL, NULL, 0, NULL}
// };

// static PyTypeObject LuaFunctionType = {
// 	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
// 	.tp_name = "qamar.lua.function",
// 	.tp_doc = "Lua function",
// 	.tp_basicsize = sizeof(LuaFunction),
// 	.tp_itemsize = 0,
// 	.tp_flags = Py_TPFLAGS_DEFAULT
// };

static PyModuleDef qamar_module = {
	PyModuleDef_HEAD_INIT,
	"qamar",
	"A Python module that prints 'Hello, World!'",
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
