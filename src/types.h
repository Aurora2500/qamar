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

#endif // QAMAR_TYPES_H