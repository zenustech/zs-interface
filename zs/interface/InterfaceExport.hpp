#pragma once

// https://abseil.io/docs/cpp/platforms/macros

#ifndef ZS_INTERFACE_EXPORT
#  if defined(_MSC_VER) || defined(__CYGWIN__)
#    ifdef Zs_Interface_EXPORT
#      define ZS_INTERFACE_EXPORT __declspec(dllexport)
#    else
#      define ZS_INTERFACE_EXPORT __declspec(dllimport)
#    endif
#  elif defined(__clang__) || defined(__GNUC__)
#    define ZS_INTERFACE_EXPORT __attribute__((visibility("default")))
#  endif
#endif

#if defined(_WIN64)
#  define ZS_EXPORT __declspec(dllexport)
#  define ZS_IMPORT __declspec(dllimport)

#elif defined(__GNUC__)
#  define ZS_EXPORT __attribute__((visibility("default")))
#  define ZS_IMPORT __attribute__((visibility("default")))

#elif defined(__clang__)
#  define ZS_EXPORT __attribute__((visibility("default")))
#  define ZS_IMPORT __attribute__((visibility("default")))

#elif defined(ZS_JIT_MODE)
#  define ZS_EXPORT
#  define ZS_IMPORT

#else
#  error "unknown compiler!"
#endif

// @image html zensim_logo.png
/**

 * \~english @defgroup handle_types value/ handle types
	@brief value/handle-semantics types, often used for parameters and expression
 * \~chinese @defgroup handle_types 值/句柄类型
	@brief 值/句柄语义的API类型，常用于参数传递和表达式

 * \~english @defgroup valctor_apis value/ handle creation C functions
	@brief value/handle-semantics type object creation C functions
 * \~chinese @defgroup valctor_apis 值/句柄创建C函数
	@brief 值/句柄创建相关的C API函数

 * \~english @defgroup obj_types RAII object types
	@brief RAII object types, often used for data members and local variables
 * \~chinese @defgroup obj_types 对象类型
	@brief 对象语义的API类型，具备生命周期，常用作数据成员或局部变量

 * \~english @defgroup objctor_apis object creation C functions
	@brief Python object creation C functions
	@note these C-APISs handles Python Exception internally
	@note returns a new reference of a python object in a handle (i.e. ZsValue) by default, make sure the reference is decremented on destruction
 * \~chinese @defgroup objctor_apis 对象创建C函数
	@brief Python对象创建相关的C API函数
	@note 这些C-API函数会在内部处理python异常
	@note 默认返回一个python对象的新引用的句柄（即ZsValue），析构时需手动减去一个引用计数
 */

/**
 * \~english @defgroup init_steps library initialization

	@brief library initialization related API functions

 * \~chinese @defgroup init_steps 库初始化函数

	@brief 库初始化相关API函数
 */