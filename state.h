#ifndef __LUA_STATE_H_
#define __LUA_STATE_H_ 
#include "base.h"
#include <cstddef>
#include <cstdint>

#define LUA_NAMESPACE lua

namespace LUA_NAMESPACE
{
	class State
	{
	public:
		typedef size_t uintmax_t;
		typedef ptrdiff_t intmax_t;
		typedef ptrdiff_t index_t;
	public:
		State(lua_State *vm) : _vm(vm) {} ;
		virtual ~State() {};

		//LuaVM *getVM() const;
	  	lua_State *getVMState() const { return _vm; };
	private:
		lua_State *_vm;
	};
}

#endif