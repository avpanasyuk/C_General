#pragma once
/**
 * @file Vector.h
 * @author Sasha
 * @brief equivalent to std::vector, only comprehensible
 * @version 0.1
 * @date 2023-06-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <stdlib.h>
#include <initializer_list>
#include "RefCountArrPtr.h"
#include "Error.h"

namespace avp {
  template<typename T>
  class Vector {
    RefCountArrPtr<T> p;
    size_t L; //< length
    // size_t Reserved; //< reserved space, now in RefCountArrPtr.size()
   public:
    Vector() : L(0) {}

    explicit Vector(size_t reserve) : p(reserve), L(0) {}

    Vector(size_t sz, const T &fill) :p(sz), L(sz) {
      for(auto &x:p) x = fill;
    } // constructor

    Vector(const Vector<T> &v) : p(v.p), L(v.L) {}

    Vector(const std::initializer_list<T> &t): p(t.size()), L(0) {
      for(auto x:t) push_back(x);
    }

    void push_back(const T &x) {
      if(L == p.size()) // no more space
        reserve((L + 1) << 1);
      p.get()[L++] = x;
    } // push_back

    T *begin() const { return p.get(); }
    T *end() const { return p.get() + L; }
    const T *cbegin() const { return p.get(); }
    const T *cend() const { return p.get() + L; }

    T &operator[](size_t i) {
      AVP_ASSERT(i < L);
      return p.get()[i];
    }

    const T &operator[](size_t i) const {
      AVP_ASSERT(i < L);
      return p.get()[i];
    }

    size_t size() const { return L; }

    void reserve(size_t sz) {
      if(sz > p.size()) {
        avp::RefCountArrPtr<T> temp(p);
        p = avp::RefCountArrPtr<T>(sz);
        for(size_t i=0; i<L; ++i) p.get()[i] = temp.get()[i];
      }
    } // reserve

    const Vector<T> &operator =(const Vector<T> &v) {
      p = v.p; L = v.L;
      return *this;
    } // operator =

    T *data() const { return p.get(); }

    bool empty() const { return size() == 0;}

    void clear() { L = 0; }
  }; // class Vector
} // namespace avp
