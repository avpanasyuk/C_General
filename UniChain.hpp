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

class UniChain {
	class Link {
		friend class UniChain;
		const Link *pPrev;
		virtual ~Link() {}; 
		
		
		} *pLast;
	UniChain(): pLast(nullptr) {}
	void Add(Link *pNew) { pNew->pPrev = pLast; pLast = pNew; }
				
	};
#endif /* UNICHAIN_HPP_INCLUDED */
