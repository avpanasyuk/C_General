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
#define FOR_ARRAY(array,ptr)     for(auto ptr = array; ptr < AFTER_LAST(array); ++ptr)


#define SINGLE_ARG(...) __VA_ARGS__ // in case there a commas in arguments to a macro

// things to suppress warning for a bit
// how it really works
//         #pragma GCC diagnostic push
//          #pragma GCC diagnostic ignored "-Wno-psabi"
//            foo(b);                       /* no diagnostic for this one */
//          #pragma GCC diagnostic pop

#define IGNORE_WARNING(x) _Pragma ("GCC diagnostic push") \
  DO_PRAGMA(GCC diagnostic ignored #x)
#define STOP_IGNORING _Pragma ("GCC diagnostic pop")

// IGNORE(-Wno-psabi)
// IGNORE(-Wunused-function)

#if 1
/*
Why is this important? Well when a macro is scanned and expanding, it creates a disabling context.
This disabling context will cause a token, that refers to the currently expanding macro, to be painted blue.
Thus, once its painted blue, the macro will no longer expand. This is why macros don't expand recursively.
However, a disabling context only exists during one scan, so by deferring an expansion we can prevent our
macros from becoming painted blue. We will just need to apply more scans to the expression. We can do that
using this EVAL macro:
*/

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__


#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__


#if 0
#define INC(x) PRIMITIVE_CAT(INC_, x)
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10
#define INC_10 11
#define INC_11 12
#define INC_12 13
#define INC_13 14
#define INC_14 15
#define INC_15 16
#define INC_16 17
#define DEC(x) PRIMITIVE_CAT(DEC_, x)
#define DEC_0 0
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7
#define DEC_9 8
#define DEC_10 9
#define DEC_11 10
#define DEC_12 11
#define DEC_13 12
#define DEC_14 13
#define DEC_15 14
#define DEC_16 15
#define DEC_17 16
#endif

// LOGIC
/*
* CHECK(PROBE(~)) is 1 if a is not empty, and 0 otherwise
*/

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x,1,


#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)

// NOT(0) -> CHECK(NOT_0) -> CHECK(~,1,) -> CHECK_N(~,1,0,) -> 1
// NOT(anything) -> CHECK(NOT_anything) -> CHECK_N(NOT_anything,0,) -> 0

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BOOL(x) COMPL(NOT(x))

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define IF(c) IIF(BOOL(c))

#define EAT(...)
#define EXPAND(...) __VA_ARGS__
#define WHEN(c) IF(c)(EXPAND, EAT)

#define SPLIT(i, ...) PRIMITIVE_CAT(SPLIT_, i)(__VA_ARGS__)
#define SPLIT_0(a, ...) a
#define SPLIT_1(a, ...) __VA_ARGS__

#define IS_VARIADIC(...) \
  SPLIT(0, CAT(IS_VARIADIC_R_, IS_VARIADIC_C __VA_ARGS__)) \
  /**/
#define IS_VARIADIC_C(...) 1
#define IS_VARIADIC_R_1 1,
#define IS_VARIADIC_R_IS_VARIADIC_C 0,

#define IS_EMPTY_NON_FUNCTION(...) \
  IIF(IS_VARIADIC(__VA_ARGS__))( \
                                 0, \
                                 IS_VARIADIC(IS_EMPTY_NON_FUNCTION_C __VA_ARGS__ ()) \
                               ) \
  /**/
#define IS_EMPTY_NON_FUNCTION_C() ()

// #define IS_EMPTY(x) IIF(BOOL(PRIMITIVE_CAT(CHECK_AFTER_,x)))
#define CHECK_AFTER_ 0/// deferred expressions
#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__
/// repeat
#define REPEAT(count, macro, ...) \
  WHEN(count) \
  ( \
  OBSTRUCT(REPEAT_INDIRECT) () \
  ( \
  DEC(count), macro, __VA_ARGS__ \
  ) \
  OBSTRUCT(macro) \
  ( \
  DEC(count), __VA_ARGS__ \
  ) \
  )
#define REPEAT_INDIRECT() REPEAT
//An example of using this macro
//#define M(i, _) i
//EVAL(REPEAT(8, M, ~))
// generates - 0 1 2 3 4 5 6 7

// Accept any number of args >= N, but expand to just the Nth one. The macro
// that calls us still only supports 4 args, but the set of values we might
// need to return is 1 larger, so we increase N to 6.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, N, ...) N

// Count how many args are in a variadic macro. We now use GCC/Clang's extension to
// handle the case where … expands to nothing. We must add a placeholder arg before
// ##__VA_ARGS__ (its value is totally irrelevant, but it's necessary to preserve
// the shifting offset we want). In addition, we must add 0 as a valid value to be in
// the N position.
#define COUNT_VARARGS(...) _GET_NTH_ARG("ignored", ##__VA_ARGS__, 4, 3, 2, 1, 0)

#endif

// creating STRING __LINE__ so we can concat it with strings
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define __LINE_STR__ STRINGIZE(__LINE__)

// following is for counting a number of macro arguments
#define SELECT_from9(_1,_2,_3,_4,_5,_6,_7,_8,_9,num,...) num
#define HAS_MORE_THAN_ONE(...)  SELECT_from9(~,##__VA_ARGS__,1,1,1,1,1,1,1,0,0)
#define IS_EMPTY(...)  SELECT_from9(~,##__VA_ARGS__,0,0,0,0,0,0,0,0,1)

#if 0 // EXAMPLE OF USE OF THE MACROS ABOVE. Here we add ": " only if there is first __VA_ARGS__ argument,
and .arg(...) if there are two and more.
#define ARG_IF_NOT_EMPTY_0(...) .arg(__VA_ARGS__)
#define ARG_IF_NOT_EMPTY_1(...)
#define COL_IF_NOT_EMPTY_0(...) ": " __VA_ARGS__
#define COL_IF_NOT_EMPTY_1(...)

#define ADD_ARG_IF_NOT_EMPTY(...) CAT(ARG_IF_NOT_EMPTY_,IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)
#define ADD_COL_IF_NOT_EMPTY(...) CAT(COL_IF_NOT_EMPTY_,IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)
#define ERROR_PLACE " '" + __PRETTY_FUNCTION__ + "' in file " __BASE_FILE__ " at line " __LINE_STR__

#define RETURN_ERROR(...) do{ return (QString("Error in") + ERROR_PLACE \
  ADD_COL_IF_NOT_EMPTY(SPLIT(0,__VA_ARGS__)))ADD_ARG_IF_NOT_EMPTY(SPLIT(1,__VA_ARGS__)); }while(0)
#endif

/// creates a class to be precise alias of another class
#define PURE_CHILD(class_name,parent) \
struct class_name: public parent { \
    template<typename... Types> \
    class_name(Types... args) : parent(args...) {}  \
};


#endif /* MACROS_H_INCLUDED */

