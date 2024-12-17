#pragma once
#include "interface/InterfaceExport.hpp"

using sint_t = long long int;

#ifdef __cplusplus
extern "C" {
#endif

struct PyVar;

/**
  @brief These are the most frequently-used official python object types (long, float, tuple, list,
  set, dict, etc.), other types including user-defined type extensions are recognized as
  **zs_obj_type_custom**
 */
enum zs_obj_type_ : unsigned {
  zs_obj_type_bytes,
  zs_obj_type_bytearray,
  zs_obj_type_string,
  zs_obj_type_tuple,
  zs_obj_type_long,
  zs_obj_type_float,
  zs_obj_type_list,
  zs_obj_type_set,
  zs_obj_type_dict,
  zs_obj_type_module,
  zs_obj_type_num_inherent,
  zs_obj_type_custom,
  zs_obj_type_unknown = ~((unsigned)0)  // not an obj
};
enum zs_obj_feature_ : unsigned {  // mainly used for custom object types
  zs_obj_feature_none = 0,
  zs_obj_feature_iterable = (unsigned)1 << 0,

  zs_obj_feature_num
};
/**
  @brief These are the value categories for ZsValue
 */
enum zs_var_type_ : unsigned {
  zs_var_type_object = 0,
  zs_var_type_cstr,  // const char*
  zs_var_type_i64,
  zs_var_type_f64,
  zs_var_type_i32,
  zs_var_type_f32,
  zs_var_type_i8,
  zs_var_type_num,
  zs_var_type_none = ~((unsigned)0)  // empty state
};

struct ZsVarOps;
struct ZsValueOps;
ZS_INTERFACE_EXPORT extern ZsVarOps g_zs_variable_apis;
ZS_INTERFACE_EXPORT extern ZsValueOps g_zs_value_apis;

/// @note these value/handle types does not involve any lifetime management
struct ZsObject;
struct ZsModule;
struct ZsBytes;
struct ZsString;
struct ZsTuple;
struct ZsList;
struct ZsSet;
struct ZsDict;

/// @note _idx of zs_var_type_object with an empty obj (i.e. nullptr) is
/// INVALID!
struct ZsValuePort {
  union {
    void *obj;  // underlying type PyObject*, nullptr means valueless
    const char *cstr;
    long long int i64;
    double f64;
    int i32;
    float f32;
    char i8;
  } _v;
  zs_var_type_ _idx;
};

/**
  @addtogroup handle_types
  @{
  */
/**
  @brief the core value-semantic type that represents any data/ class.

  @note
    Currently, besides the empty value type (_idx == zs_var_type_none), there are these inherent C++
  value types: string literal (const char*), integral (long long int, int, char) and float (f64,
  f32). \n The only other value type is a handle (void*) to an object (_idx == zs_var_type_object),
  which is essentially a python object.
 */
struct ZS_INTERFACE_EXPORT ZsValue : ZsValuePort {  // behave like a handle
  /// @brief construct ZsValue from ZsValuePort
  explicit constexpr ZsValue(ZsValuePort val) noexcept : ZsValuePort{val} {}
  /// @brief _idx = zs_var_type_none, and _v.obj = nullptr
  constexpr ZsValue() noexcept : ZsValuePort{} {
    _v.obj = 0;
    _idx = zs_var_type_none;
  }
  ZsValue &operator=(const ZsValuePort &val) = delete;
  ZsValue &operator=(ZsValuePort &&val) = delete;
  /// @brief initialized by a pointer \a obj
  /// @param obj a void pointer (python object)
  ZsValue(void *obj) noexcept {
    _v.obj = obj;
    _idx = obj ? zs_var_type_object : zs_var_type_none;
  }
  /// @brief initialized by a NULL-terminated string literal \a cstr
  /// @param cstr string literal
  explicit ZsValue(const char *cstr) noexcept {
    _v.cstr = cstr;
    _idx = zs_var_type_cstr;
  }

  explicit operator long long int() const;
  explicit operator double() const;
  explicit operator int() const;
  explicit operator float() const;
  explicit operator char() const;
  explicit operator void *() const;

  /// @brief check if value type is of **zs_var_type_none**, or is Py_None when value type is
  /// **zs_var_type_object**
  bool isNone() const;
  /// @brief check if value type is of **zs_var_type_cstr**
  bool isCstr() const { return _idx == zs_var_type_cstr; }
  /// @brief check if value type is integral (including PyLong_Type)
  bool isIntegral() const;
  /// @brief check if value type is floating pointer (including PyFloat_Type)
  bool isFloatingPoint() const;
  /// @brief check if value type is arithmetic (integral or floating point)
  bool isNumeric() const { return isIntegral() || isFloatingPoint(); }
  /// @brief check if value type is python object handle
  bool isObject() const { return _idx == zs_var_type_object; }
  /// @brief check if value type is python **bool**
  bool isBoolObject() const;
  /// @brief check if value type is python **module**
  bool isModule() const;
  /// @brief check if value type is python **bytes**
  bool isBytes() const;
  /// @brief check if value type is python **bytearray**
  bool isByteArray() const;
  /// @brief check if value type is python **bytes** or **bytearray**
  bool isBytesOrByteArray() const { return isBytes() || isByteArray(); }
  /// @brief check if value type is python **str** (unicode)
  bool isString() const;
  /// @brief check if value type is python **tuple**
  bool isTuple() const;
  /// @brief check if value type is python **list**
  bool isList() const;
  /// @brief check if value type is python **set**
  bool isSet() const;
  /// @brief check if value type is python **dict**
  bool isDict() const;
  /// @brief check if value type is not none (i.e. !isNone())
  operator bool() const { return !isNone(); }

#define ZS_VALUE_DECLARE_OBJECT_REF(TYPE) \
  inline Zs##TYPE &as##TYPE();            \
  inline const Zs##TYPE &as##TYPE() const;
  ZS_VALUE_DECLARE_OBJECT_REF(Value)
  ZS_VALUE_DECLARE_OBJECT_REF(Object)
  ZS_VALUE_DECLARE_OBJECT_REF(Module)
  ZS_VALUE_DECLARE_OBJECT_REF(Bytes)
  ZS_VALUE_DECLARE_OBJECT_REF(String)
  ZS_VALUE_DECLARE_OBJECT_REF(Tuple)
  ZS_VALUE_DECLARE_OBJECT_REF(Dict)
  ZS_VALUE_DECLARE_OBJECT_REF(List)
  ZS_VALUE_DECLARE_OBJECT_REF(Set)
#undef ZS_VALUE_DECLARE_OBJECT_REF
};

/**
  @brief holds a python object (isObject() is true)
  @note To use it as one of its subtypes ZsXYZ (C++ strong type),
  isXYZ() should be queried and confirmed before using as one.
  @note ZsObject is **NOT** expected to hold a nullptr handle!
 */
struct ZS_INTERFACE_EXPORT ZsObject : ZsValue {
  ZsObject() noexcept;  // Py_None
  ZsObject(ZsValue handle) noexcept;
  ZsObject(ZsValuePort handle) noexcept : ZsObject{ZsValue{handle}} {}
  ZsObject(void *obj) noexcept;  // Py_None if nullptr
  ZsObject(const char *) = delete;

  ZsObject newRef();  // increase ref by 1
  inline void reflect() const;
  void *handle() const { return _v.obj; }
  void **pHandle() { return &_v.obj; }
  void *pytype() const;
  zs_obj_type_ type() const;
};

#define ZS_OBJECT_CONSTRUCTION_DEF(TYPE)                      \
  Zs##TYPE() noexcept = default;                              \
  Zs##TYPE(ZsValue handle) noexcept : ZsObject(handle) {}     \
  Zs##TYPE(ZsValuePort handle) noexcept : ZsObject(handle) {} \
  Zs##TYPE(ZsObject handle) noexcept : ZsObject(handle) {}    \
  Zs##TYPE(void *obj) noexcept : ZsObject{obj} {}

/// @brief holds a PyModule_Type object or Py_None
struct ZS_INTERFACE_EXPORT ZsModule : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(Module)
  const char *name() const;
  ZsDict dict();
  bool addObject(const char *name, ZsObject obj);
  bool addObjectSteal(const char *name, ZsObject obj);
  bool addStringConstant(const char *name, const char *val);
  bool addType(ZsObject type);
  // int addObject(const char *name, ZsObject object);
};
/// @brief holds a PyBytes_Type object, PyByteArray_Type or Py_None
struct ZS_INTERFACE_EXPORT ZsBytes : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(Bytes)

  explicit operator const char *() const { return c_str(); }
  const char *c_str() const;
  char *data();
  sint_t size() const;
  bool resize(sint_t sz);
};
/// @brief holds a PyUnicode_Type object or Py_None
struct ZS_INTERFACE_EXPORT ZsString : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(String)

  const char *c_str() const;
};

