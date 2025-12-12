/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 University of Aarhus
   Contact Bjarke Hammersholt Roune for license information (www.broune.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see http://www.gnu.org/licenses/.
*/
#ifndef OBJECT_CACHE_GUARD
#define OBJECT_CACHE_GUARD

#include "ElementDeleter.h"

#include <vector>

/**
 An ObjectCache keeps a cache of previously-used objects to cut down
 on the number of allocations. The cache accepts unused objects that
 would otherwise be due for deallocation, and returns them in place of
 a new object when asked for one. If the cache is asked for an object
 and it is not storing any, it will transparently allocate one to
 satisfy the request. Stored objects are not freed until the cache is
 destructed.
*/
template<class T>
class ObjectCache {
 public:
  ObjectCache();

  /** Returns an object. The new object is constructed using the
   standard constructor, or it is an object previously passed to freeObject().
  */
  unique_ptr<T> newObject();

  /** Returns a copy of copyOf. The new object is constructed using
   the copy constructor, or it is an object previously passed to
   freeObject() that has copyOf assigned to it.
  */
  template<class S>
  unique_ptr<T> newObjectCopy(const S& copyOf);

  /** Insert an object into the cache. */
  template<class S>
  void freeObject(unique_ptr<S> object);

 private:
  vector<T*> _cache;
  ElementDeleter<vector<T*> > _cacheDeleter;
};

// The implementation has to be inline because we are using templates.

template<class T>
inline ObjectCache<T>::ObjectCache():
  _cache(),
  _cacheDeleter(_cache) {
}

template<class T>
unique_ptr<T> ObjectCache<T>::newObject() {
  if (_cache.empty())
    return unique_ptr<T>(new T());

  unique_ptr<T> object(_cache.back());
  _cache.pop_back();
  return object;
}

template<class T> template<class S>
unique_ptr<T> ObjectCache<T>::newObjectCopy(const S& copyOf) {
  if (_cache.empty())
    return unique_ptr<T>(new T(copyOf));

  unique_ptr<T> object(_cache.back());
  _cache.pop_back();
  *object = copyOf;
  return object;
}

template<class T> template<class S>
void ObjectCache<T>::freeObject(unique_ptr<S> object) {
  ASSERT(dynamic_cast<T*>(object.get()) != 0);

  unique_ptr<T> casted(static_cast<T*>(object.release()));
  noThrowPushBack(_cache, std::move(casted));
}

#endif
