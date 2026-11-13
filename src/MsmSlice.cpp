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
#include "MsmSlice.h"

#include "TermConsumer.h"
#include "MsmStrategy.h"

MsmSlice::MsmSlice(MsmStrategy& strategy):
  Slice(strategy),
  _consumer(0) {
}

MsmSlice::MsmSlice(MsmStrategy& strategy,
                   const Ideal& ideal,
                   const Ideal& subtract,
                   const Term& multiply,
                   TermConsumer* consumer):
  Slice(strategy, ideal, subtract, multiply),
  _consumer(consumer) {
  ASSERT(consumer != 0);

  removeDoubleLcm();
}

bool MsmSlice::baseCase(bool simplified) {
  ASSERT(_consumer != 0);

  if (getIdeal().getGeneratorCount() < _varCount)
    return true;

  // Check that each variable appears in some minimal generator.
  if (getLcm().getSizeOfSupport() < _varCount)
    return true;

  ASSERT(!removeDoubleLcm());

  if (_varCount == 0) {
    if (getIdeal().isZeroIdeal())
      _consumer->consume(_multiply);
    return true;
  }

  if (_varCount == 1) {
    _consumer->consume(_multiply);
    return true;
  }

  if (!simplified) {
    if (getLcm().isSquareFree()) {
      // We know this since !removeDoubleLcm().
      ASSERT(getIdeal().isIrreducible());

      _consumer->consume(_multiply);
      return true;
    }

    if (getIdeal().getGeneratorCount() == _varCount) {
      if (getSubtract().isZeroIdeal()) {
        _lcm.decrement();
        _multiply.product(_multiply, _lcm);
      } else {
        Term tmp(getLcm());
        tmp.decrement();
        innerSlice(tmp);
        if (getIdeal().getGeneratorCount() < _varCount)
          return true;
      }
      _consumer->consume(_multiply);
      return true;
    }

    return false;
  }

  if (_varCount == 2) {
    twoVarBaseCase();
    return true;
  }

  if (getIdeal().getGeneratorCount() > _varCount) {
    if (getIdeal().getGeneratorCount() == _varCount + 1) {
      oneMoreGeneratorBaseCase();
      return true;
    }
    if (twoNonMaxBaseCase())
      return true;

    return false;
  }

  // These things are ensured since the slice is simplified.
  ASSERT(getLcm().isSquareFree());
  ASSERT(getIdeal().isIrreducible());

  _consumer->consume(_multiply);
  return true;
}

Slice& MsmSlice::operator=(const Slice& slice) {
  ASSERT(dynamic_cast<const MsmSlice*>(&slice) != 0);

  Slice::operator=(slice);
  _consumer = ((MsmSlice&)slice)._consumer;
  return *this;
}

bool MsmSlice::simplifyStep() {
  ASSERT(!removeDoubleLcm());

  if (applyLowerBound())
    return true;

  pruneSubtract();

  ASSERT(!removeDoubleLcm());
  return false;
}

void MsmSlice::setToProjOf(const MsmSlice& slice,
                           const Projection& projection,
                           TermConsumer* consumer) {
  ASSERT(consumer != 0);

  Slice::setToProjOf(slice, projection);
  _consumer = consumer;
}

bool MsmSlice::innerSlice(const Term& pivot) {
  ASSERT(!removeDoubleLcm());

  bool changedMuch = Slice::innerSlice(pivot);
  if (!_lcmUpdated)
    changedMuch = removeDoubleLcm() || changedMuch;

  ASSERT(getLcm().getSizeOfSupport() < getVarCount() || !removeDoubleLcm());

  return changedMuch;
}

void MsmSlice::outerSlice(const Term& pivot) {
  ASSERT(!removeDoubleLcm());

  Slice::outerSlice(pivot);
  if (!_lcmUpdated)
    removeDoubleLcm();

  ASSERT(!removeDoubleLcm());
}

void MsmSlice::swap(MsmSlice& slice) {
  Slice::swap(slice);
  std::swap(_consumer, slice._consumer);
}

// Helper class for removeDoubleLcm().
class DoubleLcmPredicate {
public:
  DoubleLcmPredicate(const Term& lcm):
    _lcm(lcm) {
  }
  DoubleLcmPredicate(const DoubleLcmPredicate&) = default;
  DoubleLcmPredicate& operator=(const DoubleLcmPredicate&) = delete;

  bool operator()(const Exponent* term) {
    bool seenMatch = false;
    for (size_t var = 0; var < _lcm.getVarCount(); ++var) {
      if (term[var] == _lcm[var]) {
        if (seenMatch)
          return true;
        seenMatch = true;
      }
    }
    return false;
  }

private:
  const Term& _lcm;
};

bool MsmSlice::removeDoubleLcm() {
  if (_ideal.getGeneratorCount() == 0)
    return false;

  bool removedAny = false;

  while (true) {
    DoubleLcmPredicate pred(getLcm());
    if (!_ideal.removeIf(pred))
      break;

    removedAny = true;
    _lcmUpdated = false;
  };

  return removedAny;
}

bool MsmSlice::getLowerBound(Term& bound, size_t var) const {
  const Term& lcm = getLcm();
  bound = lcm;

  Ideal::const_iterator stop = getIdeal().end();
  for (Ideal::const_iterator it = getIdeal().begin(); it != stop; ++it) {
    Exponent* term = *it;
    if (term[var] == 0)
      continue;

    // Use the fact that terms with a maximal exponent somewhere not
    // at var cannot be a var-label.
    for (size_t var2 = 0; var2 < _varCount; ++var2)
      if (term[var2] == lcm[var2] && var2 != var)
        goto skip;

    bound.gcd(bound, *it);
  skip:;
  }

  ASSERT(_varCount >= 2);
  if (bound[0] == lcm[0] && bound[1] == lcm[1]) {
    // No possible var-label, so the content is empty.
    return false;
  }

  ASSERT(bound[var] >= 1);
  bound[var] -= 1;
  return true;
}

