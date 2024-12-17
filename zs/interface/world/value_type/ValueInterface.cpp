#include "ValueInterface.hpp"

#include <Python.h>
#include <float.h>

#include "interface/details/PyHelper.hpp"

static_assert(sizeof(sint_t) == sizeof(Py_ssize_t),
              "sint_t not a proper replacement for Py_ssize_t");
#ifdef __cplusplus
extern "C" {
#endif

void zs_reflect_value(ZsValue v, const char *msg) {
  switch (v._idx) {
    case zs_var_type_object: {
      static_assert(sizeof(void *) == sizeof(unsigned long long), "...");
      PyVar str = zs_string_obj_repr(v);
      PyVar typeStr = zs_string_obj_type(v);
      if (str && typeStr) {
        printf("repr: %s. \n\ttype [%s], handle [%llx], ref cnt: %d.\n", str.asString().c_str(),
               typeStr.asString().c_str(), (unsigned long long)v._v.obj,
               (int)g_zs_variable_apis.refcnt(v));
      }
    } break;
    case zs_var_type_cstr:
      printf("type [cstr(const char*)], val [%s]\n", v._v.cstr);
      break;
    case zs_var_type_i64:
      printf("type [i64(long long int)], val [%lld]\n", v._v.i64);
      break;
    case zs_var_type_f64:
      printf("type [f64(double)], val [%f]\n", v._v.f64);
      break;
    case zs_var_type_i32:
      printf("type [i32(int)], val [%d]\n", v._v.i32);
      break;
    case zs_var_type_f32:
      printf("type [f32(float)], val [%f]\n", v._v.f32);
      break;
    case zs_var_type_i8:
      printf("type [i8(char)], val [%d]\n", (int)v._v.i8);
      break;
    case zs_var_type_none:
      printf("value not initialized yet\n");
      break;
    default:;
  }
}
zs_obj_type_ zs_get_obj_type(ZsValue obj) {
  if (obj._idx == zs_var_type_object) {
    auto pyobj = static_cast<PyObject *>(obj._v.obj);
    auto tp = Py_TYPE(pyobj);
    if (tp == &PyBytes_Type)
      return zs_obj_type_bytes;
    else if (tp == &PyByteArray_Type)
      return zs_obj_type_bytearray;
    else if (tp == &PyUnicode_Type)
      return zs_obj_type_string;
    else if (tp == &PyTuple_Type)
      return zs_obj_type_tuple;
    else if (tp == &PyLong_Type)
      return zs_obj_type_long;
    else if (tp == &PyFloat_Type)
      return zs_obj_type_float;
    else if (tp == &PyList_Type)
      return zs_obj_type_list;
    else if (tp == &PySet_Type)
      return zs_obj_type_set;
    else if (tp == &PyDict_Type)
      return zs_obj_type_dict;
    else if (tp == &PyModule_Type)
      return zs_obj_type_module;
    else
      return zs_obj_type_custom;
  }
  return zs_obj_type_unknown;
}

ZsValuePort zs_cstr(const char *cstr) {
  ZsValue ret;
  ret._v.cstr = cstr;
  ret._idx = zs_var_type_cstr;
  return ret;
}
ZsValuePort zs_i64(long long int v) {
  ZsValue ret;
  ret._v.i64 = v;
  ret._idx = zs_var_type_i64;
  return ret;
}
ZsValuePort zs_f64(double v) {
  ZsValue ret;
  ret._v.f64 = v;
  ret._idx = zs_var_type_f64;
  return ret;
}
ZsValuePort zs_i32(int v) {
  ZsValue ret;
  ret._v.i32 = v;
  ret._idx = zs_var_type_i32;
  return ret;
}
ZsValuePort zs_f32(float v) {
  ZsValue ret;
  ret._v.f32 = v;
  ret._idx = zs_var_type_f32;
  return ret;
}
ZsValuePort zs_i8(char v) {
  ZsValue ret;
  ret._v.i8 = v;
  ret._idx = zs_var_type_i8;
  return ret;
}

ZsValuePort zs_obj(void *obj) {
  ZsValue ret;
  ret._v.obj = obj ? obj : Py_None;
  ret._idx = zs_var_type_object;
  return ret;
}

bool update_zs_value_maintenance_apis() { return false; }

///

ZsValue::operator long long int() const {
  switch (_idx) {
    case zs_var_type_i64:
      return _v.i64;
    case zs_var_type_i32:
      return _v.i32;
    case zs_var_type_i8:
      return _v.i8;
    case zs_var_type_object: {
      int overflow = 0;
      long long res = PyLong_AsLongLongAndOverflow(static_cast<PyObject *>(_v.obj), &overflow);
      if (overflow == 0) {
        if (res == -1)
          if (PyErr_Occurred()) PyErr_Print();
      } else
        zs_print_err_py_cstr("[ZsValue::operator long long int()]: overflow!\n");
      return res;
    }
    default:
      zs_print_err_py_cstr("[ZsValue::operator long long int()]: no proper conversion");
      return (long long int)-1;
  }
}
ZsValue::operator double() const {
  switch (_idx) {
    case zs_var_type_f64:
      return _v.f64;
    case zs_var_type_f32:
      return _v.f32;
    case zs_var_type_i64:
      return _v.i64;
    case zs_var_type_i32:
      return _v.i32;
    case zs_var_type_i8:
      return _v.i8;
    case zs_var_type_object: {
      double res = PyFloat_AsDouble(static_cast<PyObject *>(_v.obj));
      if (res == -1.0)
        if (PyErr_Occurred()) PyErr_Print();
      return res;
    }
    default:
      zs_print_err_py_cstr("[ZsValue::operator double()]: no proper conversion");
      return -1.0;
  }
}
ZsValue::operator int() const {
  switch (_idx) {
    case zs_var_type_i32:
      return _v.i32;
    case zs_var_type_i64:
      return static_cast<int>(_v.i64);
    case zs_var_type_i8:
      return _v.i8;
    case zs_var_type_object: {
      int overflow = 0;
      long long res = PyLong_AsLongLongAndOverflow(static_cast<PyObject *>(_v.obj), &overflow);
      if (overflow == 0) {
        if (res == -1)
          if (PyErr_Occurred()) PyErr_Print();
        if (res <= INT_MAX && res >= INT_MIN) return res;
      }
      zs_print_err_py_cstr("[ZsValue::operator int()]: overflow!\n");
      return -1;
    }
    default:
      zs_print_err_py_cstr("[ZsValue::operator int()]: no proper conversion");
      return (int)-1;
  }
}
ZsValue::operator float() const {
  switch (_idx) {
    case zs_var_type_f32:
      return _v.f32;
    case zs_var_type_f64:
      return _v.f64;
    case zs_var_type_i64:
      return _v.i64;
    case zs_var_type_i32:
      return _v.i32;
    case zs_var_type_i8:
      return _v.i8;
    case zs_var_type_object: {
      double res = PyFloat_AsDouble(static_cast<PyObject *>(_v.obj));
      if (res == -1.0)
        if (PyErr_Occurred()) PyErr_Print();
      if (res >= -FLT_MAX && res <= FLT_MAX) return res;
      zs_print_err_py_cstr("[ZsValue::operator float()]: overflow!\n");
      return -1.f;
    }
    default:
      zs_print_err_py_cstr("[ZsValue::operator float()]: no proper conversion");
      return -1.0f;
  }
}
ZsValue::operator char() const {
  if (_idx == zs_var_type_i8)
    return _v.i8;
  else {
    zs_print_err_py_cstr("[ZsValue::operator char()]: no proper conversion");
    return -1;
  }
}
ZsValue::operator void *() const {
  switch (_idx) {
    case zs_var_type_cstr:
      return static_cast<void *>(const_cast<char *>(_v.cstr));
    case zs_var_type_none:
      return nullptr;
    case zs_var_type_object:
      return _v.obj;
    default:
      zs_print_err_py_cstr("[ZsValue::operator void*()]: no proper conversion");
      return nullptr;
  }
}

bool ZsValue::isIntegral() const {
  switch (_idx) {
    case zs_var_type_i8:;
    case zs_var_type_i64:;
    case zs_var_type_i32:
      return true;
    case zs_var_type_object:
      if (Py_TYPE(_v.obj) == &PyLong_Type) return true;
    default:
      return false;
  }
}
bool ZsValue::isFloatingPoint() const {
  switch (_idx) {
    case zs_var_type_f64:;
    case zs_var_type_f32:
      return true;
    case zs_var_type_object:
      if (Py_TYPE(_v.obj) == &PyFloat_Type) return true;
    default:
      return false;
  }
}
bool ZsValue::isNone() const {
  switch (_idx) {
    case zs_var_type_object:
      assert(_v.obj != nullptr && "ZsObject should not hold a NULL handle");
      return _v.obj == Py_None;
    case zs_var_type_none:
      return true;
    default:
      return false;
  }
}
bool ZsValue::isBoolObject() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyBool_Type;
}
bool ZsValue::isModule() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyModule_Type;
}
bool ZsValue::isBytes() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyBytes_Type;
}
bool ZsValue::isByteArray() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyByteArray_Type;
}
bool ZsValue::isString() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyUnicode_Type;
}
bool ZsValue::isTuple() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyTuple_Type;
}
bool ZsValue::isList() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyList_Type;
}
bool ZsValue::isSet() const { return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PySet_Type; }
bool ZsValue::isDict() const {
  return _idx == zs_var_type_object && Py_TYPE(_v.obj) == &PyDict_Type;
}

