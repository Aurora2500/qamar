#ifndef QAMAR_TYPES_H
#define QAMAR_TYPES_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

PyObject*
lua_stack_to_pyobj(lua_State *L, int index, int type);

typedef struct {
	PyObject_HEAD
	PyObject *interpreter;
	int func_ref;
} LuaFunction;

int
qamar_lua_func_init();

extern PyMethodDef LuaFunctionMethods[];


extern PyTypeObject LuaFunctionType;

#endif // QAMAR_TYPES_H