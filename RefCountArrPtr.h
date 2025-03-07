#pragma once

#include "../C_General/Error.h"

namespace avp {
  template<typename T>
  class RefCountArrPtr {
    struct PtrWithRefCnt_ {
      T* p;
      size_t Allocated, RefN;
      PtrWithRefCnt_(size_t sz): RefN(1) { p = new T[Allocated = sz]; }
      ~PtrWithRefCnt_() { delete[] p; }
    } *pPtrWithRefCnt;

    void ReplaceWith(const RefCountArrPtr &r) {
      ReleasePtrWithRefCnt();
      if((pPtrWithRefCnt = r.pPtrWithRefCnt) != nullptr)
        pPtrWithRefCnt->RefN++;
    } // ReplaceWith

   public:
    RefCountArrPtr():pPtrWithRefCnt(nullptr) {}

    RefCountArrPtr(size_t N) {
      pPtrWithRefCnt = N != 0?new PtrWithRefCnt_(N):nullptr;
    } // constructor

    RefCountArrPtr(const RefCountArrPtr &r) { ReplaceWith(r); }

    ~RefCountArrPtr() { ReleasePtrWithRefCnt(); }

    void ReleasePtrWithRefCnt() {
      if(pPtrWithRefCnt != nullptr && --pPtrWithRefCnt->RefN == 0)
        delete pPtrWithRefCnt;
    } // ReleasePtrWithRefCnt

    T* get() const {
      AVP_ASSERT(pPtrWithRefCnt != nullptr);
      return pPtrWithRefCnt->p;
    } // get

    const RefCountArrPtr &operator=(const RefCountArrPtr &r) {
      if(this != &r && pPtrWithRefCnt != r.pPtrWithRefCnt) ReplaceWith(r);
      return *this;
    }

    void clear() { ReleasePtrWithRefCnt(); pPtrWithRefCnt = nullptr; }
    size_t size() { return pPtrWithRefCnt == nullptr?0:pPtrWithRefCnt->Allocated; }

    T *begin() const { return get(); }
    T *end() const { return get() + pPtrWithRefCnt->Allocated; }
  }; // class RefCountPtr
} // namespace avp
