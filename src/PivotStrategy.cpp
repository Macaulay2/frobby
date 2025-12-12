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
#include "PivotStrategy.h"

#include "EulerState.h"
#include "NameFactory.h"
#include "RawSquareFreeTerm.h"
#include "ElementDeleter.h"
#include "PivotEulerAlg.h"

#include <sstream>
#include <limits>

namespace Ops = SquareFreeTermOps;

namespace {
  inline size_t getPopVar(const size_t* divCounts, const size_t varCount) {
	return max_element(divCounts, divCounts + varCount) - divCounts;
  }

  inline size_t getRareVar(const size_t* divCounts, const size_t varCount) {
	const size_t* end = divCounts + varCount;
	const size_t* rare = divCounts;
    while (*rare == 0) {
      ++rare;
      ASSERT(rare != end); // not all zero
    }

	const size_t* it = rare + 1;
	for (; it != end; ++it)
	  if (*it > 0 && *it < *rare)
		rare = it;

	return rare - divCounts;
  }

  inline void makeRareVarsMask
    (Word* mask, const size_t* divCounts, const size_t varCount) {
    const size_t rareVar = getRareVar(divCounts, varCount);
    ASSERT(rareVar < varCount); // not all zero
    const size_t maxCount = divCounts[rareVar];
    Ops::setToIdentity(mask, varCount);
    for (size_t var = 0; var < varCount; ++var)
      if (divCounts[var] == maxCount)
        Ops::setExponent(mask, var, true);
  }

  class RawSquareFreeTerm {
  public:
	RawSquareFreeTerm(): _term(0), _capacity(0) {}
	~RawSquareFreeTerm() {delete _term;}

	operator Word*() {return _term;}
	operator const Word*() const {return _term;}

	void reserve(const size_t varCount) {
	  if (varCount > _capacity) {
		Ops::deleteTerm(_term);
		_term = Ops::newTerm(varCount);
		_capacity = varCount;
	  }
	}

  private:
	Word* _term;
	size_t _capacity;
  };

  class WithPivotTerm : public PivotStrategy {
  protected:
	Word* termWithCapacity(const size_t varCount) {
	  _term.reserve(varCount);
	  return _term;
	}

  private:
	RawSquareFreeTerm _term;
  };

  class StdStrategy : public WithPivotTerm {
  public:
	virtual Word* getPivot(const EulerState& state,
						   const size_t* divCounts) = 0;
	virtual void computationCompleted(const PivotEulerAlg& alg) {}
    virtual bool shouldTranspose(const EulerState& state) const {
      return state.getVarCount() > state.getIdeal().getGeneratorCount();
    }
  };

