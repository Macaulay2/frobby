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
#include "PolynomialConsolidator.h"

PolynomialConsolidator::PolynomialConsolidator
(unique_ptr<CoefBigTermConsumer> consumer):
  _consumer(std::move(consumer)) {
}

void PolynomialConsolidator::consumeRing(const VarNames& names) {
  _poly.clearAndSetNames(names);
}

void PolynomialConsolidator::beginConsuming() {
  ASSERT(_poly.getTermCount() == 0);
}

void PolynomialConsolidator::consume
(const mpz_class& coef,
 const Term& term,
 const TermTranslator& translator) {
  _poly.add(coef, term, translator);
}

void PolynomialConsolidator::consume
(const mpz_class& coef, const vector<mpz_class>& term) {
  _poly.add(coef, term);
}

void PolynomialConsolidator::doneConsuming() {
  _consumer->consume(_poly);
  _poly.clear();
}

void PolynomialConsolidator::consume(const BigPolynomial& poly) {
  _consumer->consume(poly);
}
