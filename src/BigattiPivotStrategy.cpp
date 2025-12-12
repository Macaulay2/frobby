/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 University of Aarhus
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
#include "BigattiPivotStrategy.h"

#include "BigattiState.h"
#include "Ideal.h"
#include "Term.h"
#include "NameFactory.h"
#include "error.h"

BigattiPivotStrategy::BigattiPivotStrategy() {
}

BigattiPivotStrategy::~BigattiPivotStrategy() {
}

namespace {
  class MedianPivot : public BigattiPivotStrategy {
  public:
    const Term& getPivot(BigattiState& state) {
      _counts.reset(state.getVarCount());
      state.getIdeal().getSupportCounts(_counts);
      size_t var = _counts.getFirstMaxExponent();

      _pivot.reset(state.getVarCount());
      _pivot[var] = state.getMedianPositiveExponentOf(var);
      return _pivot;
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "median";
    }

  private:
    Term _counts;
    Term _pivot;
  };

  /** Common base class for pivot strategies depending on genericity
      with code that makes that convenient. */
  class GenericPivotCommon : public BigattiPivotStrategy {
  public:
    virtual const Term& getPivot(BigattiState& state) {
      _state = &state;
      _ideal = &(state.getIdeal());
      driveMe();
      return _pivot;
    }

  protected:
    /** Sub-classes should override driveMe and call a considerX
     method to consider some specific degree var^exp, and then call a
     selectX method to select a pivot that is computed based on the
     considered degree. */
    virtual void driveMe() = 0;

    void considerTypical() {
      size_t count = _ideal->getTypicalExponent(_var, _exp);
      if (count <= 1)
        _exp = 0; // fall back to median.
    }

    void considerMostNonGeneric() {
      _ideal->getMostNonGenericExponent(_var, _exp);
    }

    void considerTypicalNonGeneric() {
      _ideal->getTypicalNonGenericExponent(_var, _exp);
    }

    void considerSomeNonGeneric() {
      _ideal->getNonGenericExponent(_var, _exp);
    }

    void selectPurePower() {
      if (_exp == 0)
        _pivot = getMedian();
      else {
        _pivot.reset(_ideal->getVarCount());
        _pivot[_var] = _exp;
      }
    }

    void selectGcd() {
      if (_exp == 0)
        _pivot = getMedian();
      else {
        _pivot.reset(_ideal->getVarCount());
        _ideal->getGcdAtExponent(_pivot, _var, _exp);
        ASSERT(!_pivot.isIdentity());
      }
    }

    void selectTight() {
      if (_exp == 0) {
        _pivot = getMedian();
        return;
      }

      _ideal->singleDegreeSort(_var);

      Ideal::const_iterator blockBegin = _ideal->begin();
      Ideal::const_iterator stop = _ideal->end();
      ASSERT(blockBegin != stop);

      // Find the start of the relevant block.
      while ((*blockBegin)[_var] != _exp) {
        ++blockBegin;
        ASSERT(blockBegin != stop);
      }
      ASSERT((*blockBegin)[_var] == _exp);

      Ideal::const_iterator blockEnd = blockBegin;
      do {
        ++blockEnd;
      } while (blockEnd != stop && (*blockEnd)[_var] == _exp);

      // At this point the range [blockBegin, blockEnd) contains every
      // generator that raises _var to _exp.

      bool first = true;
      _pivot.reset(_ideal->getVarCount());

      Term lcm(_ideal->getVarCount()); // For a temporary value
      for (; blockBegin != blockEnd; ++blockBegin) {
        Ideal::const_iterator it = blockBegin;
        for (++it; it != blockEnd; ++it) {
          lcm.lcm(*blockBegin, *it);
          if (!_ideal->strictlyContains(lcm)) {
            // We only need to have the pivot remove one of
            // *blockBegin and *it. We choose to let it be *blockBegin
            // that is included in the gcd (and thus removed for sure)
            // just because that is easier to keep track of.
            if (first) {
              first = false;
              _pivot.gcd(*blockBegin, *it);
            } else {
              _pivot.gcd(_pivot, *blockBegin);
              _pivot.gcd(_pivot, *it);
            }
            break;
          }
        }
      }

      if (first)
        _pivot[_var] = _exp;

      ASSERT(!_pivot.isIdentity());
    }

