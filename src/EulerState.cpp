/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2011 Bjarke Hammersholt Roune (www.broune.com)

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
#include "EulerState.h"

#include "RawSquareFreeTerm.h"
#include "Ideal.h"
#include "Arena.h"

#include <limits>

namespace Ops = SquareFreeTermOps;

EulerState* EulerState::construct(const Ideal& idealParam, Arena* arena) {
  ASSERT(arena != 0);

  const size_t varCount = idealParam.getVarCount();
  const size_t capacity = idealParam.getGeneratorCount();
  EulerState* state = rawConstruct(varCount, capacity, arena);

  state->ideal->insert(idealParam);
  Ops::setToIdentity(state->eliminated, varCount);
  ASSERT(state->debugIsValid());

  return state;
}

EulerState* EulerState::construct
(const RawSquareFreeIdeal& idealParam, Arena* arena) {
  ASSERT(arena != 0);

  const size_t varCount = idealParam.getVarCount();
  const size_t capacity = idealParam.getGeneratorCount();
  EulerState* state = rawConstruct(varCount, capacity, arena);

  state->ideal->insert(idealParam);
  Ops::setToIdentity(state->eliminated, varCount);
  ASSERT(state->debugIsValid());

  return state;
}

EulerState* EulerState::rawConstruct(size_t varCount, size_t capacity,
									 Arena* arena) {
  ASSERT(arena != 0);
  // Do both ways around to support transpose.
  size_t bytesIdeal = std::max(
	RawSquareFreeIdeal::getBytesOfMemoryFor(varCount, capacity),
    RawSquareFreeIdeal::getBytesOfMemoryFor(capacity, varCount));
  size_t wordsElim = std::max(
    Ops::getWordCount(varCount), Ops::getWordCount(capacity));
  if (bytesIdeal == 0 || wordsElim == 0)
	throw bad_alloc();

  EulerState* state =
	static_cast<EulerState*>(arena->alloc(sizeof(EulerState)));
  try {
	state->_alloc = arena;
	state->ideal =
	  RawSquareFreeIdeal::construct(arena->alloc(bytesIdeal), varCount);
	state->eliminated = arena->allocArrayNoCon<Word>(wordsElim).first;
  } catch (...) {
	arena->freeAndAllAfter(state);
	throw;
  }
  state->sign = 1;
  state->_parent = 0;

  return state;
}

EulerState* EulerState::inPlaceStdSplit(size_t pivotVar) {
  ASSERT(pivotVar < getVarCount());
  EulerState* subState = makeSumSubState(pivotVar);
  toColonSubState(pivotVar);
  return subState;
}

EulerState* EulerState::inPlaceStdSplit(Word* pivot) {
  ASSERT(pivot != 0);
  EulerState* subState = makeSumSubState(pivot);
  toColonSubState(pivot);
  return subState;
}

EulerState* EulerState::inPlaceGenSplit(size_t pivotIndex) {
  ASSERT(pivotIndex < getIdeal().getGeneratorCount());

  const size_t varCount = ideal->getVarCount();
  const size_t capacity = ideal->getGeneratorCount();
  EulerState* subState = rawConstruct(varCount, capacity, _alloc);
  subState->_parent = this;

  *subState->ideal = *ideal;

  Ops::assign(subState->eliminated, eliminated, varCount);
  subState->sign = sign;

  const Word* pivot = getIdeal().getGenerator(pivotIndex);
  subState->removeGenerator(pivotIndex);
  subState->toColonSubState(pivot);
  subState->flipSign();
  removeGenerator(pivotIndex);

  ASSERT(subState->debugIsValid());
  return subState;
}

bool EulerState::toColonSubState(const Word* pivot) {
  ASSERT(pivot != 0);
  ASSERT(Ops::isRelativelyPrime(pivot, getEliminatedVars(), getVarCount()));

  const size_t genCountBefore = getIdeal().getGeneratorCount();
  ideal->colonReminimize(pivot);
  Ops::lcmInPlace(eliminated, pivot, ideal->getVarCount());
  ASSERT(debugIsValid());
  return genCountBefore != getIdeal().getGeneratorCount();
}

