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
#include "BigTermConsumer.h"

#include "Term.h"
#include "TermTranslator.h"
#include "BigIdeal.h"

BigTermConsumer::~BigTermConsumer() {
}

// Do not use this implementation for anything time-critical, since
// the allocation of a new vector and mpz_class'es each time really is
// not efficient.
void BigTermConsumer::consume
(const Term& term, const TermTranslator& translator) {
  ASSERT(term.getVarCount() == translator.getVarCount());

  vector<mpz_class> bigTerm(term.getVarCount());
  for (size_t var = 0; var < term.getVarCount(); ++var)
    bigTerm[var] = translator.getExponent(var, term);

  consume(bigTerm);
}

// Do not use this implementation for anything time-critical.
void BigTermConsumer::consume(const Term& term) {
  vector<mpz_class> bigTerm(term.getVarCount());
  for (size_t var = 0; var < term.getVarCount(); ++var)
    bigTerm[var] = term[var];

  consume(bigTerm);
}

void BigTermConsumer::beginConsuming(const VarNames& names) {
  consumeRing(names);
  beginConsuming();
}

void BigTermConsumer::consume(const BigIdeal& ideal) {
  consumeRing(ideal.getNames());
  beginConsuming(ideal.getNames());
  for (size_t term = 0; term < ideal.getGeneratorCount(); ++term)
    consume(ideal.getTerm(term));
  doneConsuming();
}

void BigTermConsumer::consume(unique_ptr<BigIdeal> ideal) {
  consume(*ideal);
}