/**
  @brief holds a PyTuple_Type object or Py_None

  To iterate a tuple, see the following example:
  \code
ZsTuple &t = obj.asTuple();
for (auto e : t) {
  // e is ZsObject
  // e.reflect();
}
  \endcode
 */
struct ZS_INTERFACE_EXPORT ZsTuple : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(Tuple)

  ZsObject operator[](sint_t i);

  /**
    @brief iterator for ZsTuple
   */
  struct ZS_INTERFACE_EXPORT Iterator {
    Iterator(ZsTuple tup, sint_t i) noexcept : _tup{tup}, _pos{i} {}

    sint_t pos() const { return _pos; }
    ZsObject value() { return static_cast<ZsTuple &>(_tup)[_pos]; }

    ZsObject operator*() { return static_cast<ZsTuple &>(_tup)[_pos]; }

    // pre increment (++var)
    Iterator &operator++() noexcept {
      _pos++;
      return *this;
    }
    // post increment (var++)
    Iterator operator++(int) noexcept {
      Iterator it = *this;
      operator++();
      return it;
    }
    // pre decrement (--var)
    Iterator &operator--() noexcept {
      _pos--;
      return *this;
    }
    // post decrement (var--)
    Iterator operator--(int) noexcept {
      Iterator it = *this;
      operator--();
      return it;
    }

    Iterator &operator+=(sint_t offset) noexcept {
      _pos += offset;
      return *this;
    }
    Iterator &operator-=(sint_t offset) noexcept {
      _pos -= offset;
      return *this;
    }
    Iterator operator+(sint_t offset) const noexcept {
      Iterator ret = *this;
      return ret += offset;
    }
    Iterator operator-(sint_t offset) const noexcept { return operator+(-offset); }
    sint_t operator-(const Iterator &o) const noexcept { return _pos - o._pos; }

    ZsObject operator[](sint_t offset) { return static_cast<ZsTuple &>(_tup)[_pos + offset]; }

#define ZS_LIST_COMPARE_OP(OP) \
  bool operator OP(const Iterator &o) { return (_pos - o._pos) OP 0; }
    ZS_LIST_COMPARE_OP(<)
    ZS_LIST_COMPARE_OP(<=)
    ZS_LIST_COMPARE_OP(>)
    ZS_LIST_COMPARE_OP(>=)
#undef ZS_LIST_COMPARE_OP
    bool operator==(const Iterator &o) const { return _pos == o._pos && _tup == o._tup; }
    bool operator!=(const Iterator &o) const { return !(operator==(o)); }

    ZsObject _tup;
    sint_t _pos;
  };
  Iterator begin() { return Iterator(*this, 0); }
  Iterator end() { return Iterator(*this, size()); }

  sint_t size() const;
};
/**
  @brief holds a PyDict_Type object or Py_None

  To iterate a dict, see the following example:
  \code
auto it = obj.asDict().begin();
while (it) {
  ZsString key = it.key();
  ZsObject val = it.value();
  // key.reflect();
  // val.reflect();
  ++it; // or 'it++'
}
  \endcode
 */
