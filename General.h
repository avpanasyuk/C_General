/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_H_
#define GENERAL_H_

// following are operators which can be universaly derived from others
template<typename T> inline T operator++(T &v) { return v += 1; }
template<typename T> inline T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> inline T operator--(T &v) { return v -= 1; }
template<typename T> inline T operator--(T &v, int) { T old(v); v -= 1; return old; }
template<typename T> inline T operator-(const T &x) { return 0 - x; } // unitary minus

#if 0
// DO NOT REDEFINE BUILT-IN OPERATORS!!!!! INFINITE RECURSION HELL
// DO NOT DEFINE BINARTY OPERATORS FROM UNITARY USING TEMPLATE FUNCTIONS. COMPLILER DOES NOT
// TRY USING IMPLICIT CONVERSION TO FIT PARAMETER TYPES
// USE TEMPLATE CLASS FRIEND FUNCTIONS - THEY ARE SYMMETRIC AND COMPILER TRIES
// IMPLICIT CONVERSIONS
// LIKE THIS: for template<typename type> class T:

#define BINARY_OP_FROM_SELF(op) \
inline friend T operator op (const T &x1, const T &x2) { return T(x1) op##= x2; }

BINARY_OP_FROM_SELF(-)
BINARY_OP_FROM_SELF(+)
BINARY_OP_FROM_SELF(*)
BINARY_OP_FROM_SELF(/)
BINARY_OP_FROM_SELF(&)
BINARY_OP_FROM_SELF(|)
#undef BINARY_OP_FROM_SELF

inline friend bool operator==(T const &v1, T const &v2) { return equal(v1,v2); }
inline friend bool operator!=(T const &v1, T const &v2) { return !(v1 == v2); }

#endif

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

#endif /* GENERAL_H_ */
