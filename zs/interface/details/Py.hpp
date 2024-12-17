#pragma once
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Global singleton class for managing python context
 *  @note Developer should never touch this class.
 */
struct PyEnvInitializer {
  PyObject *g_globalDict = NULL;
  PyObject *g_localDict = NULL;

  char *g_buffer = NULL;
  unsigned long long g_bufferOffset = 0;
  unsigned long long g_bufferLen = 256;

  bool needMoreInput = false;

  bool pendingInput() const noexcept { return needMoreInput; }
  void setInputPending() noexcept { needMoreInput = true; }
  void resetInputPending() noexcept { needMoreInput = false; }

  PyEnvInitializer();
  ~PyEnvInitializer();
  [[maybe_unused]] bool reserveBuffer(unsigned long long newLen,
                                      bool preserve = true);
  void appendBuffer(const char *src, unsigned long long numBytes);
  void rewindBuffer();
  char *getBuffer();
};

#ifdef __cplusplus
}
#endif