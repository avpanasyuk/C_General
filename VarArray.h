#pragma once
#include "Array.h"

namespace avp {
/**
  * class similar to Array but it cam be filled only partially
  * @tparam ReservedSz - maximum Size
  */
  template <typename T, size_t ReservedSz>
  class VarArray : public Array<T, ReservedSz> {
    // index checking Array
  protected:
    // T *Start = Array<T, ReservedSz>::Data;
    size_t Size; //!< actual array size

  public:
    VarArray() : Size(0) {}
    VarArray(const std::initializer_list<T> &t) : Size(t.size()), Array(t) {}

    template <size_t L, typename T1 = T>
    VarArray(const Array<T1, L> &rhs) {
      AVP_ASSERT(L <= ReservedSz);
      for(size_t i; i < L; ++i) Data[i] = T(rhs[i]);
      Size = L;
    }

    template <size_t L, typename T1 = T>
    VarArray(const VarArray<T1, L> &rhs) {
      AVP_ASSERT(rhs.Size <= ReservedSz);
      for(size_t i; i < rhs.Size; ++i) Data[i] = T(rhs[i]);
      Size = rhs.Size;
    }
    virtual Iterator<const T> const end() const { // const version
      return Iterator<const T>(Data + Size);
    }
    virtual Iterator<T> end() { // non-const version
      return Iterator<T>(Data + Size);
    }

    template <size_t L, typename T1 = T>
    const VarArray &operator = (const Array<T1, L> &rhs) {

      static_assert(L <= ReservedSz, "RHS array is too big!");
      for(size_t i = 0; i < L; i++) Data[i] = T(rhs[i]);
      Size = L;
      return *this;
    }
    template <size_t L, typename T1 = T>
    const VarArray &operator  = (const VarArray<T1, L> &rhs) {
      AVP_ASSERT(rhs.Size <= ReservedSz);
      for(size_t i = 0; i < rhs.Size; i++) Data[i] = T(rhs[i]);
      Size = rhs.Size;
      return *this;
    }

    virtual T &operator[](size_t i) {
      AVP_ASSERT(i < Size);
      return Data[i];
    }

    virtual const T &operator[](size_t i) const {
      AVP_ASSERT(i < Size);
      return Data[i];
    }

    virtual size_t size() const { return Size; }

    virtual const T *after_last() const {
      return Data + ReservedSz;
    }

    void push_back(const T &x) { 
      AVP_ASSERT(Size < Length);
      (*this)[Size++] = x; 
    }

    void set_size(size_t sz = 0) { Size = sz; }

    // FRIENDS
    inline friend bool operator!=(VarArray const &v1, VarArray const &v2) {
      if(&v1 == &v2) return false;
      if(v1.Size != v2.Size) return true;
      for(size_t i = 0; i < v1.Size; ++i)
        if(v1.Data[i] != v2.Data[i]) return true;
      return false;
    }  // operator!=
    inline friend bool operator==(VarArray const &v1, VarArray const &v2) {
      return !(v1 != v2);
    }
  };  // class VarArray
} //  namespace avp

