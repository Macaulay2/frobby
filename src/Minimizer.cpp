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
#include "Minimizer.h"

#include "TermPredicate.h"
#include "Term.h"
#include <algorithm>

/** @todo: Convert everything in this file to be in terms of
    IdealTree. After that, consider converting all clients of this
    code to use IdealTree directly. */

namespace {
  typedef vector<Exponent*>::iterator TermIterator;
}

TermIterator simpleMinimize(TermIterator begin, TermIterator end, size_t varCount) {
  if (begin == end)
    return end;

  std::sort(begin, end, LexComparator(varCount));

  TermIterator newEnd = begin;
  ++newEnd; // The first one is always kept
  TermIterator dominator = newEnd;
  for (; dominator != end; ++dominator) {
    bool remove = false;
    for (TermIterator divisor = begin; divisor != newEnd; ++divisor) {
      if (Term::divides(*divisor, *dominator, varCount)) {
        remove = true;
        break;
      }
    }

    if (!remove) {
      *newEnd = *dominator;
      ++newEnd;
    }
  }
  return newEnd;
}

TermIterator twoVarMinimize(TermIterator begin, TermIterator end) {
  if (begin == end)
    return end;

  std::sort(begin, end, LexComparator(2));

  TermIterator last = begin;
  TermIterator it = begin;
  ++it;
  for (; it != end; ++it) {
    if ((*it)[1] < (*last)[1]) {
      ++last;
      *last = *it;
    }
  }

  ++last;
  return last;
}

class TreeNode {
  typedef vector<Exponent*>::iterator iterator;

public:
  TreeNode(iterator begin, iterator end, size_t varCount):
    _lessOrEqual(0),
    _greater(0),
    _var(0),
    _pivot(0),
    _varCount(varCount),
    _begin(begin),
    _end(end) {
  }

  ~TreeNode() {
    delete _lessOrEqual;
    delete _greater;
  }

  void makeTree() {
    ASSERT(_greater == 0);
    ASSERT(_lessOrEqual == 0);

    if (distance(_begin, _end) > 20) {

      Term lcm(_varCount);
      for (iterator it = _begin; it != _end; ++it)
        lcm.lcm(lcm, *it);

      while (true) {
        size_t maxVar = 0;

        for (size_t var = 0; var < _varCount; ++var)
          if (lcm[var] > lcm[maxVar])
            maxVar = var;
        if (lcm[maxVar] == 0) {
          break; // we are not making any progress anyway
        }

        ASSERT(lcm[maxVar] >= 1);
        _var = maxVar;
        _pivot = lcm[maxVar] / 4;
        lcm[maxVar] = 0; // So we do not process this same var again.

        iterator left = _begin;
        iterator right = _end - 1;
        while (left != right) {
          while ((*left)[_var] <= _pivot && left != right)
            ++left;
          while ((*right)[_var] > _pivot && left != right)
            --right;
          swap(*left, *right);
        }
        ASSERT((*right)[_var] > _pivot);
        if ((*_begin)[_var] > _pivot)
          continue;

        iterator middle = right;
        while ((*middle)[maxVar] > _pivot)
          --middle;
        ++middle;

        ASSERT(middle != _begin);

        _lessOrEqual = FROBBY_NEW TreeNode(_begin, middle, _varCount);
        _greater = FROBBY_NEW TreeNode(middle, _end, _varCount);
        _end = _begin;

        _lessOrEqual->makeTree();
        _greater->makeTree();
        return;
      }
    }

    _end = simpleMinimize(_begin, _end, _varCount);

  }

  bool isRedundant(const Exponent* term) {
    if (_begin != _end) {
      ASSERT(_lessOrEqual == 0);
      ASSERT(_greater == 0);

      for (iterator it = _begin; it != _end; ++it)
        if (Term::dominates(term, *it, _varCount))
          return true;
      return false;
    } else {
      ASSERT(_lessOrEqual != 0);
      ASSERT(_greater != 0);

      if (term[_var] > _pivot && _greater->isRedundant(term))
        return true;
      if (_lessOrEqual->isRedundant(term))
        return true;
      return false;
    }
  }

  void collect(vector<Exponent*>& terms) {
    if (_begin != _end) {
      ASSERT(_lessOrEqual == 0);
      ASSERT(_greater == 0);
      terms.insert(terms.end(), _begin, _end);
      return;
    }
    ASSERT(_lessOrEqual != 0);
    ASSERT(_greater != 0);

    size_t oldSize = terms.size();
    _greater->collect(terms);
    for (size_t i = oldSize; i < terms.size();) {
      if (_lessOrEqual->isRedundant(terms[i])) {
        swap(terms[i], terms.back());
        terms.pop_back();
      } else
        ++i;
    }

    _lessOrEqual->collect(terms);
  }

  void print(FILE* out) {
    if (_begin == _end) {
      ASSERT(_lessOrEqual != 0);
      ASSERT(_greater != 0);

      fprintf(out, "NODE (pivot=%lu^%lu, varCount = %lu\n",
              (unsigned long)_var,
              (unsigned long)_pivot,
              (unsigned long)_varCount);
      fputs("-lessOrEqual: ", out);
      _lessOrEqual->print(out);
      fputs("-greater: ", out);
      _greater->print(out);
      fputs(")\n", out);
    } else {
      ASSERT(_lessOrEqual == 0);
      ASSERT(_greater == 0);

      fprintf(out, "NODE (_varCount = %lu terms:\n", (unsigned long)_varCount);
      for (iterator it = _begin; it != _end; ++it) {
        fputc(' ', out);
        Term::print(out, *it, _varCount);
        fprintf(out, " %p\n", (void*)*it);
      }
      fputs(")\n", out);
    }
  }

private:
  TreeNode* _lessOrEqual;
  TreeNode* _greater;
  size_t _var;
  Exponent _pivot;
  size_t _varCount;

