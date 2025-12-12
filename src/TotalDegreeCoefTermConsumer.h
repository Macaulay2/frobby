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
#ifndef TOTAL_DEGREE_COEF_TERM_CONSUMER_GUARD
#define TOTAL_DEGREE_COEF_TERM_CONSUMER_GUARD

#include "CoefTermConsumer.h"
#include "UniHashPolynomial.h"

class Term;
class CoefBigTermConsumer;
class TermTranslator;

#include <map>

// Substitutes the same single variable for each variable, in effect
// grading by the total degree. Passes consumed items on in descending
// order of exponent, i.e. reverse lex order. This requires storing
// all items before any can be passed on.
//
// TODO: get rid of translator in constructor.
class TotalDegreeCoefTermConsumer : public CoefTermConsumer {
 public:
  TotalDegreeCoefTermConsumer(unique_ptr<CoefBigTermConsumer> consumer,
                              const TermTranslator& translator);
  TotalDegreeCoefTermConsumer(CoefBigTermConsumer& consumer,
                              const TermTranslator& translator);

  virtual void consumeRing(const VarNames& names);

  virtual void beginConsuming();
  virtual void consume(const mpz_class& coef, const Term& term);
  virtual void doneConsuming();

 private:
  CoefBigTermConsumer& _consumer;
  unique_ptr<CoefBigTermConsumer> _consumerOwner;
  const TermTranslator& _translator;
  mpz_class _tmp;

  UniHashPolynomial _poly;
};

#endif
