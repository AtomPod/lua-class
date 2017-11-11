#ifndef __UTILS_HPP_
#define __UTILS_HPP_ 

template <typename T>
struct remove_all_extend_member {

};

template <typename R , typename C , typename... Args>
struct remove_all_extend_member<R (C::*)(Args...)> {
  typedef R (C::*type)(Args...);
};

template <typename R , typename C , typename... Args>
struct remove_all_extend_member<R (C::*)(Args...) const> {
  typedef R (C::*type)(Args...);
};

template <typename R , typename C , typename... Args>
struct remove_all_extend_member<R (C::*)(Args...) const volatile> {
  typedef R (C::*type)(Args...);
};

template <typename CFUNC>
struct FunctionType {
	typedef CFUNC f_type;
};

#endif		