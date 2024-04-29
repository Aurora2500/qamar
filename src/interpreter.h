#ifndef QAMAR_INTERPRETER_H
#define QAMAR_INTERPRETER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "list.h"

typedef struct {
	PyObject_HEAD
	lua_State *L;
	struct list dangling_funcs;
} LuaInterpreter;

int
qamar_lua_init(LuaInterpreter *self, PyObject *args, PyObject *kwds);

void
qamar_lua_dealloc(LuaInterpreter *self);

PyObject*
qamar_lua_exec(LuaInterpreter *self, PyObject *args);

PyObject*
qamar_lua_get_var(LuaInterpreter *self, PyObject *args);

PyObject*
qamar_lua_set_var(LuaInterpreter *self, PyObject *args);

extern PyMethodDef LuaInterpreterMethods[];
extern PyTypeObject LuaInterpreterType;
#endif // QAMAR_INTERPRETER_H