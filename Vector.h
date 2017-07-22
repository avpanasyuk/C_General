#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdint.h>
#include "General.h"
#include "Error.h"

#define SELF_OP_T(Class,...) \
  const Class &operator __VA_ARGS__##=(const T &rhs) { \
    size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs; while(++i < Length); return *this; }
#define SELF_OP_V(Class,...) \
  const Class &operator __VA_ARGS__##=(const Class<T,Length> &rhs) { \
    size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs.Data[i]; while(++i < Length); return *this; }

template<class tPlacedVector>
void * operator new(size_t s, tPlacedVector &Vector) {
  AVP_ASSERT(s == sizeof(Vector.Data[0]));
  AVP_ASSERT(Vector.Nfilled < N_ELEMENTS(Vector.Data));
  return (void *)&Vector.Data[Vector.Nfilled++];
} // placement new

namespace avp {
  template<typename T, size_t Length>
  class Vector {
      // index checking vector
    protected:
      T Data[Length];
    public:
      Vector() {}
      Vector(const T &rhs) { *this = rhs; }
      Vector(const Vector<T,Length> &rhs) { *this = rhs; }

      SELF_OP_V(Vector) // assignment operator
      SELF_OP_T(Vector) // assign a single value T to the whole vector
      const Vector &operator=(const T *p) { for(size_t i=0; i < Length; i++) Data[i] = p[i];  return *this;}

      T &operator[](uint8_t i) { AVP_ASSERT(i < Length); return Data[i]; }

      const T &operator[](uint8_t i) const { AVP_ASSERT(i < Length); return Data[i];}

      constexpr size_t N() const { return Length; }

      const T* get_ptr() const { return Data; }
      const T* after_last() const { return Data + Length; }


      inline friend bool operator!=(Vector const &v1, Vector const &v2) {
        if(&v1 == &v2) return false;
        for(size_t i=0; i < Length; ++i) if(v1.Data[i] != v2.Data[i]) return true;
        return false;
      } // operator!=
      inline friend bool operator==(Vector const &v1, Vector const &v2) { return !(v1 != v2); }
      // SELF_OP_V(Vector) // this is default operation=
  }; // class Vector

  template<typename T, size_t Length>
  class PlacedVector: public Vector<T,Length> {
    protected:
      size_t Nfilled;
    public:
      PlacedVector():Nfilled(0) {}
      friend void * ::operator new<PlacedVector>(size_t, PlacedVector &);
  }; // PlacedVector

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
      BINARY_OP_FROM_SELF(*)
      BINARY_OP_FROM_SELF(/)
#undef BINARY_OP_FROM_SELF

  }; // class ArithVector
} // namespace avp

#endif /* VECTOR_H_ */
