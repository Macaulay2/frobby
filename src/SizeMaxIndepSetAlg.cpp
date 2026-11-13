/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 Bjarke Hammersholt Roune (www.broune.com)

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
#include "SizeMaxIndepSetAlg.h"

#include "Ideal.h"
#include "Term.h"

void SizeMaxIndepSetAlg::run(Ideal& ideal) {
  ASSERT(ideal.isSquareFree());
  ASSERT(ideal.isMinimallyGenerated());

  if (ideal.getGeneratorCount() == 1 && // for efficiency
      ideal.containsIdentity()) {
    _noIndependentSets = true;
    return;
  } else
    _noIndependentSets = false;

  // Improves efficiency by putting related edges together.
  ideal.sortReverseLex();

  _varCount = ideal.getVarCount();

  // OK since we now know that ideal does have independent sets.
  _minExcluded = _varCount;

  // Allocate this now so we don't have to ensure this at every step
  // in the algorithm.
  _undo.resize(_varCount + 1);

  // Encode the hypergraph of the ideal into _edges.
  for (size_t term = 0; term < ideal.getGeneratorCount(); ++term) {
    _edges.push_back(Term::getSizeOfSupport(ideal[term], _varCount));
    for (size_t var = 0; var < _varCount; ++var) {
      if (ideal[term][var] != 0) {
        ASSERT(ideal[term][var] == 1);
        _edges.push_back(var);
      }
    }
  }

  _endPos = _edges.size();

  // Make state
  _state.clear();
  _state.resize(_varCount);

  // Run the algorithm.
  recurse(0, 0);
}

mpz_class SizeMaxIndepSetAlg::getMaxIndepSetSize() {
  // We can't just let _minExcluded itself be _varCount + 1, as that
  // may cause an overflow due to non-infinite precision of size_t.
  if (_noIndependentSets)
    return -1;
  else {
    ASSERT(_varCount >= _minExcluded);
    return _varCount - _minExcluded;
  }
}

bool SizeMaxIndepSetAlg::isIndependentIncludingMaybe(size_t pos) {
  while (pos != _endPos) {
    size_t nextPos = pos + _edges[pos] + 1;
    while (true) {
      ++pos;
      if (pos == nextPos)
        return false;
      if (_state[_edges[pos]] == IsNotInSet)
        break;
    }
    pos = nextPos;
  }
  return true;
}

inline bool SizeMaxIndepSetAlg::couldBeDependence(size_t pos, size_t nextPos, size_t& maybeCount) {
  maybeCount = 0;
  for (size_t p = pos + 1; p != nextPos; ++p) {
    VarState varState = _state[_edges[p]];
    if (varState == IsNotInSet) {
      // In this case the term cannot make the set dependent.
      return false;
    } else if (varState == IsMaybeInSet)
      ++maybeCount;
  }
  return true;
}

void SizeMaxIndepSetAlg::recurse(size_t pos, size_t excluded) {
  ASSERT(_undo[excluded].empty());
  ASSERT(pos <= _endPos);
  ASSERT(excluded <= _varCount);

  // Branch-and-bound criterion.
  if (excluded >= _minExcluded)
    return;

  // An optimization made possible by branch-and-bound. If we are only
  // 1 node from being excluded by the branch-and-bound criterion
  // above, then every IsMaybeInSet must be a InSet if we are to make
  // an improvement. So there is no need for further backtracking - it
  // only matters if the set we are looking at now is independent
  // where maybe's are treated as yes.
  if (excluded + 1 == _minExcluded) {
    if (isIndependentIncludingMaybe(pos))
      _minExcluded = excluded;
    return;
  }

  // Run through the edges only one becomes undecided (and then
  // consider cases) or we know that the set is independent according
  // to all edges.
  while (true) {
    // The set is independent according to all edges.
    if (pos == _endPos) {
      // The set has not been eliminated by brand-and-bound, so there
      // must be an improvement.
      ASSERT(excluded < _minExcluded);
      _minExcluded = excluded;
      break;
    }

    // The starting point of the encoding of the next term.
    size_t nextPos = pos + _edges[pos] + 1;

    // Set to the number of maybe's in the support of the term at pos.
    size_t maybeCount;
    if (!couldBeDependence(pos, nextPos, maybeCount)) {
      // This edge cannot make the set dependent, so move on to the
      // next one.
      pos = nextPos;
      continue;
    }

    if (maybeCount == 0) {
      // This edge definitely makes the set dependent, so stop looking
      // at this further.
      break;
    }

    // Now we consider the two cases for each undecided variable.

    vector<size_t>& undo = _undo[excluded]; // for convenience
    for (size_t p = pos + 1; p != nextPos; ++p) {
      size_t var = _edges[p];
      VarState& varState = _state[var];

      if (varState != IsMaybeInSet)
        continue;

      // The case of var definitely not in the set.
      varState = IsNotInSet;
      recurse(nextPos, excluded + 1);
      // recurse may change temporarily change _state, but it restores
      // it to the way it was before it returns, so consider it as
      // though this call did not change _state. This is done because
      // this way is more efficient than copying since often
      // maybeCount is much less than _varCount.

      // We have considered the other case, so now let var definitely
      // be in the set, moving on to the next undecided variable, if
      // any.

      if (maybeCount == 1) {
        // There are no more undecided vars, so restore _state to the
        // way it was when we were called and then return to the
        // caller.
        varState = IsMaybeInSet;
        while (!undo.empty()) {
          _state[undo.back()] = IsMaybeInSet;
          undo.pop_back();
        }
        return;
      }

      varState = IsInSet;
      --maybeCount;

      // Store information needed to restore _state to the way it was
      // before.
      undo.push_back(var);
    }
    break;
  }
}
