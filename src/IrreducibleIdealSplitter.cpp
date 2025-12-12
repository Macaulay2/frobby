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
#include "IrreducibleIdealSplitter.h"

#include "VarNames.h"
#include "TermTranslator.h"

IrreducibleIdealSplitter::IrreducibleIdealSplitter
(BigTermConsumer& consumer):
  _consumer(consumer),
  _inList(false) {
}

IrreducibleIdealSplitter::IrreducibleIdealSplitter
(unique_ptr<BigTermConsumer> consumer):
  _consumer(*consumer),
  _consumerDeleter(std::move(consumer)),
  _inList(false) {
}

void IrreducibleIdealSplitter::consumeRing(const VarNames& names) {
  _tmp.reset(names.getVarCount());
  _bigTmp.resize(names.getVarCount());
  _consumer.consumeRing(names);

  ASSERT(_tmp.isIdentity());
  ASSERT(_bigTmp == vector<mpz_class>(_bigTmp.size()));
}

void IrreducibleIdealSplitter::beginConsumingList() {
  ASSERT(!_inList);
  _inList = true;
  _consumer.beginConsumingList();
}

void IrreducibleIdealSplitter::beginConsuming() {
  if (!_inList)
    _consumer.beginConsumingList();
}

void IrreducibleIdealSplitter::consume(const Term& term) {
  ASSERT(term.getVarCount() == _tmp.getVarCount());
  ASSERT(_tmp.isIdentity());

  _consumer.beginConsuming();
  for (size_t var = 0; var < term.getVarCount(); ++var) {
    if (term[var] != 0) {
      _tmp[var] = term[var];
      _consumer.consume(_tmp);
      _tmp[var] = 0;
    }
  }
  _consumer.doneConsuming();

  ASSERT(_tmp.isIdentity());
}

void IrreducibleIdealSplitter::consume
(const Term& term, const TermTranslator& translator) {
  ASSERT(term.getVarCount() == _tmp.getVarCount());
  ASSERT(_tmp.isIdentity());

  _consumer.beginConsuming();
  for (size_t var = 0; var < term.getVarCount(); ++var) {
    if (translator.getExponent(var, term) != 0) {
      _tmp[var] = term[var];
      _consumer.consume(_tmp, translator);
      _tmp[var] = 0;
    }
  }
  _consumer.doneConsuming();

  ASSERT(_tmp.isIdentity());
}

void IrreducibleIdealSplitter::consume(const vector<mpz_class>& term) {
  ASSERT(term.size() == _bigTmp.size());
  ASSERT(_bigTmp == vector<mpz_class>(_bigTmp.size()));

  _consumer.beginConsuming();
  for (size_t var = 0; var < term.size(); ++var) {
    if (term[var] != 0) {
      _bigTmp[var] = term[var];
      _consumer.consume(_bigTmp);
      _bigTmp[var] = 0;
    }
  }
  _consumer.doneConsuming();

  ASSERT(_bigTmp == vector<mpz_class>(_bigTmp.size()));
}

void IrreducibleIdealSplitter::doneConsuming() {
  if (!_inList)
    _consumer.doneConsumingList();
}

void IrreducibleIdealSplitter::doneConsumingList() {
  ASSERT(_inList);
  _consumer.doneConsumingList();
}
