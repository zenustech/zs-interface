#pragma once
#include "value_type/ValueInterface.hpp"

struct ZsObjectConcept {
  virtual ~ZsObjectConcept() = default;
  // modify (zs_var_type_none indicates failure)
  virtual void copyAssign(ZsValue value) = 0;
  virtual void moveAssign(ZsValue value) = 0;
  // query
  virtual bool isObject() noexcept = 0;
  virtual const char *asCharString() noexcept { return nullptr; }
  virtual void *asString() noexcept { return nullptr; }
  virtual void *asSet() noexcept { return nullptr; }
  virtual void *asList() noexcept { return nullptr; }
  virtual void *asMap() noexcept { return nullptr; }
  virtual ZsValue getValue() noexcept = 0;
  virtual zs_var_type_ getType() noexcept { return zs_var_type_none; }
};

struct Deletable {
  virtual void deinit() = 0;
};

/**
  @addtogroup obj_types
  @{
  */
/**
    \~english @brief C++ wrapper class for ZsValue

    A RAII C++ wrapper class that holds a ZsValue, which stores a python object pointer or a
   value-semantic type variable. On destruction, Py_DECREF is called on the python object handle if
   an object is being stored, otherwise do nothing.

   **Although ZsVar is also a RAII class as PyVar, its copy behavior is essentially a deepcopy,
   which is fundamentally different from PyVar's inc/dec of reference counting.**

    \~chinese @brief ZsValue的C++包装类型

    ZsValue的C++包装类。析构时，若ZsValue存储的是Python对象指针，会对其指针调用Py_DECREF，否则不会有其他行为。

    **虽然ZsVar与PyVar一样是RAII类型，但其拷贝行为是深拷贝，与PyVar增减引用计数的做法有本质区别。**
 */
struct ZsVar {
  ZsVar() noexcept = default;

  /**
    @brief Takes the ownership of a ZsValue

    Like wrapping a raw pointer in std::unique_ptr
    */
  ZsVar(ZsValuePort v) noexcept : _var{v} {}

  /// @note g_zs_variable_apis.initObj has not been formalized yet!

  /// @brief Construct ZsValue with a string literal in place
  ZsVar(const char *cstr) noexcept : _var{cstr} {}
  /// @brief Construct ZsValue with a python object handle in place
  /// @note prefer ZsObject over ZsValue
  ZsVar(void *obj) noexcept : _var{ZsObject{obj}} {}

  /// @brief If ZsValue is holding a python object, Py_XDECREF is called upon it. Otherwise do
  /// nothing.
  ~ZsVar() { g_zs_variable_apis.deinit(&_var); }

  /// @brief move-assigned with another ZsVar's ZsValue
  /// @param o the other moved-from ZsVar
  /// @note leave \a o in NONE state
  ZsVar(ZsVar &&o) noexcept {
    _var = o._var;
    o._var._v.obj = nullptr;
    o._var._idx = zs_var_type_none;
  }
  /// @brief copy-assigned with another ZsVar's ZsValue
  /// @param o the other copied-from ZsVar
  ZsVar(const ZsVar &o) { _var = static_cast<ZsValue>(g_zs_variable_apis.clone(o._var)); }

  /// @brief destructs the previously-stored ZsValue and move assigned with another ZsVar's ZsValue
  /// @param o the other moved-from ZsVar
  /// @note leave \a o in NONE state
  /// @note use swap-idiom to guarantee replacement only after successful move creation
  ZsVar &operator=(ZsVar &&o) noexcept {
    ZsVar tmp{static_cast<ZsVar &&>(o)};
    swap(*this, tmp);
    return *this;
  }
  /// @brief destructs the previously-stored ZsValue and copy assigned with another ZsVar's ZsValue
  /// @param o the other copied-from ZsVar
  /// @note use swap-idiom to guarantee replacement only after successful copy creation
  ZsVar &operator=(const ZsVar &o) {
    ZsVar tmp{static_cast<const ZsVar &>(o)};
    swap(*this, tmp);
    return *this;
  }

  /// @brief swap ZsValues between two ZsVar's
  friend void swap(ZsVar &l, ZsVar &r) noexcept {
    ZsValue tmp{l._var};
    l._var = r._var;
    r._var = tmp;
  }
  /// @brief share with another ZsVar's ZsValue
  /// @param o the ZsValue to be shared
  /// @note does not affect the reference count of \a o
  [[maybe_unused]] ZsVar &share(ZsValue o) {
    ZsVar tmp{g_zs_variable_apis.share(o)};
    swap(*this, tmp);
    return *this;
  }
  /**
    @brief Releases the ownership of the stored ZsValue handle

     Like releasing a raw pointer from std::unique_ptr
   */
  ZsValue release() noexcept {
    ZsValue ret = _var;
    _var = ZsValue{};
    return ret;
  }

  ///
  /// query
  ///
  /// @brief query the python object type zs_obj_type_
  /// @note if ZsValue is not an object, **zs_obj_type_unknown** is returned
  zs_obj_type_ get_obj_type() const { return zs_get_obj_type(_var); }
  /// @brief query if _var is not **None**
  /// @note different from PyVar
  operator bool() const { return static_cast<bool>(_var); }
  /// @brief retrieve the ZsValue
  constexpr ZsValue getValue() const noexcept { return _var; }
  /// @brief retrieve the ZsValue
  ZsObject getObject() const noexcept { return getValue(); }
  
  constexpr bool isObject() const noexcept { return _var._idx == zs_var_type_object; }
  constexpr bool isValidObject() const noexcept { return isObject() && _var._v.obj != nullptr; }
  constexpr ZsValue &getRef() noexcept { return _var; }
  constexpr const ZsValue &getRef() const noexcept { return _var; }

  constexpr zs_var_type_ getType() const noexcept { return _var._idx; }

  void reflect() const noexcept { g_zs_variable_apis.reflect(_var); }
  long long int refcnt() const noexcept { return g_zs_variable_apis.refcnt(_var); }

  /// rich compare
  bool operator==(ZsValue &o) const { return g_zs_variable_apis.eq(_var, o); }
  friend bool operator==(const ZsVar &l, const ZsVar &r) {
    return g_zs_variable_apis.eq(l._var, r._var);
  }
  friend bool operator!=(const ZsVar &l, const ZsVar &r) {
    return g_zs_variable_apis.ne(l._var, r._var);
  }
  /// @brief identity compare
  /// @note check if two ZsVar is holding the same underlying python object
  bool is(const ZsVar &r) const {
    if (&r == this) return true;
    return g_zs_variable_apis.is(_var, r._var);
  }

  operator ZsValue &() noexcept { return _var; }
  operator const ZsValue &() const noexcept { return _var; }

#define ZS_VAR_DEF_EXPLICIT_CONVERSION(TYPE) \
  explicit operator TYPE() const noexcept { return static_cast<TYPE>(_var); }

  // ZS_VAR_DEF_EXPLICIT_CONVERSION(const char *)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(long long int)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(double)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(int)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(float)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(char)
  ZS_VAR_DEF_EXPLICIT_CONVERSION(void *)

#undef ZS_VAR_DEF_EXPLICIT_CONVERSION

  ZsValue _var{};
};
/**
  @}
  */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
