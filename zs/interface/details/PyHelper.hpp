#pragma once
#include "interface/world/value_type/ValueInterface.hpp"

#ifdef __cplusplus
extern "C" {
#endif

struct Buffer {
  [[maybe_unused]] bool reserve(unsigned long long newLen,
                                bool preserve = true);
  void append(const char *src, unsigned long long numBytes);
  void rewind();
  char *data();

  char *_buffer;
  unsigned long long _offset = 0;
  unsigned long long _capacity = 0;
};

ZS_INTERFACE_EXPORT void zs_print_py_cstr(const char *cstr);
ZS_INTERFACE_EXPORT void zs_print_err_py_cstr(const char *cstr);

#ifdef __cplusplus
}
#endif

/**
 *  @brief RAII guard for holding current thread's GIL
 *  On construction, call PyGILState_Ensure().
 *  On destruction, call PyGILState_Release() on the handled returned during construction.
 */
struct ZS_INTERFACE_EXPORT GILGuard {
  GILGuard();
  ~GILGuard();
  GILGuard(const GILGuard &) = delete;
  GILGuard(GILGuard &&) = delete;

  int _state;
};