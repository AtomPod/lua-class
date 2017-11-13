#include "class.hpp"

namespace LUA_NAMESPACE
{
	const char *ClassProxyImpl::_metaname = "lua-meta-name";
	ClassProxyImpl::ClassProxyImpl(lua_State *vm , const char *cname ) 
					: State(vm) , _name(cname) {

	}

	ClassProxyImpl::~ClassProxyImpl() {

	}

	void ClassProxyImpl::createMetatable(
						const function_t &conct, 
						const function_t *gc) {
		lua_State *lvm = getVMState();
		int btop = lua_gettop(lvm);
		luaL_newmetatable(lvm, _name);

		//set gc func
		if (gc != nullptr) {
			lua_pushcfunction(lvm, *gc);
			lua_setfield(lvm, -2, "__gc");
		}	

		lua_newtable(lvm);
		lua_pushvalue(lvm, -1); 
		lua_setfield(lvm, -3, _metaname); 
		lua_setfield(lvm, -2, "__index"); 

		lua_newtable(lvm);
		lua_pushcfunction(lvm, conct);
		lua_setfield(lvm, -2, "new");
		lua_setglobal(lvm, _name);
		lua_settop(lvm, btop);
	}

	void ClassProxyImpl::inheritMetatable(const char *name) {
		lua_State *lvm = getVMState();
		int btop = lua_gettop(lvm);
		luaL_getmetatable(lvm, _name); 
		lua_getfield(lvm, -1, _metaname); 
		
		luaL_getmetatable(lvm, name);
		lua_setmetatable(lvm, -2);

		lua_settop(lvm, btop);
	}

	void ClassProxyImpl::addMethodImpl(const char *memname ,
							const function_t &proxyfn , 
							void *mempoint , bool mem_t) {
		lua_State *lvm = getVMState();
		int btop = lua_gettop(lvm);

		if (mem_t) {
			luaL_getmetatable(lvm , _name);
			lua_getfield(lvm, -1, _metaname);
		} else {
			lua_getglobal(lvm, _name);
		}
		
		lua_pushlightuserdata(lvm, mempoint);
		lua_pushcclosure(lvm, proxyfn, 1);
		lua_setfield(lvm, -2, memname);
		lua_settop(lvm, btop);
	}
}