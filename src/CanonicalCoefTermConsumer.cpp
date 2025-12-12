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
#include "CanonicalCoefTermConsumer.h"

#include "Term.h"

CanonicalCoefTermConsumer::
CanonicalCoefTermConsumer(unique_ptr<CoefTermConsumer> consumer):
  _consumer(std::move(consumer)) {
  ASSERT(_consumer.get() != 0);
}

void CanonicalCoefTermConsumer::consumeRing(const VarNames& names) {
  _polynomial.clearAndSetVarCount(names.getVarCount());
  _names = names;
}

void CanonicalCoefTermConsumer::beginConsuming() {
}

void CanonicalCoefTermConsumer::consume
(const mpz_class& coef, const Term& term) {
  ASSERT(term.getVarCount() == _polynomial.getVarCount());

  _polynomial.add(coef, term);
}

void CanonicalCoefTermConsumer::doneConsuming() {
  _polynomial.sortTermsReverseLex();

  _consumer->consumeRing(_names);
  _consumer->beginConsuming();
  for (size_t index = 0; index < _polynomial.getTermCount(); ++index)
    _consumer->consume(_polynomial.getCoef(index), _polynomial.getTerm(index));
  _consumer->doneConsuming();
  _polynomial.clear();
}
