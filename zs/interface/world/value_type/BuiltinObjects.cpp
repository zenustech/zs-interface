#include "ValueInterface.hpp"
#include "interface/details/Py.hpp"
#include "interface/details/PyHelper.hpp"

#ifdef __cplusplus
extern "C" {
#endif

ZsValuePort zs_obj_new_ref(ZsValue val) {
  if (val.isObject()) {
    assert(val._v.obj != nullptr && "obj ptr should not be nullptr");
    Py_INCREF(val._v.obj);
  }
  return val;
}

///
/// bool
///
ZsValuePort zs_bool_obj(bool b) {
  if (b) return zs_obj(Py_True);
  return zs_obj(Py_False);
}

///
/// module
///
ZsValuePort zs_module_cstr(const char *name) {
  if (!name) return zs_obj(Py_None);
  if (auto ret = PyModule_New(name)) return zs_obj(ret);
  return zs_obj(Py_None);
}

///
/// bytes
///
ZsValuePort zs_bytes_obj_cstr(const char *cstr) {
  if (!cstr) return zs_obj(Py_None);
  if (auto ret = PyBytes_FromString(cstr)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_bytes_obj_cstr_range(const char *st, sint_t len) {
  if (auto ret = PyBytes_FromStringAndSize(st, len)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_bytes_obj(ZsValue args) {
  if (args._idx == zs_var_type_cstr) {
    return zs_bytes_obj_cstr(args._v.cstr);
  } else if (args._idx == zs_var_type_object) {
    PyObject *pyobj = static_cast<PyObject *>(args._v.obj);
    PyTypeObject *tp = Py_TYPE(pyobj);
    if (tp == &PyUnicode_Type) {
      PyObject *str = PyUnicode_AsUTF8String(pyobj);
      if (str) {
        return ZsBytes(str);
      } else {
        PyErr_Print();
      }
    } else if (tp == &PyByteArray_Type) {
      Py_INCREF(args._v.obj);
      return args;
    } else if (tp == &PyBytes_Type) {
      Py_INCREF(args._v.obj);
      return args;
    }
  }
  zs_print_err_py_cstr("[zs_bytes_obj]: no proper bytes representation");
  return zs_obj(Py_None);
}

///
/// bytearray
///
ZsValuePort zs_bytearray_obj_cstr_range(const char *st, sint_t len) {
  if (auto ret = PyByteArray_FromStringAndSize(st, len)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_bytearray_obj_cstr(const char *str) {
  return zs_bytearray_obj_cstr_range(str, strlen(str));
}

///
/// string
///
ZsValuePort zs_string_obj(ZsValue obj) {
  if (obj.isObject()) {
    if (obj._v.obj)
      if (auto ret = PyObject_Str(static_cast<PyObject *>(obj._v.obj))) return zs_obj(ret);
  } else {
    switch (obj._idx) {
      case zs_var_type_cstr:
        return zs_string_obj_cstr(obj._v.cstr);
      case zs_var_type_i64:
        return zs_string_obj_long_long(obj._v.i64);
      case zs_var_type_f64:
        return zs_string_obj_double(obj._v.f64);
      case zs_var_type_i32:
        return zs_string_obj_long_long(obj._v.i32);
      case zs_var_type_f32:
        return zs_string_obj_double(obj._v.f32);
      case zs_var_type_i8:
        return zs_string_obj_long_long(obj._v.i8);
      default:;
    }
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_string_obj_long_long(long long n) {
  if (auto ret = PyUnicode_FromFormat("%lld", n)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_string_obj_double(double f) {
  if (auto ret = PyUnicode_FromFormat("%f", f)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_string_obj_cstr(const char *cstr) {
  if (!cstr) return zs_obj(Py_None);
  if (auto ret = PyUnicode_FromString(cstr)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_string_obj_repr(ZsValue obj) {
  if (obj.isObject()) {
    if (obj._v.obj)
      if (auto ret = PyObject_Repr(static_cast<PyObject *>(obj._v.obj))) return zs_obj(ret);
  } else {
    switch (obj._idx) {
      case zs_var_type_cstr:
        return zs_string_obj_cstr(obj._v.cstr);
      case zs_var_type_i64:
        return zs_string_obj_long_long(obj._v.i64);
      case zs_var_type_f64:
        return zs_string_obj_double(obj._v.f64);
      case zs_var_type_i32:
        return zs_string_obj_long_long(obj._v.i32);
      case zs_var_type_f32:
        return zs_string_obj_double(obj._v.f32);
      case zs_var_type_i8:
        return zs_string_obj_long_long(obj._v.i8);
      default:;
    }
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_string_obj_type(ZsValue obj) {
  if (obj.isObject()) {
    if (obj)
      if (auto ret = PyObject_GetAttrString(
              (PyObject *)Py_TYPE(static_cast<PyObject *>(obj._v.obj)), "__name__"))
        return zs_obj(ret);
  } else {
    switch (obj._idx) {
      case zs_var_type_cstr:
        return zs_string_obj_cstr("zs_cstr");
      case zs_var_type_i64:
        return zs_string_obj_cstr("zs_i64");
      case zs_var_type_f64:
        return zs_string_obj_cstr("zs_f64");
      case zs_var_type_i32:
        return zs_string_obj_cstr("zs_i32");
      case zs_var_type_f32:
        return zs_string_obj_cstr("zs_f32");
      case zs_var_type_i8:
        return zs_string_obj_cstr("zs_i8");
      default:;
    }
  }
  return zs_obj(Py_None);
}

///
/// numeric
///
// floating point
ZsValuePort zs_float_obj_double(double f) {
  if (auto ret = PyFloat_FromDouble(f)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_float_obj_str(ZsValue str) {
  if (str._idx == zs_var_type_cstr) {
    auto pystr = zs_string_obj_cstr(str._v.cstr);
    if (auto ret = PyFloat_FromString(as_ptr_<PyObject>(pystr._v.obj))) {
      Py_DECREF(pystr._v.obj);
      return zs_obj(ret);
    }
  } else if (auto ret = PyFloat_FromString(as_ptr_<PyObject>(str)))
    return zs_obj(ret);
  return zs_obj(Py_None);
}
// integral
ZsValuePort zs_long_obj_double(double f) {
  if (auto ret = PyLong_FromDouble(f)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_long_obj_long_long(long long n) {
  if (auto ret = PyLong_FromLongLong(n)) return zs_obj(ret);
  return zs_obj(Py_None);
}
ZsValuePort zs_long_obj_str(ZsValue str) {
  if (str._idx == zs_var_type_cstr) {
    if (auto ret = PyLong_FromString(str._v.cstr, NULL, 0)) return zs_obj(ret);
  } else {
    ZsBytes pystr = zs_bytes_obj(str);
    if (pystr) {
      if (auto ret = PyLong_FromString(static_cast<const char *>(pystr), NULL, 0)) {
        Py_DECREF(static_cast<void *>(pystr));
        return zs_obj(ret);
      } else {
        PyErr_Print();
      }
      Py_DECREF(static_cast<void *>(pystr));
    }
  }
  return zs_obj(Py_None);
}

///
/// tuple
///
ZsValuePort zs_tuple_obj(ZsValue arg) {
  if (arg.isTuple()) {
    ZsTuple ret = g_zs_variable_apis.clone(arg);
    return ret;
  } else if (arg.isList()) {
    ZsList &l = arg.asList();
  } else if (arg.isSet()) {
    ZsSet &l = arg.asSet();
  } else if (arg.isFloatingPoint()) {
    ;
  } else if (arg.isIntegral()) {
    long long int len = static_cast<long long int>(arg);
    return zs_tuple_obj_default(len);
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_tuple_obj_pack_ptrs(sint_t count, ...) {
  if (auto tup = PyTuple_New(count)) {
    va_list args;
    va_start(args, count);
    for (sint_t i = 0; i < count; ++i) {
      PyObject *nextArg = va_arg(args, PyObject *);
      auto ret = PyTuple_SetItem(tup, i, nextArg);
      assert(ret == 0);
    }
    va_end(args);
    return zs_obj(tup);
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_tuple_obj_pack_zsobjs(sint_t count, ...) {
  if (auto tup = PyTuple_New(count)) {
    va_list args;
    va_start(args, count);
    for (sint_t i = 0; i < count; ++i) {
      ZsValuePort nextArg = va_arg(args, ZsValuePort);
      auto ret = PyTuple_SetItem(tup, i, static_cast<PyObject *>(nextArg._v.obj));
      assert(ret == 0);
    }
    va_end(args);
    return zs_obj(tup);
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_tuple_obj_default(long len) {
  ZsTuple ret;
  if ((ret._v.obj = PyTuple_New(len))) {
    for (int i = 0; i < len; ++i)
      if (PyTuple_SetItem(static_cast<PyObject *>(ret._v.obj), i, PyLong_FromLong(-1)) == -1) {
        PyErr_Print();
        Py_DECREF(ret.handle());
        return zs_obj(Py_None);
      }
    ret._idx = zs_var_type_object;
    return ret;
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_tuple_obj_long(long n) {
  ZsTuple ret;
  if ((ret._v.obj = PyTuple_New(1))) {
    if (PyTuple_SetItem(static_cast<PyObject *>(ret._v.obj), 0, PyLong_FromLong(n)) == -1) {
      PyErr_Print();
      Py_DECREF(ret.handle());
      return zs_obj(Py_None);
    }
    ret._idx = zs_var_type_object;
    return ret;
  }
  return zs_obj(Py_None);
}

///
/// dict
///
ZsValuePort zs_dict_obj_default() {
  ZsDict ret;
  if ((ret._v.obj = PyDict_New())) {
    ret._idx = zs_var_type_object;
    return ret;
  }
  return zs_obj(Py_None);
}
ZsValuePort zs_dict_obj_copy(ZsDict dict) {
  ZsDict ret;
  if ((ret._v.obj = PyDict_Copy(as_ptr_<PyObject>(dict)))) return ret;
  PyErr_Print();
  return zs_obj(Py_None);
}

///
/// list
///
ZsValuePort zs_list_obj_default() {
  ZsList ret;
  if ((ret._v.obj = PyList_New(0))) {
    ret._idx = zs_var_type_object;
    return ret;
  }
  return zs_obj(Py_None);
}

///
/// set
///
ZsValuePort zs_set_obj_default() {
  ZsSet ret;
  if ((ret._v.obj = PySet_New(NULL))) {
    ret._idx = zs_var_type_object;
    return ret;
  }
  return zs_obj(Py_None);
}

#ifdef __cplusplus
}
#endif

PyVar::PyVar(ZsValuePort handle) noexcept
    : _obj{handle._idx == zs_var_type_object ? (handle._v.obj != Py_None ? handle._v.obj : nullptr)
                                             : nullptr} {
  // assert(handle._idx == zs_var_type_object);
}
PyVar::~PyVar() {
  Py_XDECREF(_obj);
  _obj = nullptr;
}
PyVar::PyVar(const PyVar &o) {
  if ((_obj = o._obj)) Py_INCREF(_obj);
}
PyVar &PyVar::operator=(PyVar &&o) {
  Py_XDECREF(_obj);
  _obj = o._obj;
  o._obj = nullptr;
  return *this;
}
PyVar &PyVar::operator=(const PyVar &o) {
  Py_XDECREF(_obj);
  if ((_obj = o._obj)) Py_INCREF(_obj);
  return *this;
}

#define PY_VAR_DEFINE_COMPARATOR(SYM, TAG)                                              \
  bool PyVar::operator SYM(const PyVar &o) const {                                      \
    if (auto ret = PyObject_RichCompareBool((PyObject *)_obj, (PyObject *)o._obj, TAG); \
        ret == 1) {                                                                     \
      return true;                                                                      \
    } else {                                                                            \
      if (ret == -1) PyErr_Print();                                                     \
      return false;                                                                     \
    }                                                                                   \
  }

PY_VAR_DEFINE_COMPARATOR(<, Py_LT)
PY_VAR_DEFINE_COMPARATOR(<=, Py_LE)
PY_VAR_DEFINE_COMPARATOR(==, Py_EQ)
PY_VAR_DEFINE_COMPARATOR(!=, Py_NE)
PY_VAR_DEFINE_COMPARATOR(>, Py_GT)
PY_VAR_DEFINE_COMPARATOR(>=, Py_GE)

#undef PY_VAR_DEFINE_COMPARATOR

void *PyVar::pytype() const noexcept {
  if (_obj)
    return Py_TYPE(_obj);
  else
    return nullptr;
}

PyVar PyVar::operator()() {
  PyVar ret = PyObject_CallNoArgs((PyObject *)handle());
  if (!ret) PyErr_Print();
  return ret;
}
PyVar PyVar::operator()(ZsObject obj) {
  PyVar ret = PyObject_CallOneArg((PyObject *)handle(), as_ptr_<PyObject>(obj));
  if (!ret) PyErr_Print();
  return ret;
}
PyVar PyVar::callObject(ZsTuple args) {
  PyVar ret = PyObject_CallObject((PyObject *)handle(), as_ptr_<PyObject>(args));
  if (!ret) PyErr_Print();
  return ret;
}

PyVar PyVar::operator()(const char *method) {
  if (PyVar callable = attr(method)) return callable();
  fprintf(stderr, "method [%s] does not exist.\n", method);
  return NULL;
}
PyVar PyVar::operator()(const char *method, ZsObject obj) {
  if (PyVar callable = attr(method)) return callable(obj);
  fprintf(stderr, "method [%s] does not exist.\n", method);
  return NULL;
}
PyVar PyVar::callMethod(const char *method, ZsTuple args) {
  if (PyVar callable = attr(method)) return callable.callObject(args);
  fprintf(stderr, "method [%s] does not exist.\n", method);
  return NULL;
}

bool PyVar::hasAttr(const char *name) { return PyObject_HasAttrString((PyObject *)handle(), name); }
bool PyVar::setAttr(const char *name, ZsObject attr) {
  if (!attr) return false;
  if (PyObject_SetAttrString((PyObject *)handle(), name, as_ptr_<PyObject>(attr)) == 0) return true;
  PyErr_Print();
  return false;
}
bool PyVar::delAttr(const char *name) {
  return PyObject_DelAttrString((PyObject *)handle(), name) != -1;
}
PyVar PyVar::attr(const char *name) {
  PyVar ret = PyObject_GetAttrString((PyObject *)handle(), name);
  if (!ret) PyErr_Print();
  return ret;
}

bool PyVar::hasItem(const char *name) {
  PyVar str = zs_string_obj_cstr(name);
  PyVar ret = PyObject_GetItem((PyObject *)handle(), str);
  return ret;
}
bool PyVar::setItem(const char *name, ZsObject item) {
  PyVar str = zs_string_obj_cstr(name);
  if (PyObject_SetItem((PyObject *)handle(), str, as_ptr_<PyObject>(item)) == 0) return true;
  PyErr_Print();
  return false;
}
bool PyVar::delItem(const char *name) {
  PyVar str = zs_string_obj_cstr(name);
  if (PyObject_DelItem((PyObject *)handle(), str) != -1) return true;
  return false;
}
PyVar PyVar::item(const char *name) {
  PyVar str = zs_string_obj_cstr(name);
  PyVar ret = PyObject_GetItem((PyObject *)handle(), str);
  if (!ret) PyErr_Print();
  return ret;
}

PyVar PyVar::repr() { return zs_string_obj_repr(ZsObject{_obj}); }
PyVar PyVar::str() { return PyObject_Str(static_cast<PyObject *>(_obj)); }
PyVar PyVar::bytes() {
  if (!_obj) return Py_None;
  auto ret = PyObject_Dir(static_cast<PyObject *>(_obj));
  if (!ret) {
    if (PyErr_Occurred()) PyErr_Print();
  }
  return ret;
}

/// object