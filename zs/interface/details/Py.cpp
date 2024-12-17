#include "Py.hpp"
#include "zensim/zpc_tpls/whereami/whereami.h"

#ifdef __cplusplus
extern "C" {
#endif

PyEnvInitializer::PyEnvInitializer() {
/// preconfig (through builder initiated from static would be better)
#if 0
    PyPreConfig preconfig;
    PyPreConfig_InitPythonConfig(&preconfig);

    preconfig.utf8_mode = 1;

    // Step 4: Apply the configuration
    PyStatus status = Py_PreInitialize(&preconfig);
    if (PyStatus_Exception(status)) {
      fprintf(stderr, "Failed to preinitialize Python: %s\n", status.err_msg);
      Py_ExitStatusException(status);
    }
#endif

  /// initialize
  Py_Initialize();

  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  ///
  PyObject *sysPath = PySys_GetObject("path");
  char *s{};
  int length, dirname_length;
  length = wai_getModulePath(NULL, 0, &dirname_length);
  s = (char *)malloc(length + 1);
  wai_getModulePath(s, length, &dirname_length);
  s[dirname_length] = '\0';
  PyList_Append(sysPath, PyUnicode_FromString(s));
  printf("\tappending py path : [%s]\n", s);
  free(s);

  /// @note
  /// https://groups.google.com/g/dev-python/c/o0e9c3kNeK4/m/q-1i77P6blwJ
  // PyImport_AddModule("__main__");
  PyObject *compiledCode = Py_CompileString(R"(
import traceback
import subprocess
import sys
from io import StringIO
zs_old_stdout = sys.stdout
zs_old_stderr = sys.stderr
sys.stdout = StringIO()
sys.stderr = StringIO()

from copy import deepcopy
zs_deepcopy = deepcopy

def zs_install(package):
	subprocess.check_call([sys.executable, "-m", "pip", "install", package])

import faulthandler
if __name__ == '__main__':
	faulthandler.enable()
)",
                                            "<initialize>", Py_file_input);
  if (compiledCode) {
    g_globalDict = PyDict_New();
    if (g_globalDict) {
      PyDict_SetItemString(g_globalDict, "__builtins__", PyEval_GetBuiltins());

      g_localDict = g_globalDict;
      if (g_localDict) {
        auto res = PyEval_EvalCode(compiledCode, g_globalDict, g_localDict);
        if (res) {
          Py_DECREF(res);
        }
      }
    }
    Py_DECREF(compiledCode);
  } else {
    PyErr_Print();
  }

  PyGILState_Release(gstate);
  g_bufferLen = 256;
  g_buffer = (char *)malloc(sizeof(char) * g_bufferLen);
}

PyEnvInitializer::~PyEnvInitializer() {
  if (g_buffer) {
    free(static_cast<void *>(g_buffer));
    g_buffer = NULL;
  }
  if (g_globalDict && g_localDict) {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    PyObject *compiledCode = Py_CompileString("sys.stdout = zs_old_stdout\n"
                                              "sys.stderr = zs_old_stderr\n",
                                              "<de-initialize>", Py_file_input);
    if (compiledCode) {
      auto res = PyEval_EvalCode(compiledCode, g_globalDict, g_localDict);
      if (res) {
        Py_DECREF(res);
      }
      Py_DECREF(compiledCode);
    }
    PyGILState_Release(gstate);
  }
  g_globalDict = nullptr;
  g_localDict = nullptr;
}

[[maybe_unused]] bool PyEnvInitializer::reserveBuffer(unsigned long long newLen,
                                                      bool preserve) {
  if (newLen > g_bufferLen) {
    char *oldBuffer = g_buffer;
    g_bufferLen = newLen * 2;
    g_buffer = (char *)malloc(sizeof(char) * g_bufferLen);
    if (preserve && g_bufferOffset)
      strncpy(g_buffer, oldBuffer, g_bufferOffset);
    free(static_cast<void *>(oldBuffer));
    return true;
  }
  return false;
}
void PyEnvInitializer::appendBuffer(const char *src,
                                    unsigned long long numBytes) {
  auto newOffset = g_bufferOffset + numBytes;
  if (newOffset + 1 >= g_bufferLen)
    reserveBuffer(newOffset + 1);
  strncpy(g_buffer + g_bufferOffset, src, numBytes);
  g_bufferOffset = newOffset;
  g_buffer[newOffset] = '\0';
}
void PyEnvInitializer::rewindBuffer() {
  g_bufferOffset = 0;
  resetInputPending();
}
char *PyEnvInitializer::getBuffer() { return g_buffer; }

#ifdef __cplusplus
}
#endif
