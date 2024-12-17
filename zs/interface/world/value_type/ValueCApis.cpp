#include "ValueInterface.hpp"
#include "interface/details/Py.hpp"
#include "interface/details/PyHelper.hpp"

#ifdef __cplusplus
extern "C" {
#endif

static PyEnvInitializer g_py_initializer;

unsigned g_zs_last_error = 0;
unsigned g_zs_last_warn = 0;

unsigned zs_last_error() {
  auto ret = g_zs_last_error;
  g_zs_last_error = 0;  // reset to success (0)
  return ret;
}
unsigned zs_last_warn() {
  auto ret = g_zs_last_warn;
  g_zs_last_warn = 0;  // reset to success (0)
  return ret;
}

ZsValuePort zs_world_handle() {
  assert(g_py_initializer.g_globalDict != NULL);
  ZsValue ret;
  ret._v.obj = g_py_initializer.g_globalDict;
  ret._idx = zs_var_type_object;
  return ret;
}
ZsValuePort zs_world_local_handle() {
  assert(g_py_initializer.g_localDict != NULL);
  ZsValue ret;
  ret._v.obj = g_py_initializer.g_localDict;
  ret._idx = zs_var_type_object;
  return ret;
}

bool zs_world_pending_input() { return g_py_initializer.pendingInput(); }

void *zs_execute_statement(const char *cmd, int *state) {
  PyObject *resultStr = nullptr;
  *state = -1;  // -1: not evaluated/executed
  // 0: success, 1: execution error, 2: pending intput, 3: other syntax error,
  // 4: unknown error, 5: request exit

  g_py_initializer.appendBuffer(cmd, strlen(cmd));
  g_py_initializer.appendBuffer("\n", 1);
  if (g_py_initializer.pendingInput()) {
    if (cmd[0] != '\0')  // do not execute yet
      return nullptr;
  } else if (cmd[0] == '\0')  // do not execute yet
    return nullptr;

  /// @ref
  /// https://stackoverflow.com/questions/78216015/issue-with-gil-on-python-3-12-2
  PyGILState_STATE gstate = PyGILState_Ensure();
  {
    // sprintf(g_py_initializer.g_buffer, "%s\n", cmd);
    if (PyVar compiledCode
        = Py_CompileString(g_py_initializer.getBuffer(), "<execute user py command>",
                           Py_single_input))  // Py_file_input, Py_eval_input, Py_single_input
    {
      /// able to evaluate the script
      if (PyVar res = PyEval_EvalCode(compiledCode, g_py_initializer.g_globalDict,
                                      g_py_initializer.g_localDict)) {
        if (auto sysStdout = PySys_GetObject("stdout")) {
          if (PyVar result = PyObject_CallMethod(sysStdout, "getvalue", NULL)) {
            resultStr = PyUnicode_AsUTF8String(result);
            if (resultStr) {
              auto resultCstr = PyBytes_AsString(resultStr);

              if (resultCstr && resultCstr[0] != '\0') {
                *state = 0;
              } else {
                Py_DECREF(resultStr);
                resultStr = nullptr;
              }
            }
            /// @ref
            /// https://stackoverflow.com/questions/4330812/how-do-i-clear-a-stringio-object
            // reset output buffer
            PyObject_CallMethod(sysStdout, "seek", "i", 0);
            PyObject_CallMethod(sysStdout, "truncate", "i", 0);
          } else
            PyErr_Print();
        } else
          PyErr_Print();
      } else if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
        PyErr_Clear();
        *state = 5;
        printf("SystemExit called, exiting script execution cleanly.\n");
      }
      // eval code error
      else {
        *state = 1;
        PyErr_Print();
      }
      g_py_initializer.rewindBuffer();
    }
    // syntax error
    else if (PyErr_ExceptionMatches(PyExc_SyntaxError)) {
      PyObject *type, *val, *tb;
      PyErr_Fetch(&type, &val, &tb);
      PyErr_NormalizeException(&type, &val, &tb);
      if (PyErr_GivenExceptionMatches(type, PyExc_SyntaxError)) {
        // PyVar msg = PyObject_GetAttrString(val, "msg");
        PyVar msg = zs_string_obj(val);
        PyVar msgStr = PyUnicode_AsUTF8String(msg);
        auto msgCstr = PyBytes_AsString(msgStr);
        /// compound type
        /// 0: : if/for/func_def
        /// 1: []
        /// 2: {}
        if (cmd[0] != '\0'
            && (!strcmp(msgCstr, "unexpected EOF while parsing")
                || !strncmp(msgCstr, "expected an indented block after", 32)
                || !strncmp(msgCstr + 2, "\' was never closed", 18))) {
          *state = 2;
          // printf("incomplete input, require more\n");
          if (!g_py_initializer.pendingInput())
            g_py_initializer.setInputPending();  // *
          else {
            PyErr_Restore(type, val, tb);  // restore error indicator
            PyErr_Print();
          }
        } else {
          // other syntax error, like "invalid syntax"
          *state = 3;
          PyErr_Restore(type, val, tb);  // restore error indicator
          PyErr_Print();
          g_py_initializer.rewindBuffer();
        }
        // printf("syntax error: %s\n", msgCstr);
      } else {
        // likely non-syntax error
        *state = 4;
        PyErr_Restore(type, val, tb);  // restore error indicator
        PyErr_Print();
        g_py_initializer.rewindBuffer();
      }
    }
    // non-syntax error
    else {
      *state = 4;
      PyErr_Print();
      g_py_initializer.rewindBuffer();
    }
  }

  /// error handling
  if (*state != -1 && *state != 0 && *state != 2) {
    if (auto sysStderr = PySys_GetObject("stderr")) {
      if (PyVar result = PyObject_CallMethod(sysStderr, "getvalue", NULL)) {
        resultStr = PyUnicode_AsUTF8String(result);

        if (resultStr) {
          auto resultCstr = PyBytes_AsString(resultStr);
          if (resultCstr && resultCstr[0] != '\0') {
            *state = 1;
            PyObject_CallMethod(sysStderr, "seek", "i", 0);
            PyObject_CallMethod(sysStderr, "truncate", "i", 0);
          } else {
            /// unable to get error string
            Py_DECREF(resultStr);
            resultStr = nullptr;
          }
        }
      }  // result
    }  // sysStderr
  }
  PyGILState_Release(gstate);

  return resultStr;
}
void *zs_eval_expr(const char *expr, void **errLogBytes) {
  if (errLogBytes) *errLogBytes = nullptr;

  if (expr[0] == '\0')  // do not execute yet
    return nullptr;

  PyObject *result = nullptr;
  {
    GILGuard gilGuard;

    if (PyVar compiledCode
        = Py_CompileString(expr, "<evaluate user py expression>", Py_eval_input)) {
      /// able to evaluate the script
      PyVar copiedDict = zs_dict_obj_copy(g_py_initializer.g_globalDict);
      if (copiedDict) {
        if (result = PyEval_EvalCode(compiledCode, copiedDict, copiedDict); !result) {
          if (PyErr_Occurred()) {
            /// @note evaluation error happened
            if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
              PyErr_Clear();
              if (errLogBytes)
                *errLogBytes
                    = ((ZsBytes)zs_bytes_obj_cstr("\'exit()\' not allowed for evaluation!"))
                          .handle();
            } else {
              if (errLogBytes) {
                PyObject *type, *val, *tb;
                PyErr_Fetch(&type, &val, &tb);
                PyErr_NormalizeException(&type, &val, &tb);
                PyVar msg = zs_string_obj(val);
                // PyVar msg = PyObject_GetAttrString(val, "msg");
                if (msg) *errLogBytes = PyUnicode_AsUTF8String(msg);
                PyErr_Restore(type, val, tb);  // restore error indicator
              }
              PyErr_Clear();
            }
          }
        }
      } else {
        if (errLogBytes)
          *errLogBytes
              = ((ZsBytes)zs_bytes_obj_cstr("unable to preserve global dict for eval.")).handle();
      }
    } else {
      if (PyErr_Occurred()) {
        /// @note compilation error happend
        if (errLogBytes) {
          PyObject *type, *val, *tb;
          PyErr_Fetch(&type, &val, &tb);
          PyErr_NormalizeException(&type, &val, &tb);
          PyVar msg = zs_string_obj(val);
          // PyVar msg = PyObject_GetAttrString(val, "msg");
          if (msg) *errLogBytes = PyUnicode_AsUTF8String(msg);
          PyErr_Restore(type, val, tb);  // restore error indicator
        }
        PyErr_Clear();
      }
    }
  }  // GIL context

  return result;
}
void *zs_execute_script(const char *cmd, int *state) {
  PyObject *resultStr = nullptr;
  *state = -1;  // -1: not evaluated/executed
  // 0: success, 1: execution error, 2: compilation error,
  // 5: request exit

  if (cmd[0] == '\0')  // do not execute yet
    return nullptr;

  /// @ref
  /// https://stackoverflow.com/questions/78216015/issue-with-gil-on-python-3-12-2
  PyGILState_STATE gstate = PyGILState_Ensure();
  {
    // printf("compiling %s\n", cmd);
    // sprintf(g_py_initializer.g_buffer, "%s\n", cmd);
    if (PyVar compiledCode
        = Py_CompileString(cmd, "<execute user py file script>",
                           Py_file_input))  // Py_file_input, Py_eval_input, Py_single_input
    {
      /// able to evaluate the script
      if (PyVar res = PyEval_EvalCode(compiledCode, g_py_initializer.g_globalDict,
                                      g_py_initializer.g_localDict)) {
        if (auto sysStdout = PySys_GetObject("stdout")) {
          if (PyVar result = PyObject_CallMethod(sysStdout, "getvalue", NULL)) {
            resultStr = PyUnicode_AsUTF8String(result);
            if (resultStr) {
              auto resultCstr = PyBytes_AsString(resultStr);

              *state = 0;
              if (resultCstr && resultCstr[0] != '\0') {
                /// @ref
                /// https://stackoverflow.com/questions/4330812/how-do-i-clear-a-stringio-object
                // reset output buffer
                PyObject_CallMethod(sysStdout, "seek", "i", 0);
                PyObject_CallMethod(sysStdout, "truncate", "i", 0);
              } else {
                Py_DECREF(resultStr);
                resultStr = nullptr;
              }
            } else
              PyErr_Print();
          } else
            PyErr_Print();
        } else
          PyErr_Print();
      } else if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
        PyErr_Clear();
        *state = 5;
      }
      // evaluation error
      else {
        *state = 1;
        PyErr_Print();
      }
    }
    // compilation error
    else {
      *state = 2;
      PyErr_Print();
    }
  }

  /// error handling
  if (*state != -1 && *state != 0) {
    if (auto sysStderr = PySys_GetObject("stderr")) {
      if (PyVar result = PyObject_CallMethod(sysStderr, "getvalue", NULL)) {
        resultStr = PyUnicode_AsUTF8String(result);

        if (resultStr) {
          auto resultCstr = PyBytes_AsString(resultStr);
          if (resultCstr && resultCstr[0] != '\0') {
            *state = 1;
            PyObject_CallMethod(sysStderr, "seek", "i", 0);
            PyObject_CallMethod(sysStderr, "truncate", "i", 0);
          } else {
            /// unable to get error string
            Py_DECREF(resultStr);
            resultStr = nullptr;
          }
        }
      }  // result
    }  // sysStderr
  }
  PyGILState_Release(gstate);

  return resultStr;
}

