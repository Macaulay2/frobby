/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2007 Bjarke Hammersholt Roune (www.broune.com)

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
#include "stdinc.h"
#include "Term.h"

#include "TermPredicate.h"
#include <sstream>
#include <vector>

const unsigned int PoolCount = 50;
const unsigned int ObjectPoolSize = 1000;

Term::Term(const string& str):
  _exponents(0), _varCount(0) {
  istringstream in(str);

  vector<Exponent> exponents;
  mpz_class ex;
  while (in >> ex) {
    ASSERT(ex.fits_uint_p());
    exponents.push_back(ex.get_ui());
  }

  if (!exponents.empty())
    initialize(&(exponents[0]), exponents.size());
}

namespace {
  struct ObjectPool {
    ObjectPool(): objectsStored(0), objects(0) {}

    void ensureInit() {
      if (objects == 0)
        objects = FROBBY_NEW Exponent*[ObjectPoolSize];
    }

    bool empty() const {
      return objectsStored == 0;
    }

    bool canStoreMore() const {
      return objectsStored < ObjectPoolSize;
    }

    Exponent* removeObject() {
      ASSERT(!empty());
      --objectsStored;
      return objects[objectsStored];
    }

    void addObject(Exponent* object) {
      ASSERT(canStoreMore());
      ASSERT(objects != 0);

      objects[objectsStored] = object;
      ++objectsStored;
    }

    ~ObjectPool() {
      if (objects == 0)
        return;
      for (size_t i = 0; i < objectsStored; ++i)
        delete[] objects[i];
      delete[] objects;
    }

    unsigned int objectsStored;
    Exponent** objects;
  } pools[PoolCount];
}

Exponent* Term::allocate(size_t size) {
  ASSERT(size > 0);

  if (size < PoolCount) {
    pools[size].ensureInit();
    if (!pools[size].empty())
      return pools[size].removeObject();
  }

  return FROBBY_NEW Exponent[size];
}

void Term::deallocate(Exponent* p, size_t size) {
  if (p == 0)
    return;

  ASSERT(size > 0);

  if (size < PoolCount && pools[size].canStoreMore())
    pools[size].addObject(p);
  else
    delete[] p;
}

void Term::print(FILE* file, const Exponent* e, size_t varCount) {
  ostringstream out;
  print(out, e, varCount);
  fputs(out.str().c_str(), file);
}

void Term::print(ostream& out, const Exponent* e, size_t varCount) {
  ASSERT(e != 0 || varCount == 0);

  out << '(';
  for (size_t var = 0; var < varCount; ++var) {
    if (var != 0)
      out << ", ";
    out << e[var];
  }
  out << ')';
}

bool Term::operator==(const Exponent* term) const {
  return equals(begin(), term, getVarCount());
}
