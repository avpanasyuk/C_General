# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

`README.md` documents the public API (header inventory, gotchas) — read it first. This file covers
what the headers themselves don't say.

## There is no build here — verify by compiling

The library is header-only apart from `common_c.c` / `common_cpp.cpp`; it has no build system, no
tests, and is always consumed as a submodule inside a project (PlatformIO `src/`, EmBitz, Atmel
Studio via the checked-in `*.cppproj`). So the only local verification of an edit is a syntax check:

```powershell
g++ -std=c++17 -fsyntax-only -I. -x c++ <Header>.hpp     # single header
g++ -std=c++17 -fsyntax-only -I. -c common_c.c common_cpp.cpp
```

`g++` (msys2 ucrt64), `clang++` and `clang-format` are on PATH. Real confidence still requires
building a consumer project (`pio run` in the project that vendors this submodule) — a header can
pass a host syntax check and break on AVR/ESP/STM32.

**Not every header compiles standalone today.** Cleanly syntax-checking a header you didn't touch is
not a prerequisite; don't "fix" these as a side quest, but don't be surprised by them either:

- Platform-dependent by design (`Port.hpp`, `Protocol.hpp` need STM32 HAL; `CommandChain.hpp`).
- Genuinely missing includes / rotted: `BitBang.hpp` (no `Macros.hpp`), `OutStream.hpp` (no
  `<stdint.h>`), `VarArray.hpp`, `TriggerVar.hpp`, `DoubleLinearBuffer.hpp` (`null_ptr`),
  `ChainAsTemplate.hpp`, `Chain.hpp` (`#include <../C_General/Error.h>` — everything else uses plain
  `#include "Error.h"`; keep it that way, the vendored directory name varies per project).

`.clang-format` (LLVM base, 2-space, 100 cols, short-ifs on one line) is authoritative — run
`clang-format -i` on files you touch, and note `SortIncludes: Never`: include order is deliberate.

## Cross-cutting invariants

**Weak-symbol override layer.** Everything in `Error.h` (`debug_putchar`/`debug_puts`/`debug_vprintf`/
`debug_printf`/`hang_cpu`/`new_handler`) is defined `__weak` in `common_c.c` so a consumer can replace
any of them at link time. MSVC has no `__weak`, so that file carries a parallel
`#pragma comment(linker, "/alternatename:...")` block — **any change to a weak default must be made in
both branches** or MSVC builds silently lose it.

**Two build-flag dimensions gate large parts of the code:**
- `NO_STL=1` (embedded default) compiles out `std::string`/STL paths in `Error.hpp`, `General.hpp`,
  `MyTime.hpp`, `millis_micros.hpp`, `common_cpp.cpp`. Anything new touching STL needs the guard.
- `NDEBUG` removes the `StaticStr` claim tracking (`sprintf_static_claimed`) and turns `AVP_ASSERT`
  into a bare expression evaluation — **the expression still runs, the check does not**. Never put a
  side-effect-free check in `AVP_ASSERT` expecting it to catch anything in release.

**The shared `sprintf_static` buffer** (256 B static in `common_c.c`) is the library's one piece of
global mutable state, and its safety rests on two coupled facts: `debug_vprintf` formats into its own
stack buffer (`DEBUG_PRINTF_BUFFER_SIZE`) so debug output can never clobber a live result, and
`avp::StaticStr` (returned by the C++ `sprintf_static`) claims the buffer for its lifetime so reuse
asserts. Breaking either — e.g. routing `debug_vprintf` back through `svprintf_static` to save RAM —
reintroduces silent corruption *and* makes the assert re-enter. `General.h` and `StaticStr.hpp`
include each other; the cycle resolves only in the current order, so don't reshuffle those includes.

## Repo facts

- Remote: `GitHub` only (`github.com/avpanasyuk/C_General`) — the `HOME` bsd mirror the README names
  is **not configured** as a remote here.
- `doxyfile` is stale: it holds absolute `c:\GIT_REPS\PROJECTS\JefCore\...` input paths and pre-rename
  `.h` filenames (`Time.h`, `Math.h`, `Vector.h`) that no longer exist. Don't run it expecting output;
  regenerate its INPUT list if doxygen is actually needed.
</content>
</invoke>
