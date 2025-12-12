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
#ifndef IRREDUCIBLE_IDEAL_SPLITTER_GUARD
#define IRREDUCIBLE_IDEAL_SPLITTER_GUARD

#include "BigTermConsumer.h"
#include "Term.h"

#include <vector>

class IrreducibleIdealSplitter : public BigTermConsumer {
public:
  IrreducibleIdealSplitter(BigTermConsumer& consumer);
  IrreducibleIdealSplitter(unique_ptr<BigTermConsumer> consumer);

  virtual void consumeRing(const VarNames& names);
  virtual void beginConsumingList();
  virtual void beginConsuming();
  virtual void consume(const Term& term);
  virtual void consume(const Term& term, const TermTranslator& translator);
  virtual void consume(const vector<mpz_class>& term);
  virtual void doneConsuming();
  virtual void doneConsumingList();

private:
  BigTermConsumer& _consumer;
  unique_ptr<BigTermConsumer> _consumerDeleter;
  Term _tmp;
  vector<mpz_class> _bigTmp;
  bool _inList;
};

#endif
