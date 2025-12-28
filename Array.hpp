#pragma once

/// @cond
#include <cstdint>
#include <cmath>
#include <initializer_list>
/// @endcond
#include "Error.h"
#include "General.h"

namespace avp {
  template<typename T, size_t Length>
  class Array {
    // index checking Array
  protected:
    T Data[Length];

  public:
    Array() {}
    Array(const std::initializer_list<T> &t) {
      AVP_ASSERT(t.size() <= Length);
      std::copy(t.begin(), t.end(), Data);
    }
    Array(const T &rhs) {
      *this = rhs;
    }

    template<typename T1 = T>
    Array(const Array<T1, Length> &rhs) {
      for(size_t i = 0; i < Length; ++i) Data[i] = rhs[i];
    }

    // following functions are to ab able to use this class in range-for loop
    template<typename Q> // we need a template because Q can be T or "const T"
    class Iterator {
      Q *p;

    public:
      Iterator(Q *ptr) : p(ptr) {}

      Q &operator*() const {
        return *p;
      }
      bool operator!=(const Iterator &rhs) const {
        return p != rhs.p;
      }
      Iterator& operator++() { ++p; return *this; }
    }; // class Iterator

    Iterator<const T> const begin() const { // const version
      return Iterator<const T>(Data);
    }
    virtual Iterator<const T> const end() const { // const version
      return Iterator<const T>(Data + Length);
    }
    Iterator<T> begin() { // non-const version
      return Iterator<T>(Data);
    }
    virtual Iterator<T> end() { // non-const version
      return Iterator<T>(Data + Length);
    }

#define SELF_OP_V(Class, ...)                                                            \
  template<typename T1 = T>                                                              \
  Class &operator __VA_ARGS__##=(const Class<T1, Length> &rhs) {                   \
    for(size_t i = 0; i < Length; i++) Array<T, Length>::Data[i] __VA_ARGS__## = rhs[i]; \
    return *this;                                                                        \
  }

    SELF_OP_V(Array) // assignment operator

#define SELF_OP_T(Class, ...)                                                         \
  template<typename T1 = T>                                                           \
  Class &operator __VA_ARGS__##=(const T1 & rhs) {                              \
    for(size_t i = 0; i < Length; i++) Array<T, Length>::Data[i] __VA_ARGS__## = rhs; \
    return *this;                                                                     \
  }

    SELF_OP_T(Array) // assign a single value T to the whole Array
                     // const Array &operator=(const T *p) { for(size_t i=0; i < ReservedSz; i++) Data[i] = p[i];  return *this;}

    virtual T &operator[](size_t i) {
      AVP_ASSERT(i < Length);
      return Data[i];
    }

    virtual const T &operator[](size_t i) const {
      AVP_ASSERT(i < Length);
      return Data[i];
    }

    static constexpr size_t N() { return Length; }

    virtual size_t size() const { return N(); }

    const T *get_ptr() const {
      return Data;
    }
    virtual const T *after_last() const {
      return Data + Length;
    }

    inline friend bool operator!=(Array const &v1, Array const &v2) {
      if(&v1 == &v2) return false;
      for(size_t i = 0; i < Length; ++i)
        if(v1.Data[i] != v2.Data[i]) return true;
      return false;
    } // operator!=
    inline friend bool operator==(Array const &v1, Array const &v2) {
      return !(v1 != v2);
    }
    // SELF_OP_V(Array) // this is default operation=
    template<size_t from, size_t to = Length - 1>
    Array<T, to - from + 1> Sub() const {
      Array<T, to - from + 1> t;
      for(size_t i = from; i <= to; i++) t[i - from] = Data[i];
      return t;
    }
  }; // class Array

