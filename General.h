/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_H_
#define GENERAL_H_

// following are operators which can be universaly derived from others
template<typename T> inline T &operator++(T &v) { return v += 1; }
template<typename T> inline T operator++(T volatile &v) { return v += 1; }
template<typename T> inline T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> inline bool operator!=(T const &v1, T const &v2) { return !(v1 == v2); }

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

#endif /* GENERAL_H_ */
