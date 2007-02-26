#ifndef SM_HASH_H
#define SM_HASH_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2005 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  <URL:http://www.sim.no/>.
 *
\**************************************************************************/

// *************************************************************************
// This class (SmHash<Type, Key>) is internal and must not be exposed
// in the Coin API.

// *************************************************************************

#include <assert.h>
#include <stddef.h> // NULL
#include <string.h> // memset()

#include <Inventor/lists/SbList.h>
#include <Inventor/C/base/memalloc.h>
#include <Inventor/C/tidbits.h>

/* table containing the next prime number for all power of twos from
   2^1 to 2^32 (the 2^32 prime is obviously less than 2^32 though) */
static unsigned long smhash_prime_table[32] = {
  2,
  5,
  11,
  17,
  37,
  67,
  131,
  257,
  521,
  1031,
  2053,
  4099,
  8209,
  16411,
  32771,
  65537,
  131101,
  262147,
  524309,
  1048583,
  2097169,
  4194319,
  8388617,
  16777259,
  33554467,
  67108879,
  134217757,
  268435459,
  536870923,
  1073741827,
  2147483659U,
  4294967291U /* 2^32 = 4294967296 */
};

inline unsigned long 
smhash_geq_prime_number(unsigned long num)
{
  int i;
  for (i = 0; i < 32; i++) {
    if (smhash_prime_table[i] >= num) {
      return smhash_prime_table[i];
    }
  }
  /* just return num if we can't find a bigger prime number */
  return num;
}

// *************************************************************************

// We usually implement inline functions below the class definition,
// since we think that makes the file more readable. However, this is
 // not done for this class, since Microsoft Visual C++ is not too
// happy about having functions declared as inline for a template
// class.

// *************************************************************************

template <class Type, class Key>
class SmHashEntry {
public:
  
  void * operator new(size_t size, cc_memalloc * memhandler) {
    SmHashEntry<Type, Key> * entry = (SmHashEntry<Type, Key> *)
      cc_memalloc_allocate(memhandler);
    entry->memhandler = memhandler;    
    return (void*) entry;
  }
  void operator delete(void * ptr) {
    SmHashEntry<Type, Key> * entry = (SmHashEntry<Type, Key> *) ptr;
    cc_memalloc_deallocate(entry->memhandler, ptr);
  }
  void operator delete(void * ptr, cc_memalloc * memhandler) {
    SmHashEntry<Type, Key> * entry = (SmHashEntry<Type, Key> *) ptr;
    cc_memalloc_deallocate(entry->memhandler, ptr);
  }
  
  Key key;
  Type obj;
  SmHashEntry<Type, Key> * next;
  cc_memalloc * memhandler;
};

// *************************************************************************

template <class Type, class Key>
class SmHash {
public:
  typedef uintptr_t SmHashFunc(const Key & key);
  typedef void SmHashApplyFunc(const Key & key, const Type & obj, void * closure);

public:
  SmHash(unsigned int sizearg = 256, float loadfactorarg = 0.0f)
  {
    this->commonConstructor(sizearg, loadfactorarg);
  }

  SmHash(const SmHash & from)
  {
    this->commonConstructor(from.size, from.loadfactor);
    this->operator=(from);
  }

  SmHash & operator=(const SmHash & from)
  {
    this->clear();
    from.apply(SmHash::copy_data, this);
    return *this;
  }

  ~SmHash()
  {
    this->clear();
    cc_memalloc_destruct(this->memhandler);
    delete [] this->buckets;
  }

  void clear(void)
  {
    unsigned int i;
    for ( i = 0; i < this->size; i++ ) {
      while ( this->buckets[i] ) {
        SmHashEntry<Type, Key> * entry = this->buckets[i];
        this->buckets[i] = entry->next;
        delete entry;
      }
    }
    memset(this->buckets, 0, this->size * sizeof(SmHashEntry<Type, Key> *));
    this->elements = 0;
  }

  SbBool put(const Key & key, const Type & obj)
  {
    unsigned int i = this->getIndex(key);
    SmHashEntry<Type, Key> * entry = this->buckets[i];
    while ( entry ) {
      if ( entry->key == key ) {
        /* Replace the old value */
        entry->obj = obj;
        return FALSE;
      }
      entry = entry->next;
    }

    /* Key not already in the hash table; insert a new
     * entry as the first element in the bucket
     */
    entry = new (this->memhandler) SmHashEntry<Type, Key>;
    entry->key = key;
    entry->obj = obj;
    entry->next = this->buckets[i];
    this->buckets[i] = entry;

    if ( this->elements++ >= this->threshold ) { 
      this->resize((unsigned int) smhash_geq_prime_number(this->size + 1)); 
    }
    return TRUE;
  }

