---
name: headers-not-self-contained
description: Which C_General headers fail a standalone syntax check, and why — these are pre-existing, not regressions
metadata:
  type: project
---

`g++ -std=c++17 -fsyntax-only -I. -x c++ <hdr>` is the only local verification in this
repo (no build system, no tests). As of 2026-07-20 these headers fail it:

- **Platform-dependent by design** — need vendor headers a host check can't supply:
  `Port.hpp`, `Protocol.hpp` (STM32 HAL: `HAL_GetTick`), `CommandChain.hpp`.
- **Genuinely missing includes / rotted:** `BitBang.hpp` (uses `FORCE_INLINE` without
  including `Macros.hpp`), `OutStream.hpp` (uses `uint8_t` without `<stdint.h>`),
  `DoubleLinearBuffer.hpp` (`null_ptr`, never declared), `VarArray.hpp` (`Iterator` not
  in scope), `TriggerVar.hpp` (`CLASS_PLUS_MINUS_BLOCK` macro not visible),
  `ChainAsTemplate.hpp` (unbalanced brace).

**How to apply:** when a syntax sweep reports these, they are the pre-existing baseline —
not something the current edit broke. Compare against this list before investigating.
They are real bugs worth fixing eventually, but fixing them is its own task; don't fold
it into unrelated work.

Everything else in the repo passes, including `common_c.c` as C and `common_cpp.cpp` as
C++. Real confidence still needs a consumer build (`pio run` in a project vendoring this
submodule) — a header can pass here and break on AVR/ESP/STM32.