  iterator _begin;
  iterator _end;
};

Minimizer::iterator Minimizer::minimize(iterator begin, iterator end) const {
  if (_varCount == 2)
    return twoVarMinimize(begin, end);
  if (distance(begin, end) < 1000 || _varCount == 0)
    return simpleMinimize(begin, end, _varCount);

  vector<Exponent*> terms;
  terms.clear();
  terms.reserve(distance(begin, end));

  TreeNode node(begin, end, _varCount);
  node.makeTree();
  node.collect(terms);

  return copy(terms.begin(), terms.end(), begin);
}

pair<Minimizer::iterator, bool> Minimizer::colonReminimize
(iterator begin, iterator end, const Exponent* colon) {
  ASSERT(isMinimallyGenerated(begin, end));

  if (Term::getSizeOfSupport(colon, _varCount) == 1) {
    size_t var = Term::getFirstNonZeroExponent(colon, _varCount);
    return colonReminimize(begin, end, var, colon[var]);
  }

  iterator blockBegin = end;
  for (iterator it = begin; it != blockBegin;) {
    bool block = true;
    bool strictDivision = true;
    for (size_t var = 0; var < _varCount; ++var) {
      if (colon[var] >= (*it)[var]) {
        if ((*it)[var] > 0)
          block = false;
        if (colon[var] > 0)
          strictDivision = false;
        (*it)[var] = 0;
      } else
        (*it)[var] -= colon[var];
    }

    if (strictDivision) {
      swap(*begin, *it);
      ++begin;
      ++it;
    } else if (block) {
      --blockBegin;
      swap(*it, *blockBegin);
    } else
      ++it;
  }

  if (begin == blockBegin)
    return make_pair(end, false);

  iterator newEnd = minimize(begin, blockBegin);

  for (iterator it = blockBegin; it != end; ++it) {
    if (!dominatesAny(begin, blockBegin, *it)) {
      *newEnd = *it;
      ++newEnd;
    }
  }

  ASSERT(isMinimallyGenerated(begin, newEnd));
  return make_pair(newEnd, true);
}

bool Minimizer::isMinimallyGenerated
(const_iterator begin, const_iterator end) {
  if (distance(begin, end) < 1000 || _varCount == 0) {
    for (const_iterator divisor = begin; divisor != end; ++divisor)
      for (const_iterator dominator = begin; dominator != end; ++dominator)
        if (Term::divides(*divisor, *dominator, _varCount) &&
            divisor != dominator)
          return false;
    return true;
  }

  vector<Exponent*> terms(begin, end);
  TreeNode node(terms.begin(), terms.end(), _varCount);
  node.makeTree();

  vector<Exponent*> terms2;
  node.collect(terms2);

  return terms.size() == terms2.size();
}

bool Minimizer::dominatesAny
(iterator begin, iterator end, const Exponent* term) {
  for (; begin != end; ++begin)
    if (Term::dominates(term, *begin, _varCount))
      return true;
  return false;
}

bool Minimizer::dividesAny
(iterator begin, iterator end, const Exponent* term) {
  for (; begin != end; ++begin)
    if (Term::divides(term, *begin, _varCount))
      return true;
  return false;
}

pair<Minimizer::iterator, bool> Minimizer::colonReminimize
(iterator begin, iterator end, size_t var, Exponent exponent) {

  // Sort in descending order according to exponent of var while
  // ignoring everything that is strictly divisible by
  // var^exponent. We put the zero entries at the right end
  // immediately, before calling sort, because there are likely to be
  // many of them, and we can do so while we are anyway looking for
  // the strictly divisible monomials. The combination of these
  // significantly reduce the number of monomials that need to be
  // sorted.
  iterator zeroBegin = end;
  for (iterator it = begin; it != zeroBegin;) {
    if ((*it)[var] > exponent) {
      (*it)[var] -= exponent; // apply colon
      swap(*it, *begin);
      ++begin;
      ++it;
    } else if ((*it)[var] == 0) {
      // no need to apply colon in this case
      --zeroBegin;
      swap(*it, *zeroBegin);
    } else
      ++it;
  }

  if (begin == zeroBegin)
    return make_pair(end, false);

  // Sort the part of the array that we have not handled yet.
  std::sort(begin, zeroBegin,
            ReverseSingleDegreeComparator(var, _varCount));

  // We group terms into blocks according to term[var].
  iterator previousBlockEnd = begin;
  iterator newEnd = begin;

  Exponent block = (*begin)[var];

  for (iterator it = begin; it != end; ++it) {
    // Detect if we are moving on to next block.
    if ((*it)[var] != block) {
      block = (*it)[var];
      previousBlockEnd = newEnd;
    }

    ASSERT((*it)[var] <= exponent);
    (*it)[var] = 0;

    bool remove = false;

    for (iterator divisor = begin; divisor != previousBlockEnd; ++divisor) {
      if (Term::divides(*divisor, *it, _varCount)) {
        remove = true;
        break;
      }
    }

    if (!remove) {
      *newEnd = *it;
      ++newEnd;
    }
  }

  return make_pair(newEnd, true);
}
