#include "PyHelper.hpp"
#include <Python.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

[[maybe_unused]] bool Buffer::reserve(unsigned long long newLen,
                                      bool preserve) {
  if (newLen > _capacity) {
    char *oldBuffer = _buffer;
    _capacity = newLen * 2;
    _buffer = (char *)malloc(sizeof(char) * _capacity);
    if (preserve && _offset)
      strncpy(_buffer, oldBuffer, _offset);
    free(static_cast<void *>(oldBuffer));
    return true;
  }
  return false;
}
void Buffer::append(const char *src, unsigned long long numBytes) {
  auto newOffset = _offset + numBytes;
  if (newOffset + 1 >= _capacity)
    reserve(newOffset + 1);
  strncpy(_buffer + _offset, src, numBytes);
  _offset = newOffset;
  _buffer[newOffset] = '\0';
}
void Buffer::rewind() { _offset = 0; }
char *Buffer::data() { return _buffer; }

void zs_print_py_cstr(const char *cstr) {
  if (auto sysStdout = PySys_GetObject("stdout")) {
    PyVar pystr = PyUnicode_FromString(cstr);
    PyFile_WriteObject((PyObject *)(void *)pystr, sysStdout, Py_PRINT_RAW);
  }
}

void zs_print_err_py_cstr(const char *cstr) {
  if (auto sysStdErr = PySys_GetObject("stderr")) {
    PyVar pystr = PyUnicode_FromString(cstr);
    // PyObject_CallMethod(sysStdErr, "write", "O", pystr.handle());
    PyFile_WriteObject(as_ptr_<PyObject>(pystr), sysStdErr, Py_PRINT_RAW);
  }
}

#ifdef __cplusplus
}
#endif

GILGuard::GILGuard() { _state = PyGILState_Ensure(); }
GILGuard::~GILGuard() {
  PyGILState_Release(static_cast<PyGILState_STATE>(_state));
}