  SbBool get(const Key & key, Type & obj) const
  {
    SmHashEntry<Type, Key> * entry;
    unsigned int i = this->getIndex(key);
    entry = this->buckets[i];
    while ( entry ) {
      if ( entry->key == key ) {
        obj = entry->obj;
        return TRUE;
      }
      entry = entry->next;
    }
    return FALSE;
  }

  SbBool remove(const Key & key)
  {
    unsigned int i = this->getIndex(key);
    SmHashEntry<Type, Key> * entry = this->buckets[i], * next, * prev = NULL;
    while ( entry ) {
      next = entry->next;
      if ( entry->key == key ) {
        this->elements--;
        if ( prev == NULL) {
          this->buckets[i] = next;
        }
        else {
          prev->next = next;
        }
        delete entry;
        return TRUE;
      }
      prev = entry;
      entry = next;
    }
    return FALSE;
  }

  void apply(SmHashApplyFunc * func, void * closure) const
  {
    unsigned int i;
    SmHashEntry<Type, Key> * elem;
    for ( i = 0; i < this->size; i++ ) {
      elem = this->buckets[i];
      while ( elem ) {
        func(elem->key, elem->obj, closure);
        elem = elem->next;
      }
    }
  }

  void makeKeyList(SbList<Key> & l) const
  {
    this->apply(SmHash::add_to_list, &l);
  }

  unsigned int getNumElements(void) const { return this->elements; }

  void setHashFunc(SmHashFunc * func) { this->hashfunc = func; }

protected:
  static uintptr_t default_hash_func(const Key & key) {
    return (uintptr_t) key;
  }

  unsigned int getIndex(const Key & key) const {
    unsigned int idx = this->hashfunc(key);
    return (idx % this->size);
  }

  void resize(unsigned int newsize) {
    /* we don't shrink the table */
    if (this->size >= newsize) return;

    unsigned int oldsize = this->size;
    SmHashEntry<Type, Key> ** oldbuckets = this->buckets;

    this->size = newsize;
    this->elements = 0;
    this->threshold = (unsigned int) (newsize * this->loadfactor);
    this->buckets = new SmHashEntry<Type, Key> * [newsize];
    memset(this->buckets, 0, this->size * sizeof(SmHashEntry<Type, Key> *));

    /* Transfer all mappings */
    unsigned int i;
    for ( i = 0; i < oldsize; i++ ) {
      SmHashEntry<Type, Key> * entry = oldbuckets[i];
      while ( entry ) {
        this->put(entry->key, entry->obj);
        SmHashEntry<Type, Key> * preventry = entry;
        entry = entry->next;
        delete preventry;
      }
    }
    delete [] oldbuckets;
  }

private:
  void commonConstructor(unsigned int sizearg, float loadfactorarg)
  {
    if ( loadfactorarg <= 0.0f ) { loadfactorarg = 0.75f; }
    unsigned int s = smhash_geq_prime_number(sizearg);
    this->memhandler = cc_memalloc_construct(sizeof(SmHashEntry<Type, Key>));
    this->size = s;
    this->elements = 0;
    this->threshold = (unsigned int) (s * loadfactorarg);
    this->loadfactor = loadfactorarg;
    this->buckets = new SmHashEntry<Type, Key> * [this->size];
    memset(this->buckets, 0, this->size * sizeof(SmHashEntry<Type, Key> *));
    this->hashfunc = default_hash_func;
  }

  void getStats(int & buckets_used, int & buckets, int & elements, float & chain_length_avg, int & chain_length_max)
  {
    unsigned int i;
    buckets_used = 0, chain_length_max = 0;
    for ( i = 0; i < this->size; i++ ) {
      if ( this->buckets[i] ) {
        unsigned int chain_l = 0;
        SmHashEntry<Type, Key> * entry = this->buckets[i];
        buckets_used++;
        while ( entry ) {
          chain_l++;
          entry = entry->next;
        }
        if ( chain_l > chain_length_max ) { chain_length_max = chain_l; }
      }
    }
    buckets = this->size;
    elements = this->elements;
    chain_length_avg = (float) this->elements / buckets_used;
  }

  static void copy_data(const Key & key, const Type & obj, void * closure)
  {
    SmHash * thisp = (SmHash *)closure;
    thisp->put(key, obj);
  }

  static void add_to_list(const Key & key, const Type & obj, void * closure)
  {
    SbList<Key> * l = (SbList<Key> *)closure;
    l->append(key);
  }

  float loadfactor;
  unsigned int size;
  unsigned int elements;
  unsigned int threshold;

  SmHashEntry<Type, Key> ** buckets;
  SmHashFunc * hashfunc;
  cc_memalloc * memhandler;
};

#endif // !SM_HASH_H
