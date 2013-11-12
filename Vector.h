/*
 * Vector.h
 *
 * Created: 11/9/2013 5:09:10 PM
 *  Author: panasyuk
 */


#ifndef VECTOR_H_
#define VECTOR_H_

extern void (*bad_index_func)();

namespace avp {
  template<typename T, uint8_t Length> class Vector8 {
    protected:
    T Data[Length];
    public:
    T &operator[](uint8_t i) {
#ifdef DEBUG
      if(i>=Length) (*bad_index_func)();
#endif
      return Data[i];
    }
    // operator T *() { return Data; }
  }; // class Vector8
} // namespace avp

#endif /* VECTOR_H_ */