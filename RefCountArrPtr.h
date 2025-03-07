#pragma once

namespace avp {
  template<typename T>
  class RefCountArrPtr {
    struct PtrWithRefCnt_ {
      T* p;
      size_t RefN;
    } *pPtrWithRefCnt;

    void ReplaceWith(const RefCountArrPtr &r) {
      ReleasePtrWithRefCnt();
      if((pPtrWithRefCnt = r.pPtrWithRefCnt) != nullptr)
        pPtrWithRefCnt->RefN++;
    } // ReplaceWith

   public:
    RefCountArrPtr():pPtrWithRefCnt(nullptr) {}

    RefCountArrPtr(size_t N) {
      if(N != 0) {
        pPtrWithRefCnt = new PtrWithRefCnt_;
        pPtrWithRefCnt->p = new T[N];
        pPtrWithRefCnt->RefN = 1;
      } else pPtrWithRefCnt = nullptr;
    } // constructor

    RefCountArrPtr(const RefCountArrPtr &r) { ReplaceWith(r); }

    ~RefCountArrPtr() { ReleasePtrWithRefCnt(); }

    void ReleasePtrWithRefCnt() {
      if(pPtrWithRefCnt != nullptr && --pPtrWithRefCnt->RefN == 0) {
        delete[] pPtrWithRefCnt->p;
        delete pPtrWithRefCnt;
      }
    } // ReleasePtrWithRefCnt

    T* get() const { return pPtrWithRefCnt != nullptr?pPtrWithRefCnt->p:nullptr; }

    const RefCountArrPtr &operator=(const RefCountArrPtr &r) {
      if(this != &r && pPtrWithRefCnt != r.pPtrWithRefCnt) ReplaceWith(r);
      return *this;
    }

    void clear() { ReleasePtrWithRefCnt(); pPtrWithRefCnt = nullptr; }
  }; // class RefCountPtr
} // namespace avp
