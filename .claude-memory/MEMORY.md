# Memory index — C_General

- [sprintf_static is unguarded by design](sprintf-static-unguarded-by-design.md) — an RAII claim was tried and rejected; the debug_vprintf stack buffer is the protection that works.
- [Headers that don't compile standalone](headers-not-self-contained.md) — pre-existing missing includes, not regressions; don't chase them as new breakage.
