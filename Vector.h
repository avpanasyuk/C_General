#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdlib.h>
#include "General.h"

#define SELF_OP_T(Class,...) \
      const Class &operator __VA_ARGS__##=(const T &rhs) { \
      size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs; while(++i < Length); return *this; }
#define SELF_OP_V(Class,...) \
      const Class &operator __VA_ARGS__##=(const Class<T,Length> &rhs) { \
        size_t i=0;  do Vector<T,Length>::Data[i] __VA_ARGS__##= rhs.Data[i]; while(++i < Length); return *this; }

namespace avp {
  extern Fail::function bad_index_func;
  extern Fail::function bad_pointer_func;

  template<typename T, size_t Length>
  class Vector {
    // index checking vector
  protected:
    T Data[Length];
  public:
    Vector() {}
    Vector(const T &rhs) { *this = rhs; }
    // Vector(const Vector<T,Length> &rhs) { *this = rhs; } // the same as default

    T &operator[](uint8_t i) {
#ifdef DEBUG
      if(i>=Length) (*bad_index_func)();
#endif
      return Data[i];
    }

   const T &operator[](uint8_t i) const {
#ifdef DEBUG
      if(i>=Length) (*bad_index_func)();
#endif
      return Data[i];
    }

    constexpr size_t N() const { return Length; }
    SELF_OP_T(Vector)
    // SELF_OP_V(Vector) // this is default operation=
  }; // class Vector

  template<typename T, size_t Length>
  class ArithVector: public Vector<T,Length> {
  public:
    ArithVector() {}
    ArithVector(const T &rhs) { *this = rhs; }
    // ArithVector(const Vector<T,Length> &rhs) { *this = rhs; } // it is default

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
  }; // class ArithVector

  template<typename T, size_t Length>
  class PointerVector: public Vector<T *, Length> {
    // this is vector of pointers which never lets dereference NULL pointer
    // instead of Vector::operator[] which returns T* &
    // PointerVector::operator[] return Token which can be dereferenced
    // as a T*, but checks it for non-NULL first
    class Token {
      T * &Ref;
      friend class PointerVector;
      Token(T *& r):Ref(r) {}
    public:
      operator T * &() const { return Ref; }
      Token &operator=(T *Ptr) { Ref = Ptr; return *this; }
      T &operator*() const {
        if(Ref == NULL) (*bad_pointer_func)();
        return *Ref; // we may be dereferencing NULL pointer here, but program
        // should terminate on the line above
      }
      T *operator-> () const { return &(this->operator*()); }
    }; // Token
  public:
    Token operator[](uint8_t i) {
      // Token Out; Out->Ref = Vector<T *,SizeType,Length>::operator[](i); return Out;
      return Vector<T *,Length>::operator[](i);
    }
  }; // class  PointerVector
} // namespace avp

#endif /* VECTOR_H_ */
