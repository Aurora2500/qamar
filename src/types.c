#include "types.h"
#include "interpreter.h"

PyObject*
lua_stack_to_pyobj(lua_State* L, int index, int type, struct stack_to_pyobj_extra *extra) {
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
				PyObject *key = lua_stack_to_pyobj(L, -2, -1, extra);
				PyObject *value = lua_stack_to_pyobj(L, -1, -1, extra);
				if(!key || !value) {
					Py_DECREF(dict);
					lua_pop(L, 2);
					return NULL;
				}
				PyDict_SetItem(dict, key, value);
				lua_pop(L, 1);
			}
			return dict;
		}
		case LUA_TFUNCTION: {
			lua_pushvalue(L, index);
			int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
			LuaFunction *func = PyObject_New(LuaFunction, &LuaFunctionType);
			func->interpreter = (LuaInterpreter*)Py_NewRef(extra->lua);
			func->func_ref = func_ref;
			return (PyObject*)func;
		}
		default:
			PyErr_Format(PyExc_TypeError, "Type conversion not implemented for %s", lua_typename(L, type));
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
	} else if (PyList_Check(obj)) {
		lua_newtable(L);
		for (int i = 0; i < PyList_Size(obj); i++)
		{
			lua_pushinteger(L, i+1);
			qamar_python_to_lua(L, PyList_GET_ITEM(obj, i));
			lua_settable(L,-3);
		}
	} else if (PyTuple_Check(obj)) {
		lua_newtable(L);
		for (int i = 0; i < PyTuple_Size(obj); i++)
		{
			lua_pushinteger(L, i+1);
			qamar_python_to_lua(L, PyTuple_GetItem(obj, i));
			lua_settable(L,-3);
		}
	} else if (PyFunction_Check(obj)) {
		PyObject **udata = lua_newuserdata(L, sizeof(PyObject*));
		luaL_getmetatable(L, "qamar.func");
		lua_setmetatable(L, -2);
		*udata = Py_NewRef(obj);
		
		lua_pushcclosure(L, qamar_exec_pyfunc, 1);
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

	int status = lua_pcall(L, num_args, LUA_MULTRET, 0);

	if(status != LUA_OK) {
		PyObject *exc = PyExc_RuntimeError;
		if (status == LUA_ERRMEM) {
			exc = PyExc_MemoryError;
		}
		PyErr_Format(exc, "Error calling Lua function: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		return NULL;
	}

	int num_results = lua_gettop(L) - before_size;
	struct stack_to_pyobj_extra extra = {
		.lua = self->interpreter,
	};
	if (num_results == 0) {
		Py_RETURN_NONE;
	} else if (num_results == 1) {
		PyObject *res = lua_stack_to_pyobj(self->interpreter->L, -1, -1, &extra);
		lua_pop(L, 1);
	return res;
	} else {
		PyObject *tuple = PyTuple_New(num_results);
		for (int i = 0; i < num_results; i++) {
			PyObject *result = lua_stack_to_pyobj(self->interpreter->L, before_size + i + 1, -1, &extra);
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

int
qamar_exec_pyfunc(lua_State *L) {
	int num_args = lua_gettop(L);
	PyObject *func = * (PyObject**)lua_touserdata(L, lua_upvalueindex(1));

	lua_pushstring(L, "qamar.interpreter");
	lua_gettable(L, LUA_REGISTRYINDEX);
	LuaInterpreter *lua = lua_touserdata(L, -1);
	struct stack_to_pyobj_extra extra = {
		.lua = lua,
	};

	PyGILState_STATE gstate = PyGILState_Ensure();
	PyObject *args = PyTuple_New(num_args);
	for (int i = 0; i < num_args; i++) {
		PyObject *arg = lua_stack_to_pyobj(L, i+1, -1, &extra);
		PyTuple_SetItem(args, i, arg);
	}

	PyObject *res = PyObject_CallObject(func, args);
	Py_DECREF(args);

	if (PyTuple_Check(res)) {
		int num_results = PyTuple_Size(res);
		for (int i = 0; i < num_results; i++) {
			qamar_python_to_lua(L, PyTuple_GetItem(res, i));
		}
		Py_DECREF(res);
		PyGILState_Release(gstate);
		return num_results;
	} else if (Py_IsNone(res)) {
		Py_DECREF(res);
		PyGILState_Release(gstate);
		return 0;
	} else {
		qamar_python_to_lua(L, res);
		Py_DECREF(res);
		PyGILState_Release(gstate);
		return 1;
	}
}

int
qamar_gc_pyfunc(lua_State *L) {
	PyObject *func = * (PyObject**)lua_touserdata(L, 1);
	PyGILState_STATE gstate = PyGILState_Ensure();
	if (func) Py_DECREF(func);
	PyGILState_Release(gstate);
	return 0;
}