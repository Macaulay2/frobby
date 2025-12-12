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
#ifndef CANONICAL_TERM_CONSUMER_GUARD
#define CANONICAL_TERM_CONSUMER_GUARD

#include "TermConsumer.h"
#include "Ideal.h"
#include "ElementDeleter.h"

class Term;
class TermTranslator;

/** Passes consumed items on in a canonical order. This requires
 storing all items before any can be passed on, which can take a lot
 of memory. The ideals are not minimized, so adding non-minimal
 generators can have an effect on the sorted order.
*/
class CanonicalTermConsumer : public TermConsumer {
 public:
  /** The translator, if non-null, is used to identify exponents that
   map to zero, which influences the sorted order.
  */
  CanonicalTermConsumer(unique_ptr<TermConsumer> consumer,
                        size_t varCount,
                        TermTranslator* translator = 0);

  /** Passes on the call immediately. Thus the ordering between when
   this gets called and when consume gets called on the wrapped
   consumer can be switched around if consumeRing is called while
   consuming a list of ideals.

   @todo fix this.
  */
  virtual void consumeRing(const VarNames& names);

  /** This method is not required to be called. If it is called, the
   list of ideals will be sorted and then passed on. If it is not
   called, each ideal will be passed on immediately.
  */
  virtual void beginConsumingList();

  virtual void beginConsuming();
  virtual void consume(const Term& term);
  virtual void doneConsuming();

  virtual void doneConsumingList();

 private:
  void passLastIdeal();
  void canonicalizeIdeal(Ideal& ideal);

  size_t _varCount;
  bool _storingList;
  vector<Ideal*> _ideals;
  ElementDeleter<vector<Ideal*> > _idealsDeleter;
  unique_ptr<TermConsumer> _consumer;
  TermTranslator* _translator;
};

#endif
