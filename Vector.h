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
#include <memory>
#include <initializer_list>
#include "Error.h"

namespace avp {
  template<typename T>
  class Vector {
    std::shared_ptr<T> p;
    size_t L; //< length
    size_t Reserved; //< reserved space
   public:
    explicit Vector(size_t sz = 0) : p(nullptr), L(0), Reserved(0) {
      reserve(sz);
    } // constructor

    Vector(size_t sz, const T &fill) : p(nullptr), L(0), Reserved(0) {
      reserve(sz);
      for(auto ptr = begin(); ptr < end(); ++ptr) *ptr = fill;
    } // constructor

    Vector(const Vector<T> &v) : p(v.p), L(v.L), Reserved(v.Reserved) {}

    Vector(const std::initializer_list<T> &t): p(nullptr), Reserved(0) {
      reserve(L = t.size());
      size_t i = 0;
      for(auto x:t) p.get()[i++] = x;
    }

    void push_back(const T &x) {
      if(L == Reserved) // no more space
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
      if(sz > Reserved) {
        std::shared_ptr<T> temp(p);
        p = std::shared_ptr<T>(new T[Reserved = sz], std::default_delete<T[]>());
        for(size_t i=0; i<L; ++i) p.get()[i] = temp.get()[i];
      }
    } // reserve

    const Vector<T> &operator =(const Vector<T> &v) {
      p = v.p; L = v.L; Reserved = v.Reserved;
      return *this;
    } // operator =

    T *data() const { return p.get(); }

    bool empty() const { return L == 0;}

    void clear() { p = nullptr; L = 0; }
  }; // class Vector
} // namespace avp
