/*
 * Vector.h
 *
 * Created: 11/9/2013 5:09:10 PM
 *  Author: panasyuk
 */


#ifndef VECTOR_H_
#define VECTOR_H_


namespace avp {
  extern void (*bad_index_func)();
  extern void (*bad_pointer_func)();

  template<typename T, typename SizeType, SizeType Length> class Vector {
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
  }; // class Vector

  template<typename T, typename SizeType, SizeType Length> class PointerVector:
    public Vector<T *, SizeType, Length> {
    // this is vector of pointers which never lets dereference NULL pointer
    // instead of Vector::operator[] which returns T* &
    // PointerVector::operator[] return Token which can be dereferenced
    // as a T*, but checks it for non-NULL first
    class Token {
      T *& Ref;
      friend class PointerVector;
      Token(T *& r):Ref(r) {}
    public:
      operator T *& () const { return Ref; }
      Token & operator=(T *Ptr) { Ref = Ptr; return *this; }  
      T& operator*() const {
        if(Ref == NULL) (*bad_pointer_func)();
        return *Ref; // we dereferencing NULL pointer here, but program
        // should terminate on the line above
      }
      T* operator-> () const { return &(this->operator*()); }
    }; // Token
  public:
    Token operator[](uint8_t i) {
      // Token Out; Out->Ref = Vector<T *,SizeType,Length>::operator[](i); return Out;
      return Vector<T *,SizeType,Length>::operator[](i);
    }
  }; // class  PointerVector

  template<typename T, uint8_t Length> class Vector8: // Vector with uint8_t size, so 255 elements max 
    public Vector<T, uint8_t, Length> {};
} // namespace avp

#endif /* VECTOR_H_ */