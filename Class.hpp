#ifndef __LUA_CLASS_HPP_
#define __LUA_CLASS_HPP_ 

#include "state.h"
#include "converter.hpp"
#include <vector>

namespace LUA_NAMESPACE
{
	class ClassProxyImpl : public State
	{
	public:
		ClassProxyImpl(lua_State *vm , const char *name);
		virtual ~ClassProxyImpl();
	protected:
		const char *getMetaName() const { return _name; };
		void createMetatable(const function_t &conct, 
							const function_t *gc);
		void inheritMetatable(const char *name);
		void addMethodImpl(const char *memname ,
							const function_t &proxyfn , 
							void *mempoint , bool mem_t);
	private:
 		const char *_name;
 	private:
 		static const char *_metaname;
	};

	class MethodSet
	{
	public:
		MethodSet() {}
		void add(void *method) {_methods.push_back(method);}
		~MethodSet() {
			for (auto begin = _methods.begin(); begin != _methods.end(); ++begin) {
				delete (char*)*begin;
			}
		};
	protected:
		std::vector<void*> _methods;
	};

	template <typename C>
	class ClassProxy : public ClassProxyImpl
	{
	private:
		static MethodSet methodSet;
	public:
		ClassProxy(lua_State *vm , const char *name , const char *base)
				: ClassProxyImpl(vm , name) , _base(base){
			Converter<C>::metaname = name;		
		}
		~ClassProxy() {};
	public:
		template <typename... Args>
		ClassProxy &construct() {
			function_t cont = Converter<C>::template toConstruct<Args...>();
			function_t gc = Converter<C>::toGC();

			createMetatable(cont , &gc);
			if (_base != nullptr)
				inheritMetatable(_base);
			return *this;
		}

		template <typename Func>
		ClassProxy &addMethod(const char *name , 
				Func method ,
				typename std::enable_if< 
					std::is_member_pointer<Func>::value
					|| std::is_function<
						typename 
						std::remove_pointer<Func>::type>::value 
					, 
					bool>::type = true) {
			function_t luaMethod = Converter<C>::toFunction(method);
			void *mempointer = new Func(method);
			methodSet.add(mempointer);
			addMethodImpl(name , luaMethod , mempointer ,
					std::is_member_pointer<Func>::value);
			return *this;
		}

		template <typename LAMBDA_T , typename LAMBDA>
		ClassProxy &addMethod(const char *name , 
				LAMBDA method ,
				typename std::enable_if< 
					std::is_function<LAMBDA_T>::value &&
					std::is_member_pointer<decltype(&LAMBDA::operator())>::value, 
					bool>::type = true) {
			function_t luaMethod = toLambda<LAMBDA_T>(method);
			void *mempointer = new LAMBDA(method);
			methodSet.add(mempointer);
			addMethodImpl(name , luaMethod , mempointer , false);
			return *this;
		}
	private:
		const char *_base;
	};

	template <typename C>
	MethodSet ClassProxy<C>::methodSet;
}

#endif