///
/// global apis
///
void zs_default_init_val(ZsValue *val, ZsValue arg) {
  val->_v = arg._v;
  val->_idx = arg._idx;
}
bool zs_default_init_obj(ZsValue *obj, zs_obj_type_ objType, ZsValue arg) {
  switch (objType) {
    case zs_obj_type_bytes: {
      *obj = ZsValue(zs_bytes_obj(arg));
      return obj->_idx != zs_var_type_none;
    }
    case zs_obj_type_string: {
      if (PyVar pybytes = zs_bytes_obj(arg)) {
        *obj = ZsValue(zs_string_obj_cstr(pybytes.getValue().asBytes().c_str()));
      } else
        obj->_idx = zs_var_type_none;
      return obj->_idx != zs_var_type_none;
    }

    case zs_obj_type_tuple:
      *obj = ZsValue(zs_tuple_obj(arg));
      return obj->_idx != zs_var_type_none;
      ///
      /// TBD
      ///
    case zs_obj_type_custom:
      obj->_v.obj = static_cast<void *>(arg);
      obj->_idx = obj->_v.obj ? zs_var_type_object : zs_var_type_none;
      return obj->_idx != zs_var_type_none;
    default:;
  }
  return false;
}
void zs_default_deinit(ZsValue *var) {
  if (var->_idx == zs_var_type_object) {
    Py_DECREF(static_cast<PyObject *>(var->_v.obj));
    var->_idx = zs_var_type_none;
  }
}
ZsValuePort zs_default_clone(ZsValue v) {
  if (v._idx == zs_var_type_object) {
    auto original = static_cast<PyObject *>(v._v.obj);
    auto deepcopyFunction = PyDict_GetItemString(g_py_initializer.g_globalDict, "zs_deepcopy");
    auto copied = PyObject_CallFunctionObjArgs(deepcopyFunction, original, NULL);
    if (copied) {
      return zs_obj(copied);
    } else {
      PyErr_Print();
      return ZsValue{};
    }
  } else {
    return v;
  }
}
ZsValuePort zs_default_share(ZsValue var) {
  ZsValue ret = var;
  if (var._idx == zs_var_type_object) Py_INCREF(static_cast<PyObject *>(var._v.obj));
  return ret;
}
bool zs_default_is(ZsValue l, ZsValue r) {
  if (l._idx == zs_var_type_object && r._idx == zs_var_type_object) {
    auto lobj = static_cast<PyObject *>(l._v.obj);
    auto robj = static_cast<PyObject *>(r._v.obj);
    return Py_Is(lobj, robj);
  }
  /// @note literals are never the same
  return false;
}
bool zs_default_eq_comp_lobj_with_r(const ZsValue &l, const ZsValue &r) {
  switch (r._idx) {
      // cstr
    case zs_var_type_cstr: {
      PyVar lPyBytes = zs_bytes_obj(l);
      if (!lPyBytes) return false;
      auto lStr = lPyBytes.getValue().asBytes().c_str();
      if (!lStr) return false;
      const char *a = lStr, *b = r._v.cstr;
      for (; *a != '\0' && *b != '\0'; a++, b++)
        if (*a != *b) return false;
      return *a == *b;  // both '\0'
    }
    // integral
    case zs_var_type_i8:
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i8);
        auto lInt = static_cast<long long int>(l);
        return lInt == rInt;
      }
    case zs_var_type_i64:
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i64);
        auto lInt = static_cast<long long int>(l);
        return lInt == rInt;
      }
    case zs_var_type_i32: {
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i32);
        auto lInt = static_cast<long long int>(l);
        return lInt == rInt;
      }
      return false;
    }
    // floating point
    case zs_var_type_f64:;
      if (l.isFloatingPoint()) {
        auto rReal = static_cast<double>(r._v.f64);
        auto lReal = static_cast<double>(l);
        return lReal == rReal;
      }
    case zs_var_type_f32: {
      if (l.isFloatingPoint()) {
        auto rReal = static_cast<double>(r._v.f32);
        auto lReal = static_cast<double>(l);
        return lReal == rReal;
      }
      return false;
    }
    default:
      return false;
  }
  return false;
}
bool zs_default_eq(ZsValue l, ZsValue r) {
  // -1 error, 0 false, 1 otherwise
  if (l._idx == zs_var_type_object && r._idx == zs_var_type_object) {
    auto lobj = static_cast<PyObject *>(l._v.obj);
    auto robj = static_cast<PyObject *>(r._v.obj);
    if (Py_TYPE(lobj) == Py_TYPE(robj)) {
      int res = PyObject_RichCompareBool(lobj, robj, Py_EQ);
      if (res == -1) {
        PyErr_Print();
      }
      return res == 1;
    } else
      return false;
  } else {
    // none
    if (l._idx == zs_var_type_none || r._idx == zs_var_type_none) return false;

    if (l._idx == zs_var_type_object) {
      return zs_default_eq_comp_lobj_with_r(l, r);
    } else if (r._idx == zs_var_type_object) {
      return zs_default_eq_comp_lobj_with_r(r, l);
    } else {
      /// both fundamental
      if (l._idx == zs_var_type_cstr && r._idx == zs_var_type_cstr) {
        return l._v.cstr == r._v.cstr;
      }
      if ((l.isFloatingPoint() && r.isIntegral()) || (r.isFloatingPoint() && l.isIntegral())
          || l.isFloatingPoint() == r.isFloatingPoint()) {
        auto lReal = static_cast<double>(l);
        auto rReal = static_cast<double>(r);
        return lReal == rReal;
      } else if (l.isIntegral() && r.isIntegral()) {
        auto lInt = static_cast<long long int>(l);
        auto rInt = static_cast<long long int>(r);
        return lInt == rInt;
      }
    }
    return false;
  }
}
bool zs_default_ne_comp_lobj_with_r(const ZsValue &l, const ZsValue &r) {
  switch (r._idx) {
      // cstr
    case zs_var_type_cstr: {
      PyVar lPyBytes = zs_bytes_obj(l);
      if (!lPyBytes) return false;
      auto lStr = lPyBytes.getValue().asBytes().c_str();
      if (!lStr) {
        return true;
      }
      const char *a = lStr, *b = r._v.cstr;
      for (; *a != '\0' && *b != '\0'; a++, b++)
        if (*a != *b) return true;
      return *a != *b;  // both '\0'
    }
    // integral
    case zs_var_type_i8:
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i8);
        auto lInt = static_cast<long long int>(l);
        return lInt != rInt;
      }
    case zs_var_type_i64:
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i64);
        auto lInt = static_cast<long long int>(l);
        return lInt != rInt;
      }
    case zs_var_type_i32: {
      if (l.isIntegral()) {
        auto rInt = static_cast<long long int>(r._v.i32);
        auto lInt = static_cast<long long int>(l);
        return lInt != rInt;
      }
      return true;
    }
    // floating point
    case zs_var_type_f64:;
      if (l.isFloatingPoint()) {
        auto rReal = static_cast<double>(r._v.f64);
        auto lReal = static_cast<double>(l);
        return lReal != rReal;
      }
    case zs_var_type_f32: {
      if (l.isFloatingPoint()) {
        auto rReal = static_cast<double>(r._v.f32);
        auto lReal = static_cast<double>(l);
        return lReal != rReal;
      }
      return true;
    }
    default:
      return false;
  }
  return true;
}
bool zs_default_ne(ZsValue l, ZsValue r) {
  // -1 error, 0 false, 1 otherwise
  if (l._idx == zs_var_type_object && r._idx == zs_var_type_object) {
    auto lobj = static_cast<PyObject *>(l._v.obj);
    auto robj = static_cast<PyObject *>(r._v.obj);
    if (Py_TYPE(lobj) == Py_TYPE(robj)) {
      int res = PyObject_RichCompareBool(lobj, robj, Py_NE);
      if (res == -1) {
        PyErr_Print();
      }
      return res == 1;
    } else
      return false;
  } else {
    // none
    if (l._idx == zs_var_type_none || r._idx == zs_var_type_none) return true;

    if (l._idx == zs_var_type_object) {
      return zs_default_ne_comp_lobj_with_r(l, r);
    } else if (r._idx == zs_var_type_object) {
      return zs_default_ne_comp_lobj_with_r(r, l);
    } else {
      /// both fundamental
      if (l._idx == zs_var_type_cstr && r._idx == zs_var_type_cstr) {
        return l._v.cstr != r._v.cstr;
      }
      if ((l.isFloatingPoint() && r.isIntegral()) || (r.isFloatingPoint() && l.isIntegral())
          || l.isFloatingPoint() == r.isFloatingPoint()) {
        auto lReal = static_cast<double>(l);
        auto rReal = static_cast<double>(r);
        return lReal != rReal;
      } else if (l.isIntegral() && r.isIntegral()) {
        auto lInt = static_cast<long long int>(l);
        auto rInt = static_cast<long long int>(r);
        return lInt != rInt;
      }
    }
    return true;
  }
}
void zs_default_reflect(ZsValue var) { zs_reflect_value(var); }
long long int zs_default_refcnt(ZsValue val) {
  if (val._idx == zs_var_type_object) return Py_REFCNT(static_cast<PyObject *>(val._v.obj));
  return 1;
}
ZsVarOps g_zs_variable_apis = {
    zs_default_init_val, zs_default_init_obj, zs_default_deinit, zs_default_clone,
    zs_default_share,    zs_default_is,       zs_default_eq,     zs_default_ne,
    zs_default_reflect,  zs_default_refcnt,
};

ZsValueOps g_zs_value_apis = {};

#ifdef __cplusplus
}
#endif