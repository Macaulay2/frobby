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
#ifndef HASH_MAP_GUARD
#define HASH_MAP_GUARD

/** \file

 Defines HashMap, a hash map whose hash function is FrobbyHash<Key>,
 which must be specialized for each key type.
*/

#include <unordered_map>

template<class Key>
class FrobbyHash {};

template<class Key, class Value>
  class HashMap : public std::unordered_map<Key, Value, FrobbyHash<Key>> { };

#endif
