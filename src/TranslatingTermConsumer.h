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
#ifndef TRANSLATING_TERM_CONSUMER_GUARD
#define TRANSLATING_TERM_CONSUMER_GUARD

#include "BigTermConsumer.h"

class BigTermConsumer;
class TermTranslator;
class VarNames;
class Term;

// The purpose of this class is to replace calls to consume(term) by
// consume(term, translator) when term is a Term. Thus untranslated
// terms get translated by the translator specified in the translator.
class TranslatingTermConsumer : public BigTermConsumer {
 public:
  // Does not copy the consumer or translator, so they need to remain
  // valid for the lifetime of this object. Does not take over
  // ownership of consumer, as is strongly indicated by it being a
  // reference as opposed to an unique_ptr.
  TranslatingTermConsumer(BigTermConsumer& consumer,
                          const TermTranslator& translator);

  // Does not copy the translator, so it needs to remain valid for the
  // lifetime of this object. Takes over ownership of consumer.
  TranslatingTermConsumer(unique_ptr<BigTermConsumer> consumer,
                          const TermTranslator& translator);

  virtual void beginConsumingList();
  virtual void consumeRing(const VarNames& names);
  virtual void beginConsuming();

  virtual void consume(const Term& term);
  virtual void consume(const vector<mpz_class>& term);
  virtual void consume(const Term& term, const TermTranslator& translator);

  virtual void doneConsuming();
  virtual void doneConsumingList();

  virtual void consume(const BigIdeal& ideal);

 private:
  const TermTranslator& _translator;
  BigTermConsumer& _consumer;
  unique_ptr<BigTermConsumer> _consumerOwner;
};

#endif
