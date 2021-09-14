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
#include "IdealConsolidator.h"

#include "Term.h"
#include "TermTranslator.h"

IdealConsolidator::IdealConsolidator(auto_ptr<BigTermConsumer> consumer):
  _consumer(std::move(consumer)),
  _inList(false),
  _inIdeal(false) {
}

void IdealConsolidator::consumeRing(const VarNames& names) {
  ASSERT(!_inIdeal);

  _consumer->consumeRing(names);

  _names = names;
  _ideal.clearAndSetNames(names);
  _tmp.resize(names.getVarCount());
}

void IdealConsolidator::beginConsumingList() {
  ASSERT(!_inList);
  ASSERT(!_inIdeal);

  _consumer->beginConsumingList();
  _inList = true;
}

void IdealConsolidator::beginConsuming() {
  ASSERT(!_inIdeal);
  _inIdeal = true;
}

void IdealConsolidator::consume(const Term& term) {
  ASSERT(term.getVarCount() == _tmp.size());

  for (size_t var = 0; var < term.getVarCount(); ++var)
    _tmp[var] = term[var];
  consume(_tmp);
}

void IdealConsolidator::consume
(const Term& term, const TermTranslator& translator) {
  ASSERT(term.getVarCount() == _tmp.size());

  for (size_t var = 0; var < term.getVarCount(); ++var)
    _tmp[var] = translator.getExponent(var, term);
  consume(_tmp);
}

void IdealConsolidator::consume(const vector<mpz_class>& term) {
  ASSERT(term.size() == _ideal.getVarCount());
  // TODO: Add a method to BigIdeal that does this in one step.
  _ideal.newLastTerm();
  _ideal.getLastTermRef() = term;
}

void IdealConsolidator::doneConsuming() {
  ASSERT(_inIdeal);

  _inIdeal = false;
  _consumer->consume(_ideal);
  _ideal.clear();
}

void IdealConsolidator::doneConsumingList() {
  ASSERT(_inList);
  _inList = false;
  _consumer->doneConsumingList();
}

void IdealConsolidator::consume(const BigIdeal& ideal) {
  _consumer->consume(ideal);
}
