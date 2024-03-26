#ifndef VECTOR_H_
#define VECTOR_H_

/// @cond
#include <cstdint>
#include <cmath>
#include <initializer_list>
/// @endcond
#include "Error.h"
#include "General.h"

namespace avp {
template <typename T, size_t Length>
class Vector {
  // index checking vector
 protected:
  T Data[Length];

 public:
  Vector() {}
  Vector(const std::initializer_list<T> &t) {
    // static_assert(t.size() <= Length, ": Initializer list too long!");
    std::copy(t.begin(), t.end(), Data);
    // T *p;
    // for(auto &i = t.begin(), p = Data; i < t.end(); ++i, ++p) *p = *i ;
  }
  Vector(const T &rhs) { *this = rhs; }

  template <typename T1 = T>
  Vector(const Vector<T1, Length> &rhs) { for(size_t i; i < Length; ++i) Data[i] = rhs[i]; }

 // following functions are to ab able to use this class in range-for loop
 template<typename Q> // we need a template because Q can be T or "const T"
 class Iterator {
  Q* p;
  public:
  Iterator(Q* ptr) : p(ptr) {}

  Q& operator*() const { return *p; }
  bool operator != (const Iterator& rhs) const {
    return p != rhs.p;
  }
  void operator ++() { ++p; }
 }; // class Iterator

 Iterator<const T> const begin() const { // const version
   return Iterator<const T>(Data);
 }
 Iterator<const T> const end() const { // const version
  return Iterator<const T>(Data + Length);
 }
 Iterator<T> begin()  { // const version
   return Iterator<T>(Data);
 }
 Iterator<T> end()  { // const version
  return Iterator<T>(Data + Length);
 }

#define SELF_OP_V(Class, ...)                                                              \
  template <typename T1 = T>                                                               \
  const Class &operator __VA_ARGS__##=(const Class<T1, Length> &rhs) {                     \
    for (size_t i = 0; i < Length; i++) Vector<T, Length>::Data[i] __VA_ARGS__## = rhs[i]; \
    return *this;                                                                          \
  }

  SELF_OP_V(Vector)  // assignment operator

#define SELF_OP_T(Class, ...)                                                           \
  template <typename T1 = T>                                                            \
  const Class &operator __VA_ARGS__##=(const T1 &rhs) {                                 \
    for (size_t i = 0; i < Length; i++) Vector<T, Length>::Data[i] __VA_ARGS__## = rhs; \
    return *this;                                                                       \
  }

  SELF_OP_T(Vector)  // assign a single value T to the whole vector
  // const Vector &operator=(const T *p) { for(size_t i=0; i < Length; i++) Data[i] = p[i];  return *this;}

  T &operator[](size_t i) {
    AVP_ASSERT(i < Length);
    return Data[i];
  }

  const T &operator[](size_t i) const {
    AVP_ASSERT(i < Length);
    return Data[i];
  }

  static constexpr size_t N() { return Length; }

  inline friend bool operator!=(Vector const &v1, Vector const &v2) {
    if (&v1 == &v2) return false;
    for (size_t i = 0; i < Length; ++i)
      if (v1.Data[i] != v2.Data[i]) return true;
    return false;
  }  // operator!=
  inline friend bool operator==(Vector const &v1, Vector const &v2) { return !(v1 != v2); }
  // SELF_OP_V(Vector) // this is default operation=
  template <size_t from, size_t to = Length - 1>
  Vector<T, to - from + 1> Sub() const {
    Vector<T, to - from + 1> t;
    for (size_t i = from; i <= to; i++) t[i - from] = Data[i];
    return t;
  }
};  // class Vector

#ifdef PLACED_VECTOR
template <class tPlacedVector>
void *operator new(size_t s, tPlacedVector &Vector) {
  AVP_ASSERT(s == sizeof(Vector.Data[0]));
  AVP_ASSERT(Vector.Nfilled < N_ELEMENTS(Vector.Data));
  return (void *)&Vector.Data[Vector.Nfilled++];
}  // placement new

template <typename T, size_t Length>
class PlacedVector : public Vector<T, Length> {
 protected:
  size_t Nfilled;

 public:
  PlacedVector() : Nfilled(0) {}
  friend void * ::operator new<PlacedVector>(size_t, PlacedVector &);
};  // PlacedVector
#endif

template <typename T, size_t Length>
class ArithVector : public Vector<T, Length> {
 public:
  typedef Vector<T, Length> Vector_t;

  ArithVector() {}
  ArithVector(std::initializer_list<T> t):Vector_t{t} {}
  ArithVector(const T &rhs) : Vector<T, Length>(rhs) {}

  template <typename T1 = T>
  ArithVector(const Vector<T1, Length> &rhs):Vector<T, Length>(rhs) { }

  // SELF_OP_T(ArithVector) // it is  inherited
  SELF_OP_T(ArithVector, +)
  SELF_OP_T(ArithVector, -)
  SELF_OP_T(ArithVector, *)
  SELF_OP_T(ArithVector, /)
  // SELF_OP_V(ArithVector) // it is  inherited
  SELF_OP_V(ArithVector, +)
  SELF_OP_V(ArithVector, -)
  SELF_OP_V(ArithVector, *)
  SELF_OP_V(ArithVector, /)

// FOllowing are binary operators as friends
#define BINARY_OP_FROM_SELF(op)                                                                         \
  inline friend ArithVector operator op(ArithVector x1, const ArithVector &x2) { return x1 op## = x2; } \
  template <typename X>                                                                                 \
  inline friend ArithVector operator op(ArithVector x1, X x2) { return x1 op## = x2; }

  BINARY_OP_FROM_SELF(-)
  BINARY_OP_FROM_SELF(+)
  BINARY_OP_FROM_SELF(*)
  BINARY_OP_FROM_SELF(/)
#undef BINARY_OP_FROM_SELF

  //      template<size_t from, size_t to = Length-1>
  //      ArithVector<T,to-from+1> Sub() { ArithVector<T,to-from+1> t; for(size_t i = from; i <= to; i++) t[i-from] = Data[i]; return t; }
  T sum() const {
    T out = 0;
    for (const T &x : Vector_t::Data) out += x;
    return out;
  }  // sum
  T mean() const { return sum() / Length; }

#define FUNC(op)                                       \
  ArithVector &op() {                                  \
    for (T & x : Vector_t::Data) x = ::op(x);          \
    return *this;                                      \
  }                                                    \
  inline friend ArithVector op(const ArithVector &x) { \
    ArithVector y(x);                                  \
    y.op();                                            \
    return y;                                          \
  }

  FUNC(log)
  FUNC(exp)

  // typecast operator to a different vector type
  template <typename to_type>
  operator ArithVector<to_type, Length>() {
    ArithVector<to_type, Length> t;
    for (uint32_t i = 0; i < Length; ++i) t[i] = static_cast<to_type>(Vector_t::Data[i]);
    return t;
  }
};  // class ArithVector
}  // namespace avp

#endif /* VECTOR_H_ */
