/**
 *  @file
 *
 *  @author: panasyuk
 *  @brief:
 *
 *  @details
 */

#pragma once

#ifndef NO_STL
#include "General.hpp"

#define ASSERT_ELSE_RETURN(exp) do{ \
    return (exp)?std::string("")?\
           avp::string_printf("""" #exp """ is false in %s, file " __FILE__ ", line %d", __PRETTY_FUNCTION__, __LINE__); \
  }while(0)

#endif // NO_STL

#if defined(USE_EXCEPTIONS) && USE_EXCEPTIONS != 0

#define AVP_THROW(exception,format,...) do{ throw exception(AVP_ERROR_STR(format,  ##__VA_ARGS__)); } while(0)

#endif
