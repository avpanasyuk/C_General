#pragma once

#include <../C_General/Error.h>

namespace avp {
  struct Chain {
    class Link {
      Link *pPrev, *pNext;
     public:
       class Comparator {
         public:
         virtual bool IsIt(const Link *p) const = 0;
       }; // class Comparator
      Link(): pPrev(nullptr), pNext(nullptr) {}
      Link(Chain *pChain) {
        pChain->Append(this);
      }
      virtual ~Link() {
        if(pNext != nullptr) pNext->pPrev = pPrev;
        if(pPrev != nullptr) pPrev->pNext = pNext;
      }
      /**
      */
      friend class Chain;
    };

    Chain():pEnd(&Start) {}
    ~Chain() {
      while(pEnd != &Start) delete pEnd;
    } // destructor

    void Append(Link *pLink) {
      (pEnd->pNext = pLink)->pPrev = pEnd;
      pEnd = pLink;
    } // Append

    const Link *GetFirstP() {
      return Start.pNext;
    }
    const Link *GetNextP(const Link *p) {
      AVP_ASSERT(p != nullptr);
      return p->pNext;
    } // GetNextP

    const Link *GetPrevP(const Link *p) {
      AVP_ASSERT(p != nullptr);
      return p->pPrev;
    } // GetPrevP

    /**
    @param fComp: find first Link for which "fComp" returns true
    */
    const Link *FindFirst(const Link::Comparator &c) const {
      const Link *pCur = Start.pNext;
      while(pCur != nullptr) {
        if(c.IsIt(pCur)) return pCur;
        pCur = pCur->pNext;
      }
      return nullptr;
    } // FindFirst

    /**
    @param fComp: find last Link for which "fComp" returns true
    */
    const Link *FindLast(const Link::Comparator &c) const {
      const Link *pCur = pEnd;
      while(pCur != &Start) {
        if(c.IsIt(pCur)) return pCur;
        pCur = pCur->pPrev;
      }
      return nullptr;
    } // FindLast


   protected:
    Link Start, *pEnd; //!< Link Start is fudge Link to avoid checking for nullptr all the time
    //!< Link Start has pPrev == nullptr, that's how we detect it
  }; // class Chain
} // namespace avp
