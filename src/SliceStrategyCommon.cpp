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
#include "SliceStrategyCommon.h"
#include "ElementDeleter.h"
#include "TaskEngine.h"

#include "Slice.h"

SliceStrategyCommon::SliceStrategyCommon(const SplitStrategy* splitStrategy):
  _split(splitStrategy),
  _useIndependence(true),
  _useSimplification(true) {
  ASSERT(splitStrategy != 0);
}

SliceStrategyCommon::~SliceStrategyCommon() {
  // TODO: use ElementDeleter instead
  while (!_sliceCache.empty()) {
    delete _sliceCache.back();
    _sliceCache.pop_back();
  }
}

void SliceStrategyCommon::freeSlice(auto_ptr<Slice> slice) {
  ASSERT(slice.get() != 0);
  ASSERT(debugIsValidSlice(slice.get()));

  slice->clearIdealAndSubtract(); // To preserve memory.
  noThrowPushBack(_sliceCache, std::move(slice));
}

void SliceStrategyCommon::setUseIndependence(bool use) {
  _useIndependence = use;
}

void SliceStrategyCommon::setUseSimplification(bool use) {
  _useSimplification = use;
}

bool SliceStrategyCommon::simplify(Slice& slice) {
  if (getUseSimplification())
    return slice.simplify();
  else if (_split->isLabelSplit()) {
    // The label split code requires at least this simplification.
    return slice.adjustMultiply();
  }
  return false;
}

auto_ptr<Slice> SliceStrategyCommon::newSlice() {
  auto_ptr<Slice> slice;
  if (!_sliceCache.empty()) {
    slice.reset(_sliceCache.back());
    _sliceCache.pop_back();
  } else
    slice = allocateSlice();

  ASSERT(debugIsValidSlice(slice.get()));
  return slice;
}

void SliceStrategyCommon::pivotSplit(auto_ptr<Slice> slice) {
  ASSERT(slice.get() != 0);

  _pivotTmp.reset(slice->getVarCount());
  getPivot(_pivotTmp, *slice);

  // Assert valid pivot.
  ASSERT(_pivotTmp.getVarCount() == slice->getVarCount());
  ASSERT(!_pivotTmp.isIdentity());
  ASSERT(!slice->getIdeal().contains(_pivotTmp));
  ASSERT(!slice->getSubtract().contains(_pivotTmp));

  // Set slice2 to the inner slice.
  auto_ptr<Slice> slice2 = newSlice();
  *slice2 = *slice;
  slice2->innerSlice(_pivotTmp);
  simplify(*slice2);

  // Set slice to the outer slice.
  slice->outerSlice(_pivotTmp);
  simplify(*slice);

  // Process the smaller slice first to preserve memory.
  if (slice2->getIdeal().getGeneratorCount() <
      slice->getIdeal().getGeneratorCount()) {
    // std::swap() may not work correctly on auto_ptr, so we have to
    // do the swap by hand.
    auto_ptr<Slice> tmp = std::move(slice2);
    slice2 = std::move(slice);
    slice = std::move(tmp);
  }

  _tasks.addTask(slice2.release());
  _tasks.addTask(slice.release());
}

bool SliceStrategyCommon::getUseIndependence() const {
  return _useIndependence;
}

bool SliceStrategyCommon::getUseSimplification() const {
  return _useSimplification;
}
