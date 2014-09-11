/**
  * @file
  * @author Alexander Panasyuk
  */

#ifndef UNICHAIN_HPP_INCLUDED
#define UNICHAIN_HPP_INCLUDED

/** @brief Creates unidirectionally linked chain, with every link pointing to an object
 * User should keep pointer to the last Unilink.
 *
 */

template<typename T> class UniLink {
protected:
  const UniLink * const pNext;
  const T *obj;
public:
  UniLink(const UniLink *pLast): pNext(pLast) {}

  // UniLink *GetNext() { return pLast; }
  virtual bool IsIt(IsItType T) const = 0;
}; // class UniLink
#endif /* UNICHAIN_HPP_INCLUDED */
