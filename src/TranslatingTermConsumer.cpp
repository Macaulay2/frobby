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
#include "TranslatingTermConsumer.h"

#include "Term.h"
#include "TermTranslator.h"
#include "BigTermConsumer.h"

TranslatingTermConsumer::TranslatingTermConsumer
(BigTermConsumer& consumer, const TermTranslator& translator):
  _translator(translator),
  _consumer(consumer) {
}

TranslatingTermConsumer::TranslatingTermConsumer
(auto_ptr<BigTermConsumer> consumer, const TermTranslator& translator):
  _translator(translator),
  _consumer(*consumer) {
  ASSERT(consumer.get() != 0);
  _consumerOwner = std::move(consumer);
}

void TranslatingTermConsumer::beginConsumingList() {
  _consumer.beginConsumingList();
}

void TranslatingTermConsumer::consumeRing(const VarNames& names) {
  _consumer.consumeRing(names);
}

void TranslatingTermConsumer::beginConsuming() {
  _consumer.beginConsuming();
}

void TranslatingTermConsumer::consume(const Term& term) {
  ASSERT(term.getVarCount() == _translator.getVarCount());

  _consumer.consume(term, _translator);
}

void TranslatingTermConsumer::consume(const vector<mpz_class>& term) {
  _consumer.consume(term);
}

void TranslatingTermConsumer::consume
(const Term& term, const TermTranslator& translator) {
  _consumer.consume(term, translator);
}

void TranslatingTermConsumer::doneConsuming() {
  _consumer.doneConsuming();
}

void TranslatingTermConsumer::doneConsumingList() {
  _consumer.doneConsumingList();
}

void TranslatingTermConsumer::consume(const BigIdeal& ideal) {
  _consumer.consume(ideal);
}