void MsmSlice::twoVarBaseCase() {
  ASSERT(_varCount == 2);

  singleDegreeSortIdeal(0);

  // We use _lcm to store the maximal standard monomial in, since we
  // are not going to be using _lcm later anyway, and this way we
  // avoid having to do a potentially costly allocation of an array of
  // size 2 in this method that can be called a lot on small ideals.
  _lcmUpdated = false;

  Ideal::const_iterator stop = getIdeal().end();
  Ideal::const_iterator it = getIdeal().begin();
  if (it == stop)
    return;

  while (true) {
    _lcm[1] = (*it)[1] - 1;

    ++it;
    if (it == stop)
      break;

    _lcm[0] = (*it)[0] - 1;

    ASSERT(!getIdeal().contains(_lcm));

    if (!_subtract.contains(_lcm)) {
      _lcm[0] += _multiply[0];
      _lcm[1] += _multiply[1];

      _consumer->consume(_lcm);
    }
  }
}

void MsmSlice::oneMoreGeneratorBaseCase() {
  ASSERT(_varCount + 1 == getIdeal().getGeneratorCount());

  // Since the slice is fully simplified, we must be dealing with an
  // artinian power in each variable, and then a single generator
  // whose support is at least 2. We can then simply run through all
  // the possibilities for that generator to be a label.

  Ideal::const_iterator it = getIdeal().begin();
  while (Term::getSizeOfSupport(*it, _varCount) == 1) {
    ++it;
    ASSERT(it != getIdeal().end());
  }

  Term msm(_varCount);
  for (size_t var = 0; var < _varCount; ++var) {
    ASSERT((*it)[var] <= 1);
    ASSERT((*it)[var] < getLcm()[var]);
    msm[var] = getLcm()[var] - 1;
    _multiply[var] += msm[var];
  }

  for (size_t var = 0; var < _varCount; ++var) {
    if ((*it)[var] == 1) {
      msm[var] = 0; // try *it as var-label
      if (!getSubtract().contains(msm)) {
        _multiply[var] -= getLcm()[var] - 1;
        _consumer->consume(_multiply);
        _multiply[var] += getLcm()[var] - 1;
      }
      msm[var] = getLcm()[var] - 1;
    }
  }
}

// Helper function for the method twoNonMaxBaseCase.
bool getTheOnlyTwoNonMax(Ideal::const_iterator it,
             const Exponent*& first,
             const Exponent*& second,
             Ideal::const_iterator end,
             const Term& lcm) {
  size_t count = 0;
  for (; it != end; ++it) {
    bool nonMax = true;
    for (size_t var = 0; var < lcm.getVarCount(); ++var) {
      if ((*it)[var] == lcm[var]) {
    nonMax = false;
    break;
      }
    }
    if (nonMax) {
      if (count == 0)
    first = *it;
      else if (count == 1)
    second = *it;
      else
    return false;
      ++count;
    }
  }
  return count == 2;
}

bool MsmSlice::twoNonMaxBaseCase() {
  const Term& lcm = getLcm();
  Ideal::const_iterator stop = getIdeal().end();

  const Exponent* nonMax1;
  const Exponent* nonMax2;
  if (!getTheOnlyTwoNonMax(getIdeal().begin(), nonMax1, nonMax2, stop, lcm))
    return false;

  Term msm(_lcm);
  for (size_t var = 0; var < _varCount; ++var)
    msm[var] -= 1;
  Term tmp(_varCount);

  for (size_t var1 = 0; var1 < _varCount; ++var1) {
    if (nonMax1[var1] == 0)
      continue;
    if (nonMax1[var1] <= nonMax2[var1])
      continue;

    for (size_t var2 = 0; var2 < _varCount; ++var2) {
      if (var1 == var2 || nonMax2[var2] == 0)
        continue;
      if (nonMax2[var2] <= nonMax1[var2])
        continue;

      // Use tmp to record those variables for which labels have been
      // found. If some variable has no label, then we are not dealing
      // with an actual maximal standard monomial.
      tmp.setToIdentity();
      tmp[var1] = true;
      tmp[var2] = true;
      for (Ideal::const_iterator it = getIdeal().begin(); it != stop; ++it) {
        if ((*it)[var1] >= nonMax1[var1] ||
            (*it)[var2] >= nonMax2[var2])
          continue;

        for (size_t var = 0; var < lcm.getVarCount(); ++var) {
          if ((*it)[var] == lcm[var]) {
            tmp[var] = true;
            break;
          }
        }
      }

      if (tmp.getSizeOfSupport() < _varCount)
        continue;

      msm[var1] = nonMax1[var1] - 1;
      msm[var2] = nonMax2[var2] - 1;
      if (!getSubtract().contains(msm)) {
        tmp.product(msm, _multiply);
        _consumer->consume(tmp);
      }
      msm[var2] = lcm[var2] - 1;
      msm[var1] = lcm[var1] - 1;
    }
  }

  for (size_t var = 0; var < _varCount; ++var) {
    Exponent e;
    if (nonMax1[var] < nonMax2[var])
      e = nonMax1[var];
    else
      e = nonMax2[var];
    if (e == 0)
      continue;

    msm[var] = e - 1;
    if (!getSubtract().contains(msm)) {
      tmp.product(msm, _multiply);
      _consumer->consume(tmp);
    }
    msm[var] = lcm[var] - 1;
  }

  return true;
}
