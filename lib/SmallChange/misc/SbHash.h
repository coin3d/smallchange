#ifndef COIN_SBHASH_H
#define COIN_SBHASH_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <cassert>
#include <cstddef> // NULL
#include <cstring> // memset()

#include <Inventor/SbBasic.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/C/base/memalloc.h>

// *************************************************************************

// We usually implement inline functions below the class definition,
// since we think that makes the file more readable. However, this is
// not done for this class, since Microsoft Visual C++ is not too
// happy about having functions declared as inline for a template
// class.

// *************************************************************************

template <class Type, class Key>
class SbHashEntry {
public:
  
  void * operator new(size_t size, cc_memalloc * memhandler) {
    SbHashEntry<Type, Key> * entry = (SbHashEntry<Type, Key> *)
      cc_memalloc_allocate(memhandler);
    entry->memhandler = memhandler;    
    return (void*) entry;
  }
  void operator delete(void * ptr) {
    SbHashEntry<Type, Key> * entry = (SbHashEntry<Type, Key> *) ptr;
    cc_memalloc_deallocate(entry->memhandler, ptr);
  }
  void operator delete(void * ptr, cc_memalloc * memhandler) {
    SbHashEntry<Type, Key> * entry = (SbHashEntry<Type, Key> *) ptr;
    cc_memalloc_deallocate(entry->memhandler, ptr);
  }
  
  Key key;
  Type obj;
  SbHashEntry<Type, Key> * next;
  cc_memalloc * memhandler;
};

// *************************************************************************

template <class Type, class Key>
class SbHash {
public:
  typedef uintptr_t SbHashFunc(const Key & key);
  typedef void SbHashApplyFunc(const Key & key, const Type & obj, void * closure);

public:
  SbHash(unsigned int sizearg = 256, float loadfactorarg = 0.0f)
  {
    this->commonConstructor(sizearg, loadfactorarg);
  }

  SbHash(const SbHash & from)
  {
    this->commonConstructor(from.size, from.loadfactor);
    this->operator=(from);
  }

  SbHash & operator=(const SbHash & from)
  {
    this->clear();
    from.apply(SbHash::copy_data, this);
    return *this;
  }

  ~SbHash()
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
        SbHashEntry<Type, Key> * entry = this->buckets[i];
        this->buckets[i] = entry->next;
        delete entry;
      }
    }
    memset(this->buckets, 0, this->size * sizeof(SbHashEntry<Type, Key> *));
    this->elements = 0;
  }

  SbBool put(const Key & key, const Type & obj)
  {
    unsigned int i = this->getIndex(key);
    SbHashEntry<Type, Key> * entry = this->buckets[i];
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
    entry = new (this->memhandler) SbHashEntry<Type, Key>;
    entry->key = key;
    entry->obj = obj;
    entry->next = this->buckets[i];
    this->buckets[i] = entry;

    if ( this->elements++ >= this->threshold ) { this->resize(this->size * 2); }
    return TRUE;
  }

  SbBool get(const Key & key, Type & obj) const
  {
    SbHashEntry<Type, Key> * entry;
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
    SbHashEntry<Type, Key> * entry = this->buckets[i], * next, * prev = NULL;
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

  void apply(SbHashApplyFunc * func, void * closure) const
  {
    unsigned int i;
    SbHashEntry<Type, Key> * elem;
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
    this->apply(SbHash::add_to_list, &l);
  }

  unsigned int getNumElements(void) const { return this->elements; }

  void setHashFunc(SbHashFunc * func) { this->hashfunc = func; }

protected:
  static uintptr_t default_hash_func(const Key & key) {
    return (uintptr_t) key;
  }

  unsigned int getIndex(const Key & key) const {
    unsigned int idx = this->hashfunc(key);
    idx -= (idx << 7); /* i.e. key = key * -127; */
    return idx & (this->size - 1);
  }

  void resize(unsigned int newsize) {
    /* we don't shrink the table */
    if (this->size >= newsize) return;
    /* assert(coin_is_power_of_two(newsize)); */

    unsigned int oldsize = this->size;
    SbHashEntry<Type, Key> ** oldbuckets = this->buckets;

    this->size = newsize;
    this->elements = 0;
    this->threshold = (unsigned int) (newsize * this->loadfactor);
    this->buckets = new SbHashEntry<Type, Key> * [newsize];
    memset(this->buckets, 0, this->size * sizeof(SbHashEntry<Type, Key> *));

    /* Transfer all mappings */
    unsigned int i;
    for ( i = 0; i < oldsize; i++ ) {
      SbHashEntry<Type, Key> * entry = oldbuckets[i];
      while ( entry ) {
        this->put(entry->key, entry->obj);
        SbHashEntry<Type, Key> * preventry = entry;
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
    unsigned int s = 1;
    while ( s < sizearg ) { s <<= 1; } // power-of-two size
    this->memhandler = cc_memalloc_construct(sizeof(SbHashEntry<Type, Key>));
    this->size = s;
    this->elements = 0;
    this->threshold = (unsigned int) (s * loadfactorarg);
    this->loadfactor = loadfactorarg;
    this->buckets = new SbHashEntry<Type, Key> * [this->size];
    memset(this->buckets, 0, this->size * sizeof(SbHashEntry<Type, Key> *));
    this->hashfunc = default_hash_func;
  }

  void getStats(int & buckets_used, int & buckets, int & elements, float & chain_length_avg, int & chain_length_max)
  {
    unsigned int i;
    buckets_used = 0, chain_length_max = 0;
    for ( i = 0; i < this->size; i++ ) {
      if ( this->buckets[i] ) {
        unsigned int chain_l = 0;
        SbHashEntry<Type, Key> * entry = this->buckets[i];
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
    SbHash * thisp = (SbHash *)closure;
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

  SbHashEntry<Type, Key> ** buckets;
  SbHashFunc * hashfunc;
  cc_memalloc * memhandler;
};

#endif // !COIN_SBHASH_H
