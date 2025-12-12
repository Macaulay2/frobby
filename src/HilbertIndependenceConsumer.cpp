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
#include "HilbertIndependenceConsumer.h"

#include "IndependenceSplitter.h"
#include "HilbertStrategy.h"

HilbertIndependenceConsumer::
HilbertIndependenceConsumer(HilbertStrategy* strategy):
  _rightConsumer(this),
  _strategy(strategy) {
  ASSERT(strategy != 0);
  clear();
}

void HilbertIndependenceConsumer::reset(CoefTermConsumer* parent,
                                        IndependenceSplitter& splitter,
                                        size_t varCount) {
  ASSERT(parent != 0);

  _tmpTerm.reset(varCount);
  _parent = parent;

  splitter.getBigProjection(_leftProjection);
  splitter.getRestProjection(_rightProjection);

  _rightTerms.clearAndSetVarCount(_rightProjection.getRangeVarCount());
}

void HilbertIndependenceConsumer::clear() {
  _parent = 0;
  _rightTerms.clear();
  _rightCoefs.clear();
}

void HilbertIndependenceConsumer::dispose() {
  ASSERT(_strategy != 0);
  _strategy->freeConsumer(unique_ptr<HilbertIndependenceConsumer>(this));
}

void HilbertIndependenceConsumer::run(TaskEngine&) {
  dispose();
}

CoefTermConsumer* HilbertIndependenceConsumer::getLeftConsumer() {
  ASSERT(_parent != 0);
  return this;
}

void HilbertIndependenceConsumer::consumeRing(const VarNames& names) {
}

void HilbertIndependenceConsumer::beginConsuming() {
}

void HilbertIndependenceConsumer::doneConsuming() {
}

void HilbertIndependenceConsumer::consume(const mpz_class& coef,
                                          const Term& term) {
  ASSERT(_parent != 0);
  ASSERT(coef != 0);
  consumeLeft(coef, term);
}

CoefTermConsumer* HilbertIndependenceConsumer::getRightConsumer() {
  ASSERT(_parent != 0);
  return &_rightConsumer;
}

const Projection& HilbertIndependenceConsumer::getLeftProjection() const {
  ASSERT(_parent != 0);
  return _leftProjection;
}

const Projection& HilbertIndependenceConsumer::getRightProjection() const {
  ASSERT(_parent != 0);
  return _rightProjection;
}

HilbertIndependenceConsumer::RightConsumer::
RightConsumer(HilbertIndependenceConsumer* parent):
  _parent(parent) {
}

void HilbertIndependenceConsumer::RightConsumer::consumeRing
(const VarNames& names) {
}

void HilbertIndependenceConsumer::RightConsumer::beginConsuming() {
}

void HilbertIndependenceConsumer::RightConsumer::
consume(const mpz_class& coef, const Term& term) {
  _parent->consumeRight(coef, term);
}

void HilbertIndependenceConsumer::RightConsumer::doneConsuming() {
}

void HilbertIndependenceConsumer::consumeLeft(const mpz_class& leftCoef,
                                              const Term& leftTerm) {
  ASSERT(_tmpTerm.getVarCount() ==
         _leftProjection.getRangeVarCount() +
         _rightProjection.getRangeVarCount());

  _leftProjection.inverseProject(_tmpTerm, leftTerm);

  size_t rightSize = _rightTerms.getGeneratorCount();
  ASSERT(rightSize == _rightCoefs.size());
  for (size_t right = 0; right < rightSize; ++right) {
    _rightProjection.inverseProject(_tmpTerm,
                                    *(_rightTerms.begin() + right));
    ASSERT(leftCoef != 0);
    ASSERT(_rightCoefs[right] != 0);
    _tmpCoef = leftCoef * _rightCoefs[right];

    ASSERT(_tmpCoef != 0);
    _parent->consume(_tmpCoef, _tmpTerm);
  }
}

void HilbertIndependenceConsumer::consumeRight(const mpz_class& coef,
                                               const Term& term) {
  ASSERT(term.getVarCount() == _rightProjection.getRangeVarCount());
  ASSERT(_rightTerms.getVarCount() == term.getVarCount());
  ASSERT(coef != 0);

  _rightTerms.insert(term);
  _rightCoefs.push_back(coef);
}
