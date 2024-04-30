#ifndef QAMAR_TYPES_H
#define QAMAR_TYPES_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "interpreter.h"
#include "treemap.h"

struct stack_to_pyobj_extra {
	LuaInterpreter *lua;
	struct treemap viewed_tables;
};

PyObject*
lua_stack_to_pyobj(lua_State* L, int index, int type, struct stack_to_pyobj_extra *extra);

typedef struct {
	PyObject_HEAD
	LuaInterpreter *interpreter;
	int func_ref;
} LuaFunction;

void qamar_python_to_lua(lua_State *L, PyObject *obj, struct treemap *visited_objs);

void qamar_visited_obj_free_dtor(void *_key, intptr_t value, void* luastate);

int
qamar_lua_func_init();

extern PyMethodDef LuaFunctionMethods[];


extern PyTypeObject LuaFunctionType;

int
qamar_exec_pyfunc(lua_State *L);

int
qamar_gc_pyfunc(lua_State *L);

#endif // QAMAR_TYPES_H