/// object
ZsObject::ZsObject() noexcept {
  _v.obj = Py_None;
  _idx = zs_var_type_object;
}
ZsObject::ZsObject(void *obj) noexcept {
  _v.obj = obj ? obj : Py_None;
  _idx = zs_var_type_object;
}
ZsObject::ZsObject(ZsValue handle) noexcept : ZsValue(handle) {
  assert(handle.isObject() && handle._v.obj != nullptr
         && "ZsObject is initialized with an invalid ZsValue");
}
ZsObject ZsObject::newRef() {
  assert(isObject() && _v.obj != nullptr && "ZsObject is invalid (empty obj ptr).");
  return Py_NewRef(_v.obj);
}
void *ZsObject::pytype() const {
  assert(_idx == zs_var_type_object && "querying pytype of an invalid ZsObject");
  return Py_TYPE(_v.obj);
}
zs_obj_type_ ZsObject::type() const { return zs_get_obj_type(*this); }

/// module
const char *ZsModule::name() const {
  if (auto ret = PyModule_GetName(static_cast<PyObject *>(_v.obj))) return ret;
  PyErr_Print();
  return nullptr;
}
ZsDict ZsModule::dict() {
  if (auto ret = PyModule_GetDict(static_cast<PyObject *>(_v.obj))) return zs_obj(ret);
  PyErr_Print();
  return zs_obj(Py_None);
}
bool ZsModule::addObject(const char *name, ZsObject obj) {
  if (!obj) return false;
  if (int ret = PyModule_AddObjectRef(static_cast<PyObject *>(_v.obj), name,
                                      static_cast<PyObject *>(obj.handle()));
      ret == 0) {
    return true;
  }
  PyErr_Print();
  return false;
}
bool ZsModule::addObjectSteal(const char *name, ZsObject obj) {
  if (!obj) return false;
  if (int ret = PyModule_AddObject(static_cast<PyObject *>(_v.obj), name,
                                   static_cast<PyObject *>(obj.handle()));
      ret == 0) {
    return true;
  }
  Py_DECREF(static_cast<PyObject *>(obj.handle()));
  PyErr_Print();
  return false;
}
bool ZsModule::addStringConstant(const char *name, const char *val) {
  return PyModule_AddStringConstant(static_cast<PyObject *>(_v.obj), name, val) == 0;
}
bool ZsModule::addType(ZsObject type) {
  return PyModule_AddType(static_cast<PyObject *>(_v.obj),
                          static_cast<PyTypeObject *>(type.handle()))
         == 0;
}

