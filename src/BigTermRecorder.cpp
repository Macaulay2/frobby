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
#include "BigTermRecorder.h"

#include "BigIdeal.h"
#include "Term.h"
#include "TermTranslator.h"

BigTermRecorder::BigTermRecorder():
  _ideals(),
  _idealsDeleter(_ideals) {
}

void BigTermRecorder::consumeRing(const VarNames& names) {
  _names = names;
}

void BigTermRecorder::consume(unique_ptr<BigIdeal> ideal) {
  consumeRing(ideal->getNames());
  exceptionSafePushBack(_ideals, std::move(ideal));
}

void BigTermRecorder::beginConsuming() {
  unique_ptr<BigIdeal> ideal(new BigIdeal(_names));
  exceptionSafePushBack(_ideals, std::move(ideal));
}

void BigTermRecorder::consume
(const Term& term, const TermTranslator& translator) {
  ASSERT(!_ideals.empty());
  BigIdeal& ideal = *(_ideals.back());

  ASSERT(term.getVarCount() == ideal.getVarCount());
  ASSERT(translator.getVarCount() == ideal.getVarCount());

  ideal.newLastTerm();
  size_t varCount = ideal.getVarCount();
  for (size_t var = 0; var < varCount; ++var)
    ideal.getLastTermExponentRef(var) = translator.getExponent(var, term);
}

void BigTermRecorder::consume(const vector<mpz_class>& term) {
  ASSERT(!_ideals.empty());
  BigIdeal& ideal = *(_ideals.back());

  ideal.newLastTerm();
  size_t varCount = ideal.getVarCount();
  for (size_t var = 0; var < varCount; ++var)
    ideal.getLastTermExponentRef(var) = term[var];
}

void BigTermRecorder::doneConsuming() {
}

bool BigTermRecorder::empty() const {
  return _ideals.empty();
}

unique_ptr<BigIdeal> BigTermRecorder::releaseIdeal() {
  ASSERT(!empty());
  unique_ptr<BigIdeal> ideal(_ideals.front());
  _ideals.pop_front();
  return ideal;
}

const VarNames& BigTermRecorder::getRing() {
  return _names;
}
