#pragma once


#warning NOT FINISHED, USE "CHAIN" IF POSSIBLE
namespace avp {
  template<typename T>
  struct Chain {
    class Link {
      const T *pContent;
      Link *pPrev, *pNext;
     public:
      Link(const T *pContent_):pContent(pContent_), pPrev(nullptr), pNext(nullptr) {}
      Link(const T *pContent_, Chain *pChain): pContent(pContent_) { pChain->Append(this); }
      ~Link() {
        if(pNext != nullptr) pNext->pPrev = pPrev;
        if(pPrev != nullptr) pPrev->pNext = pNext;
      }
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

    void Append(const T *pItem) { new Link(pItem, this); }

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

    const T *GetpItemP(const Link *p) {
      AVP_ASSERT(p != nullptr);
      return p->pContent;
    } // GetpItemP

    bool Contains(const T *pItem) {
      while(pEnd != &Start) {
        if(pEnd->pContent == pItem) return true;
        pEnd = pEnd->pPrev;
      }
      return false;
    } // Contains
   protected:
    Link Start, *pEnd; //!< Link Start is fudge Link to avoid checking for nullptr all the time
    //!< Link Start has pPrev == nullptr, that's how we detect it
  }; // class Chain