struct ZS_INTERFACE_EXPORT ZsDict : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(Dict)

  ZsObject operator[](const char *key);
  ZsObject at(const char *key) { return operator[](key); }

  /// @note non-standard iterator
  struct ZS_INTERFACE_EXPORT Iterator {
    Iterator(ZsDict dict);

    bool valid() const { return _valid; }
    sint_t pos() const { return _pos; }
    ZsString key() { return valid() ? _key : ZsString(); }
    ZsObject value() { return valid() ? _value : ZsObject(); }

    // pre increment (++var)
    Iterator &operator++();

    // post increment (var++)
    Iterator operator++(int) {
      Iterator it = *this;
      operator++();
      return it;
    }

    operator bool() const { return valid(); }

    ZsObject _dict;
    ZsString _key;
    ZsObject _value;
    sint_t _pos;
    int _valid;
  };
  Iterator begin() { return Iterator(*this); }
  int set(const char *key, ZsObject item);
  int setSteal(const char *key, ZsObject item);
  sint_t size() const;
};
/**
  @brief holds a PyList_Type object or Py_None

  To iterate a list, see the following example:
  \code
ZsList &l = obj.asList();
for (auto e : l) {
  // e.reflect();
}
  \endcode
 */
struct ZS_INTERFACE_EXPORT ZsList : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(List)

  ZsObject operator[](sint_t i);

  struct ZS_INTERFACE_EXPORT Iterator {
    Iterator(ZsList list, sint_t i) noexcept : _list{list}, _pos{i} {}

    sint_t pos() const { return _pos; }
    ZsObject value() { return static_cast<ZsList &>(_list)[_pos]; }

    ZsObject operator*() { return static_cast<ZsList &>(_list)[_pos]; }

    // pre increment (++var)
    Iterator &operator++() noexcept {
      _pos++;
      return *this;
    }
    // post increment (var++)
    Iterator operator++(int) noexcept {
      Iterator it = *this;
      operator++();
      return it;
    }
    // pre decrement (--var)
    Iterator &operator--() noexcept {
      _pos--;
      return *this;
    }
    // post decrement (var--)
    Iterator operator--(int) noexcept {
      Iterator it = *this;
      operator--();
      return it;
    }

    Iterator &operator+=(sint_t offset) noexcept {
      _pos += offset;
      return *this;
    }
    Iterator &operator-=(sint_t offset) noexcept {
      _pos -= offset;
      return *this;
    }
    Iterator operator+(sint_t offset) const noexcept {
      Iterator ret = *this;
      return ret += offset;
    }
    Iterator operator-(sint_t offset) const noexcept { return operator+(-offset); }
    sint_t operator-(const Iterator &o) const noexcept { return _pos - o._pos; }

    ZsObject operator[](sint_t offset) { return static_cast<ZsList &>(_list)[_pos + offset]; }

#define ZS_LIST_COMPARE_OP(OP) \
  bool operator OP(const Iterator &o) { return (_pos - o._pos) OP 0; }
    ZS_LIST_COMPARE_OP(<)
    ZS_LIST_COMPARE_OP(<=)
    ZS_LIST_COMPARE_OP(>)
    ZS_LIST_COMPARE_OP(>=)
#undef ZS_LIST_COMPARE_OP
    bool operator==(const Iterator &o) const { return _pos == o._pos && _list == o._list; }
    bool operator!=(const Iterator &o) const { return !(operator==(o)); }

    ZsObject _list;
    sint_t _pos;
  };
  Iterator begin() { return Iterator(*this, 0); }
  Iterator end() { return Iterator(*this, size()); }

  int setItemSteal(sint_t index, ZsObject item);
  int insert(sint_t index, ZsObject item);
  int append(ZsObject item);
  int insertSteal(sint_t index, ZsObject item);
  int appendSteal(ZsObject item);
  sint_t size() const;
};
/**
  @brief holds a PySet_Type object or Py_None

  To iterate a set, see the following example:
  \code
auto it = obj.asSet().begin();
while (it) {
  it.value().reflect();
  ++it;
}
  \endcode
 */
