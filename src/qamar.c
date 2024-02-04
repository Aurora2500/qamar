#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
	PyObject_HEAD
	lua_State *L;
} LuaInterpreter;

static int 
qamar_lua_init(LuaInterpreter *self, PyObject *args, PyObject *kwds) {
	self->L = luaL_newstate();
	if (self->L == NULL) {
		return -1;
	}

	static char *kwlist[] = {
		"all",
		"base",
		"coroutine",
		"package",
		"string",
		"utf8",
		"table",
		"math",
		"io",
		"os",
		"debug",
		NULL
	};
	bool all = false, base = false, coroutine = false, package = false, string = false, utf8 = false, table = false, math = false, io = false, os = false, debug = false;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ppppppppppp", kwlist,
			&all,
			&base,
			&coroutine,
			&package,
			&string,
			&utf8,
			&table,
			&math,
			&io,
			&os,
			&debug
		)) {
		return -1;
	}

	if (all) {
		luaL_openlibs(self->L);
	} else {
		if (base) {
			luaL_requiref(self->L, "_G", luaopen_base, 1);
			lua_pop(self->L, 1);
		}
		if (coroutine) {
			luaL_requiref(self->L, "coroutine", luaopen_coroutine, 1);
			lua_pop(self->L, 1);
		}
		if (package) {
			luaL_requiref(self->L, "package", luaopen_package, 1);
			lua_pop(self->L, 1);
		}
		if (string) {
			luaL_requiref(self->L, "string", luaopen_string, 1);
			lua_pop(self->L, 1);
		}
		if (utf8) {
			luaL_requiref(self->L, "utf8", luaopen_utf8, 1);
			lua_pop(self->L, 1);
		}
		if (table) {
			luaL_requiref(self->L, "table", luaopen_table, 1);
			lua_pop(self->L, 1);
		}
		if (math) {
			luaL_requiref(self->L, "math", luaopen_math, 1);
			lua_pop(self->L, 1);
		}
		if (io) {
			luaL_requiref(self->L, "io", luaopen_io, 1);
			lua_pop(self->L, 1);
		}
		if (os) {
			luaL_requiref(self->L, "os", luaopen_os, 1);
			lua_pop(self->L, 1);
		}
		if (debug) {
			luaL_requiref(self->L, "debug", luaopen_debug, 1);
			lua_pop(self->L, 1);
		}
	}

	return 0;
}

static void
qamar_lua_dealloc(LuaInterpreter *self) {
	lua_close(self->L);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject*
qamar_lua_exec(LuaInterpreter *self, PyObject *args) {
	const char *code;
	if (!PyArg_ParseTuple(args, "s", &code)) {
		return NULL;
	}

	int result = luaL_dostring(self->L, code);

	if (result != LUA_OK) {
		PyErr_SetString(PyExc_RuntimeError, lua_tostring(self->L, -1));
		lua_pop(self->L, 1);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject*
lua_stack_to_pyobj(lua_State *L, int index, int type)
{
	if (type == -1) {
		type = lua_type(L, index);
	}
	switch (type)
	{
		case LUA_TNIL:
			return Py_None;
		case LUA_TBOOLEAN: {
			PyObject *obj =  PyBool_FromLong(lua_toboolean(L, index));
			lua_pop(L, 1);
			return obj;
		}
		case LUA_TNUMBER: {
			PyObject *obj = PyFloat_FromDouble(lua_tonumber(L, index));
			lua_pop(L, 1);
			return obj;
		}
		case LUA_TSTRING: {
			const char *s = lua_tostring(L, index);
			return PyUnicode_FromString(s);
		}
		case LUA_TTABLE: {
			lua_pushnil(L);
			PyObject *dict = PyDict_New();
			while (lua_next(L, index) != 0) {
				PyObject *key = lua_stack_to_pyobj(L, -2, -1);
				PyObject *value = lua_stack_to_pyobj(L, -1, -1);
				PyDict_SetItem(dict, key, value);
				lua_pop(L, 1);
			}
			return dict;
		}
		default:
			assert("Not implemented yet!");
			return NULL;
	}
}

static PyObject*
qamar_lua_get_var(LuaInterpreter *self, PyObject *args) {
	char *arg_name;
	if (!PyArg_ParseTuple(args, "s", &arg_name))
	{
		printf("arg_name: %s\n", arg_name);
		return NULL;
	}
	int type = lua_getglobal(self->L, arg_name);
	PyObject *pyobj = lua_stack_to_pyobj(self->L, -1, type);
}

static PyMethodDef LuaInterpreterMethods[] = {
	{"init", (PyCFunction)qamar_lua_init, METH_VARARGS | METH_KEYWORDS, "Initialize Lua interpreter"},
	{"execute", (PyCFunction)qamar_lua_exec, METH_VARARGS, "Execute Lua code"},
	{"get", (PyCFunction)qamar_lua_get_var, METH_VARARGS, "Get Lua variable"},
	{NULL, NULL, 0, NULL}
};

static PyTypeObject LuaInterpreterType = {
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "qamar.lua",
	.tp_doc = "Lua interpreter",
	.tp_basicsize = sizeof(LuaInterpreter),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,

	.tp_new = PyType_GenericNew,
	.tp_init = (initproc)qamar_lua_init,
	.tp_dealloc = (destructor)qamar_lua_dealloc,

	.tp_methods = LuaInterpreterMethods
};

static PyObject*
qamar_hello_world(PyObject* self, PyObject* args) {
	return Py_BuildValue("s", "Hello, World!");
}

static PyMethodDef module_methods[] = {
	{"hello_world", qamar_hello_world, METH_VARARGS, "Print 'Hello, World!'"},
	{NULL, NULL, 0, NULL}
};

static PyModuleDef qamar_module = {
	PyModuleDef_HEAD_INIT,
	"qamar",
	"A Python module that prints 'Hello, World!'",
	-1,
	module_methods
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