#ifdef PLACED_ARRAY
  template<class tPlacedArray>
  void *operator new(size_t s, tPlacedArray &Array) {
    AVP_ASSERT(s == sizeof(Array.Data[0]));
    AVP_ASSERT(Array.Nfilled < N_ELEMENTS(Array.Data));
    return (void *)&Array.Data[Array.Nfilled++];
  } // placement new

  template<typename T, size_t ReservedSz>
  class PlacedArray : public Array<T, ReservedSz> {
  protected:
    size_t Nfilled;

  public:
    PlacedArray() : Nfilled(0) {}
    friend void * ::operator new<PlacedArray>(size_t, PlacedArray &);
  }; // PlacedArray
#endif

  template<typename T, size_t Length>
  class ArithArray : public Array<T, Length> {
  public:
    typedef Array<T, Length> ARRAY_t;

    ArithArray() {}
    ArithArray(std::initializer_list<T> t) : ARRAY_t{t} {}
    ArithArray(const T &rhs) : Array<T, Length>(rhs) {}

    template<typename T1 = T>
    ArithArray(const Array<T1, Length> &rhs) : Array<T, Length>(rhs) {}

    // SELF_OP_T(ArithArray) // it is  inherited
    SELF_OP_T(ArithArray, +)
    SELF_OP_T(ArithArray, -)
    SELF_OP_T(ArithArray, *)
    SELF_OP_T(ArithArray, /)
    // SELF_OP_V(ArithArray) // it is  inherited
    SELF_OP_V(ArithArray, +)
    SELF_OP_V(ArithArray, -)
    SELF_OP_V(ArithArray, *)
    SELF_OP_V(ArithArray, /)

    // FOllowing are binary operators as friends
#define BINARY_OP_FROM_SELF(op)                                                                      \
  inline friend ArithArray operator op(ArithArray x1, const ArithArray &x2) { return x1 op## = x2; } \
  template<typename X>                                                                               \
  inline friend ArithArray operator op(ArithArray x1, X x2) { return x1 op## = x2; }

    BINARY_OP_FROM_SELF(-)
    BINARY_OP_FROM_SELF(+)
    BINARY_OP_FROM_SELF(*)
    BINARY_OP_FROM_SELF(/)
#undef BINARY_OP_FROM_SELF

    //      template<size_t from, size_t to = ReservedSz-1>
    //      ArithArray<T,to-from+1> Sub() { ArithArray<T,to-from+1> t; for(size_t i = from; i <= to; i++) t[i-from] = Data[i]; return t; }
    T sum() const {
      T out = 0;
      for(const T &x : ARRAY_t::Data) out += x;
      return out;
    } // sum
    T mean() const {
      return sum() / Length;
    }

#define FUNC(op)                                     \
  ArithArray &op() {                                 \
    for(T &x : ARRAY_t::Data) x = ::op(x);           \
    return *this;                                    \
  }                                                  \
  inline friend ArithArray op(const ArithArray &x) { \
    ArithArray y(x);                                 \
    y.op();                                          \
    return y;                                        \
  }

    FUNC(log)
    FUNC(exp)

    // typecast operator to a different Array type
    template<typename to_type>
    operator ArithArray<to_type, Length>() {
      ArithArray<to_type, Length> t;
      for(uint32_t i = 0; i < Length; ++i) t[i] = static_cast<to_type>(ARRAY_t::Data[i]);
      return t;
    }
  }; // class ArithArray

#if 0
 // for VarArithArray

#define VAR_SELF_OP_V(...)                                                                          \
  template<size_t L, typename T1 = T>                                                               \
  const VarArray &operator __VA_ARGS__##=(const Array<T1, L> &rhs) {                                \
    for(size_t i = 0; i < ReservedSz; i++) Array<T, ReservedSz>::Data[i] __VA_ARGS__## = T(rhs[i]); \
    return *this;                                                                                   \
  }                                                                                                 \
  template<size_t L, typename T1 = T>                                                               \
  const VarArray &operator __VA_ARGS__##=(const Array<T1, L> &rhs) {                                \
    for(size_t i = 0; i < ReservedSz; i++) Array<T, ReservedSz>::Data[i] __VA_ARGS__## = T(rhs[i]); \
    return *this;                                                                                   \
  }
#endif

} // namespace avp