struct ZS_INTERFACE_EXPORT ZsSet : ZsObject {
  ZS_OBJECT_CONSTRUCTION_DEF(Set)

  struct ZS_INTERFACE_EXPORT Iterator {
    Iterator(ZsSet set);
    ~Iterator();

    Iterator(const Iterator &) = delete;
    Iterator(Iterator &&) = delete;
    Iterator &operator=(const Iterator &) = delete;
    Iterator &operator=(Iterator &&) = delete;

    bool valid() const { return _valid; }
    ZsObject value() { return valid() ? _item : ZsObject(); }

    // pre increment (++var)
    Iterator &operator++();

    operator bool() const { return valid(); }

    ZsObject _it;
    ZsObject _item;
    int _valid;
  };
  Iterator begin() { return Iterator(*this); }
  sint_t size() const;
};

#define ZS_VALUE_DEF_OBJECT_REF(TYPE)                                      \
  Zs##TYPE &ZsValue::as##TYPE() { return static_cast<Zs##TYPE &>(*this); } \
  const Zs##TYPE &ZsValue::as##TYPE() const { return static_cast<const Zs##TYPE &>(*this); }
ZS_VALUE_DEF_OBJECT_REF(Object)
ZS_VALUE_DEF_OBJECT_REF(Module)
ZS_VALUE_DEF_OBJECT_REF(Bytes)
ZS_VALUE_DEF_OBJECT_REF(String)
ZS_VALUE_DEF_OBJECT_REF(Tuple)
ZS_VALUE_DEF_OBJECT_REF(Dict)
ZS_VALUE_DEF_OBJECT_REF(List)
ZS_VALUE_DEF_OBJECT_REF(Set)
#undef ZS_VALUE_DEF_OBJECT_REF

