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
#include "HilbertBasecase.h"

#include "Ideal.h"
#include "Term.h"
#include "error.h"

HilbertBasecase::HilbertBasecase():
  _idealCacheDeleter(_idealCache),
  _stepsPerformed(0) {
}

HilbertBasecase::~HilbertBasecase() {
  ASSERT(_todo.empty());
}

// Performs one or more steps of computation. Steps are performed on
// entry until entry becomes a base case, or until it needs to be
// split into two.
//
// If entry becomes a base case, then false is returned, entry can be
// discarded and newEntry is unchanged.
//
// If entry needs to be split into two, then true is returned, entry
// becomes one of the subcomputations, and newEntry becomes the other.
bool HilbertBasecase::stepComputation(Entry& entry, Entry& newEntry) {
  ++_stepsPerformed;

  size_t varCount = entry.ideal->getVarCount();

  // This loops keeps iterating as long as entry is not a base case
  // and there is some computation that can be done on entry that does
  // not require splitting it into two.
  while (true) {
    // Here _term is used to contain support counts to choose best pivot and
    // to detect base case.

    // We start off checking different ways that entry can be a base
    // case.
    entry.ideal->getSupportCounts(_term);
    if (_term.getSizeOfSupport() + entry.extraSupport != varCount)
      return false;

    if (_term.isSquareFree()) {
      if ((entry.ideal->getGeneratorCount() % 2) == 1)
        entry.negate = !entry.negate;
      if (entry.negate)
        --_sum;
      else
        ++_sum;
      return false;
    }

    if (entry.ideal->getGeneratorCount() == 2) {
      if (entry.negate)
        --_sum;
      else
        ++_sum;
      return false;
    }

    // This is a simplification step, and if we can perform it, we
    // just start over with the new entry we get that way. This is
    // necessary because the base case checks below assume that this
    // simplification has been performed.
    size_t ridden = eliminate1Counts(*entry.ideal, _term, entry.negate);
    if (ridden != 0) {
      entry.extraSupport += ridden;
      continue;
    }

    if (entry.ideal->getGeneratorCount() == 3) {
      if (entry.negate)
        _sum -= 2;
      else
        _sum += 2;
      return false;
    }

    if (entry.ideal->getGeneratorCount() == 4 &&
        _term[_term.getFirstMaxExponent()] == 2 &&
        _term.getSizeOfSupport() == 4) {
      if (entry.negate)
        ++_sum;
      else
        --_sum;
      return false;
    }

    // At this point entry is not a base case, and it cannot be
    // simplified, so we have to split it into two.

    size_t bestPivotVar = _term.getFirstMaxExponent();

    // Handle outer slice.
    unique_ptr<Ideal> outer = getNewIdeal();
    outer->clearAndSetVarCount(varCount);
    outer->insertNonMultiples(bestPivotVar, 1, *entry.ideal);

    // outer is subtracted instead of added due to having added the
    // pivot to the ideal.
    newEntry.negate = !entry.negate;
    newEntry.extraSupport = entry.extraSupport + 1;
    newEntry.ideal = outer.get();

    // Handle inner slice in-place on entry.
    entry.ideal->colonReminimize(bestPivotVar, 1);
    ++entry.extraSupport;

    outer.release();
    return true;
  }
}

