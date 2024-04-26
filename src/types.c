#include "types.h"
#include "interpreter.h"

PyObject*
lua_stack_to_pyobj(lua_State *L, int index, int type) {
	if (type == -1) {
		type = lua_type(L, index);
	}
	switch (type)
	{
		case LUA_TNIL:
			return Py_None;
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
				PyObject *key = lua_stack_to_pyobj(L, -2, -1);
				PyObject *value = lua_stack_to_pyobj(L, -1, -1);
				PyDict_SetItem(dict, key, value);
				lua_pop(L, 1);
			}
			//lua_pop(L, 1);
			return dict;
		}
		case LUA_TFUNCTION: {

		}
		default:
			assert("Not implemented yet!");
			return NULL;
	}
}

int
qamar_lua_func_init() {

	return 0;
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
	.tp_flags = Py_TPFLAGS_DEFAULT
};