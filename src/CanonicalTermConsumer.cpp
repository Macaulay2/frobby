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
#include "CanonicalTermConsumer.h"
#include "TermTranslator.h"
#include "IdealComparator.h"

#include "Term.h"

CanonicalTermConsumer::CanonicalTermConsumer(unique_ptr<TermConsumer> consumer,
                                             size_t varCount,
                                             TermTranslator* translator):
  _varCount(varCount),
  _storingList(false),
  _ideals(),
  _idealsDeleter(_ideals),
  _consumer(std::move(consumer)),
  _translator(translator) {
  ASSERT(_consumer.get() != 0);
}

void CanonicalTermConsumer::consumeRing(const VarNames& names) {
  _consumer->consumeRing(names);
}

void CanonicalTermConsumer::beginConsumingList() {
  ASSERT(!_storingList);
  ASSERT(_ideals.empty());

  _storingList = true;
}

void CanonicalTermConsumer::beginConsuming() {
  ASSERT(_storingList || _ideals.empty());

  unique_ptr<Ideal> ideal(new Ideal(_varCount));
  exceptionSafePushBack(_ideals, std::move(ideal));
}

void CanonicalTermConsumer::consume(const Term& term) {
  ASSERT(term.getVarCount() == _varCount);
  ASSERT(!_ideals.empty());

  _ideals.back()->insert(term);
}

void CanonicalTermConsumer::doneConsuming() {
  if (!_storingList) {
    ASSERT(_ideals.size() == 1);
    passLastIdeal();
    ASSERT(_ideals.empty());
  }
}

void CanonicalTermConsumer::doneConsumingList() {
  ASSERT(_storingList);

  vector<Ideal*>::iterator end = _ideals.end();
  for (vector<Ideal*>::iterator it = _ideals.begin(); it != end; ++it)
    canonicalizeIdeal(**it);

  // We are sorting in reverse because we are processing the ideals from
  // the back, so they get passed on in the correct order.
  if (_translator == 0) {
    IdealComparator comparator;
    sort(_ideals.rbegin(), _ideals.rend(), comparator);
  } else {
    TranslatedIdealComparator comparator(*_translator);
    sort(_ideals.rbegin(), _ideals.rend(), comparator);
  }

  _consumer->beginConsumingList();
  while (!_ideals.empty())
    passLastIdeal();
  _consumer->doneConsumingList();
}

void CanonicalTermConsumer::passLastIdeal() {
  ASSERT(!_ideals.empty());
  ASSERT(_ideals.back() != 0);

  unique_ptr<Ideal> ideal(_ideals.back());
  _ideals.pop_back();

  canonicalizeIdeal(*ideal);

  _consumer->beginConsuming();
  Term tmp(_varCount);
  Ideal::const_iterator end = ideal->end();
  for (Ideal::const_iterator it = ideal->begin(); it != end; ++it) {
    tmp = *it;
    _consumer->consume(tmp);
  }
  ideal.reset(0);

  _consumer->doneConsuming();
}

void CanonicalTermConsumer::canonicalizeIdeal(Ideal& ideal) {
  if (_translator == 0)
    ideal.sortReverseLex();
  else {
    TranslatedReverseLexComparator comparator(*_translator);
    sort(ideal.begin(), ideal.end(), comparator);
  }
}