  class StdPopVar : public StdStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  return state.inPlaceStdSplit(getPopVar(divCounts, varCount));
	}

	virtual Word* getPivot(const EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  Word* pivot = termWithCapacity(varCount);
	  Ops::setToIdentity(pivot, varCount);
	  Ops::setExponent(pivot, getPopVar(divCounts, varCount), 1);
	  return pivot;
	}

	static const char* staticGetName() {
	  return "popvar";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class StdRareVar : public StdStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  return state.inPlaceStdSplit(getRareVar(divCounts, varCount));
	}

	virtual Word* getPivot(const EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  Word* pivot = termWithCapacity(varCount);
	  Ops::setToIdentity(pivot, varCount);
	  Ops::setExponent(pivot, getRareVar(divCounts, varCount), 1);
	  return pivot;
	}

	static const char* staticGetName() {
	  return "rarevar";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class StdPopGcd : public StdStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  return state.inPlaceStdSplit(getPivot(state, divCounts));
	}

	virtual Word* getPivot(const EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  const size_t popVar = getPopVar(divCounts, varCount);
	  Word* pivot = termWithCapacity(varCount);

	  if (divCounts[popVar] == 1) {
		Ops::setToIdentity(pivot, varCount);
		Ops::setExponent(pivot, getPopVar(divCounts, varCount), 1);
		return pivot;
	  }

	  size_t seen = 0;
	  RawSquareFreeIdeal::const_iterator it = state.getIdeal().begin();
	  RawSquareFreeIdeal::const_iterator end = state.getIdeal().end();
	  for (; it != end; ++it) {
		if (Ops::getExponent(*it, popVar) != 0) {
		  if (seen == 0)
			Ops::assign(pivot, *it, varCount);
		  else
			Ops::gcdInPlace(pivot, *it, varCount);
		  ++seen;
		  if (seen == 3)
			break;
		}
	  }
	  ASSERT(seen > 1);
	  return pivot;
	}

	static const char* staticGetName() {
	  return "popgcd";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class StdRandom  : public StdStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t random = getRandomNotEliminatedVar(state);
	  return state.inPlaceStdSplit(random);
	}

	virtual Word* getPivot(const EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  const size_t random = getRandomNotEliminatedVar(state);
	  Word* pivot = termWithCapacity(varCount);
	  Ops::setToIdentity(pivot, varCount);
	  Ops::setExponent(pivot, random, 1);
	  return pivot;
	}

	static const char* staticGetName() {
	  return "random";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}

  private:
	size_t getRandomNotEliminatedVar(const EulerState& state) {
	  while (true) {
		size_t random = rand() % state.getVarCount();
		if (Ops::getExponent(state.getEliminatedVars(), random) == 0)
		  return random;
	  }
	}
  };

  class StdAny  : public StdStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t any = getAnyNotEliminatedVar(state);
	  return state.inPlaceStdSplit(any);
	}

	virtual Word* getPivot(const EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  const size_t any = getAnyNotEliminatedVar(state);
	  Word* pivot = termWithCapacity(varCount);
	  Ops::setToIdentity(pivot, varCount);
	  Ops::setExponent(pivot, any, 1);
	  return pivot;
	}

	static const char* staticGetName() {
	  return "any";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}

  private:
	size_t getAnyNotEliminatedVar(const EulerState& state) {
	  for (size_t var = 0; ; ++var) {
		ASSERT(var < state.getVarCount());
		if (Ops::getExponent(state.getEliminatedVars(), var) == 0)
		  return var;
	  }
	}
  };

  class StdWiden  : public WithPivotTerm {
  public:
	StdWiden(unique_ptr<StdStrategy> strat):
	  _strat(std::move(strat)) {
	  ASSERT(_strat.get() != 0);
	}

	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  Word* narrow = _strat->getPivot(state, divCounts);
	  Word* wide = termWithCapacity(varCount);
	  state.getIdeal().getGcdOfMultiples(wide, narrow);
	  return state.inPlaceStdSplit(wide);
	}

	virtual void getName(ostream& out) const {
	  out << "widen_";
	  _strat->getName(out);
	}

	virtual void computationCompleted(const PivotEulerAlg& alg) {
	  _strat->computationCompleted(alg);
	}

    virtual bool shouldTranspose(const EulerState& state) const {
      return _strat->shouldTranspose(state);
    }

  private:
	unique_ptr<StdStrategy> _strat;
  };

  class GenStrategy : public WithPivotTerm {
  public:
	typedef RawSquareFreeIdeal::iterator iterator;

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) = 0;
	virtual void computationCompleted(const PivotEulerAlg& alg) {}
    virtual bool shouldTranspose(const EulerState& state) const {
      return state.getVarCount() < state.getIdeal().getGeneratorCount();
    }
  };

  class GenPopVar : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  size_t pivotIndex =
		state.getIdeal().getMultiple(getPopVar(divCounts, varCount));
	  return state.inPlaceGenSplit(pivotIndex);
	}

	virtual iterator filter(iterator begin, iterator end,
						   const size_t* divCounts,
						   const size_t varCount) {
	  size_t popVar = getPopVar(divCounts, varCount);
	  Word* term = termWithCapacity(varCount);
	  Ops::setToIdentity(term, varCount);
	  for (size_t var = 0; var < varCount; ++var)
		if (divCounts[var] == divCounts[popVar])
		  Ops::setExponent(term, var, 1);

	  iterator newEnd = begin;
	  for (iterator it = begin; it != end; ++it) {
		if (Ops::isRelativelyPrime(term, *it, varCount))
		  continue;
		Ops::swap(*it, *newEnd, varCount);
		++newEnd;
	  }
	  return newEnd;
	}

	static const char* staticGetName() {
	  return "popvar";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenRareMax : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
      typedef RawSquareFreeIdeal::const_iterator const_iterator;
      const RawSquareFreeIdeal& ideal = state.getIdeal();
	  const size_t varCount = state.getVarCount();
      const size_t rareVar = getRareVar(divCounts, varCount);

      const const_iterator end = ideal.end();
      size_t bestSupport = 0;
      const_iterator best = end;
      for (const_iterator it = ideal.begin(); it != end; ++it) {
        if (!Ops::getExponent(*it, rareVar))
          continue;
        const size_t support = Ops::getSizeOfSupport(*it, varCount);
        if (bestSupport < support) {
          bestSupport = support;
          best = it;
        }
      }
      ASSERT(best != end);
      return state.inPlaceGenSplit(best - ideal.begin());
  	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
      const size_t rareVar = getRareVar(divCounts, varCount);

      size_t bestSupport = 0;
      iterator newEnd = begin;
	  for (iterator it = begin; it != end; ++it) {
        if (!Ops::getExponent(*it, rareVar))
          continue;
        const size_t support = Ops::getSizeOfSupport(*it, varCount);
        if (bestSupport > support)
          continue;
        if (bestSupport < support) {
          newEnd = begin;
          bestSupport = support;
        }
		Ops::swap(*it, *newEnd, varCount);
		++newEnd;
      }
	  return newEnd;
	}

	static const char* staticGetName() {
	  return "raremax";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenRareVar : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  size_t pivotIndex =
		state.getIdeal().getMultiple(getRareVar(divCounts, varCount));
	  return state.inPlaceGenSplit(pivotIndex);
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  size_t rareVar = getRareVar(divCounts, varCount);

	  iterator newEnd = begin;
	  for (iterator it = begin; it != end; ++it) {
		if (Ops::getExponent(*it, rareVar) == 0)
		  continue;
		Ops::swap(*it, *newEnd, varCount);
		++newEnd;
	  }
	  return newEnd;
	}

	static const char* staticGetName() {
	  return "rarevar";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenComposite : public PivotStrategy {
  public:
	GenComposite():
	  _filters(0),
	  _filtersDeleter(_filters) {
	}

	typedef RawSquareFreeIdeal::iterator iterator;

	void addStrategy(unique_ptr<GenStrategy> strat) {
	  exceptionSafePushBack(_filters, std::move(strat));
	}

	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  const iterator begin = state.getIdeal().begin();
	  iterator end = state.getIdeal().end();
	  ASSERT(end - begin > 0);

	  for (size_t i = 0; i < _filters.size(); ++i)
		end = _filters[i]->filter(begin, end, divCounts, varCount);
	  ASSERT(end - begin > 0);

	  return state.inPlaceGenSplit(0);
	}

	virtual void getName(ostream& out) const {
	  for (size_t i = 0; i < _filters.size(); ++i) {
		if (i > 0)
		  out << '_';
		_filters[i]->getName(out);
	  }
	}

	virtual void computationCompleted(const PivotEulerAlg& alg) {
	  for (size_t i = 0; i < _filters.size(); ++i)
		_filters[i]->computationCompleted(alg);
	}

    virtual bool shouldTranspose(const EulerState& state) const {
      return _filters.front()->shouldTranspose(state);
    }

  private:
	vector<GenStrategy*> _filters;
	ElementDeleter<vector<GenStrategy*> > _filtersDeleter;
  };

  class GenRarestVars : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  const size_t varCount = state.getVarCount();
	  filter(state.getIdeal().begin(), state.getIdeal().end(),
			 divCounts, varCount);
	  return state.inPlaceGenSplit(0);
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  size_t lastDivCount = 0;
	  while (end - begin > 1) {
		size_t minDivCount = numeric_limits<size_t>::max();
		for (size_t var = 0; var < varCount; ++var)
		  if (divCounts[var] > lastDivCount && minDivCount > divCounts[var])
			minDivCount = divCounts[var];
		if (minDivCount == numeric_limits<size_t>::max())
		  break;

		end = filter(begin, end, divCounts, varCount, minDivCount);
		lastDivCount = minDivCount;
	  }
	  return end;
	}

	static const char* staticGetName() {
	  return "rarest";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}

  private:
	iterator filter(iterator begin, iterator end,
					const size_t* divCounts,
					const size_t varCount,
					size_t divCount) {
	  // Set the support of term to be the vars of the specified rarity
	  Word* term = termWithCapacity(varCount);
	  Ops::setToIdentity(term, varCount);
	  for (size_t var = 0; var < varCount; ++var)
		if (divCounts[var] == divCount)
		  Ops::setExponent(term, var, 1);

	  // Select the generators that are divisible by the most vars with
	  // the specified rarity.
	  iterator newEnd = begin;
	  size_t maxRareCount = 0;
	  _tmp.reserve(varCount);
	  Word* tmp = _tmp;
	  for (iterator it = begin; it != end; ++it) {
		if (Ops::isRelativelyPrime(term, *it, varCount))
		  continue; // no rare vars in *it

		Ops::gcd(tmp, term, *it, varCount);
		const size_t rareCount = Ops::getSizeOfSupport(tmp, varCount);

		if (maxRareCount > rareCount)
		  continue;
		if (maxRareCount < rareCount) {
		  maxRareCount = rareCount;
		  newEnd = begin;
		}
		Ops::swap(*newEnd, *it, varCount);
		++newEnd;
	  }
	  if (newEnd != begin)
		return newEnd;
	  else
		return end; // no rare vars in any generator, so we can't discard any
	}

	size_t getRarest(const RawSquareFreeIdeal& ideal, const size_t* divCounts) {
	  const size_t varCount = ideal.getVarCount();
	  RawSquareFreeIdeal::const_iterator it = ideal.begin();
	  const RawSquareFreeIdeal::const_iterator stop = ideal.end();
	  RawSquareFreeIdeal::const_iterator rarest = it;

	  for (; it != stop; ++it)
		if (rarer(*it, *rarest, divCounts, varCount))
		  rarest = it;
	  return rarest - ideal.begin();
	}

	/** return (a,b) where a is the minimum divCount > above in the
		support of term and b is the number of times that that divCount
		appears in the support. */
	pair<size_t, size_t> getRarity(const Word* const term,
								   const size_t* divCounts,
								   const size_t varCount,
								   size_t above) {
	  size_t rarity = varCount;
	  size_t multiplicity = 0;
	  for (size_t var = 0; var < varCount; ++var) {
		const size_t co = divCounts[var];
		if (Ops::getExponent(term, var) != 0 && co <= rarity && co > above) {
		  ASSERT(divCounts[var] != 0);
		  if (co == rarity)
			++multiplicity;
		  else {
			rarity = divCounts[var];
			multiplicity = 1;
		  }
		}
	  }
	  return make_pair(rarity, multiplicity);
	}

	bool rarer(const Word* const a, const Word* const b,
			   const size_t* divCounts,
			   const size_t varCount) {
	  size_t lookAbove = 0;
	  while (true) {
		pair<size_t, size_t> rarityA =
		  getRarity(a, divCounts, varCount, lookAbove);
		pair<size_t, size_t> rarityB =
		  getRarity(b, divCounts, varCount, lookAbove);

		if (rarityA.first < rarityB.first)
		  return true;
		if (rarityA.first > rarityB.first)
		  return false;

		if (rarityA.second > rarityB.second)
		  return true;
		if (rarityA.second < rarityB.second)
		  return false;

		if (rarityA.second == 0)
		  return false; // a and b are equally rare

		lookAbove = rarityA.first;
	  }
	}

	RawSquareFreeTerm _tmp;
  };

  class GenMaxSupport : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  return state.inPlaceGenSplit(state.getIdeal().getMaxSupportGen());
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  size_t maxSupp = 0;
	  iterator newEnd = begin;
	  for (iterator it = begin; it != end; ++it) {
		const size_t supp = Ops::getSizeOfSupport(*it, varCount);
		if (maxSupp > supp)
		  continue;
		if (maxSupp < supp) {
		  maxSupp = supp;
		  newEnd = begin;
		}
		Ops::swap(*newEnd, *it, varCount);
		++newEnd;
	  }
	  return newEnd;
	}

	static const char* staticGetName() {
	  return "maxsupp";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenMinSupport : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  return state.inPlaceGenSplit(state.getIdeal().getMinSupportGen());
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  size_t minSupp = varCount;
	  iterator newEnd = begin;
	  for (iterator it = begin; it != end; ++it) {
		const size_t supp = Ops::getSizeOfSupport(*it, varCount);
		if (minSupp < supp)
		  continue;
		if (minSupp > supp) {
		  minSupp = supp;
		  newEnd = begin;
		}
		Ops::swap(*newEnd, *it, varCount);
		++newEnd;
	  }
	  return newEnd;
	}

	static const char* staticGetName() {
	  return "minsupp";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenAny  : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  return state.inPlaceGenSplit(0);
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  return ++begin;
	}

	static const char* staticGetName() {
	  return "any";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class GenRandom  : public GenStrategy {
  public:
	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  size_t pivotIndex = rand() % state.getIdeal().getGeneratorCount();
	  return state.inPlaceGenSplit(pivotIndex);
	}

	virtual iterator filter(iterator begin, iterator end,
							const size_t* divCounts,
							const size_t varCount) {
	  const size_t genCount = end - begin;
	  const size_t choice = rand() % genCount;
	  Ops::swap(*begin, *(begin + choice), varCount);
	  return ++begin;
	}

	static const char* staticGetName() {
	  return "random";
	}

	virtual void getName(ostream& out) const {
	  out << staticGetName();
	}
  };

  class HybridPivotStrategy : public PivotStrategy {
  public:
	HybridPivotStrategy(unique_ptr<PivotStrategy> stdStrat,
						unique_ptr<PivotStrategy> genStrat):
	  _stdStrat(std::move(stdStrat)), _genStrat(std::move(genStrat)) {}

	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  if (state.getNonEliminatedVarCount() <
		  state.getIdeal().getGeneratorCount())
		return _stdStrat->doPivot(state, divCounts);
	  else
		return _genStrat->doPivot(state, divCounts);
	}

	virtual void getName(ostream& out) const {
	  out << "hybrid (";
	  _stdStrat->getName(out);
	  out << ", ";
	  _genStrat->getName(out);
	  out << ')';
	}

	virtual void computationCompleted(const PivotEulerAlg& alg) {
	  _stdStrat->computationCompleted(alg);
	  _genStrat->computationCompleted(alg);
	}

    virtual bool shouldTranspose(const EulerState& state) const {
      return false;
    }

  private:
	unique_ptr<PivotStrategy> _stdStrat;
	unique_ptr<PivotStrategy> _genStrat;
  };

  class DebugStrategy : public PivotStrategy {
  public:
	DebugStrategy(unique_ptr<PivotStrategy> strat, FILE* out):
	  _strat(std::move(strat)), _out(out) {}

	virtual EulerState* doPivot(EulerState& state,
						 const size_t* divCounts) {
	  const char* str1 = "\n\n\n"
		"********************************(debug output)********************************\n"
		"********** Processing this simplified state that is not a base case **********\n";
	  fputs(str1, _out);
	  state.print(_out);

	  EulerState* subState = _strat->doPivot(state, divCounts);
	  ASSERT(subState != 0);

	  const char* str2 =
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<< Substate 1 of 2 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	  fputs(str2, _out);
	  state.print(_out);

	  const char* str3 =
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<< Substate 2 of 2 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n";
	  fputs(str3, _out);
	  subState->print(_out);

	  return subState;
	}

	virtual void getName(ostream& out) const {
	  _strat->getName(out);
	}

	virtual void computationCompleted(const PivotEulerAlg& alg) {
	  _strat->computationCompleted(alg);
	  fputs("Debug: Euler characteristic computation completed.\n", _out);
	  gmp_fprintf(_out, "Debug: Computed Euler characteristics was %Zd.\n",
				  alg.getComputedEulerCharacteristic().get_mpz_t());
	}

    virtual bool shouldTranspose(const EulerState& state) const {
      return _strat->shouldTranspose(state);
    }

  private:
	unique_ptr<PivotStrategy> _strat;
	FILE* _out;
  };

  class StatisticsStrategy : public PivotStrategy {
  public:
	StatisticsStrategy(unique_ptr<PivotStrategy> strat, FILE* out):
	  _strat(std::move(strat)), _out(out), _statesSplit(0), _transposes(0) {}

	virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) {
	  ++_statesSplit;
	  return _strat->doPivot(state, divCounts);
	}

	virtual void getName(ostream& out) const {
	  _strat->getName(out);
	}

	virtual void computationCompleted(const PivotEulerAlg& alg) {
	  _strat->computationCompleted(alg);
	  fputs("******** Statistics for Euler characteristic computation *****\n", _out);
	  fprintf(_out, "* Using unique div simplify: %s\n",
			  alg.getUseUniqueDivSimplify() ? "yes" : "no");
	  fprintf(_out, "* Using many div simplify: %s\n",
			  alg.getUseManyDivSimplify() ? "yes" : "no");
	  fprintf(_out, "* Using implied div simplify: %s\n",
			  alg.getUseAllPairsSimplify() ? "yes" : "no");
	  fprintf(_out, "* Do initial autotranspose: %s\n",
			  alg.getInitialAutoTranspose() ? "yes" : "no");
	  fprintf(_out, "* Do autotranspose at each step: %s\n",
			  alg.getAutoTranspose() ? "yes" : "no");
	  ostringstream strategyName;
	  getName(strategyName);
	  fprintf(_out, "* Pivot strategy: %s\n", strategyName.str().c_str());

	  // The computation is a binary tree and we only see the internal
	  // nodes that each generate two more nodes. The number of leaves
	  // in a binary tree is one plus the number of internal nodes.
	  unsigned long totalStates = 2 * _statesSplit + 1;
	  fprintf(_out, "* States processed: %lu\n", (unsigned long)totalStates);
	  fprintf(_out, "* Transposes taken: %lu\n", (unsigned long)_transposes);
	  fputs("********\n", _out);
	}

    virtual bool shouldTranspose(const EulerState& state) const {
      const bool should = _strat->shouldTranspose(state);
      if (should)
        ++_transposes;
      return should;
    }

  private:
	unique_ptr<PivotStrategy> _strat;
	FILE* _out;
	unsigned long _statesSplit;
    mutable unsigned long _transposes;
  };

  typedef NameFactory<StdStrategy> StdFactory;
  StdFactory getStdStratFactory() {
    StdFactory factory("standard pivot strategy");
    nameFactoryRegister<StdRandom>(factory);
    nameFactoryRegister<StdAny>(factory);
    nameFactoryRegister<StdPopVar>(factory);
    nameFactoryRegister<StdPopGcd>(factory);
    nameFactoryRegister<StdRareVar>(factory);
    return factory;
  }

  typedef NameFactory<GenStrategy> GenFactory;
  GenFactory getGenStratFactory() {
    GenFactory factory("generator pivot strategy");
    nameFactoryRegister<GenRareMax>(factory);
    nameFactoryRegister<GenRareVar>(factory);
    nameFactoryRegister<GenRarestVars>(factory);
    nameFactoryRegister<GenPopVar>(factory);
    nameFactoryRegister<GenMaxSupport>(factory);
    nameFactoryRegister<GenMinSupport>(factory);
    nameFactoryRegister<GenAny>(factory);
    nameFactoryRegister<GenRandom>(factory);
    return factory;
  }
}

