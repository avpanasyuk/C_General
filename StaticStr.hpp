/**
 * @file StaticStr.hpp
 * @brief What the C++ sprintf_static() returns: an RAII claim on the shared static buffer.
 *
 * sprintf_static() formats into one static buffer, so its result is valid only until the
 * next call. That contract is easy to break by accident and fails as silently wrong output
 * rather than a crash -- the second format overwrites the first caller's string mid-use.
 *
 * So in C++, sprintf_static() (defined at the bottom of General.h) returns a move-only
 * StaticStr that holds the buffer for its lifetime; while one is alive, any further
 * formatting into the buffer trips the assert in svprintf_static(). Since a temporary lives
 * to the end of the full expression, the dangerous cases are caught where they happen:
 *
 *   f(sprintf_static("a=%d", a), sprintf_static("b=%d", b)); // asserts: two live claims
 *   g(sprintf_static("outer %s", sprintf_static("in=%d", i))); // asserts: nested format
 *   avp::StaticStr s = sprintf_static("rssi=%d", r);         // hold it to keep the claim
 *   h(s);                                                    // fine: converts to const char *
 *
 * Two limits worth knowing. The claim lasts exactly as long as the object, so
 * `const char *p = sprintf_static(...)` releases it at the semicolon -- p is then as
 * unprotected as it was before this existed; bind an `avp::StaticStr` (or `auto`) to keep it.
 * And detection is debug-only: under NDEBUG the claim compiles to nothing and this is a
 * zero-cost wrapper around the plain pointer.
 *
 * Passing one where a CLASS type is expected (e.g. Arduino `String`) needs an explicit
 * .c_str() -- that would be two user-defined conversions, which C++ will not do implicitly.
 * Plain `const char *` parameters, and `return`ing one as `const char *`, work unchanged.
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
} // namespace avp

/// The C++ sprintf_static: same name and call syntax as the C one (which stays for C
/// translation units), but the result carries a claim on the buffer for its lifetime.
/// Deliberately at global scope so existing unqualified call sites need no change.
__attribute__((format(printf, 1, 2))) inline avp::StaticStr sprintf_static(char const *format, ...) {
  va_list ap;
  va_start(ap, format);
  const char *s = svprintf_static(format, ap); // the claim is taken after this, by StaticStr
  va_end(ap);
  return avp::StaticStr(s);
} // sprintf_static
