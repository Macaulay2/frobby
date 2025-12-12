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
#include "SplitStrategy.h"

#include "Slice.h"
#include "Term.h"
#include "Ideal.h"
#include "NameFactory.h"
#include "TermGrader.h"
#include "error.h"
#include "display.h"

SplitStrategy::SplitStrategy() {
}

SplitStrategy::~SplitStrategy() {
}

/** This common base class provides code that is useful for writing
 pivot split strategies.
*/
class SplitStrategyCommon : public SplitStrategy {
public:
  virtual void getPivot(Term& pivot, Slice& slice) const {
    ASSERT(false);
    reportInternalError("Requested pivot split of non-pivot split strategy.\n");
  }

  virtual void getPivot(Term& pivot, Slice& slice, const TermGrader& grader) const {
    getPivot(pivot, slice);
  }

  virtual size_t getLabelSplitVariable(const Slice& slice) const {
    ASSERT(false);
    reportInternalError("Requested label split of non-label split strategy.");
    return 0; // To avoid spurious warnings from static analyses.
  }

  virtual bool isPivotSplit() const {
    return false;
  }

  virtual bool isLabelSplit() const {
    return false;
  }

protected:
  Exponent getMedianPositiveExponentOf(Slice& slice, size_t var) const {
    slice.singleDegreeSortIdeal(var);
    Ideal::const_iterator end = slice.getIdeal().end();
    Ideal::const_iterator begin = slice.getIdeal().begin();
    while ((*begin)[var] == 0) {
      ++begin;
      ASSERT(begin != end);
    }
    return (*(begin + (distance(begin, end) ) / 2))[var];
  }

  // Returns the variable that divides the most minimal generators of
  // those where some minimal generator is divisible by the square of
  // that variable.
  mutable Term coVariableForGetBestVar;
  size_t getBestVar(const Slice& slice) const {
    Term& co = coVariableForGetBestVar;
    co.reset(slice.getVarCount());
    slice.getIdeal().getSupportCounts(co);

    const Term& lcm = slice.getLcm();
    for (size_t var = 0; var < slice.getVarCount(); ++var)
      if (lcm[var] <= 1)
        co[var] = 0;

    ASSERT(!co.isIdentity());

    Exponent maxCount = co[co.getFirstMaxExponent()];
    for (size_t var = 0; var < slice.getVarCount(); ++var)
      if (co[var] < maxCount)
        co[var] = 0;

    // Choose the middle variable among those that are best. This
    // is better at avoiding a bad pattern than just choosing the
    // first one.
    return co.getMiddleNonZeroExponent();
  }
};

class LabelSplit : public SplitStrategyCommon {
protected:
  mutable Term _counts;
  void setCounts(const Slice& slice) const {
    _counts.reset(slice.getVarCount());
    slice.getIdeal().getSupportCounts(_counts);
  }

  mutable Term _oneCounts;
  void setOneCounts(const Slice& slice) const {
    ASSERT(!const_cast<Slice&>(slice).adjustMultiply());
    ASSERT(!const_cast<Slice&>(slice).baseCase(false));
    // For each variable, count number of terms with exponent equal to 1,
    // not counting pure powers.
    _oneCounts.reset(slice.getVarCount());

    Ideal::const_iterator end = slice.getIdeal().end();
    for (Ideal::const_iterator it = slice.getIdeal().begin();
         it != end; ++it) {
      if (Term::getSizeOfSupport(*it, slice.getVarCount()) == 1)
        continue; // Not counting pure powers.
      for (size_t var = 0; var < slice.getVarCount(); ++var)
        if ((*it)[var] == 1)
          _oneCounts[var] += 1;
    }
  }

  virtual bool isLabelSplit() const {
    return true;
  }
};

// Use the variable that divides the most minimal generators.
class MaxLabelSplit : public LabelSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "maxlabel";
  }

  virtual size_t getLabelSplitVariable(const Slice& slice) const {
    setCounts(slice);
    return _counts.getFirstMaxExponent();
  }
};