/// bytes
const char *ZsBytes::c_str() const {
  assert(pytype() == &PyBytes_Type || pytype() == &PyByteArray_Type || _v.obj == Py_None);
  const char *ret = nullptr;
  PyObject *pyobj = static_cast<PyObject *>(_v.obj);
  PyTypeObject *tp = Py_TYPE(pyobj);
  if (tp == &PyByteArray_Type) {
    ret = PyByteArray_AsString(pyobj);
  } else if (tp == &PyBytes_Type) {
    /// @note exception only raised upon TypeError, which excluded already
    ret = PyBytes_AsString(pyobj);
  }
  return ret;
}
char *ZsBytes::data() {
  char *ret = nullptr;
  PyObject *pyobj = static_cast<PyObject *>(_v.obj);
  PyTypeObject *tp = Py_TYPE(pyobj);
  if (tp == &PyByteArray_Type) {
    ret = PyByteArray_AsString(pyobj);
  }
  return ret;
}
sint_t ZsBytes::size() const {
  if (isNone())
    return -1;
  else if (Py_TYPE(_v.obj) == &PyBytes_Type)
    return PyBytes_Size(static_cast<PyObject *>(_v.obj));
  else
    return PyByteArray_Size(static_cast<PyObject *>(_v.obj));
}
bool ZsBytes::resize(sint_t sz) {
  PyObject *pyobj = static_cast<PyObject *>(_v.obj);
  if (PyByteArray_Resize(pyobj, sz) != 0) {
    PyErr_Print();
    return false;
  }
  return true;
}

