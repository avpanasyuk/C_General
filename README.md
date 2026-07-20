# C_General

Platform-agnostic C / C++ utility library used across the author's
embedded (ESP8266 / ESP32 / Arduino) and host projects. Header-only for the most
part, with a small amount of out-of-line code in `common_c.c` / `common_cpp.cpp`.
Sibling of [`C_ESP`](../C_ESP) and [`C_ARDUINO`](../C_ARDUINO); typically vendored
under a project's `src/`.

> Embedded consumers often build with `-DNO_STL=1`, which compiles out the
> `std::string`-based paths — don't rely on those in firmware.

## Key headers

| Header | What it provides |
|--------|------------------|
| `Error.h` / `Error.hpp` | `extern "C"` `debug_putchar`, `debug_puts`, `debug_printf`, `debug_puts_free`. Defaults are `__weak` in `common_c.c` so a project can override `debug_puts` to tee into Serial / OLED / log buffers (`debug_printf`/`debug_vprintf` route through it). |
| `MyTime.hpp` | `avp::TimeOut`, `avp::TimePeriod`, `avp::TimePeriod1<period>`, `avp::Periodically<Fn>::Run(ms)` (the cooperative scheduler used pervasively in `loop()`), `RunPeriodically<Fn, period>`. |
| `General.h` / `General.hpp` | `sprintf_static`, `sprintf_alloc`, `svprintf_puts`/`printf_wrapper` (printf → a `puts`-style sink, used for log vprintf), `Crc16`, `avp::CallWhenOutOfScope`, `RestoreOnReturn`, `ReleaseWhenOutOfScope`, `unsigned_is_smaller`. `.h` = C-callable, `.hpp` = C++-only. Also `DEBUG_PRINTF_BUFFER_SIZE` (default 256) — the per-call stack buffer `debug_vprintf` formats into; override it on RAM-tight targets. |
| `StaticStr.hpp` | `avp::sprintf_guarded()` → a move-only `avp::StaticStr` holding `sprintf_static`'s shared buffer for its lifetime, so a second format into it asserts instead of silently corrupting the first result. Detection only, and only where `NDEBUG` is absent. |
| `Macros.h` / `Macros.hpp` | `IGNORE_WARNING(-Wfoo)` / `STOP_IGNORING_WARNING`, `FORCE_INLINE`, `N_ELEMENTS`, `TODO(...)`. |
| `CircBuffer.hpp`, `CircBufferWithCont.hpp`, `VirtCircBuffer.hpp`, `DoubleLinearBuffer.hpp` | Ring / linear buffer variants. |
| `CommandParser.hpp`, `CommandTable.hpp`, `CommandChain.hpp`, `Protocol.hpp` | Command parsing and the binary protocol layer. |
| `Array.hpp`, `Vector.hpp`, `VarArray.hpp`, `BitVar.hpp`, `Complex.hpp`, `safe_ptr.hpp` | Containers / numeric / pointer helpers. |
| `ISR_Message.hpp`, `BG_message.hpp`, `TriggerVar.hpp` | ISR-to-main messaging primitives. |
| `MyMath.hpp`, `sort.c`, `sort_and_median.h` | Math / sorting / median helpers. |
| `BitBang.hpp`, `IO.hpp`, `Port.hpp`, `OutStream.hpp` | Bit-banged IO and stream abstractions; `get_be16`/`get_be32` portable big-endian byte-buffer reads. |

## Gotchas

- `avp::Periodically<Fn>::Run(ms)` keys storage on the function pointer `Fn` —
  each unique `Fn` gets its own deadline. Don't collapse periodic jobs into one
  dispatch function or you lose the per-job timers.
- Override `debug_puts` (not `debug_putchar`) to redirect all debug output.

## Repo notes

Canonical: `github.com/avpanasyuk/C_General`. `HOME` mirror:
`ssh://BSD/~panasyuk/GIT_REPS/LIBS/C/General.git` (bare).