unique_ptr<PivotStrategy> newStdPivotStrategy(const string& name) {
  if (name.compare(0, 6, "widen_") != 0) {
	unique_ptr<StdStrategy> strat = getStdStratFactory().create(name);
	return unique_ptr<PivotStrategy>(strat.release());
  }

  unique_ptr<StdStrategy> subStrat =
	getStdStratFactory().create(name.substr(6, name.size() - 6));
  return unique_ptr<PivotStrategy>(new StdWiden(std::move(subStrat)));
}

unique_ptr<PivotStrategy> newGenPivotStrategy(const string& name) {
  GenFactory factory = getGenStratFactory();
  if (name.find('_') == string::npos) {
	unique_ptr<GenStrategy> strat = factory.create(name);
	return unique_ptr<PivotStrategy>(strat.release());
  }

  unique_ptr<GenComposite> composite(new GenComposite());

  size_t pos = 0;
  string part;
  do {
	size_t nextPos = name.find('_', pos);
	if (nextPos == string::npos) {
	  part = name.substr(pos, string::npos);
	  pos = string::npos;
	} else {
	  part = name.substr(pos, nextPos - pos);
	  pos = nextPos + 1;
	}

	unique_ptr<GenStrategy> strat = factory.create(part);
	composite->addStrategy(std::move(strat));
  } while (pos != string::npos);
  return unique_ptr<PivotStrategy>(composite.release());
}

unique_ptr<PivotStrategy> newHybridPivotStrategy
(unique_ptr<PivotStrategy> stdStrat, unique_ptr<PivotStrategy> genStrat) {
  PivotStrategy* strat = new HybridPivotStrategy(std::move(stdStrat), std::move(genStrat));
  return unique_ptr<PivotStrategy>(strat);
}

unique_ptr<PivotStrategy> newDebugPivotStrategy(unique_ptr<PivotStrategy> strat,
											  FILE* out) {
  return unique_ptr<PivotStrategy>(new DebugStrategy(std::move(strat), out));
}

unique_ptr<PivotStrategy> newStatisticsPivotStrategy
(unique_ptr<PivotStrategy> strat, FILE* out) {
  return unique_ptr<PivotStrategy>(new StatisticsStrategy(std::move(strat), out));
}

unique_ptr<PivotStrategy> newDefaultPivotStrategy() {
  return newStdPivotStrategy("pivot");
}
