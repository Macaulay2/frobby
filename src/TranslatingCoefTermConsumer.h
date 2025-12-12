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
#ifndef TRANSLATING_COEF_TERM_CONSUMER_GUARD
#define TRANSLATING_COEF_TERM_CONSUMER_GUARD

#include "CoefBigTermConsumer.h"

class CoefBigTermConsumer;
class Term;
class TermTranslator;

class TranslatingCoefTermConsumer : public CoefBigTermConsumer {
 public:
  TranslatingCoefTermConsumer
    (CoefBigTermConsumer& consumer, const TermTranslator& translator);
  TranslatingCoefTermConsumer
    (unique_ptr<CoefBigTermConsumer> consumer, const TermTranslator& translator);

  virtual void consumeRing(const VarNames& names);

  virtual void beginConsuming();
  virtual void consume(const mpz_class& coef, const Term& term);
  virtual void consume(const mpz_class& coef,
                       const Term& term,
                       const TermTranslator& translator);
  virtual void consume(const mpz_class& coef,
                       const vector<mpz_class>& term);
  virtual void doneConsuming();

  virtual void consume(const BigPolynomial& poly);

 private:
  const TermTranslator& _translator;
  CoefBigTermConsumer& _consumer;
  unique_ptr<CoefBigTermConsumer> _consumerOwner;
};

#endif
