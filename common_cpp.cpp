/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
// Vector.h
#include <stdlib.h>

namespace avp {
  void (*bad_pointer_func)() = abort;
  void (*bad_index_func)() = abort;
}
