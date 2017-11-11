#ifndef __LUA_CONVERTER_HPP_
#define __LUA_CONVERTER_HPP_ 

#include "stack.hpp"
#include "luaInc.h"
#include <stdio.h>
#include <string>
#include "utils.hpp"

namespace LUA_NAMESPACE
{
	typedef int(*function_t)(lua_State *);

	template <size_t... N>
	struct fake_index_sequence {};

	template <size_t N , size_t... M>
	struct make_fake_index_sequence : make_fake_index_sequence<N - 1 , N , M...> {};

	template <size_t... M>
	struct make_fake_index_sequence<0, M...> : fake_index_sequence<M...> {};


	// template <typename LAMBDA>
	// struct fake_is_lambda :
	// public std::is_member_pointer<decltype(&LAMBDA::operator())> {};


	//staic or c function
	template <typename CFUNC>
	struct NormalInvoker {};

	template <typename R , typename... Args>
	struct NormalInvoker<R(Args...)>{

		typedef typename FunctionType<R(Args...)>::f_type f_type; 

		template <size_t... N>
		static int callImpl(lua_State *vm , size_t offset ,f_type mem , 
						const fake_index_sequence<N...> &empty) {
			LUA_NAMESPACE::Stack stack(vm);
			R ret = mem(stack.to<Args>(N + offset)...);
			stack.push(ret);
			return 1;
		}

		inline static int call(lua_State *vm , size_t offset ,f_type mem) {
			return callImpl(vm, offset, mem, make_fake_index_sequence<sizeof...(Args)>());
		}
	};

	template <typename... Args>
	struct NormalInvoker<void(Args...)> {
		typedef typename FunctionType<void(Args...)>::f_type f_type; 

		template <size_t... N>
		static int callImpl(lua_State *vm , size_t offset ,f_type mem , 
						const fake_index_sequence<N...> &empty) {
			LUA_NAMESPACE::Stack stack(vm);
			mem(stack.to<Args>(N + offset)...);
			return 0;
		}

		inline static int call(lua_State *vm , size_t offset ,
						f_type mem) {
			return callImpl(vm, offset, mem, make_fake_index_sequence<sizeof...(Args)>());
		}
	};

	template <typename CFUNC>
	static function_t toCFunction(CFUNC mem , 
							typename std::enable_if<
								std::is_function<typename std::remove_pointer<CFUNC>::type>::value
								, bool
							>::type = true) {
		return [](lua_State *lvm) -> int {
			size_t fnidx = lua_upvalueindex(1);
			CFUNC *mem_ptr = (CFUNC*)lua_touserdata(lvm, fnidx);
			CFUNC mem = *mem_ptr;
			return NormalInvoker<
					typename std::remove_pointer<CFUNC>::type
				   >::call(lvm , 0 , mem);
		};
	}




	//lambda function
	template <typename LAMBDA_T , typename LAMBDA>
	struct LambdaInvoker {};

	template <typename LAMBDA , typename R , typename... Args>
	struct LambdaInvoker<R(Args...) , LAMBDA> {
		//typedef typename FunctionType<void(Args...)>::f_type f_type; 

		template <size_t... N>
		static int callImpl(lua_State *vm , size_t offset ,LAMBDA mem , 
						const fake_index_sequence<N...> &empty) {
			LUA_NAMESPACE::Stack stack(vm);
			R ret = mem(stack.to<Args>(N + offset)...);
			stack.push(ret);
			return 1;
		}

		inline static int call(lua_State *vm , size_t offset ,
						LAMBDA mem) {
			return callImpl(vm, offset, mem, make_fake_index_sequence<sizeof...(Args)>());
		}
	};

	template <typename LAMBDA , typename... Args>
	struct LambdaInvoker<void(Args...) , LAMBDA> {
		//typedef typename FunctionType<void(Args...)>::f_type f_type; 

		template <size_t... N>
		static int callImpl(lua_State *vm , size_t offset ,LAMBDA mem , 
						const fake_index_sequence<N...> &empty) {
			LUA_NAMESPACE::Stack stack(vm);
			mem(stack.to<Args>(N + offset)...);
			return 0;
		}

		inline static int call(lua_State *vm , size_t offset ,
						LAMBDA mem) {
			return callImpl(vm, offset, mem, make_fake_index_sequence<sizeof...(Args)>());
		}
	};

