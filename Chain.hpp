#pragma once

#include <../C_General/Error.h>

namespace avp {
  /**
    This class allows to link, e.g. instances of static template classes by including into each a static field inherited from "Link"
  */
  struct Chain {
    /**
    this is pure virtual which does not include key
    */
    class Link {
      Link *pPrev, *pNext;
     public:
      class Comparator {
       public:
        virtual bool IsIt(const Link *pLink) const = 0;
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
    }; // class Link

    Chain():pEnd(&Start) {}
    virtual ~Chain() {
      while(pEnd != &Start) delete pEnd;
    } // destructor

    void Append(Link *pLink) {
      (pEnd->pNext = pLink)->pPrev = pEnd;
      pEnd = pLink;
    } // Append


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

  template<typename key_type>
  struct ChainByKey: public Chain {
    class Link: public Chain::Link {
      key_type Key;
     public:
      class Comparator: public Chain::Link::Comparator {
        key_type Key; // key
       public:
        Comparator(key_type Key_): Key(Key_) {}
        virtual bool IsIt(const Chain::Link *pLink) const {
          return ((const Link *)pLink)->Key == Key;
        };
      }; // class Comparator
      Link(key_type Key_): Key(Key_) {}
      Link(key_type Key_, Chain *pChain): Chain::Link(pChain), Key(Key_) {}
    }; // class LinkPointer

    /**
    @param fComp: find first Link for which "fComp" returns true
    */
    const Link *FindFirst(key_type Key) const {
      return (const Link *)Chain::FindFirst(typename ChainByKey<key_type>::Link::Comparator(Key));
    } // FindFirst

    /**
    @param fComp: find last Link for which "fComp" returns true
    */
    const Link *FindLast(key_type Key) const {
      return (const Link *)Chain::FindLast(typename ChainByKey<key_type>::Link::Comparator(Key));
    } // FindLast
  }; // class ChainByKey
} // namespace avp