  private:
    Term _pivot;
    BigattiState* _state;
    Ideal* _ideal;
    size_t _var;
    Exponent _exp;

    /** The generic pivot strategies are designed to do away with
        non-genericity. If we have turned detecting of generic ideals as
        a base case off, they can then be faced with completely generic
        ideals for which they have nothing sensible to do. Thus it is
        necessary to have a fall-back for those cases, which is what
        getMedian offers.
    */
    const Term& getMedian() {
      return _median.getPivot(*_state);
    }

    /** Used by getMedian(). */
    MedianPivot _median;
  };

  class MostNGPurePivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerMostNonGeneric();
      selectPurePower();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "mostNGPure";
    }
  };

  class MostNGGcdPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerMostNonGeneric();
      selectGcd();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "mostNGGcd";
    }
  };

  class MostNGTightPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerMostNonGeneric();
      selectTight();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "mostNGTight";
    }
  };

  class TypicalNGPurePivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypicalNonGeneric();
      selectPurePower();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalNGPure";
    }
  };

  class TypicalNGGcdPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypicalNonGeneric();
      selectGcd();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalNGGcd";
    }
  };

  class TypicalNGTightPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypicalNonGeneric();
      selectTight();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalNGTight";
    }
  };

  class TypicalPurePivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypical();
      selectPurePower();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalPure";
    }
  };

  class TypicalGcdPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypical();
      selectGcd();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalGcd";
    }
  };

  class TypicalTightPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerTypical();
      selectTight();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "typicalTight";
    }
  };

  class SomeNGPurePivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerSomeNonGeneric();
      selectPurePower();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "someNGPure";
    }
  };

  class SomeNGGcdPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerSomeNonGeneric();
      selectGcd();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "someNGGcd";
    }
  };

  class SomeNGTightPivot : public GenericPivotCommon {
  public:
    virtual void driveMe() {
      considerSomeNonGeneric();
      selectTight();
    }

    virtual const char* getName() const {
      return staticGetName();
    }

    static const char* staticGetName() {
      return "someNGTight";
    }
  };

  /** Widens the pivots selected by another pivot selection
      strategy. */
  class WidenPivot : public BigattiPivotStrategy {
  public:
    WidenPivot(unique_ptr<BigattiPivotStrategy> strategy):
      _strategy(std::move(strategy)) {
      _name = _strategy->getName();
      _name += " (wide)";
    }

    const Term& getPivot(BigattiState& state) {
      const Term& pivot = _strategy->getPivot(state);
      _widePivot.reset(state.getVarCount());
      state.getIdeal().getGcdOfMultiplesOf(_widePivot, pivot);
      return _widePivot;
    }

    virtual const char* getName() const {
      return _name.c_str();
    }

  private:
    unique_ptr<BigattiPivotStrategy> _strategy;
    string _name;
    Term _widePivot;
  };

  typedef NameFactory<BigattiPivotStrategy> StrategyFactory;

  StrategyFactory makeStrategyFactory() {
    StrategyFactory factory("Bigatti et.al. pivot strategy");

    nameFactoryRegister<MedianPivot>(factory);

    nameFactoryRegister<TypicalPurePivot>(factory);
    nameFactoryRegister<TypicalNGPurePivot>(factory);
    nameFactoryRegister<MostNGPurePivot>(factory);
    nameFactoryRegister<SomeNGPurePivot>(factory);

    nameFactoryRegister<TypicalGcdPivot>(factory);
    nameFactoryRegister<TypicalNGGcdPivot>(factory);
    nameFactoryRegister<MostNGGcdPivot>(factory);
    nameFactoryRegister<SomeNGGcdPivot>(factory);

    nameFactoryRegister<TypicalTightPivot>(factory);
    nameFactoryRegister<TypicalNGTightPivot>(factory);
    nameFactoryRegister<MostNGTightPivot>(factory);
    nameFactoryRegister<SomeNGTightPivot>(factory);

    return factory;
  }
}

unique_ptr<BigattiPivotStrategy> BigattiPivotStrategy::
createStrategy(const string& prefix, bool widen) {
  unique_ptr<BigattiPivotStrategy> strategy =
    createWithPrefix(makeStrategyFactory(), prefix);
  ASSERT(strategy.get() != 0);

  if (widen)
    strategy = unique_ptr<BigattiPivotStrategy>(new WidenPivot(std::move(strategy)));

  return strategy;
}
