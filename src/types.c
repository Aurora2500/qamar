#include "types.h"
#include "interpreter.h"

PyObject*
lua_stack_to_pyobj(LuaInterpreter* lua, int index, int type) {
	lua_State *L = lua->L;
	if (type == -1) {
		type = lua_type(L, index);
	}
	switch (type)
	{
		case LUA_TNIL:
			Py_RETURN_NONE;
		case LUA_TBOOLEAN: {
			return PyBool_FromLong(lua_toboolean(L, index));
		}
		case LUA_TNUMBER: {
			return PyFloat_FromDouble(lua_tonumber(L, index));
		}
		case LUA_TSTRING: {
			return PyUnicode_FromString(lua_tostring(L, index));
		}
		case LUA_TTABLE: {
			PyObject *dict = PyDict_New();
			int abs_index = lua_absindex(L, index);
			lua_pushnil(L);
			while (lua_next(L, abs_index) != 0) {
				PyObject *key = lua_stack_to_pyobj(lua, -2, -1);
				PyObject *value = lua_stack_to_pyobj(lua, -1, -1);
				PyDict_SetItem(dict, key, value);
				lua_pop(L, 1);
			}
			return dict;
		}
		case LUA_TFUNCTION: {
			lua_pushvalue(L, index);
			int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
			Py_INCREF(lua);
			LuaFunction *func = PyObject_New(LuaFunction, &LuaFunctionType);
			func->interpreter = lua;
			func->func_ref = func_ref;
			return (PyObject*)func;
		}
		default:
			assert("Not implemented yet!");
			return NULL;
	}
}

void
qamar_python_to_lua(lua_State *L, PyObject *obj) {
	if (Py_IsNone(obj)) {
		lua_pushnil(L);
	} else if (PyBool_Check(obj)) {
		lua_pushboolean(L, obj == Py_True);
	} else if (PyLong_Check(obj) || PyFloat_Check(obj)) {
		lua_pushnumber(L, PyFloat_AsDouble(obj));
	} else if (PyUnicode_Check(obj)) {
		lua_pushstring(L, PyUnicode_AsUTF8(obj));
	} else if (PyDict_Check(obj)) {
		lua_newtable(L);
		PyObject *key, *value;
		Py_ssize_t pos = 0;
		while (PyDict_Next(obj, &pos, &key, &value)) {
			qamar_python_to_lua(L, key);
			qamar_python_to_lua(L, value);
			lua_settable(L, -3);
		}
	} else {
		assert("Not implemented yet!");
	}
}

int
qamar_lua_func_init() {

	return 0;
}

PyObject*
qamar_lua_function_call(PyObject *callable, PyObject *args, PyObject *kwargs)
{
	LuaFunction *self = (LuaFunction*)callable;
	lua_State *L = self->interpreter->L;

	int before_size = lua_gettop(L);

	lua_pushinteger(L, self->func_ref);
	lua_gettable(L, LUA_REGISTRYINDEX);
	int num_args = PyTuple_Size(args);
	for (int i = 0; i < num_args; i++) {
		PyObject *arg = PyTuple_GetItem(args, i);
		qamar_python_to_lua(L, arg);
	}
	lua_call(L, num_args, LUA_MULTRET);
	int num_results = lua_gettop(L) - before_size;
	if (num_results == 0) {
		Py_RETURN_NONE;
	} else if (num_results == 1) {
		PyObject *res = lua_stack_to_pyobj(self->interpreter, -1, -1);
		lua_pop(L, 1);
	return res;
	} else {
		PyObject *tuple = PyTuple_New(num_results);
		for (int i = 0; i < num_results; i++) {
			PyObject *result = lua_stack_to_pyobj(self->interpreter, before_size + i + 1, -1);
			PyTuple_SetItem(tuple, i, result);
		}
		lua_pop(L, num_results);
		return tuple;
	}
};

void
qamar_lua_func_dealloc(LuaFunction *self) {
	luaL_unref(self->interpreter->L, LUA_REGISTRYINDEX, self->func_ref);
	Py_DECREF(self->interpreter);
	PyObject_Del(self);
}

PyMethodDef LuaFunctionMethods[] = {
	{NULL, NULL, 0, NULL}
};

PyTypeObject LuaFunctionType = {
	.ob_base = PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "qamar.lua.function",
	.tp_doc = "Lua function",
	.tp_basicsize = sizeof(LuaFunction),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_call = qamar_lua_function_call,
	.tp_dealloc = (destructor)qamar_lua_func_dealloc,
};