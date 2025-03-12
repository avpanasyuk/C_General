#pragma once

#include <stdlib.h>
#include <initializer_list>
#include <limits>
#include "Error.h"

namespace avp {
  /** Vector class with reference counting and automatic extension by a factor of two
    * All copies done with constructor or assignment
    * point to the same data. Use "make_copy" to make a separate copy of data.
    */

  template<typename T>
  class Vector {
    /**
    * this is a reference counting class for Vectors. The object of this class is shared between
    * Vectors containing the same array of type T pointed on by "p". If Vectgor decides not to point to this
    * particular object any more it decreases it's reference number.
    *
    * This class never decreases memory pointer by "p".
    */
    struct PtrWithRefCnt_ {
      T* p;
      size_t Allocated, Used, RefN;

      PtrWithRefCnt_(size_t reserve): Used(0), RefN(1) { p = new T[Allocated = reserve]; }
      ~PtrWithRefCnt_() { delete[] p; }

      void push_back(const T &x) {
        if(Used == Allocated) // no more space
          resize((Used + 1) << 1); // allocate twice as much and then some
        p[Used++] = x;
      } // push_back

      void resize(size_t sz) {
        if(sz > Allocated) {
          auto temp = p;
          p = new T[Allocated = sz];
          for(size_t i=0; i<Used; ++i) p[i] = temp[i];
          delete[] temp;
        }
      } // resize
    } *pPtrWithRefCnt; // this pointer to a reference counting class is always allocated, never nullptr!

    void ReleasePtrWithRefCnt() {
      if(--pPtrWithRefCnt->RefN == 0) delete pPtrWithRefCnt;
    } // ReleasePtrWithRefCnt

    void ReplaceWith(const Vector &r) {
      if(this != &r && pPtrWithRefCnt != r.pPtrWithRefCnt) {
        ReleasePtrWithRefCnt();
        (pPtrWithRefCnt = r.pPtrWithRefCnt)->RefN++;
      }
    } // ReplaceWith

   public:
    struct Range { T Min, Max; };

    Vector():pPtrWithRefCnt(new PtrWithRefCnt_(1)) {}

    Vector(size_t reserve) {
      pPtrWithRefCnt = reserve != 0?new PtrWithRefCnt_(reserve):new PtrWithRefCnt_(1);
    } // constructor

    Vector(size_t sz, const T &fill) {
      if(sz != 0) {
        pPtrWithRefCnt = new PtrWithRefCnt_(sz);
        while(sz--) pPtrWithRefCnt->push_back(fill);
      } else pPtrWithRefCnt = new PtrWithRefCnt_(1);
    } // constructor

    Vector(const Vector &r) { (pPtrWithRefCnt = r.pPtrWithRefCnt)->RefN++; }

    Vector(const std::initializer_list<T> &t) {
      if(t.size() == 0) pPtrWithRefCnt = new PtrWithRefCnt_(1);
      else {
        pPtrWithRefCnt = new PtrWithRefCnt_(t.size());
        for(auto x:t) pPtrWithRefCnt->push_back(x);
      }
    } // initializer list constructor

    ~Vector() { ReleasePtrWithRefCnt(); }

    void resize(size_t sz) { pPtrWithRefCnt->resize(sz); }

    void push_back(const T &x) { pPtrWithRefCnt->push_back(x); }

    const Vector &operator=(const Vector &r) {
      ReplaceWith(r);
      return *this;
    }

    void clear() { pPtrWithRefCnt->Used = 0; } // does not change reserved space
    bool empty() const { return size() == 0; }
    size_t size() const { return pPtrWithRefCnt->Used; }
    /**
      * Makes copy which does not share data
      */
    Vector<T> const make_copy() {
      Vector<T> Out(pPtrWithRefCnt->Used); // allocate only used size
      const T *p = cbegin();
      for(auto &x:Out) x = *(p++);
      return Out;
    } // make _copy

    const T &operator[](size_t i) const {
      AVP_ASSERT(i < size());
      return pPtrWithRefCnt->p[i];
    }

    T &operator[](size_t i) {
      resize(i+1);
      if(i >= size()) set_size(i+1);
      return pPtrWithRefCnt->p[i];
    }

    Range MinMax() const {
      Range Out = { std::numeric_limits<T>::max(), std::numeric_limits<T>::min()};
      for(const auto &d:*this) {
        if(d < Out.Min) Out.Min = d;
        if(d > Out.Max) Out.Max = d;
      }
      return Out;
    } // MinMax

    // begin/end go only over used portion
    const T *cbegin() const { return pPtrWithRefCnt->p; }
    const T *cend() const { return pPtrWithRefCnt->p + pPtrWithRefCnt->Used; }
    T *begin() { return pPtrWithRefCnt->p; }
    T *end() { return pPtrWithRefCnt->p + pPtrWithRefCnt->Used; }
    const T *begin() const { return pPtrWithRefCnt->p; }
    const T *end() const { return pPtrWithRefCnt->p + pPtrWithRefCnt->Used; }

    // low level access to the data
    T* data() const { return pPtrWithRefCnt->p; }
    void set_size(size_t sz) { resize(sz); pPtrWithRefCnt->Used = sz; }
  }; // class Vector
} // namespace avp