bool EulerState::toColonSubState(size_t pivotVar) {
  ASSERT(pivotVar < getVarCount());
  ASSERT(Ops::getExponent(getEliminatedVars(), pivotVar) == 0);

  const size_t genCountBefore = getIdeal().getGeneratorCount();
  ideal->colonReminimize(pivotVar);
  Ops::setExponent(eliminated, pivotVar, true);
  ASSERT(debugIsValid());
  return genCountBefore != getIdeal().getGeneratorCount();
}

void EulerState::toColonSubStateNoReminimizeNecessary(size_t pivotVar) {
  ASSERT(pivotVar < getVarCount());
  ASSERT(Ops::getExponent(getEliminatedVars(), pivotVar) == 0);

  ideal->colon(pivotVar);
  Ops::setExponent(eliminated, pivotVar, true);
  ASSERT(debugIsValid());
}

void EulerState::toColonSubStateNoReminimizeNecessary(Word* pivot) {
  ASSERT(pivot != 0);
  ASSERT(Ops::isRelativelyPrime(getEliminatedVars(), pivot, getVarCount()));

  ideal->colon(pivot);
  Ops::lcmInPlace(eliminated, pivot, getVarCount());
  ASSERT(debugIsValid());
}

EulerState* EulerState::makeSumSubState(size_t pivotVar) {
  const size_t varCount = ideal->getVarCount();
  const size_t capacity = ideal->getGeneratorCount();
  EulerState* subState = rawConstruct(varCount, capacity, _alloc);
  subState->_parent = this;

  subState->ideal->insertNonMultiples(pivotVar, *ideal);
  Ops::assign(subState->eliminated, eliminated, varCount);
  Ops::setExponent(subState->eliminated, pivotVar, 1);
  subState->sign = sign;
  subState->flipSign();

  ASSERT(subState->debugIsValid());
  return subState;
}

EulerState* EulerState::makeSumSubState(Word* pivot) {
  const size_t varCount = ideal->getVarCount();
  const size_t capacity = ideal->getGeneratorCount() + 1;
  EulerState* subState = rawConstruct(varCount, capacity, _alloc);
  subState->_parent = this;

  subState->ideal->insertNonMultiples(pivot, *ideal);
  ASSERT(subState->ideal->getGeneratorCount() < ideal->getGeneratorCount());
  subState->ideal->insert(pivot);
  Ops::assign(subState->eliminated, eliminated, varCount);
  subState->sign = sign;

  ASSERT(subState->debugIsValid());
  return subState;
}

void EulerState::transpose() {
  ideal->transpose(eliminated);
  ideal->minimize();
  Ops::setToIdentity(eliminated, ideal->getVarCount());
}

void EulerState::compactEliminatedVariablesIfProfitable() {
  const size_t varCount = getVarCount();
  const size_t varsLeft = getNonEliminatedVarCount();
  if (Ops::getWordCount(varCount) > Ops::getWordCount(varsLeft)) {
	ideal->compact(eliminated);
	Ops::setToIdentity(eliminated, ideal->getVarCount());
	ASSERT(debugIsValid());
  }
}

void EulerState::print(FILE* out) {
  fputs("** an Euler characteristic algorithm state:\n", out);
  fprintf(out, "State sign: %s\n", sign == 1 ? "+1" : "-1");
  fputs("Eliminated: ", out);
  Ops::print(out, eliminated, ideal->getVarCount());
  fputc('\n', out);
  ideal->print(out);
}

#ifdef DEBUG
bool EulerState::debugIsValid() const {
  if (ideal == 0 || eliminated == 0 || _alloc == 0)
	return false;
  if (sign != 1 && sign != -1)
	return false;
  if (!Ops::isValid(eliminated, ideal->getVarCount()))
	return false;
  if (!ideal->isMinimallyGenerated())
	return false;
  if (ideal->getVarCount() != 0 &&
	  ideal->getNotRelativelyPrime(eliminated) !=
	  ideal->getGeneratorCount())
	return false;
  return true;
}
#endif

void EulerState::toZero() {
  ideal = 0;
  eliminated = 0;
  sign = 1;
  _parent = 0;
}

size_t EulerState::getNonEliminatedVarCount() const {
  const size_t eliminatedVarCount =
	Ops::getSizeOfSupport(getEliminatedVars(), getVarCount());

  ASSERT(getVarCount() >= eliminatedVarCount);
  return getVarCount() - eliminatedVarCount;
}
