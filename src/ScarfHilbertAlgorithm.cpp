/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2010 University of Aarhus
   Contact Bjarke Hammersholt Roune for license information (www.broune.com)

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
#include "ScarfHilbertAlgorithm.h"

#include "CoefTermConsumer.h"
#include "TermTranslator.h"
#include "Deformer.h"
#include "CoefBigTermConsumer.h"
#include "HashPolynomial.h"
#include "UniHashPolynomial.h"
#include "ScarfParams.h"
#include "IdealTree.h"
#include "IdealOrderer.h"

class UndeformConsumer : public CoefTermConsumer {
public:
  UndeformConsumer(Ideal& toDeform,
                   const TermTranslator& translator,
                   CoefBigTermConsumer& consumer,
                   const IdealOrderer& order,
                   bool univar,
                   bool canonical,
                   bool doStrongDeformation):
    _univar(univar),
    _tmp(toDeform.getVarCount()),
    _deformer(toDeform, order, doStrongDeformation),
    _translator(translator),
    _canonical(canonical),
    _consumer(consumer),
    _poly(toDeform.getVarCount()) {
  }

  virtual void consumeRing(const VarNames& names) {
    ASSERT(names == _translator.getNames());
  }

  virtual void beginConsuming() {
  }

  virtual void consume(const mpz_class& coef, const Term& term) {
    ASSERT(term.getVarCount() == _tmp.getVarCount());
    _tmp = term;
    _deformer.undeform(_tmp);

    if (_univar) {
      if (_tmp.getVarCount() == 0)
        _tdeg = 0;
      else
        _tdeg = _translator.getExponent(0, _tmp);
      for (size_t var = 1; var < _tmp.getVarCount(); ++var)
        _tdeg += _translator.getExponent(var, _tmp);
      _uniPoly.add(coef, _tdeg);
    } else
      _poly.add(coef, _tmp);
  }

  virtual void doneConsuming() {
    if (_univar)
      _uniPoly.feedTo(_consumer, _canonical);
    else
      _poly.feedTo(_translator, _consumer, _canonical);
  }

private:
  bool _univar;
  Term _tmp;
  Deformer _deformer;
  const TermTranslator& _translator;
  bool _canonical;
  CoefBigTermConsumer& _consumer;
  HashPolynomial _poly;
  UniHashPolynomial _uniPoly;
  mpz_class _tdeg;
};

ScarfHilbertAlgorithm::ScarfHilbertAlgorithm
(const TermTranslator& translator,
 const ScarfParams& params,
 auto_ptr<IdealOrderer> enumerationOrder,
 auto_ptr<IdealOrderer> deformationOrder):
  _translator(translator),
  _params(params),
  _enumerationOrder(std::move(enumerationOrder)),
  _deformationOrder(std::move(deformationOrder)),
  _totalStates(0),
  _totalFaces(0) {
  ASSERT(_enumerationOrder.get() != 0);
  ASSERT(_deformationOrder.get() != 0);
}

ScarfHilbertAlgorithm::~ScarfHilbertAlgorithm() {
  // Destructor defined so auto_ptr<T> in the header does not need
  // definition of T.
}

void ScarfHilbertAlgorithm::runGeneric(const Ideal& ideal,
                                       CoefBigTermConsumer& consumer,
                                       bool univariate,
                                       bool canonical) {
  Ideal deformed(ideal);
  UndeformConsumer undeformer(deformed,
                              _translator,
                              consumer,
                              *_deformationOrder,
                              univariate,
                              canonical,
                              _params.getDeformToStronglyGeneric());

  undeformer.consumeRing(_translator.getNames());
  undeformer.beginConsuming();
  ASSERT(_enumerationOrder.get() != 0);
  _enumerationOrder->order(deformed);
  enumerateScarfComplex(deformed, undeformer);
  undeformer.doneConsuming();

  if (_params.getPrintStatistics()) {
    fputs("*** Statistics ***\n", stderr);
    fprintf(stderr, "Total states considered: %u\n",
            static_cast<unsigned int>(_totalStates));
    fprintf(stderr, "Total faces accepted: %u\n",
            static_cast<unsigned int>(_totalFaces));
  }
}

