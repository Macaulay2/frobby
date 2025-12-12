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
#ifndef IDEAL_CONSOLIDATOR_GUARD
#define IDEAL_CONSOLIDATOR_GUARD

#include "BigTermConsumer.h"
#include "VarNames.h"
#include "BigIdeal.h"

#include <vector>

// This consumer forwards everything it consumes to the wrapped
// consumer. When it consumes an ideal one term at a time, it will
// store the ideal and pass it along as one big ideal.
class IdealConsolidator : public BigTermConsumer {
 public:
  IdealConsolidator(unique_ptr<BigTermConsumer> consumer);

  virtual void beginConsumingList();
  virtual void consumeRing(const VarNames& names);

  virtual void beginConsuming();
  virtual void consume(const Term& term);
  virtual void consume(const Term& term, const TermTranslator& translator);
  virtual void consume(const vector<mpz_class>& term);
  virtual void doneConsuming();

  virtual void consume(const BigIdeal& ideal);

  virtual void doneConsumingList();

private:
  const unique_ptr<BigTermConsumer> _consumer;
  BigIdeal _ideal;
  VarNames _names;
  bool _inList;
  bool _inIdeal;
  vector<mpz_class> _tmp;
};

#endif