/**
  @}
  */

/// states
ZS_INTERFACE_EXPORT unsigned zs_last_error();
ZS_INTERFACE_EXPORT unsigned zs_last_warn();
ZS_INTERFACE_EXPORT ZsValuePort zs_world_handle();        // global dict
ZS_INTERFACE_EXPORT ZsValuePort zs_world_local_handle();  // local dict
ZS_INTERFACE_EXPORT bool zs_world_pending_input();
ZS_INTERFACE_EXPORT void *zs_execute_statement(const char *cmd, int *result);
ZS_INTERFACE_EXPORT void *zs_eval_expr(const char *expr, void **errLogBytes = nullptr);
ZS_INTERFACE_EXPORT void *zs_execute_script(const char *cmd, int *state);

/// ZsValue query
ZS_INTERFACE_EXPORT void zs_reflect_value(ZsValue v, const char *msg = "");
ZS_INTERFACE_EXPORT zs_obj_type_ zs_get_obj_type(ZsValue);

/// ZsValue construction
/**
 *  \addtogroup valctor_apis
 *  @{
 */

// new value
/// @brief string literal encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_cstr(const char *cstr = "");
/// @brief i64 (long long int) encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_i64(long long int v = 0);
/// @brief f64 (double) encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_f64(double v = 0.0);
/// @brief i32 (int) encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_i32(int v = 0);
/// @brief f32 (float) encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_f32(float v = 0.f);
/// @brief i8 (char) encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_i8(char v = 0);
/// @brief object handle encapsulated in ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_obj(void *obj = 0);  // obj lifetime unaffected
/**
 *  @}
 */

/**
 *  \addtogroup objctor_apis
 *  @{
 */
// new object
/// @brief return a copy of \a val, incref the handle if \a val is holding a python object
/// @param val any ZsValue variable
ZS_INTERFACE_EXPORT ZsValuePort zs_obj_new_ref(ZsValue val);

// bool
/// @brief return a python bool object based on \a b
/// @param b a boolean
ZS_INTERFACE_EXPORT ZsValuePort zs_bool_obj(bool b);

// module
/// @brief return a python **module** object named by \a name
/// @param name the module name
ZS_INTERFACE_EXPORT ZsValuePort zs_module_cstr(const char *name);

// bytes
/// @brief return a python **bytes** object based on \a args
/// @param args a ZsValue that holds a string literal or a python str/bytes/bytearray object.
ZS_INTERFACE_EXPORT ZsValuePort zs_bytes_obj(ZsValue args);
/// @brief return a python **bytes** object based on a string literal \a cstr
/// @param cstr a NULL-terminated string literal
ZS_INTERFACE_EXPORT ZsValuePort zs_bytes_obj_cstr(const char *cstr);
/// @brief return a python **bytes** object based on a string literal \a st with length \a len
/// @param st a string literal of length \a len
/// @param len the length of the string literal
ZS_INTERFACE_EXPORT ZsValuePort zs_bytes_obj_cstr_range(const char *st, sint_t len);

/// @brief return a python **bytearray** object based on a string literal \a cstr
/// @param cstr a NULL-terminated string literal
ZS_INTERFACE_EXPORT ZsValuePort zs_bytearray_obj_cstr(const char *cstr);
/// @brief return a python bytearray object based on a string literal \a cstr with length \a len
/// @param cstr a string literal of length \a len
/// @param len the length of the string literal
ZS_INTERFACE_EXPORT ZsValuePort zs_bytearray_obj_cstr_range(const char *cstr, sint_t len);

