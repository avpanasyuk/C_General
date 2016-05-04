#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdint.h>
#include "General.h"
#include "Error.h"

/// do SELF_OP with a single variable as RHS
#define UNITY_OP(Class,...) \
  const Class operator __VA_ARGS__##() { Class Out; \
    size_t i=0;  do Out.Data[i] = __VA_ARGS__##Vector<T,Length>::Data[i]; \
    while(++i < Length); return Out; }

/// do SELF_OP with a single variable as RHS
#define SELF_OP_T(Class,...) \
  const Class &operator __VA_ARGS__##=(const T &rhs) { \
    size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs; while(++i < Length); return *this; }

/// do SELF_OP with a vector of equal length as RHS
#define SELF_OP_V(Class,...) \
  const Class &operator __VA_ARGS__##=(const Class<T,Length> &rhs) { \
    size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs.Data[i]; while(++i < Length); return *this; }

namespace avp {
  template<typename T, size_t Length>
  class Vector {
      // index checking vector
    protected:
      T Data[Length];
    public:
      Vector() {}
      Vector(const T &rhs) { *this = rhs; }
      Vector(const Vector &rhs) { *this = rhs; }
      Vector(const T *p) { *this = p; }

      SELF_OP_V(Vector) // assignment operator
      SELF_OP_T(Vector) // assign a single value T to the whole vector

      const Vector unity_op const(T (*pFunc)(const T &x)) {
        Vector Out;
        size_t i=0;  do Out.Data[i] = (*pFunc)(Data[i]); while(++i < Length);
        return Out;
      } // Unity_op

      /// do SELF_OP with a single variable as RHS
      const Vector &self_op_t const(T (*pFunc)(const T &x1, const T &x2), const T &x) {
        size_t i=0;  do Data[i] = (*pFunc)(Data[i],x); while(++i < Length);
        return *this;
      } // Unity_op

      /// do SELF_OP with a vector of equal length as RHS
     const Vector &self_op_v const(T (*pFunc)(const T &x1, const T &x2), const Vector &v) {
        size_t i=0;  do Data[i] = (*pFunc)(Data[i],v.Data[i]); while(++i < Length);
        return *this;
      } // Unity_op

      UNITY_OP(Vector,~) // bitwise reversal for every element

      const Vector &operator=(const T &rhs) { return self_op_t([](const T &x) { return x; }, rhs); }
      const Vector &operator=(const Vector &rhs) { return self_op_v([](const T &x) { return x; }, rhs); }
      const Vector &operator=(const T *p) { for(size_t i=0; i < Length; i++) Data[i] = p[i];  return *this;}


      T &operator[](uint8_t i) { AVP_ASSERT(i < Length); return Data[i]; }

      const T &operator[](uint8_t i) const { AVP_ASSERT(i < Length); return Data[i];}

      constexpr size_t N() const { return Length; }

      T* get_ptr() const { return Data; }
      const T* after_last() const { return Data + Length; }


      inline friend Vector<bool,Length> compare(Vector const &v1, Vector const &v2) {
        Vector<bool,Length> Out = true;
        if(&v1 != &v2)
          for(size_t i=0; i < Length; ++i) Out[i] = v1.Data[i] == v2.Data[i];
        return Out;
      } // compare

      inline friend bool any(Vector<bool,Length> const &v) {
          for(size_t i=0; i < Length; ++i) if(v.Data[i]) return true;
          return false;
      } // any

      inline friend bool operator!=(Vector const &v1, Vector const &v2) {
        if(&v1 == &v2) return false;
        return any(compare(v1,v2)
      } // operator!=

      inline friend bool operator==(Vector const &v1, Vector const &v2) { return !(v1 != v2); }
      // SELF_OP_V(Vector) // this is default operation=
  }; // class Vector

  template<typename T, size_t Length>
  class ArithVector: public Vector<T,Length> {
    public:
      ArithVector() {}
      ArithVector(const T &rhs):Vector<T,Length>(rhs) {}

      SELF_OP_T(ArithVector)
      SELF_OP_T(ArithVector,+)
      SELF_OP_T(ArithVector,-)
      SELF_OP_T(ArithVector,*)
      SELF_OP_T(ArithVector,/)
      // SELF_OP_V(ArithVector) // it is default
      SELF_OP_V(ArithVector,+)
      SELF_OP_V(ArithVector,-)
      SELF_OP_V(ArithVector,*)
      SELF_OP_V(ArithVector,/)

// FOllowing are binary operators as friends
#define BINARY_OP_FROM_SELF(op) \
  inline friend ArithVector operator op (const ArithVector &x1, const ArithVector &x2) \
  { return ArithVector(x1) op##= x2; }

      BINARY_OP_FROM_SELF(-)
      BINARY_OP_FROM_SELF(+)
      BINARY_OP_FROM_SELF( *)
      BINARY_OP_FROM_SELF(/)
#undef BINARY_OP_FROM_SELF

  }; // class ArithVector
} // namespace avp

#endif /* VECTOR_H_ */