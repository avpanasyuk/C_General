---
name: sprintf-static-unguarded-by-design
description: sprintf_static's shared buffer is deliberately unguarded — an RAII claim was tried and rejected; don't propose it again
metadata:
  type: project
---

`sprintf_static()` hands out a pointer into one shared 512 B static buffer (`BUFFER_SIZE` in `common_c.c`) and is
**deliberately left unguarded**. An RAII guard (`avp::StaticStr`, a move-only claim +
assert) was implemented and then removed on 2026-07-20.

**Why:** the guard's lifetime ends at the semicolon of
`const char *p = sprintf_static(...)`, so it is blind to the failure that actually
occurs — holding `p` across a later `sprintf_static()` call. It caught only the narrow
same-full-expression case, while forcing an explicit `.c_str()` at every call site
passing the result where a class type is expected (Arduino `String`, varargs).

**How to apply:** do not re-propose an RAII/claim wrapper for this buffer — it has been
evaluated and rejected on the merits. The contract is "consume the result before the
next call." Any future guard must catch the cross-statement case to be worth anything.

The genuinely effective protection is separate and must stay: `debug_vprintf` formats
into its own per-call stack buffer (`DEBUG_PRINTF_BUFFER_SIZE`), so no `debug_*()` call
can clobber a live `sprintf_static()` result. Routing it back through `svprintf_static`
to save RAM reintroduces a corruption bug that cost real field debugging on the ESP8266
plug fleet (commit 9d164ff).
