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
#include "PivotEulerAlg.h"

#include "Ideal.h"
#include "RawSquareFreeTerm.h"
#include "RawSquareFreeIdeal.h"
#include "Task.h"
#include "TaskEngine.h"
#include "ElementDeleter.h"
#include "EulerState.h"
#include "PivotStrategy.h"
#include "Arena.h"
#include "LocalArray.h"

#include <sstream>
#include <vector>

namespace Ops = SquareFreeTermOps;

//typedef vector<size_t> DivCounts;
typedef size_t* DivCounts;

namespace {
  bool baseCaseSimple1(mpz_class& accumulator,
    const EulerState& state) {
      const size_t varCount = state.getVarCount();
      const RawSquareFreeIdeal& ideal = state.getIdeal();
      const Word* eliminated = state.getEliminatedVars();
      const size_t genCount = ideal.getGeneratorCount();

      if (!ideal.hasFullSupport(eliminated))
        return true;
      if (genCount > 2)
        return false;

      if (genCount == 0)
        accumulator += state.getSign();
      else if (genCount == 2)
        accumulator += state.getSign();
      else {
        ASSERT(genCount == 1);
        if (!Ops::hasFullSupport(eliminated, varCount))
          accumulator -= state.getSign();
      }
      return true;
  }

  bool optimizeSimpleFromDivCounts(mpz_class& accumulator,
    EulerState& state,
    DivCounts& divCounts,
    Word* termTmp) {
      const size_t varCount = state.getVarCount();
      const size_t genCount = state.getIdeal().getGeneratorCount();
      ASSERT(genCount > 2);

      for (size_t var = 0; var < varCount; ++var) {
        ASSERT(genCount == state.getIdeal().getGeneratorCount());
        ASSERT(genCount > 2);

        if (divCounts[var] < genCount - 2)
          continue;

        if (divCounts[var] == genCount - 1) {
          const Word* nonMultiple =
            state.getIdeal().getGenerator(state.getIdeal().getNonMultiple(var));
          Ops::lcm(termTmp, nonMultiple, state.getEliminatedVars(), varCount);
          Ops::setExponent(termTmp, var, 1);
          if (Ops::hasFullSupport(termTmp, varCount))
            accumulator += state.getSign();

          if (state.toColonSubState(var))
            return true;
          divCounts[var] = 0;
        } else if (divCounts[var] == genCount - 2) {
          state.getIdeal().getLcmOfNonMultiples(termTmp, var);
          Ops::lcmInPlace(termTmp, state.getEliminatedVars(), varCount);
          Ops::setExponent(termTmp, var, 1);
          if (Ops::hasFullSupport(termTmp, varCount))
            accumulator -= state.getSign();

          if (state.toColonSubState(var))
            return true;
          divCounts[var] = 0;
        } else {
          ASSERT(divCounts[var] == genCount);

          state.toColonSubStateNoReminimizeNecessary(var);
          divCounts[var] = 0;
        }
      }
      return false;
  }

  bool baseCaseSimple2(mpz_class& accumulator,
    const EulerState& state,
    const DivCounts& divCounts) {
      const size_t varCount = state.getVarCount();
      const RawSquareFreeIdeal& ideal = state.getIdeal();
      const size_t genCount = state.getIdeal().getGeneratorCount();

      for (size_t var = 0; var < varCount; ++var)
        if (divCounts[var] != 1 && divCounts[var] != genCount)
          return false;

      if ((ideal.getGeneratorCount() & 1) == 0)
        accumulator += state.getSign();
      else
        accumulator -= state.getSign();
      return true;
  }

  bool baseCasePreconditionSimplified(mpz_class& accumulator,
    const EulerState& state) {
      const RawSquareFreeIdeal& ideal = state.getIdeal();

      if (ideal.getGeneratorCount() == 3) {
        accumulator += state.getSign() + state.getSign();
        return true;
      }
      return false;
  }

  bool optimizeOneDivCounts(EulerState& state,
    DivCounts& divCounts,
    Word* tmp) {
      const size_t varCount = state.getVarCount();
      const RawSquareFreeIdeal& ideal = state.getIdeal();

      size_t var = 0;
      for (; var < varCount; ++var) {
        if (divCounts[var] != 1)
          continue;
        size_t index = ideal.getMultiple(var);
        ASSERT(ideal.getGeneratorCount() > index);
        Ops::assign(tmp, ideal.getGenerator(index), varCount);
        state.removeGenerator(index);
        state.flipSign();
        goto searchForMore;
      }
      return false;

searchForMore:
      for (++var; var < varCount; ++var) {
        if (divCounts[var] != 1 || Ops::getExponent(tmp, var) == 1)
          continue;
        size_t index = ideal.getMultiple(var);
        ASSERT(ideal.getGeneratorCount() > index);
        Ops::lcmInPlace(tmp, ideal.getGenerator(index), varCount);

        state.removeGenerator(index);
        state.flipSign();
      }

      if (state.toColonSubState(tmp) || ideal.getGeneratorCount() <= 2)
        return true;

      Ops::toZeroAtSupport(tmp, &(divCounts[0]), varCount);
      return false;
  }