// string
/// @brief return a python **str** object representing \a val
/// @param val any ZsValue
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj(ZsValue val);
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj_long_long(long long n);
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj_double(double f);
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj_cstr(const char *cstr);
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj_repr(ZsValue obj);
ZS_INTERFACE_EXPORT ZsValuePort zs_string_obj_type(ZsValue obj);

// numeric
/// @brief return a python **float** object from a double \a f
/// @param f a double
ZS_INTERFACE_EXPORT ZsValuePort zs_float_obj_double(double f);
/// @brief return a python **float** object from a python str \a str
/// @param str a python string handle
ZS_INTERFACE_EXPORT ZsValuePort zs_float_obj_str(ZsValue str);
ZS_INTERFACE_EXPORT ZsValuePort zs_long_obj_double(double f);
ZS_INTERFACE_EXPORT ZsValuePort zs_long_obj_long_long(long long n);
ZS_INTERFACE_EXPORT ZsValuePort zs_long_obj_str(ZsValue str);

// tuple
/// @brief return a python **tuple** object from \a arg
/// @param arg a python object handle
/// @note this func is WIP, not usable right now
ZS_INTERFACE_EXPORT ZsValuePort zs_tuple_obj(ZsValue arg);
/// @brief return a python **tuple** object by packing \a count number of python **pointers** \a ...
/// @param count the number of python object pointers
/// @param ... \a count number of python object pointers
ZS_INTERFACE_EXPORT ZsValuePort zs_tuple_obj_pack_ptrs(sint_t count, ...);
/// @brief return a python **tuple** object by packing \a count number of python **handles** \a ...
/// @param count the number of python object handles (as in ZsValue / ZsObject...)
/// @param ... \a count number of python object handles (as in ZsValue / ZsObject...)
ZS_INTERFACE_EXPORT ZsValuePort zs_tuple_obj_pack_zsobjs(sint_t count, ...);
ZS_INTERFACE_EXPORT ZsValuePort zs_tuple_obj_default(long len);
ZS_INTERFACE_EXPORT ZsValuePort zs_tuple_obj_long(long n);

// dict
/// @brief return an empty python **dict** object
ZS_INTERFACE_EXPORT ZsValuePort zs_dict_obj_default();
ZS_INTERFACE_EXPORT ZsValuePort zs_dict_obj_copy(ZsDict dict);

// list
/// @brief return an empty python **list** object
ZS_INTERFACE_EXPORT ZsValuePort zs_list_obj_default();

// set
/// @brief return an empty python **set** object
ZS_INTERFACE_EXPORT ZsValuePort zs_set_obj_default();

/**
 *  @}
 */

ZS_INTERFACE_EXPORT bool update_zs_value_maintenance_apis();

/// ZsValue interfaces

struct ZS_INTERFACE_EXPORT ZsValueOps {
  void (*initVal)(ZsValue *, ZsValue);
  void (*deinit)(ZsValue *);
  ZsValuePort (*copy)(ZsValue);
  void (*reflect)(ZsValue);
};
struct ZS_INTERFACE_EXPORT ZsVarOps {
  void (*initVal)(ZsValue *, ZsValue);
  bool (*initObj)(ZsValue *, zs_obj_type_, ZsValue);
  void (*deinit)(ZsValue *);
  ZsValuePort (*clone)(ZsValue);
  ZsValuePort (*share)(ZsValue);
  // compare
  bool (*is)(ZsValue, ZsValue);
  // rich compare
  bool (*eq)(ZsValue, ZsValue);
  bool (*ne)(ZsValue, ZsValue);
  // reflection
  void (*reflect)(ZsValue);
  long long int (*refcnt)(ZsValue);
};

void ZsObject::reflect() const { g_zs_variable_apis.reflect(static_cast<const ZsValue &>(*this)); }

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus
template <typename T> constexpr T *as_ptr_(void *obj) noexcept { return static_cast<T *>(obj); }
template <typename T> constexpr T *as_ptr_(ZsValuePort obj) noexcept {
  return static_cast<T *>(obj._v.obj);
}
/**
  @addtogroup obj_types
  @{
  */
