#include "Macros.h"

// #define _FIRST(a, ...) a

#if 0
#define ARGS_dummy(...) dummy,##__VA_ARGS__
#define IS_EMPTY_impl(...) _EXPAND(SELECT_from9(__VA_ARGS__,0,0,0,0,0,0,0,0,1))
#define IS_EMPTY(...) _EXPAND(IS_EMPTY_impl(ARGS_dummy(__VA_ARGS__)))

#define SPLIT(i, ...) PRIMITIVE_CAT(SPLIT_, i)(__VA_ARGS__)
#define SPLIT_0(a, ...) a
#define SPLIT_1(a, ...) __VA_ARGS__

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__
#define _EXPAND(x) x
#endif

#define IF_HMTO_1(...) .arg(__VA_ARGS__)
#define IF_HMTO_0(...)
#define ADD_ARG_IF_MTO(...) CAT(IF_HMTO_,HAS_MORE_THAN_ONE(__VA_ARGS__))(SPLIT(1,__VA_ARGS__))
// #define RETURN_ERROR(...) do{ return (QString("Error in function '") +  __PRETTY_FUNCTION__ + \
  "' at line " __LINE_STR__ SPLIT(0,__VA_ARGS__))ADD_ARG_IF_MTO(__VA_ARGS__); }while(0)

#define ARG_IF_NOT_EMPTY_0(...) .arg(__VA_ARGS__)
#define ARG_IF_NOT_EMPTY_1(...)
#define ADD_ARG_IF_NOT_EMPTY(...) CAT(ARG_IF_NOT_EMPTY_,IS_EMPTY(__VA_ARGS__))(__VA_ARGS__)
#define RETURN_ERROR(...) do{ return (QString("Error in function '") +  __PRETTY_FUNCTION__ + \
  "' at line " __LINE_STR__ SPLIT(0,__VA_ARGS__))ADD_ARG_IF_NOT_EMPTY(SPLIT(1,__VA_ARGS__)); }while(0)
	  
 
int main() {
#if 0
 SPLIT(0)
 FIRST(b,c,d,e) // b,c,d,e
#endif

RETURN_ERROR();
RETURN_ERROR("Ugu");
RETURN_ERROR("Ugu", 5);
RETURN_ERROR("Ugu", 5, 6);

IS_EMPTY(a)
IS_EMPTY()

 
  
 
}
