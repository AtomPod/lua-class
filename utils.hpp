#ifndef __UTILS_HPP_
#define __UTILS_HPP_ 

template <typename T>
struct remove_all_member_modifier {

};

template <typename R , typename C , typename... Args>
struct remove_all_member_modifier<R (C::*)(Args...)> {
  typedef R (C::*type)(Args...);
};

template <typename R , typename C , typename... Args>
struct remove_all_member_modifier<R (C::*)(Args...) const> {
  typedef R (C::*type)(Args...);
};

template <typename R , typename C , typename... Args>
struct remove_all_member_modifier<R (C::*)(Args...) const volatile> {
  typedef R (C::*type)(Args...);
};

template <typename CFUNC>
struct function_type {
	typedef CFUNC f_type;
};

#endif		