/**
    \~english @brief C++ wrapper class for python object

    A RAII C++ wrapper class that holds a python object. On destruction, Py_DECREF on the object
   handle (pointer) is called.

    Unlike ZsObject，PyVar allows its handle (_obj) to be nullptr，and upon casting to bool
   (operator bool())，it returns (_obj != nullptr). This means holding Py_None would also return
   true.

    Please make sure GIL is ensured when PyVar is being used in Python CAPIs.

    Regarding how Python treats variables, please see
   [document](https://docs.python.org/zh-cn/3/c-api/intro.html#objects-types-and-reference-counts).

    \~chinese @brief python对象的C++包装类型

    python对象的c++包装类。析构时，会对python对象指针调用Py_DECREF。

    与ZsObject不同的是，PyVar允许handle（_obj）为nullptr，且进行bool判断（operator
   bool()）时，返回(_obj != nullptr)。意味着存储Py_None时也会返回真。

    PyVar用于Python CAPIs时请确保GIL的获取。

    有关Python如何维护变量，请见[Python文档](https://docs.python.org/zh-cn/3/c-api/intro.html#objects-types-and-reference-counts)。
 */
struct ZS_INTERFACE_EXPORT PyVar {
  PyVar() noexcept = default;
  /**
    @brief Takes the ownership of a raw python object pointer

     Like wrapping a raw pointer in std::shared_ptr
    */
  PyVar(void *obj) noexcept : _obj{obj} {}
  /**
    @brief To avoid confusion with string literal pointer, delete this overload explicitly
    */
  explicit PyVar(const char *) = delete;
  /**
    @brief Any other pointer handle is allowed besides **const char* **
    */
  template <typename T> PyVar(T *obj) noexcept : _obj{obj} {}
  /**
    @brief Takes the ownership of a python object through a ZsValue's handle
    @note would convert Py_None to nullptr on initilization
    */
  PyVar(ZsValuePort handle) noexcept;
  /// @brief Py_XDECREF on object handle (if not null) on destruction
  ~PyVar();

  /// @brief move-assigned with another PyVar's handle
  /// @param o the other moved-from PyVar
  /// @note does not affect reference count of the python object, leave \a o in NULL state
  PyVar(PyVar &&o) noexcept {
    _obj = o._obj;
    o._obj = nullptr;
  }
  /// @brief copy-assigned with another PyVar's handle
  /// @param o the other copied-from PyVar
  /// @note increases the reference count of the python object by 1
  PyVar(const PyVar &o);
  /// @brief destructs the previously-stored handle and move assigned with another PyVar's
  /// handle
  /// @param o the other moved-from PyVar
  PyVar &operator=(PyVar &&o);
  /// @brief destructs the previously-stored handle and copy assigned with another PyVar's handle
  /// @param o the other copied-from PyVar
  PyVar &operator=(const PyVar &o);

  /**
    @brief Releases the ownership of the stored raw python object pointer

     Like releasing a raw pointer from std::shared_ptr
   */
  void *release() noexcept {
    auto ret = _obj;
    _obj = nullptr;
    return ret;
  }
  /// @brief query the python object type zs_obj_type_
  /// @note if ZsValue is not an object, **zs_obj_type_unknown** is returned
  zs_obj_type_ get_obj_type() const { return zs_get_obj_type(ZsObject{_obj}); }
  /// @brief query if _obj is not a nullptr
  /// @note different from ZsVar
  operator bool() const noexcept { return _obj; }
  /// @brief retrieve the python object through a ZsObject handle
  ZsObject getValue() const noexcept { return ZsObject{_obj}; }
  /// @brief retrieve the python object through a ZsObject handle
  auto getObject() const noexcept { return getValue(); }

#  define PY_VAR_DECLARE_COMPARATOR(SYM, TAG) bool operator SYM(const PyVar &o) const;

  PY_VAR_DECLARE_COMPARATOR(<, Py_LT)
  PY_VAR_DECLARE_COMPARATOR(<=, Py_LE)
  PY_VAR_DECLARE_COMPARATOR(==, Py_EQ)
  PY_VAR_DECLARE_COMPARATOR(!=, Py_NE)
  PY_VAR_DECLARE_COMPARATOR(>, Py_GT)
  PY_VAR_DECLARE_COMPARATOR(>=, Py_GE)

#  undef PY_VAR_DECLARE_COMPARATOR