void HilbertBasecase::computeCoefficient(Ideal& originalIdeal) {
  ASSERT(_todo.empty());

  try { // Here to clear _todo in case of an exception
    // _sum is updated as a side-effect of calling stepComputation.
    _sum = 0;

    // _term is reused for several different purposes in order to avoid
    // having to allocate and deallocate the underlying data structure.
    _term.reset(originalIdeal.getVarCount());

    // entry is the Entry currently being processed. Additional entries
    // are added to _todo, though this only happens if there are two,
    // since otherwise entry can just be updated to the next value
    // directly and so we avoid the overhead of using _todo when we can.
    Entry entry;
    entry.negate = false;
    entry.extraSupport = 0;
    entry.ideal = &originalIdeal;

    // This should normally point to entry.ideal, but since we do not
    // have ownership of originalIdeal, it starts out pointing nowhere.
    unique_ptr<Ideal> entryIdealDeleter;

    while (true) {
      // Do an inner loop since there is no reason to add entry to _todo
      // and then immediately take it off again.
      Entry newEntry;
      while (stepComputation(entry, newEntry)) {
        unique_ptr<Ideal> newEntryIdealDeleter(newEntry.ideal);
        _todo.push_back(newEntry);
        newEntryIdealDeleter.release();
      }

      if (_todo.empty())
        break;

      if (entryIdealDeleter.get() != 0)
        freeIdeal(std::move(entryIdealDeleter));
      entry = _todo.back();
      _todo.pop_back();

      ASSERT(entryIdealDeleter.get() == 0);
      entryIdealDeleter.reset(entry.ideal);
    }
    ASSERT(_todo.empty());

    // originalIdeal is in some state that depends on the particular
    // steps the algorithm took. This information should not escape
    // HilbertBasecase, and we ensure this by clearing originalIdeal.
    originalIdeal.clear();
  } catch (...) {
    for (vector<Entry>::iterator it = _todo.begin(); it != _todo.end(); ++it)
      delete it->ideal;
    _todo.clear();
    throw;
  }
}

const mpz_class& HilbertBasecase::getLastCoefficient() {
  return _sum;
}

bool HilbertBasecase::canSimplify(size_t var,
                                  const Ideal& ideal,
                                  const Term& counts) {
  if (counts[var] == 0)
    return false;
  Ideal::const_iterator stop = ideal.end();

  size_t varCount = counts.getVarCount();
  for (size_t other = 0; other < varCount; ++other) {
    if (other == var || counts[other] == 0)
      continue;

	// todo: the answer is always no, I think, if var appears in less
	// generators than other does, since then there must be some
	// generator that other appears in that var does not.

    bool can = true;
    for (Ideal::const_iterator it = ideal.begin(); it != stop; ++it) {
      if ((*it)[var] == 0 && (*it)[other] > 0) {
        can = false;
        break;
      }
    }
    if (can)
      return true;
  }
  return false;
}

size_t HilbertBasecase::eliminate1Counts(Ideal& ideal,
                                         Term& counts,
                                         bool& negate) {
  size_t varCount = ideal.getVarCount();
  size_t adj = 0;
  for (size_t var = 0; var < varCount; ++var) {
    if (counts[var] != 1)
      continue;

    Ideal::const_iterator it = ideal.getMultiple(var);
    ASSERT(it != ideal.end());

    for (size_t other = 0; other < varCount; ++other) {
      if ((*it)[other] > 0) {
        ++adj;
        if (counts[other] == 1)
          counts[other] = 0;
      } else
        counts[other] = 0;
    }

    for (size_t other = 0; other < varCount; ++other) {
      if (counts[other] > 0) {
        if (!ideal.colonReminimize(other, 1)) {
          ideal.clear();
          return 1;
        }
      }
    }

    it = ideal.getMultiple(var);
    if (it == ideal.end()) {
      ideal.clear();
      return 1;
    }
    ideal.remove(it);
    negate = !negate;

    return adj;
  }

  for (size_t var = 0; var < varCount; ++var) {
    if (canSimplify(var, ideal, counts)) {
      if (!ideal.colonReminimize(var, 1))
        ideal.clear();
      return adj + 1;
    }
  }

  return adj;
}

unique_ptr<Ideal> HilbertBasecase::getNewIdeal() {
  if (_idealCache.empty())
    return unique_ptr<Ideal>(new Ideal());

  unique_ptr<Ideal> ideal(_idealCache.back());
  _idealCache.pop_back();

  return ideal;
}

void HilbertBasecase::freeIdeal(unique_ptr<Ideal> ideal) {
  ASSERT(ideal.get() != 0);

  ideal->clear();
  exceptionSafePushBack(_idealCache, std::move(ideal));
}
