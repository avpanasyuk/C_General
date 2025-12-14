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

#define RETURN_ERROR_STRING(s) do{\
  return avp::string_printf("\"%s\" in %s, file " __FILE__ ", line %d\n",\
        s, __PRETTY_FUNCTION__, __LINE__);\
}while(0)


#define ASSERT_ELSE_RETURN(exp) do{ \
    if (!(exp)) RETURN_ERROR_STRING("\"" #exp "\" is false"); \
}while(0)

/**
 ERRNO_ASSERT_ELSE_RETURN works with functions returning -1 when failed and filling errno
 */
#include <cstring>
#include <cerrno>
#define ERRNO_ASSERT_ELSE_RETURN(exp) do{ \
    if((exp) == -1) RETURN_ERROR_STRING(strerror(errno)); \
}while(0)

/**
 * "exp" return error string, success is when the string is empty
 */
#define ASSERT_EMPTY_STRING(exp) do{\
    auto ErrStr = (exp); \
    if(!ErrStr.empty()) AVP_ERROR("%s",ErrStr.c_str()); \
}while(0)

/**
 * "exp" return error string, success is when the string is empty
 */
#define ASSERT_EMPTY_STRING_ELSE_RETURN(exp) do{\
    auto ErrStr = (exp); \
    if (!ErrStr.empty()) RETURN_ERROR_STRING(ErrStr.c_str()); \
}while(0)

#endif // NO_STL

#if defined(USE_EXCEPTIONS) && USE_EXCEPTIONS != 0

#define AVP_THROW(exception,format,...) do{ throw exception(AVP_DEBUG_PRINTF(format,  ##__VA_ARGS__)); } while(0)

#endif