/// string
const char *ZsString::c_str() const {
  assert(pytype() == &PyUnicode_Type || _v.obj == Py_None);
  const char *ret = nullptr;
  if (ret = PyUnicode_AsUTF8AndSize(static_cast<PyObject *>(_v.obj), NULL); ret == NULL)
    PyErr_Print();
  return ret;
}

/// tuple
sint_t ZsTuple::size() const {
  if (isNone())
    return -1;
  else
    return PyTuple_Size(static_cast<PyObject *>(_v.obj));
}

ZsObject ZsTuple::operator[](sint_t i) {
  auto ret = PyTuple_GetItem(as_ptr_<PyObject>(*this), i);
  if (!ret) PyErr_Print();
  return ret;
}

/// dict
sint_t ZsDict::size() const {
  if (isNone())
    return -1;
  else
    return PyDict_Size(static_cast<PyObject *>(_v.obj));
}

ZsObject ZsDict::operator[](const char *key) {
  PyVar keyStr = PyUnicode_FromString(key);
  auto ret = PyDict_GetItemWithError(as_ptr_<PyObject>(*this), keyStr);
  if (!ret) {
    if (auto ec = PyErr_Occurred()) PyErr_Print();
  }
  return ret;
}
int ZsDict::set(const char *key, ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyDict_SetItemString(as_ptr_<PyObject>(*this), key, as_ptr_<PyObject>(item));
    if (ret == -1) PyErr_Print();
    return ret;
  }
  return -1;
}
int ZsDict::setSteal(const char *key, ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyDict_SetItemString(as_ptr_<PyObject>(*this), key, as_ptr_<PyObject>(item));
    if (ret == -1)
      PyErr_Print();
    else
      Py_DECREF(item.handle());
    return ret;
  }
  return -1;
}

