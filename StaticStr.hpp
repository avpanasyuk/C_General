/**
 * @file StaticStr.hpp
 * @brief RAII claim on sprintf_static()'s shared buffer, so misuse asserts instead of corrupting.
 *
 * sprintf_static() hands out a pointer into one static buffer, valid only until the next
 * call. That contract is easy to break by accident, and it fails as silently wrong output
 * rather than a crash: the second format overwrites the first caller's string mid-use.
 *
 * avp::sprintf_guarded() returns a move-only avp::StaticStr that owns the buffer for its
 * lifetime. While one is alive, any further formatting into the buffer trips the assert in
 * svprintf_static(). Because a temporary lives to the end of the full expression, the case
 * that matters is caught at the point of misuse:
 *
 *   f(avp::sprintf_guarded("a=%d", a), avp::sprintf_guarded("b=%d", b)); // asserts: two claims
 *   avp::StaticStr s = avp::sprintf_guarded("rssi=%d", r);
 *   g(s);                          // fine -- implicitly converts to const char *
 *   h(sprintf_static("x=%d", x));  // also fine while no claim is live
 *
 * Detection only, and only where NDEBUG is absent: in a release build the claim compiles to
 * nothing and this is a zero-cost wrapper around the plain pointer. It is the backstop for
 * sprintf_static-into-sprintf_static; the debug_*() collision is structurally prevented
 * instead (debug_vprintf stages into its own stack buffer -- see General.h).
 *
 * NOT thread/ISR safe by design: the claim is a single flag, matching the buffer it guards.
 */
#pragma once
#include <stdarg.h>
#include "General.h"

namespace avp {
  class StaticStr {
    const char *s_;
    bool        owns_;

    static void claim(bool on) {
#ifndef NDEBUG
      sprintf_static_claimed = on ? 1 : 0;
#else
      (void)on;
#endif
    }

   public:
    /// Takes the claim AFTER formatting -- claiming first would trip the assert on our own call.
    explicit StaticStr(const char *s) : s_(s), owns_(true) { claim(true); }
    ~StaticStr() { if(owns_) claim(false); }

    StaticStr(const StaticStr &) = delete; // one claim, one owner
    StaticStr &operator=(const StaticStr &) = delete;
    StaticStr(StaticStr &&o) noexcept : s_(o.s_), owns_(o.owns_) { o.owns_ = false; }

    operator const char *() const { return s_; }
    const char *c_str() const { return s_; }
  }; // class StaticStr

  /// sprintf_static() with the buffer claimed for the returned value's lifetime.
  inline StaticStr sprintf_guarded(const char *format, ...) __attribute__((format(printf, 1, 2)));

  inline StaticStr sprintf_guarded(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    const char *s = svprintf_static(format, ap);
    va_end(ap);
    return StaticStr(s);
  } // sprintf_guarded
} // namespace avp