void ScarfHilbertAlgorithm::initializeEnumeration(const Ideal& ideal,
                                                  size_t& activeStateCount) {
  ASSERT(ideal.getVarCount() == _translator.getVarCount());

  if (_params.getPrintDebug()) {
    fputs("Enumerating faces of Scarf complex of:\n", stderr);
    ideal.print(stderr);
  }

  // Set up _states with enough entries. The maximal number of active
  // entries at any time is one for each generator plus one for the
  // empty face. We need one more than this because we take a
  // reference to the next state even when there is no next state.
  size_t statesNeeded = ideal.getGeneratorCount() + 2;
  if (_states.size() < statesNeeded) {
    _states.resize(statesNeeded);
    for (size_t i = 0; i < _states.size(); ++i) {
      _states[i].term.reset(ideal.getVarCount());
      _states[i].face.reserve(ideal.getVarCount());
    }
  }

  // Set up the initial state
  activeStateCount = 0;
  if (ideal.containsIdentity())
    return;

  ++activeStateCount;
  _states[0].plus = true;
  _states[0].pos = ideal.begin();
  ASSERT(_states[0].term.isIdentity());
}

bool ScarfHilbertAlgorithm::doEnumerationStep(const Ideal& ideal,
                                              const IdealTree& tree,
                                              State& state,
                                              State& nextState) {
  if (_params.getPrintDebug()) {
    fputs("DEBUG:*Looking at element ", stderr);
    if (state.pos == ideal.end())
      fputs("end", stderr);
    else
      Term::print(stderr, *state.pos, ideal.getVarCount());
    fputs(" with lcm(face)=", stderr);
    state.term.print(stderr);
    fputs(" and face=", stderr);
    if (state.face.empty())
      fputs("empty", stderr);
    for (size_t i = 0; i < state.face.size(); ++i) {
      fputs("\nDEBUG:   ", stderr);
      Term::print(stderr, state.face[i], ideal.getVarCount());
    }
    fputc('\n', stderr);
    fflush(stderr);
  }

  Exponent* termToAdd;
  while (true) {
    ++_totalStates;
    if (state.face.size() == ideal.getVarCount() || state.pos == ideal.end())
      return false; // A base case

    termToAdd = *state.pos;

    // This accounts for the possibility of not adding termToAdd to the
    // face. We do that in-place on state.
    ++state.pos;

    // The other possibility is to add termToAdd to the face. We record
    // this only if that is still a face of the complex, i.e. if no
    // generator strictly divides the lcm of the set.
    nextState.term.lcm(state.term, termToAdd);
    // The elements of face are more likely to become strict divisors
    // than a random generator, so check those first.
    for (size_t i = 0; i < state.face.size(); ++i) {
      if (Term::strictlyDivides(state.face[i],
                                nextState.term,
                                ideal.getVarCount())) {
        goto doNext;
      }
    }
    if (tree.strictlyContains(nextState.term))
      goto doNext;
    ASSERT(!ideal.strictlyContains(nextState.term));
    break;
  doNext:;
    ASSERT(ideal.strictlyContains(nextState.term));
  }

  nextState.plus = !state.plus;
  nextState.pos = state.pos;
  nextState.face = state.face;
  nextState.face.push_back(termToAdd);

  return true;
}

void ScarfHilbertAlgorithm::doEnumerationBaseCase(const State& state,
                                                  CoefTermConsumer& consumer) {
  if (_params.getPrintDebug()) {
    fputs("DEBUG: Found base case with lcm(face)=", stderr);
    state.term.print(stderr);
    fputc('\n', stderr);
    fflush(stderr);
  }

  consumer.consume(state.plus ? 1 : -1, state.term);

  // Every face ends up as a base case exactly once, so this is a
  // convenient place to count them.
  ++_totalFaces;
}

void ScarfHilbertAlgorithm::enumerateScarfComplex(const Ideal& ideal,
                                                  CoefTermConsumer& consumer) {
  ASSERT(Ideal(ideal).isWeaklyGeneric());

  IdealTree tree(ideal);

  size_t activeStateCount = 0;
  initializeEnumeration(ideal, activeStateCount);
  while (activeStateCount > 0) {
    ASSERT(activeStateCount < _states.size());
    State& currentState = _states[activeStateCount - 1];
    State& nextState = _states[activeStateCount];
    if (doEnumerationStep(ideal, tree, currentState, nextState))
      ++activeStateCount;
    else {
      doEnumerationBaseCase(currentState, consumer);
      --activeStateCount;
    }
  }
}
