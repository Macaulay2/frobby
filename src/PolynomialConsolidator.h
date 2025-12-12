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
#ifndef POLYNOMIAL_CONSOLIDATOR_GUARD
#define POLYNOMIAL_CONSOLIDATOR_GUARD

#include "CoefBigTermConsumer.h"
#include "BigPolynomial.h"

// Consolidates polynomials that are consumed one term at a time into
// a polynomial and then passes the polynomial along in one piece.
class PolynomialConsolidator : public CoefBigTermConsumer {
 public:
  PolynomialConsolidator(unique_ptr<CoefBigTermConsumer> consumer);

  virtual void consumeRing(const VarNames& names);

  virtual void beginConsuming();
  virtual void consume
    (const mpz_class& coef,
     const Term& term,
     const TermTranslator& translator);
  virtual void consume
    (const mpz_class& coef, const vector<mpz_class>& term);
  virtual void doneConsuming();

  virtual void consume(const BigPolynomial& poly);

 private:
  const unique_ptr<CoefBigTermConsumer> _consumer;
  BigPolynomial _poly;
};

#endif
