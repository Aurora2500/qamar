#include "interpreter.h"
#include "types.h"
#include "list.h"

int 
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

	luaL_newmetatable(self->L, "qamar.func");
	lua_pushstring(self->L, "__gc");
	lua_pushcfunction(self->L, qamar_gc_pyfunc);
	lua_settable(self->L, -3);
	lua_pop(self->L, 1);

	lua_pushstring(self->L, "qamar.interpreter");
	lua_pushlightuserdata(self->L, self);
	lua_rawset(self->L, LUA_REGISTRYINDEX);

	return 0;
}

void
qamar_lua_dealloc(LuaInterpreter *self) {
	lua_close(self->L);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject*
qamar_lua_exec(LuaInterpreter *self, PyObject *args) {
	const char *code;
	if (!PyArg_ParseTuple(args, "s", &code)) {
		PyErr_SetString(PyExc_RuntimeError, "Failed to parse arguments");
		return NULL;
	}
	int result;
	Py_BEGIN_ALLOW_THREADS
	result = luaL_dostring(self->L, code);
	Py_END_ALLOW_THREADS
	qamar_list_free(&self->dangling_funcs, (void(*)(void*))Py_DecRef);

	if (result != LUA_OK) {
		PyErr_SetString(PyExc_RuntimeError, lua_tostring(self->L, -1));
		lua_pop(self->L, 1);
		return NULL;
	}
	Py_RETURN_NONE;
}

PyObject*
qamar_lua_get_var(LuaInterpreter *self, PyObject *args) {
	char *arg_name;
	if (!PyArg_ParseTuple(args, "s", &arg_name))
	{
		PyErr_SetString(PyExc_TypeError, "Expected string argument");
		return NULL;
	}
	int type = lua_getglobal(self->L, arg_name);
	struct stack_to_pyobj_extra extra = {
		.lua = self,
		.viewed_tables = {0},
	};
	PyObject *pyobj = lua_stack_to_pyobj(self->L, -1, type, &extra);
	qamar_treemap_free(&extra.viewed_tables, NULL, NULL);
	lua_pop(self->L, 1);
	return pyobj;
}

PyObject*
qamar_lua_set_var(LuaInterpreter *self, PyObject *args) {
	char *arg_name;
	PyObject *arg_value;
	if (!PyArg_ParseTuple(args, "sO", &arg_name, &arg_value)) {
		return NULL;
	}
	struct treemap visited_obj = {0};
	qamar_python_to_lua(self->L, arg_value, &visited_obj);
	
	lua_setglobal(self->L, arg_name);
	Py_RETURN_NONE;
}

PyMethodDef LuaInterpreterMethods[] = {
	{"init", (PyCFunction)qamar_lua_init, METH_VARARGS | METH_KEYWORDS, "Initialize Lua interpreter"},
	{"execute", (PyCFunction)qamar_lua_exec, METH_VARARGS, "Execute Lua code"},
	{"get", (PyCFunction)qamar_lua_get_var, METH_VARARGS, "Get Lua variable"},
	{"set", (PyCFunction)qamar_lua_set_var, METH_VARARGS, "Set Lua variable"},
	{NULL, NULL, 0, NULL}
};


PyTypeObject LuaInterpreterType = {
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