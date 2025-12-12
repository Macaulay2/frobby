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
#ifndef BIG_TERM_RECORDER_GUARD
#define BIG_TERM_RECORDER_GUARD

#include "BigTermConsumer.h"
#include "ElementDeleter.h"

#include "VarNames.h"

#include <list>

class BigIdeal;
class TermTranslator;

/** BigTermRecorder records all the terms it consumes into an ideal. */
class BigTermRecorder : public BigTermConsumer {
public:
  BigTermRecorder();

  virtual void consumeRing(const VarNames& names);
  virtual void beginConsuming();
  virtual void consume(const Term& term, const TermTranslator& translator);
  virtual void consume(const vector<mpz_class>& term);
  virtual void doneConsuming();

  virtual void consume(unique_ptr<BigIdeal> ideal);

  // Returns true if this object currently stores no ideals.
  bool empty() const;

  // Returns the least recently consumed ideal from this and returns it.
  // It is a precondition that empty() is false.
  unique_ptr<BigIdeal> releaseIdeal();

  // Returns the most recently consumed ring.
  const VarNames& getRing();

private:
  VarNames _names;
  size_t _idealCount;
  list<BigIdeal*> _ideals; // zero entries are treated as if not there.
  ElementDeleter<list<BigIdeal*> > _idealsDeleter;
};

#endif
