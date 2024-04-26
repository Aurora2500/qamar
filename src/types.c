#include "types.h"

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
			PyObject *obj = PyUnicode_FromString(lua_tostring(L, index));
			lua_pop(L, 1);
			return obj;

		}
		case LUA_TTABLE: {
			PyObject *dict = PyDict_New();
			lua_pushnil(L);
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