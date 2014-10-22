#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdlib.h>
#include "General.h"

namespace avp {
  extern Fail::function bad_index_func;
  extern Fail::function bad_pointer_func;

  template<typename T, size_t Length> class Vector {
    // index checking vector
  protected:
    T Data[Length];
  public:
    T &operator[](uint8_t i) {
#ifdef DEBUG
      if(i>=Length) (*bad_index_func)();
#endif
      return Data[i];
    }
    // operator T const *() const { return Data; }
  }; // class Vector

  template<typename T, size_t Length> class PointerVector:
    public Vector<T *, Length> {
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
        return *Ref; // we dereferencing NULL pointer here, but program
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