// Use the first variable that is valid for a label split.
class VarLabelSplit : public LabelSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "varlabel";
  }

  virtual size_t getLabelSplitVariable(const Slice& slice) const {
    setOneCounts(slice);
    for (size_t var = 0; ; ++var) {
        ASSERT(var < slice.getVarCount());
        if (_oneCounts[var] > 0)
          return var;
      }
    }
};

// Among those variables with least number of exponents equal to one,
// use the variable that divides the most minimal generators.
class MinLabelSplit : public LabelSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "minlabel";
  }

  virtual size_t getLabelSplitVariable(const Slice& slice) const {
    setCounts(slice);
    setOneCounts(slice);

    // Zero those variables of _counts that have more than the least number
    // of exponent 1 minimal generators.
    size_t mostGeneric = 0;
    for (size_t var = 1; var < slice.getVarCount(); ++var)
      if (mostGeneric == 0 ||
          (mostGeneric > _oneCounts[var] && _oneCounts[var] > 0))
        mostGeneric = _oneCounts[var];
    for (size_t var = 0; var < slice.getVarCount(); ++var)
      if (_oneCounts[var] != mostGeneric)
        _counts[var] = 0;

    return _counts.getFirstMaxExponent();
  }
};

class PivotSplit : public SplitStrategyCommon {
public:
  virtual bool isPivotSplit() const {
    return true;
  }
};

class MinimumSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "minimum";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    size_t var = getBestVar(slice);
    pivot.setToIdentity();
    pivot[var] = 1;
  }
};

class MedianSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "median";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    size_t var = getBestVar(slice);

    pivot.setToIdentity();
    pivot[var] = getMedianPositiveExponentOf(slice, var);
    if (pivot[var] == slice.getLcm()[var])
      pivot[var] -= 1;
  }
};

class MaximumSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "maximum";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    size_t var = getBestVar(slice);
    pivot.setToIdentity();
    pivot[var] = slice.getLcm()[var] - 1;
  }
};

class IndependencePivotSplit : public MedianSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "indep";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    if (slice.getVarCount() == 1) {
      MedianSplit::getPivot(pivot, slice);
      return;
    }

    for (int attempts = 0; attempts < 10; ++attempts) {
      // Pick two distinct variables.
      size_t var1 = rand() % slice.getVarCount();
      size_t var2 = rand() % (slice.getVarCount() - 1);
      if (var2 >= var1)
        ++var2;

      // Make pivot as big as it can be while making var1 and var2
      // independent of each other.
      bool first = true;
      Ideal::const_iterator end = slice.getIdeal().end();
      for (Ideal::const_iterator it = slice.getIdeal().begin();
           it != end; ++it) {
        if ((*it)[var1] == 0 || (*it)[var2] == 0)
          continue;

        if (first)
          pivot = *it;
        else {
          for (size_t var = 0; var < slice.getVarCount(); ++var)
            if (pivot[var] >= (*it)[var])
              pivot[var] = (*it)[var] - 1;
        }
      }

      if (!first && !pivot.isIdentity())
        return;
    }

    MedianSplit::getPivot(pivot, slice);
  }
};

class GcdSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "gcd";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    size_t var = getBestVar(slice);

    size_t nonDivisibleCount = 0;
    Ideal::const_iterator end = slice.getIdeal().end();
    for (Ideal::const_iterator it = slice.getIdeal().begin();
         it != end; ++it)
      if ((*it)[var] >= 2)
        ++nonDivisibleCount;

    for (int i = 0; i < 3; ++i) {
      size_t selected = rand() % nonDivisibleCount;
      for (Ideal::const_iterator it = slice.getIdeal().begin(); ; ++it) {
        ASSERT(it != end);
        if ((*it)[var] < 2)
          continue;

        if (selected == 0) {
          if (i == 0)
            pivot = *it;
          else
            pivot.gcd(pivot, *it);
          break;
        }
        --selected;
      }
    }

    pivot.decrement();
  }
};

class MinGenSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "mingen";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    size_t nonSquareFreeCount = 0;
    Ideal::const_iterator end = slice.getIdeal().end();
    for (Ideal::const_iterator it = slice.getIdeal().begin();
         it != end; ++it)
      if (!Term::isSquareFree(*it, slice.getVarCount()))
        ++nonSquareFreeCount;

    size_t selected = rand() % nonSquareFreeCount;
    for (Ideal::const_iterator it = slice.getIdeal().begin(); ; ++it) {
      ASSERT(it != end);
      if (Term::isSquareFree(*it, slice.getVarCount()))
        continue;

      if (selected == 0) {
        pivot = *it;
        break;
      }
      --selected;
    }

    pivot.decrement();
  }
};

class DegreeSplit : public PivotSplit {
public:
  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "degree";
  }

  virtual void getPivot(Term& pivot, Slice& slice) const {
    reportInternalError("Called getPivot directly on FrobeniusSplit.");
  }

  virtual void getPivot(Term& pivot, Slice& slice, const TermGrader& grader) const {
    const Term& lcm = slice.getLcm();

    // TODO: pick a middle variable in case of ties.

    _maxDiff = -1;
    size_t maxOffset = 0u;
    for (size_t var = 0; var < slice.getVarCount(); ++var) {
      if (lcm[var] <= 1)
        continue;

      Exponent base = slice.getMultiply()[var];
      Exponent mid = base + lcm[var] / 2;

      // We could be looking at an added pure power whose exponent is
      // defined to have degree 0. We don't want to look at that.
      if (mid == grader.getMaxExponent(var) && mid > base)
        --mid;

      _diff = grader.getGrade(var, mid) - grader.getGrade(var, base);

      if (grader.getGradeSign(var) < 0)
        _diff = -_diff;

      ASSERT(_diff >= 0 || base == mid);

      if (_diff > _maxDiff) {
        maxOffset = var;
        _maxDiff = _diff;
      }
    }

    pivot.setToIdentity();
    pivot[maxOffset] = lcm[maxOffset] / 2;
  }

private:
  /** This is member variable used by getPivot. It has been made a
      field of the object to avoid having to reinitialize the object with
      each call.
  */
  mutable mpz_class _maxDiff;

  /** This is member variable used by getPivot. It has been made a
      field of the object to avoid having to reinitialize the object with
      each call.
  */
  mutable mpz_class _diff;
};

/** This class is deprecated and is only here to create the alias
 "frob" for the degree split.
*/
class DeprecatedFrobeniusSplit : public DegreeSplit {
public:
  DeprecatedFrobeniusSplit() {
    displayNote
      ("The split selection strategy \"frob\" is deprecated and will be "
       "removed in a future version of Frobby. Use the name \"degree\" "
       "to achieve the same thing.");
  }

  virtual const char* getName() const {
    return staticGetName();
  }

  static const char* staticGetName() {
    return "frob";
  }
};

namespace {
  typedef NameFactory<SplitStrategy> SplitFactory;

  SplitFactory getSplitFactory() {
    SplitFactory factory("Slice split strategy");

    nameFactoryRegister<MaxLabelSplit>(factory);
    nameFactoryRegister<MinLabelSplit>(factory);
    nameFactoryRegister<VarLabelSplit>(factory);
    nameFactoryRegister<MinimumSplit>(factory);
    nameFactoryRegister<MedianSplit>(factory);
    nameFactoryRegister<MaximumSplit>(factory);
    nameFactoryRegister<MinGenSplit>(factory);
    nameFactoryRegister<IndependencePivotSplit>(factory);
    nameFactoryRegister<GcdSplit>(factory);
    nameFactoryRegister<DegreeSplit>(factory);
    nameFactoryRegister<DeprecatedFrobeniusSplit>(factory);

    return factory;
  }
}

unique_ptr<SplitStrategy> SplitStrategy::createStrategy(const string& prefix) {
  unique_ptr<SplitStrategy> split = createWithPrefix(getSplitFactory(), prefix);
  ASSERT(split.get() != 0);
  return split;
}
