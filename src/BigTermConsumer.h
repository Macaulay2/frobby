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
#ifndef BIG_TERM_CONSUMER_GUARD
#define BIG_TERM_CONSUMER_GUARD

#include "TermConsumer.h"

#include <vector>

class Term;
class TermTranslator;
class VarNames;
class BigIdeal;

class BigTermConsumer : public TermConsumer {
 public:
  virtual ~BigTermConsumer();

  virtual void consumeRing(const VarNames& names) = 0;
  virtual void beginConsuming() = 0;
  virtual void consume(const vector<mpz_class>& term) = 0;
  virtual void doneConsuming() = 0;

  virtual void consume(const Term& term);
  virtual void consume(const Term& term, const TermTranslator& translator) = 0;
  virtual void consume(const BigIdeal& ideal);
  virtual void consume(unique_ptr<BigIdeal> ideal);

  // Calling this convenience method is equivalent to
  //   consumeRing(names);
  //   beginConsuming();
  void beginConsuming(const VarNames& names);
};

#endif
