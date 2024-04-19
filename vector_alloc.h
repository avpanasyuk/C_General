/**
 *@file vector_alloc.h
 * @author your name (you@domain.com)
 * @brief Simple efficient auto-resizing vector which uses realloca, so can do only trivial types
 * @version 0.1
 * @date 2023-06-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <cstdlib>
#include "Error.h"

namespace avp {
  template<typename T>
  class vector_alloc {
    T *p;
    size_t L; //< length
    size_t Reserved; //< reserved space
  public:
    vector_alloc(size sz = 0) : p(nullptr), L(0), Reserved(0) {
      reserve(sz);
    } // constructor

    void append(const T &x) {
      if(L == Reserved) // no more space
        p = (T *)realloc((void *)p, Reserved += (L + 2) >> 1);
      p[L++] = x;
    } // append

    T *begin() const { return p; }
    T *end() const { return p + L; }

    void remove(size_t index) {
      AVP_ASSERT(index < L);
      for(T *t = p + index; t < p + L - 1; ++t) *t = *(t + 1);
    } // remove

    T &operator[](size_t i) {
      AVP_ASSERT(i < L);
      return p[i];
    }

    const T &operator[](size_t i) const {
      AVP_ASSERT(i < L);
      return p[i];
    }

    static constexpr size_t N() { return L; }

    void reserve(size_t sz) {
      if(sz > Reserved) p = (T *)realloc((void *)p, Reserved = sz);
    } // reserve
  }; // class vector_alloc
} // namespace avp