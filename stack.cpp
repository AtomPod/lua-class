#include "stack.hpp"
#include "base.h"
#include <cassert>
#include <cmath>

#ifndef CHECK_ERROR_MESSAGE
#define CHECK_ERROR_MESSAGE(state , msg) assert((state)) 
#endif

#ifndef CHECK_VM_VALID
#define CHECK_VM_VALID(vm) CHECK_ERROR_MESSAGE(((vm) != nullptr) , "_vm assert failed")
#endif

#ifdef lua_Integer
#define LUA_SUPPORT_INTEGER
#endif

#ifndef _RESULT_FROM_CONVERT_METHOD
#define _RESULT_FROM_CONVERT_METHOD(pred , nfunc , cfunc) (pred) ? (cfunc) : (nfunc)
#endif

namespace LUA_NAMESPACE
{
Nil Stack::_nil;

Stack::Stack(lua_State *vm) : State(vm) {

}

Stack::~Stack() {

}

void Stack::resize(const size_t &sz) {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm , "Stack::resize: _vm assert failed");
	lua_settop(lvm, sz);
}

void Stack::clear() {
	resize(0);
}

void Stack::remove(const index_t &idx) {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm && (std::abs(idx) > size()) 
						, "Stack::size: _vm assert failed");
	lua_remove(lvm, idx);
}

void Stack::replace(const index_t &idx) {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm && (std::abs(idx) > size()) 
						, "Stack::replace: _vm assert failed");
	lua_replace(lvm, idx);
}

const char *Stack::typeName(const index_t &idx) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm && (std::abs(idx) <= size()) 
						, "Stack::typeName: _vm assert failed");
	return luaL_typename(lvm, idx);
}

void Stack::getGlobal(const char *name) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm && (name != nullptr) 
						, "Stack::typeName: _vm assert failed");
	lua_getglobal(lvm, name);
}

// lua_State *Stack::getVMState() const {
// 	return _vm;
// }

size_t Stack::size() const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(nullptr != lvm , "Stack::size: _vm assert failed");
	return lua_gettop(lvm);
}

void *Stack::toUserData(const index_t &sidx , const char *checkname) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr && std::abs(sidx) <= size() , 
						"Stack::toUserData: assert failed");
	return _RESULT_FROM_CONVERT_METHOD(checkname != nullptr , lua_touserdata(lvm, sidx) , luaL_checkudata(lvm, sidx, checkname));
}

float Stack::toFloat(const index_t &sidx , const bool &checked) const {
	return toDouble(sidx , checked);
}

double Stack::toDouble(const index_t &sidx , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr && std::abs(sidx) <= size() , 
						"Stack::toDouble: assert failed");
	//return lua_tonumber(lvm, sidx);
	return _RESULT_FROM_CONVERT_METHOD(checked , lua_tonumber(lvm, sidx) , luaL_checknumber(lvm, sidx));
}

Stack::uintmax_t Stack::toUInt(const index_t &sidx , const bool &checked) const {
	return toInt(sidx , checked);
}

Stack::intmax_t Stack::toInt(const index_t &sidx , const bool &checked) const {
	#ifdef LUA_SUPPORT_INTEGER
		lua_State *lvm = getVMState();
		CHECK_ERROR_MESSAGE(lvm != nullptr && std::abs(sidx) <= size() , "Stack::toInt: assert failed");
		//return lua_tointeger(lvm, sidx);
		return _RESULT_FROM_CONVERT_METHOD(checked , lua_tointeger(lvm, sidx) , luaL_checkinteger(lvm, sidx));
	#else
		return toDouble(sidx , checked);
	#endif
}

void *Stack::toCPointer(const index_t &sidx , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr  && std::abs(sidx) <= size() , 
						"Stack::toCPointer: assert failed");
	//return lua_touserdata(lvm, sidx);

	return _RESULT_FROM_CONVERT_METHOD(checked , lua_touserdata(lvm, sidx) , (luaL_checktype(lvm, sidx, LUA_TLIGHTUSERDATA) , lua_touserdata(lvm, sidx)));
}

const char *Stack::toCString(const index_t &sidx , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr  && std::abs(sidx) <= size() , 
						"Stack::toCString: assert failed");

	//return lua_tostring(lvm, sidx);
	return _RESULT_FROM_CONVERT_METHOD(checked , lua_tostring(lvm, sidx) , luaL_checkstring(lvm, sidx));
}

const char *Stack::tolCString(const index_t &sidx , size_t &len , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr  && std::abs(sidx) <= size() , 
						"Stack::toCString: assert failed");
	//return lua_tolstring(lvm, sidx, &len);
	return _RESULT_FROM_CONVERT_METHOD(checked, lua_tolstring(lvm, sidx, &len), luaL_checklstring(lvm, sidx, &len));
}

bool Stack::toBoolean(const index_t &sidx  , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr  && std::abs(sidx) <= size() , 
						"Stack::toBoolean: assert failed");

	//return lua_toboolean(lvm, sidx);
	return _RESULT_FROM_CONVERT_METHOD(checked, lua_toboolean(lvm, sidx), (luaL_checktype(lvm, sidx, LUA_TBOOLEAN) , lua_toboolean(lvm, sidx)));
}

const Nil &Stack::toNil(const index_t &sidx , const bool &checked) const {
	lua_State *lvm = getVMState();
	CHECK_ERROR_MESSAGE(lvm != nullptr  && std::abs(sidx) <= size() , 
						"Stack::toNil: assert failed");
	return _RESULT_FROM_CONVERT_METHOD(checked, _nil, (luaL_checktype(lvm, sidx, LUA_TNIL) , _nil));
}

void Stack::fromFloat(const float &value) const {
	fromDouble(value);
}

void Stack::fromDouble(const double &value) const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushnumber(lvm, value);
}

void Stack::fromUInt(const Stack::uintmax_t &value) const {
	fromInt(value);
}

void Stack::fromInt(const Stack::intmax_t &value) const {
	#ifdef LUA_SUPPORT_INTEGER
		lua_State *lvm = getVMState();
		CHECK_VM_VALID(lvm);
		lua_pushinteger(lvm, value);
	#else
		fromDouble(value);
	#endif
}

void Stack::fromCPointer(const void *value) const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushlightuserdata(lvm, const_cast<void *>(value));
}

void Stack::fromCString(const char *value) const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushstring(lvm, value);
}

void Stack::fromBoolean(const bool &value) const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushboolean(lvm, value);
}

void Stack::fromlCString(const char *value, const size_t &len) const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushlstring(lvm, value, len);
}

void Stack::fromNil() const {
	lua_State *lvm = getVMState();
	CHECK_VM_VALID(lvm);
	lua_pushnil(lvm);
}

	
}
