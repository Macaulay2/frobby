/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2007 Bjarke Hammersholt Roune (www.broune.com)
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
#include "IdealTree.h"

#include "Ideal.h"
#include "Term.h"

namespace {
  const size_t MaxLeafSize = 60;

  bool rawStrictlyDivides(Ideal::const_iterator begin,
                          Ideal::const_iterator end,
                          const Exponent* term,
                          size_t varCount) {
    for (; begin != end; ++begin)
      if (Term::strictlyDivides(*begin, term, varCount))
        return true;
    return false;
  }
}

class IdealTree::Node {
public:
  Node(Ideal::iterator begin,
       Ideal::iterator end,
       size_t varCount):
    _begin(begin), _end(end), _varCount(varCount) {
    makeTree();
  }

  void makeTree();
  bool strictlyContains(const Exponent* term) const;
  size_t getVarCount() const {return _varCount;}

private:
  unique_ptr<Node> _lessOrEqual;
  unique_ptr<Node> _greater;
  Ideal::iterator _begin;
  Ideal::iterator _end;
  size_t _varCount;
  size_t _var;
  size_t _pivot;
};

void IdealTree::Node::makeTree() {
  if ((size_t)distance(_begin, _end) <= MaxLeafSize)
    return;

  Term lcm(_varCount);
  Term gcd(*_begin, _varCount);
  for (Ideal::const_iterator it = _begin; it != _end; ++it) {
    lcm.lcm(lcm, *it);
    gcd.gcd(gcd, *it);
  }

  while (true) {
    size_t maxVar = 0;
    for (size_t var = 1; var < _varCount; ++var)
      if (lcm[var] - gcd[var] > lcm[maxVar] - gcd[maxVar])
        maxVar = var;

    // TODO: could this ever happen?
    if (lcm[maxVar] == 0)
      break; // we are not making any progress anyway.

    ASSERT(lcm[maxVar] >= 1);
    _var = maxVar;

    // It's significant that we are rounding down to ensure that
    // neither of _lessOrEqual and _greater becomes empty.
    _pivot = (lcm[maxVar] + gcd[maxVar]) >> 1; // Note: x >> 1 == x / 2

    Ideal::iterator left = _begin;
    Ideal::iterator right = _end - 1;

    // put those strictly divisible by _var^_pivot to right and the
    // rest to the left.
    while (left != right) {
      // Find left-most element that should go right.
      while ((*left)[_var] <= _pivot && left != right)
        ++left;

      // Find right-most element that should go left.
      while ((*right)[_var] > _pivot && left != right)
        --right;

      // Swap the two found elements so that they both go into the
      // right place.
      using std::swap;
      swap(*left, *right);
    }
    ASSERT((*(_end - 1))[_var] > _pivot);
    ASSERT((*_begin)[_var] <= _pivot);

    // Make middle the first element that went right, so that the two
    // ranges become [_begin, middle) and [middle, _end).
    ASSERT((*right)[_var] > _pivot)
    Ideal::iterator middle = right;
    while ((*middle)[_var] > _pivot) {
      ASSERT(middle != _begin);
      --middle;
    }
    ++middle;
    ASSERT(_begin < middle && middle <= _end);
    ASSERT((*middle)[_var] > _pivot);
    ASSERT((*(middle - 1))[_var] <= _pivot);

    _lessOrEqual.reset(new Node(_begin, middle, _varCount));
    _greater.reset(new Node(middle, _end, _varCount));

    _lessOrEqual->makeTree();
    _greater->makeTree();
    return;
  }
}

bool IdealTree::Node::strictlyContains(const Exponent* term) const {
  if (_lessOrEqual.get() != 0) {
    ASSERT(_greater.get() != 0);
    bool returnValue =
      _lessOrEqual->strictlyContains(term) ||
      _greater->strictlyContains(term);
    ASSERT(returnValue == rawStrictlyDivides(_begin, _end, term, _varCount));
    return returnValue;
  } else {
    ASSERT(_greater.get() == 0);
    return rawStrictlyDivides(_begin, _end, term, _varCount);
  }
}

IdealTree::IdealTree(const Ideal& ideal) {
  // not using initialization for this to avoid depending on order of
  // initialization of members.
  _storage.reset(new Ideal(ideal));
  _root.reset
    (new Node(_storage->begin(), _storage->end(), ideal.getVarCount()));
}

IdealTree::~IdealTree() {
  // Destructor defined so unique_ptr<T> in the header does not need
  // definition of T.
}

bool IdealTree::strictlyContains(const Exponent* term) const {
  ASSERT(_root.get() != 0);
  return _root->strictlyContains(term);
}

size_t IdealTree::getVarCount() const {
  ASSERT(_root.get() != 0);
  return _root->getVarCount();
}