  bool optimizeVarPairs(EulerState& state, Word* tmp, DivCounts& divCounts) {
    const size_t varCount = state.getVarCount();
    const RawSquareFreeIdeal& ideal = state.getIdeal();
    const Word* eliminated = state.getEliminatedVars();

    for (size_t var = 0; var < varCount; ++var) {
      if (Ops::getExponent(eliminated, var) == 1)
        continue;
      ideal.getLcmOfNonMultiples(tmp, var);
      Ops::lcmInPlace(tmp, state.getEliminatedVars(), varCount);
      Ops::setExponent(tmp, var, true);
      if (!Ops::hasFullSupport(tmp, varCount)) {
        if (state.toColonSubState(var))
          return true;
        divCounts[var] = 0;
      }
    }
    return false;
  }
}

EulerState* PivotEulerAlg::processState(EulerState& state) {
  state.compactEliminatedVariablesIfProfitable();

  // ** First optimize state and return false if a base case is detected.
  while (true) {
	ASSERT(state.debugIsValid());

	if (baseCaseSimple1(_euler, state))
	  return 0;

	state.getIdeal().getVarDividesCounts(_divCountsTmp);
	size_t* divCountsTmp = &(_divCountsTmp[0]);

	if (_useUniqueDivSimplify &&
		optimizeOneDivCounts(state, divCountsTmp, _termTmp))
	  continue;
	if (_useManyDivSimplify &&
		optimizeSimpleFromDivCounts(_euler, state, divCountsTmp, _termTmp))
	  continue;
	if (_useAllPairsSimplify) {
	  if (optimizeVarPairs(state, _termTmp, divCountsTmp))
		continue;
	  if (baseCasePreconditionSimplified(_euler, state))
		return 0;
	}
    if (_autoTranspose && autoTranspose(state))
      continue;
	break;
  }

  // ** State is not a base case so perform a split while putting the
  // two sub-states into state and newState.

  size_t* divCountsTmp = &(_divCountsTmp[0]);
  ASSERT(_pivotStrategy.get() != 0);
  EulerState* next = _pivotStrategy->doPivot(state, divCountsTmp);

  return next;
}

void PivotEulerAlg::getPivot(const EulerState& state, Word* pivot) {
  ASSERT(false);
}

PivotEulerAlg::PivotEulerAlg():
  _euler(0),
  _termTmp(0),
  _useUniqueDivSimplify(true),
  _useManyDivSimplify(true),
  _useAllPairsSimplify(false),
  _autoTranspose(true),
  _initialAutoTranspose(true) {
}

const mpz_class& PivotEulerAlg::computeEulerCharacteristic(const Ideal& ideal) {
  if (_pivotStrategy.get() == 0)
	_pivotStrategy = newDefaultPivotStrategy();

  if (ideal.getGeneratorCount() == 0)
	_euler = 0;
  else if (ideal.getVarCount() == 0)
	_euler = -1;
  else {
    const size_t maxDim = std::max(ideal.getVarCount(), ideal.getGeneratorCount());
	LocalArray<Word> termTmp(Ops::getWordCount(maxDim));
	_termTmp = termTmp.begin();
	EulerState* state = EulerState::construct(ideal, &(Arena::getArena()));
	computeEuler(state);
    _termTmp = 0;
  }
  _pivotStrategy->computationCompleted(*this);

  return _euler;
}

const mpz_class& PivotEulerAlg::computeEulerCharacteristic
(const RawSquareFreeIdeal& ideal) {
  if (_pivotStrategy.get() == 0)
	_pivotStrategy = newDefaultPivotStrategy();


  if (ideal.getGeneratorCount() == 0)
	_euler = 0;
  else if (ideal.getVarCount() == 0)
	_euler = -1;
  else {
    const size_t maxDim = std::max(ideal.getVarCount(), ideal.getGeneratorCount());
	LocalArray<Word> termTmp(Ops::getWordCount(maxDim));
	_termTmp = termTmp.begin();
	EulerState* state = EulerState::construct(ideal, &(Arena::getArena()));
	computeEuler(state);
    _termTmp = 0;
  }
  _pivotStrategy->computationCompleted(*this);

  return _euler;
}

void PivotEulerAlg::computeEuler(EulerState* state) {
  EulerState* const root = state;
  try {
	_euler = 0;
	if (_initialAutoTranspose)
	  autoTranspose(*state);
	while (state != 0) {
	  EulerState* nextState = processState(*state);
	  if (nextState == 0) {
		nextState = state->getParent();
		Arena::getArena().freeAndAllAfter(state);
	  }
	  state = nextState;
	}
  } catch (...) {
	// The root state is freed only once the computation is
	// completed, so it and every state after it are still allocated.
	Arena::getArena().freeAndAllAfter(root);
	throw;
  }
}

bool PivotEulerAlg::autoTranspose(EulerState& state) {
  if (!_pivotStrategy->shouldTranspose(state))
    return false;
  state.transpose();
  return true;
}
