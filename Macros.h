#ifndef MACROS_H_INCLUDED
#define MACROS_H_INCLUDED

// preprocessor tricks. __VA_ARGS__ is used so the last parameter may be empty
#define _COMB2(a,...) a##__VA_ARGS__
#define COMB2(a,...) _COMB2(a,__VA_ARGS__) // second parameter may be absent
#define _COMB3(a,b,...) a##b##__VA_ARGS__
#define COMB3(a,b,...) _COMB3(a,b,__VA_ARGS__)

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))

#define N_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#define AFTER_LAST(array) (array + N_ELEMENTS(array))

#define SINGLE_ARG(...) __VA_ARGS__ // in case there a commas in arguments to a macro

// things to suppress warning for a bit
// how it really works
//         #pragma GCC diagnostic push
//          #pragma GCC diagnostic ignored "-Wno-psabi"
//            foo(b);                       /* no diagnostic for this one */
//          #pragma GCC diagnostic pop

#define IGNORE_WARNING(x) _Pragma ("GCC diagnostic push") \
  DO_PRAGMA(GCC diagnostic ignored #x)
#define STOP_IGNORING_WARNING _Pragma ("GCC diagnostic pop")

// EXAMPLE:
// IGNORE_WARNING(-Wno-psabi)
// IGNORE_WARNING(-Wunused-function)

#endif /* MACROS_H_INCLUDED */