ZsDict::Iterator::Iterator(ZsDict dict) : _pos{0}, _dict{dict}, _valid{1} {
  _valid = PyDict_Next(as_ptr_<PyObject>(_dict), reinterpret_cast<Py_ssize_t *>(&_pos),
                       reinterpret_cast<PyObject **>(_key.pHandle()),
                       reinterpret_cast<PyObject **>(_value.pHandle()));
}
ZsDict::Iterator &ZsDict::Iterator::operator++() {
  if (_valid) {
    _valid = PyDict_Next(as_ptr_<PyObject>(_dict), reinterpret_cast<Py_ssize_t *>(&_pos),
                         reinterpret_cast<PyObject **>(_key.pHandle()),
                         reinterpret_cast<PyObject **>(_value.pHandle()));
  }
  return *this;
}

/// list
sint_t ZsList::size() const {
  if (isNone())
    return -1;
  else
    return PyList_Size(static_cast<PyObject *>(_v.obj));
}

ZsObject ZsList::operator[](sint_t i) {
  auto ret = PyList_GetItem(as_ptr_<PyObject>(*this), i);
  if (!ret) PyErr_Print();
  return ret;
}
int ZsList::setItemSteal(sint_t index, ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyList_SetItem(as_ptr_<PyObject>(*this), index, as_ptr_<PyObject>(item));
    if (ret == -1) PyErr_Print();
    return ret;
  }
  return -1;
}
int ZsList::insert(sint_t index, ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyList_Insert(as_ptr_<PyObject>(*this), index, as_ptr_<PyObject>(item));
    if (ret == -1) PyErr_Print();
    return ret;
  }
  return -1;
}
int ZsList::append(ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyList_Append(as_ptr_<PyObject>(*this), as_ptr_<PyObject>(item));
    if (ret == -1) PyErr_Print();
    return ret;
  }
  return -1;
}
int ZsList::insertSteal(sint_t index, ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyList_Insert(as_ptr_<PyObject>(*this), index, as_ptr_<PyObject>(item));
    if (ret == -1)
      PyErr_Print();
    else
      Py_DECREF(item.handle());
    return ret;
  }
  return -1;
}
int ZsList::appendSteal(ZsObject item) {
  if (item._idx == zs_var_type_object) {
    int ret = PyList_Append(as_ptr_<PyObject>(*this), as_ptr_<PyObject>(item));
    if (ret == -1)
      PyErr_Print();
    else
      Py_DECREF(item.handle());
    return ret;
  }
  return -1;
}

/// set
sint_t ZsSet::size() const {
  if (isNone())
    return -1;
  else
    return PySet_Size(static_cast<PyObject *>(_v.obj));
}

ZsSet::Iterator::Iterator(ZsSet set)
    : _it{PyObject_GetIter(as_ptr_<PyObject>(set))}, _item{}, _valid{0} {
  if (_it._v.obj != nullptr) {
    _valid = 1;
    _item._v.obj = PyIter_Next(as_ptr_<PyObject>(_it));
    if (!_item._v.obj) {
      _item._v.obj = Py_None;
      if (PyErr_Occurred()) PyErr_Print();
    }
  } else
    PyErr_Print();
}
ZsSet::Iterator::~Iterator() {
  if (_item) {
    Py_DECREF(_item.handle());
    _item = ZsObject{};
  }
}
ZsSet::Iterator &ZsSet::Iterator::operator++() {
  if (_valid) {
    if (_item) Py_DECREF(_item.handle());

    _item._v.obj = PyIter_Next(as_ptr_<PyObject>(_it));
    if (!_item._v.obj) {
      _item._v.obj = Py_None;
      if (PyErr_Occurred()) PyErr_Print();
      _valid = 0;
    }
  }
  return *this;
}

#ifdef __cplusplus
}
#endif

static_assert(sizeof(long long int) == sizeof(char) * 8, "long long int is assumed 64-bit!");
static_assert(sizeof(double) == sizeof(char) * 8, "double is assumed 64-bit!");
static_assert(sizeof(int) == sizeof(char) * 4, "int is assumed 32-bit!");
static_assert(sizeof(float) == sizeof(char) * 4, "float is assumed 32-bit!");
