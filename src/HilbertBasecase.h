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
#ifndef HILBERT_BASECASE_GUARD
#define HILBERT_BASECASE_GUARD

class Ideal;

#include "Term.h"
#include "ElementDeleter.h"

#include <vector>

class HilbertBasecase {
 public:
  HilbertBasecase();
  ~HilbertBasecase();

  // Clears ideal.
  void computeCoefficient(Ideal& ideal);

  const mpz_class& getLastCoefficient();

 private:
  struct Entry {
    bool negate;
    size_t extraSupport;
    Ideal* ideal;
  };
  vector<Entry> _todo;
  void clearTodo();

  bool stepComputation(Entry& entry, Entry& newEntry);

  unique_ptr<Ideal> getNewIdeal();
  void freeIdeal(unique_ptr<Ideal> ideal);
  vector<Ideal*> _idealCache;
  ElementDeleter<vector<Ideal*> > _idealCacheDeleter;

  bool canSimplify(size_t var, const Ideal& ideal, const Term& counts);
  size_t eliminate1Counts(Ideal& ideal, Term& counts, bool& negate);

  mpz_class _sum;
  Term _term;
  size_t _stepsPerformed;
};

#endif
