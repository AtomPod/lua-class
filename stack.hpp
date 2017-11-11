#ifndef __LUA_STACK_HPP_
#define __LUA_STACK_HPP_
#include "state.h"

namespace LUA_NAMESPACE
{
	struct Nil
	{};

	class Stack : public State {
	public:
		typedef size_t uintmax_t;
		typedef ptrdiff_t intmax_t;
		typedef ptrdiff_t index_t;

		template <typename T>
		struct ConvertTo {
			static T invoke(const Stack & , const index_t & , const bool &);
		};

		template <typename T>
		struct ConvertFrom {
			static void invoke(const Stack & , const T &);
		};
	private:
	  /* data */
	  //lua_State *_vm;
	  static Nil _nil;
	public:
	  Stack (lua_State *vm);
	  /*virtual*/ ~Stack ();
	public:
	  void resize(const size_t &);
	  size_t size() const;
	  void clear();

	  void remove(const index_t &);
	  void replace(const index_t &);

	  const char *typeName(const index_t &) const;

	  void getGlobal(const char *name) const;
	  //LuaVM *getVM() const;
	  //lua_State *getVMState() const;
	public:
	  template <typename T>
	  T to(const index_t & , const bool & = false) const;

	  template <typename T>
	  T to(const char *name , const bool & = false) const;

	  template <typename T>
	  void push(const T &) const;

	  void *toUserData(const index_t &sidx , const char *checkname) const;
	public:
	  float toFloat(const index_t & , const bool &) const;
	  double toDouble(const index_t & , const bool &) const;

	  Stack::uintmax_t toUInt(const index_t & , const bool &) const;
	  Stack::intmax_t toInt(const index_t & , const bool &) const;

	  void *toCPointer(const index_t & , const bool &) const;
	  const char *toCString(const index_t & , const bool &) const;
	  const char *tolCString(const index_t & , size_t &l , const bool &) const;

	  bool toBoolean(const index_t & , const bool &) const;

	  const Nil &toNil(const index_t & , const bool &) const;
	public:
	  void fromFloat(const float &) const;
	  void fromDouble(const double &) const;
	  void fromUInt(const Stack::uintmax_t &) const;
	  void fromInt(const Stack::intmax_t &) const;
	  void fromCPointer(const void *) const;
	  void fromCString(const char *) const;
	  void fromBoolean(const bool &) const;
	  void fromlCString(const char * , const size_t &) const;
	  void fromNil() const;
	};	

	template <typename T>
	T Stack::to(const index_t &sidx , const bool &checked) const {
		return Stack::ConvertTo<T>::invoke(*this , sidx , checked);
	}

	template <typename T>
	 T Stack::to(const char *name , const bool &checked) const {
	 	getGlobal(name);
	 	return Stack::ConvertTo<T>::invoke(*this , -1 , checked);
	 }

	template <typename T>
	void Stack::push(const T &value) const {
		Stack::ConvertFrom<T>::invoke(*this , value);
	}

	template <typename T>
	T Stack::ConvertTo<T>::invoke(const lua::Stack &, const index_t & , const bool &) {
		throw "type is not support";
	}

	#ifndef ConverterStruct
	#define ConverterStruct(type , fn) \
	        template <> \
			struct Stack::ConvertTo<type> { \
				static type invoke(const Stack &s, const index_t &sidx, const bool &checked) { \
					return fn; \
				} \
			}
	#endif

	//ConverterStruct(Stack::uintmax_t, s.toUInt(sidx));
	//ConverterStruct(Stack::intmax_t, s.toInt(sidx));

	ConverterStruct(signed int , s.toInt(sidx , checked));
	ConverterStruct(unsigned int  , s.toUInt(sidx , checked));

	ConverterStruct(signed long , s.toInt(sidx , checked));
	ConverterStruct(unsigned long , s.toUInt(sidx , checked));

	ConverterStruct(signed long long, s.toInt(sidx , checked));
	ConverterStruct(unsigned long long , s.toUInt(sidx , checked));

	ConverterStruct(signed char, s.toInt(sidx , checked));
	ConverterStruct(unsigned char, s.toUInt(sidx , checked));

	ConverterStruct(signed short, s.toInt(sidx , checked));
	ConverterStruct(unsigned short, s.toUInt(sidx , checked));

	ConverterStruct(const char*, s.toCString(sidx , checked));

	ConverterStruct(double, s.toDouble(sidx , checked));
	ConverterStruct(float, s.toFloat(sidx , checked));

	ConverterStruct(bool, s.toBoolean(sidx , checked));

	ConverterStruct(Nil , s.toNil(sidx, checked));

	#undef ConverterStruct

	template <typename T> 
	struct Stack::ConvertTo<T*> { 
		static T *invoke(const Stack &s, const index_t &sidx , const bool &checked) { 
			return reinterpret_cast<T*>(s.toCPointer(sidx , checked)); 
		} 
	};



	template <typename T>
	void Stack::ConvertFrom<T>::invoke(const Stack & , const T &) {
		throw "type is not support";
	}


	#ifndef ConvertFromStruct
	#define ConvertFromStruct(type , fn) \
	        template <> \
			struct Stack::ConvertFrom<type> { \
				static void invoke(const Stack &s, const type &value) { \
					return fn; \
				} \
			}
	#endif

	ConvertFromStruct( int , s.fromInt(value));
	ConvertFromStruct(unsigned int , s.fromUInt(value));

	ConvertFromStruct( long , s.fromInt(value));
	ConvertFromStruct(unsigned long , s.fromUInt(value));

	ConvertFromStruct( long long, s.fromInt(value));
	ConvertFromStruct(unsigned long long , s.fromUInt(value));

	ConvertFromStruct( char, s.fromInt(value));
	ConvertFromStruct(unsigned char, s.fromUInt(value));

	ConvertFromStruct( short, s.fromInt(value));
	ConvertFromStruct(unsigned short, s.fromUInt(value));

	ConvertFromStruct(double, s.fromDouble(value));
	ConvertFromStruct(float, s.fromFloat(value));

	ConvertFromStruct(bool , s.fromBoolean(value));
	#undef ConvertFromStruct

	template <> 
	struct Stack::ConvertFrom<const char *> { 
		static void invoke(const Stack &s, const char *value) { 
			return s.fromCString(value); 
		} 
	};

	template <typename T> 
	struct Stack::ConvertFrom<T*> { 
		static void invoke(const Stack &s, const T *value) { 
			return s.fromCPointer(value); 
		} 
	};

	template <size_t N>
	struct Stack::ConvertFrom<char[N]> {
		static void invoke(const Stack &s, const char (&value)[N]) { 
			return s.fromlCString(value, N);
		}
	};

	template <> 
	struct Stack::ConvertFrom<std::nullptr_t> { 
		static void invoke(const Stack &s, const std::nullptr_t &value) { 
			return s.fromNil(); 
		} 
	};


}


#endif