  /// @brief get the stored python object handle
  void *handle() const noexcept { return _obj; }
  /// @brief get a pointer to the stored python object handle
  void **pHandle() noexcept { return &_obj; }
  /// @brief get the PyType_Object of the corresponding stored python object
  void *pytype() const noexcept;
  /// @brief get the stored python object handle
  operator void *() noexcept { return _obj; }
  template <typename T> operator T *() noexcept { return static_cast<T *>(_obj); }

#  define PY_VAR_DEFINE_OBJECT_CAST(TYPE) \
    Zs##TYPE as##TYPE() const { return Zs##TYPE{_obj}; }

  PY_VAR_DEFINE_OBJECT_CAST(Value)
  PY_VAR_DEFINE_OBJECT_CAST(Object)
  PY_VAR_DEFINE_OBJECT_CAST(Module)
  PY_VAR_DEFINE_OBJECT_CAST(Bytes)
  PY_VAR_DEFINE_OBJECT_CAST(String)
  PY_VAR_DEFINE_OBJECT_CAST(Tuple)
  PY_VAR_DEFINE_OBJECT_CAST(Dict)
  PY_VAR_DEFINE_OBJECT_CAST(List)
  PY_VAR_DEFINE_OBJECT_CAST(Set)
#  undef PY_VAR_DEFINE_OBJECT_CAST

  /// @brief call the python callable object with no argument
  PyVar operator()();
  /// @brief call the python callable object with one argument
  /// @param obj a python object
  PyVar operator()(ZsObject obj);
  /// @brief call the python callable object with variable arguments
  /// @param args python object arguments wrapped in ZsValue / ZsObjects / ...
  template <typename... Ts> PyVar operator()(Ts... args) {
    if constexpr (sizeof...(Ts) == 1)
      return operator()(ZsObject{args}...);
    else
      return callObject(zs_tuple_obj_pack_zsobjs(sizeof...(Ts), ZsObject{args}...));
  }
  PyVar callObject(ZsTuple args);

  /// @brief call the attribute (which is a python callable object) of name \a method with no
  /// argument
  /// @param method attribute name
  /// @note In python, someObj.method()
  PyVar operator()(const char *method);
  /// @brief call the attribute (which is a python callable object) of name \a method with one
  /// argument
  /// @param method attribute name
  /// @param obj a python object
  /// @note In python, someObj.method(obj)
  PyVar operator()(const char *method, ZsObject obj);
  /// @brief call the attribute (which is a python callable object) of name \a method with
  /// variable arguments
  /// @param method attribute name
  /// @param args python object arguments wrapped in ZsValue / ZsObjects / ...
  /// @note In python, someObj.method(args...)
  template <typename... Ts> PyVar operator()(const char *method, Ts... args) {
    if constexpr (sizeof...(Ts) == 1)
      return operator()(method, ZsObject{args}...);
    else
      return callMethod(method, zs_tuple_obj_pack_zsobjs(sizeof...(Ts), ZsObject{args}...));
  }
  PyVar callMethod(const char *method, ZsTuple args);

  /// @brief query if it has an attribute of name \a name
  /// @param name attribute name
  bool hasAttr(const char *name);
  /// @brief set the attribute of name \a name with \a attr
  /// @param name attribute name
  /// @param attr the python object to set
  bool setAttr(const char *name, ZsObject attr);
  /// @brief delete the attribute of name \a name
  /// @param name attribute name
  bool delAttr(const char *name);
  /// @brief get the attribute of name \a name
  /// @param name attribute name
  PyVar attr(const char *name);

  /// @brief query if it has an item of name \a name
  /// @param name item name
  bool hasItem(const char *name);
  /// @brief set the item of name \a name with \a attr
  /// @param name item name
  /// @param item the python object to set
  bool setItem(const char *name, ZsObject item);
  /// @brief delete the item of name \a name
  /// @param name item name
  bool delItem(const char *name);
  /// @brief get the item of name \a name
  /// @param name item name
  PyVar item(const char *name);

  /// @brief get the repr representation of the stored python object
  PyVar repr();
  /// @brief get the string representation of the stored python object in **str**
  PyVar str();
  /// @brief get the string representation of the stored python object in **bytes**
  PyVar bytes();
  /// @brief get the **dir** attribute of the stored python object in **str**
  PyVar dir();

  void *_obj;
};

template <typename T> constexpr T *as_ptr_(const PyVar &obj) noexcept { return (T *)(obj._obj); }
/**
  @}
  */
#endif