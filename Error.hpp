/**
 *  @file
 *
 *  @author: panasyuk
 *  @brief:
 *
 *  @details
 */

#pragma once

#include "Error.h"
#ifndef NO_STL
#include "General.hpp"

#define ASSERT_ELSE_RETURN(exp) do{ \
    if (!(exp)) return avp::string_printf("\"" #exp "\" is false in %s, file " __FILE__ ", line %d",\
            __PRETTY_FUNCTION__, __LINE__); \
  }while(0)

/**
 ERRNO_ASSERT_ELSE_RETURN works with functions returning -1 when failed and filling errno
 */
#include <cstring>
#include <cerrno>
#define ERRNO_ASSERT_ELSE_RETURN(exp) do{ \
    if((exp) == -1)\
	return avp::string_printf("\"" #exp "\" == -1 in %s, file " __FILE__ ", line %d; error: %s",\
          __PRETTY_FUNCTION__, __LINE__,strerror(errno)); \
  }while(0)

/**
 * "exp" return error string, success is when the string is empty
 */
#define AVP_ASSERT_EMPTY_STRING(exp) do{\
	auto ErrStr = (exp); \
	if(!ErrStr.empty()) AVP_ERROR("%s",ErrStr.c_str()); \
}while(0)

#endif // NO_STL

#if defined(USE_EXCEPTIONS) && USE_EXCEPTIONS != 0

#define AVP_THROW(exception,format,...) do{ throw exception(AVP_ERROR_STR(format,  ##__VA_ARGS__)); } while(0)

#endif