	template <typename LAMBDA_T , typename LAMBDA>
	static function_t toLambda(LAMBDA lfunc , 
								typename std::enable_if<
									std::is_member_pointer<decltype(&LAMBDA::operator())>::value ,
									bool
								>::type = true) {
		return [](lua_State *lvm) -> int {
			size_t fnidx = lua_upvalueindex(1);
			LAMBDA *mem_ptr = (LAMBDA*)lua_touserdata(lvm, fnidx);
			LAMBDA mem = *mem_ptr;
			return LambdaInvoker<
					LAMBDA_T , LAMBDA
				   >::call(lvm , 0 , mem);
		};	
	}







	template <typename C>
	class Converter
	{
	public:
		static std::string metaname;
	public:
		template <typename CPP_MEM , typename R , typename... Args>
		struct Invoker {
			static const int ret_n = 1;
			template <size_t... N>
			static int call(lua_State *vm , 
							size_t offset , 
							C *const self , 
							CPP_MEM mem ,
							R (C::*args_empty)(Args... args) , 
							const fake_index_sequence<N...> &empty) {
				LUA_NAMESPACE::Stack stack(vm);
				R ret = (self->*mem)(stack.to<Args>(N + offset)...);
				stack.push(ret);
				return 1;
			}
		};

		template <typename CPP_MEM , typename... Args>
		struct Invoker<CPP_MEM , void ,  Args...> {
			static const int ret_n = 1;
			template <size_t... N>
			static int call(lua_State *vm , 
							size_t offset , 
							C *const self , 
							CPP_MEM mem ,
							void (C::*args_empty)(Args... args) , 
							const fake_index_sequence<N...> &empty) {
				LUA_NAMESPACE::Stack stack(vm);
				(self->*mem)(stack.to<Args>(N + offset)...);
				return 0;
			}
		};

		template <typename CPP_MEM , typename R , typename... Args>
		inline static int invoke(lua_State *vm , 
							size_t offset , 
							C *const self ,
							CPP_MEM mem , 
							R (C::*args_empty)(Args... args)) {
			return Invoker<CPP_MEM , R , Args...>::call(
				vm , 
				offset , 
				self , 
				mem , 
				args_empty ,
				make_fake_index_sequence<sizeof...(Args)>());
		}

		//to class member
		template <typename CPP_MEM>
		static function_t toFunction(CPP_MEM mem ,
					typename std::enable_if<
                     std::is_member_pointer<CPP_MEM>::value ,
                     bool
					>::type = true
					) {
			return [](lua_State *vm) -> int {
				size_t fnidx = lua_upvalueindex(1);
				CPP_MEM *mem_ptr = (CPP_MEM*)lua_touserdata(vm, fnidx);
				CPP_MEM mem = *mem_ptr;			
				luaL_checktype(vm, 1, LUA_TUSERDATA);
				C** object_ptr = (C**)lua_touserdata(vm, 1);
				C* object = *object_ptr;
				return Converter::invoke(
					vm , 
					1 , 
					object , 
					mem , 
					typename remove_all_extend_member<CPP_MEM>::type()
				);
			};
		}

		//to static or c function
		template <typename CFUNC>
		inline static function_t toFunction(CFUNC value ,
							typename std::enable_if<
							 	std::is_function<
							 		typename std::remove_pointer<CFUNC>::type
							 	>::value,
								bool
							>::type = true) {
			return toCFunction(value);
		}


		template <typename... Args>
		struct CInvoker
		{
			template <size_t... N>
			static int call(lua_State *vm , size_t offset , 
	                           C** object_ptr ,
							   const fake_index_sequence<N...> &empty) {
				LUA_NAMESPACE::Stack stack(vm);
				*object_ptr = new C(stack.to<Args>(N + offset)...);
				return 1;
			}
		};
		

		template <typename... Args>
		static function_t toConstruct() {
			return [](lua_State *lvm) -> int {
				const char *tname = Converter::metaname.c_str();
				C** object_ptr = (C**)lua_newuserdata(lvm, sizeof(C**));
				luaL_getmetatable(lvm, tname);
				lua_setmetatable(lvm, -2);

				return Converter::CInvoker<Args...>::call(lvm , 0 , 
					object_ptr , make_fake_index_sequence<sizeof...(Args)>());
			};
		}

		static function_t toGC() {
			return [](lua_State *lvm) -> int {
				luaL_checktype(lvm, 1, LUA_TUSERDATA);
				C** object_ptr = (C**)lua_touserdata(lvm, 1);
				delete *object_ptr;
			};
		}
	};

	template <typename C>
	std::string Converter<C>::metaname = "object";
}

#endif