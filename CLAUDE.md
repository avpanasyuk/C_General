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

`common_c.c` also has an MSVC branch (the `_MSC_VER` `/alternatename` block — see the weak-symbol note
below); GCC builds never exercise it, so verify it with clang-cl after touching that file or the
`Error.h` declarations it depends on:

```powershell
& 'C:\Program Files\LLVM\bin\clang-cl.exe' /I. /D_CRT_SECURE_NO_WARNINGS /c common_c.c /Fo:D:\TEMP\General\
```

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
- `NDEBUG` turns `AVP_ASSERT` into a bare expression evaluation — **the expression still runs, the
  check does not**. Never put a side-effect-free check in `AVP_ASSERT` expecting it to catch anything
  in release.

**The shared `sprintf_static` buffer** (`BUFFER_SIZE`, 512 B static in `common_c.c` — 256 truncated a
real SDP810 status page) is the library's one piece of
global mutable state, and it is unguarded by design: the result is valid only until the next call, and
a caller holding it past one is on its own. An RAII claim was tried and removed — the lifetime of the
guard object ends at the semicolon of `const char *p = sprintf_static(...)`, so it cannot see the case
that actually bites, while costing every `String` call site an explicit `.c_str()`. The one protection
that does hold is structural: `debug_vprintf` formats into its own stack buffer
(`DEBUG_PRINTF_BUFFER_SIZE`), so no `debug_*()` call can clobber a live result. Do not route
`debug_vprintf` back through `svprintf_static` to save RAM — that reintroduces a corruption bug that
cost real field debugging (see commit 9d164ff).

## Repo facts

**This checkout is a convenience clone, not the working copy.** The canonical C_General is
`github.com/avpanasyuk/C_General` (`HOME` mirror on bsd); every consumer vendors its own submodule
clone, and that is where library edits actually get made. So this directory goes stale silently —
found two commits behind both remotes on 2026-09-10 — and a change made *here* propagates to nobody
until it is pushed. `git fetch --all` and fast-forward before reading it as current, and never treat
its state as evidence of what the library contains. Remotes are the usual `GitHub` + `HOME` pair.
Library-wide rules and the submodule workflow live in `LIBS/CLAUDE.md`.

`doxyfile` is stale: it holds absolute `c:\GIT_REPS\PROJECTS\JefCore\...` input paths and pre-rename
`.h` filenames (`Time.h`, `Math.h`, `Vector.h`) that no longer exist. Don't run it expecting output;
regenerate its INPUT list if doxygen is actually needed.
