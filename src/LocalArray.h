/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2011 University of Aarhus
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
#ifndef LOCAL_ARRAY_GUARD
#define LOCAL_ARRAY_GUARD

#include "Arena.h"
#include <utility>

/** Emulates stack allocation of an array using an Arena.

 The scoping rules of C++ ensure stack order allocation and
 deallocation if the Arena is only accessed using objects like this.

 T is the type of the objects to make an array of.

 ArenaSource is any type that has a public and static getArena
 function. That function must return a reference to the same Arena
 each time it is called. ArenaSource is useful for testing.
*/
template<class T, class ArenaSource = Arena>
class LocalArray {
 public:
 LocalArray(const size_t sizeParam):
  _size(sizeParam),
  _range(ArenaSource::getArena().template allocArray<T>(sizeParam)) {
	ASSERT(this->size() == static_cast<size_t>(end() - begin()));
  }

  ~LocalArray() {
	ASSERT(size() == static_cast<size_t>(end() - begin()));
	ArenaSource::getArena().freeTopArray(begin(), end());
  }

  T& operator[](const size_t i) const {
    ASSERT(i < size());
	return begin()[i];
  }

  T* begin() const {return _range.first;}
  T* end() const {return _range.second;}
  size_t size() const {return _size;}

 private:
  // Everything is inline and should be on the stack and be unlikely
  // to have its address taken. So the compiler should be able to
  // eliminate any of these member variables that are not used.
 const size_t _size;
 const pair<T*, T*> _range;
};

#endif
