/* Frobby: Software for monomial ideal computations.
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
#include "RawSquareFreeIdeal.h"

#include "Arena.h"
#include "Ideal.h"
#include "RawSquareFreeTerm.h"
#include "BigIdeal.h"
#include <limits>
#include <algorithm>
#include <sstream>
#include <cstring>

typedef RawSquareFreeIdeal RSFIdeal;
namespace Ops = SquareFreeTermOps;

namespace {
  /** As the STL function std::partition except specialized to work
	  with the iterators we get from SquareFreeIdeal. std::partition
	  cannot be used with these.

	  Rearranges the elements in the range [begin, end) and returns an
	  iterator middle. Middle has the property that pred(x) is true for x
	  in [begin,middle) and false for x in [middle, end). */
  template<class Iter, class Pred>
  inline Iter RsfPartition(Iter begin, Iter end, Pred pred, size_t varCount) {
	// The invariant of the loop is that pred(x) is true if x precedes
	// begin and pred(x) is false if x is at or after end.
	while (true) {
	  if (begin == end)
		return begin;
	  while (pred(*begin))
		if (++begin == end)
		  return begin;
	  // now pred(*begin) is true and begin < end.
	  if (begin == --end)
		return begin;
	  while (!pred(*end))
		if (begin == --end)
		  return begin;
	  // now pred(*end) is false and begin < end.

	  // This swap is the reason that std::partition doesn't work
	  Ops::swap(*begin, *end, varCount);
	  ++begin;
	}
  }

  /** Removes those elements of [begin, end) that are not minimal. The
   result is in the sub-range [begin, newEnd) where newEnd is the
   returned value. */
   RSFIdeal::iterator minimize(RSFIdeal::iterator begin,
							  RSFIdeal::iterator end,
							  const size_t wordCount) {
	for (RSFIdeal::iterator it = begin; it != end;) {
	  for (RSFIdeal::const_iterator div = begin; div != end; ++div) {
		if (Ops::divides(*div, *div + wordCount, *it) && div != it) {
		  --end;
		  Ops::assign(*it, *it + wordCount, *end);
		  goto next;
		}
	  }
	  ++it;
	next:;
	}
	return end;
  }
}

RSFIdeal* RSFIdeal::construct(void* buffer, size_t varCount) {
  RSFIdeal* p = static_cast<RSFIdeal*>(buffer);
  p->_varCount = varCount;
  p->_wordsPerTerm = Ops::getWordCount(varCount);
  p->_genCount = 0;
  p->_memoryEnd = p->_memory;
  ASSERT(p->isValid());
  return p;
}

RSFIdeal* RSFIdeal::construct(void* buffer, const Ideal& ideal) {
  RSFIdeal* p = construct(buffer, ideal.getVarCount());
  p->insert(ideal);
  ASSERT(p->isValid());
  return p;
}

RSFIdeal* RSFIdeal::construct(void* buffer, const RawSquareFreeIdeal& ideal) {
  RSFIdeal* p = construct(buffer, ideal.getVarCount());
  p->insert(ideal);
  ASSERT(p->isValid());
  return p;
}

size_t RSFIdeal::getBytesOfMemoryFor(size_t varCount, size_t generatorCount) {
  // This calculation is tricky because there are many overflows that
  // can occur. If most cases if an overflow occurs or nearly occurs
  // then the amount of memory needed could not be allocated on the
  // system. In this case 0 is returned to signal the error. Note that
  // x / y rounded up is (x - 1) / y + 1 for x, y > 0. The
  // multiplication a * b does not overflow if a <= MAX /
  // b. Otherwise, there may not be an overflow, but a * b will still
  // be too much to reasonably allocate.

  size_t bytesForStruct = sizeof(RSFIdeal) - sizeof(Word);
  if (generatorCount == 0)
	return bytesForStruct;

  // Compute bytes per generator taking into account memory alignment.
  size_t bytesPerGenUnaligned = varCount == 0 ? 1 : (varCount - 1) / 8 + 1;
  size_t wordsPerGen = (bytesPerGenUnaligned - 1) / sizeof(Word) + 1;
  if (wordsPerGen > numeric_limits<size_t>::max() / sizeof(Word))
	return 0;
  size_t bytesPerGen = wordsPerGen * sizeof(Word);

  // Compute bytes in all.
  if (bytesPerGen > numeric_limits<size_t>::max() / generatorCount)
	return 0;
  size_t bytesForGens = bytesPerGen * generatorCount;
  if (bytesForGens > numeric_limits<size_t>::max() - bytesForStruct)
	return 0;
  return bytesForStruct + bytesForGens;
}

void RSFIdeal::setToTransposeOf(const RawSquareFreeIdeal& ideal, Word* eraseVars) {
  if (this == &ideal) {
    transpose(eraseVars);
    return;
  }

  const size_t idealVarCount = ideal.getVarCount();
  const size_t idealGenCount = ideal.getGeneratorCount();

  _varCount = idealGenCount;
  _wordsPerTerm = Ops::getWordCount(_varCount);
  _genCount = 0;
  _memoryEnd = _memory;

  for (size_t var = 0; var < idealVarCount; ++var) {
    if (eraseVars != 0 && Ops::getExponent(eraseVars, var))
      continue;
    insertIdentity();
    Word* newTransposedGen = back();
    for (size_t gen = 0; gen < idealGenCount; ++gen) {
      const bool value = Ops::getExponent(ideal.getGenerator(gen), var);
      Ops::setExponent(newTransposedGen, gen, value);
    }
  }

  ASSERT(isValid());
}

void RSFIdeal::transpose(Word* eraseVars) {
  const size_t varCount = getVarCount();
  const size_t genCount = getGeneratorCount();
  const size_t bytes = RSFIdeal::getBytesOfMemoryFor(varCount, genCount);
  Arena& arena = Arena::getArena();
  void* buffer = arena.alloc(bytes);
  RSFIdeal* copy = RSFIdeal::construct(buffer, *this);
  setToTransposeOf(*copy, eraseVars);
  arena.freeTop(buffer);
}

void RSFIdeal::print(FILE* file) const {
  ostringstream out;
  print(out);
  fputs(out.str().c_str(), file);
}

void RSFIdeal::print(ostream& out) const {
  const size_t varCount = getVarCount();
  out << "//------------ Ideal (Square Free):\n";
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen) {
	for (size_t var = 0; var < varCount; ++var)
	  out << Ops::getExponent(getGenerator(gen), var);
    out << '\n';
  }
  out << "------------\\\\\n";
}

size_t RSFIdeal::insert(const Ideal& ideal) {
  ASSERT(getVarCount() == ideal.getVarCount());

  size_t gen = 0;
  for (; gen < ideal.getGeneratorCount(); ++gen) {
	if (!Ops::encodeTerm(_memoryEnd, ideal[gen], getVarCount()))
	  break;
	++_genCount;
	_memoryEnd += getWordsPerTerm();
  }
  ASSERT(isValid());
  return gen;
}

size_t RSFIdeal::insert(const BigIdeal& ideal) {
  ASSERT(getVarCount() == ideal.getVarCount());

  size_t gen = 0;
  for (; gen < ideal.getGeneratorCount(); ++gen) {
    if (!Ops::encodeTerm(_memoryEnd, ideal[gen], getVarCount()))
      break;
    ++_genCount;
    _memoryEnd += getWordsPerTerm();
  }
  ASSERT(isValid());
  return gen;
}

void RSFIdeal::insert(const RawSquareFreeIdeal& ideal) {
  const_iterator stop = ideal.end();
  for (const_iterator it = ideal.begin(); it != stop; ++it)
	insert(*it);
  ASSERT(isValid());
}

bool RSFIdeal::insert(const std::vector<std::string>& term) {
  ASSERT(term.size() == getVarCount());

  if (!Ops::encodeTerm(_memoryEnd, term, getVarCount()))
    return false;
  ++_genCount;
  _memoryEnd += getWordsPerTerm();
  ASSERT(isValid());
  return true;
}

void RSFIdeal::minimize() {
  iterator newEnd = ::minimize(begin(), end(), getWordsPerTerm());
  _genCount = newEnd - begin();
  _memoryEnd = *newEnd;
  ASSERT(isValid());
}

void RSFIdeal::colon(const Word* by) {
  const size_t wordCount = getWordsPerTerm();
  const iterator stop = end();
  for (iterator it = begin(); it != stop; ++it)
	Ops::colonInPlace(*it, *it + wordCount, by);
  ASSERT(isValid());
}

void RSFIdeal::colon(size_t var) {
  const iterator stop = end();
  for (iterator it = begin(); it != stop; ++it)
	Ops::setExponent(*it, var, false);
  ASSERT(isValid());
}

void RSFIdeal::compact(const Word* remove) {
  const size_t oldVarCount = getVarCount();
  const iterator oldBegin = begin();
  const iterator oldStop = end();

  // Compact each term without moving that term.
  size_t varCompact = 0;
  for (size_t var = 0; var < oldVarCount; ++var) {
	if (Ops::getExponent(remove, var) != 0)
	  continue;
	for (iterator oldIt = oldBegin; oldIt != oldStop; ++oldIt)
	  Ops::setExponent(*oldIt, varCompact, Ops::getExponent(*oldIt, var));
	++varCompact;
  }
  // varCompact is now the number of variables in the compacted ideal.

  // The last word in each term must have zeroes at those positions
  // that are past the number of actual variables. So we need to go through and
  const size_t bitOffset = Ops::getBitOffset(varCompact);
  const size_t wordOffset = Ops::getWordOffset(varCompact);
  if (bitOffset != 0) {
	const Word mask = (((Word)1) << bitOffset) - 1;
	for (iterator oldIt = oldBegin; oldIt != oldStop; ++oldIt)
	  *(*oldIt + wordOffset) &= mask;
  }

  // Copy the new compacted terms to remove the space between them. We
  // couldn't do that before because those spaces contained exponents
  // that we had not extracted yet.
  const size_t newWordCount = Ops::getWordCount(varCompact);
  iterator newIt(_memory, newWordCount);
  for (iterator oldIt = oldBegin; oldIt != oldStop; ++oldIt, ++newIt)
	Ops::assign(*newIt, (*newIt) + newWordCount, *oldIt);

  _varCount = varCompact;
  _wordsPerTerm = newWordCount;
  _memoryEnd = *newIt;
  ASSERT(isValid());
}

void RSFIdeal::getLcmOfNonMultiples(Word* lcm, size_t var) const {
  ASSERT(var < getVarCount());

  const Word* const lcmEnd = lcm + getWordsPerTerm();
  Ops::setToIdentity(lcm, lcmEnd);

  const const_iterator stop = end();
  for (const_iterator it = begin(); it != stop; ++it)
	if (Ops::getExponent(*it, var) == 0)
	  Ops::lcmInPlace(lcm, lcmEnd, *it);
  ASSERT(isValid());
}

static inline void countVarDividesBlockUpTo15(const Word* it,
											  size_t genCount,
											  const size_t wordsPerTerm,
											  size_t* counts) {
  // mask has the bit pattern 0001 0001 ... 0001
  const Word mask = ~((Word)0u) / 15u;

  Word a0, a1, a2, a3;
  if ((genCount & 1) == 1) {
	const Word a = *it;
	a0 = a & mask;
	a1 = (a >> 1) & mask;
	a2 = (a >> 2) & mask;
	a3 = (a >> 3) & mask;
	it += wordsPerTerm;
  } else
	a0 = a1 = a2 = a3 = 0;

  genCount >>= 1;
  for (size_t i = 0; i < genCount; ++i) {
	const Word a = *it;
	it += wordsPerTerm;
	const Word aa = *it;
	it += wordsPerTerm;

	a0 += a & mask;
	a1 += (a >> 1) & mask;
	a2 += (a >> 2) & mask;
	a3 += (a >> 3) & mask;

	a0 += aa & mask;
	a1 += (aa >> 1) & mask;
	a2 += (aa >> 2) & mask;
	a3 += (aa >> 3) & mask;
  }

  for (size_t i = 0; i < BitsPerWord / 4; ++i) {
	*(counts + 0) += a0 & 0xF;
	*(counts + 1) += a1 & 0xF;
	*(counts + 2) += a2 & 0xF;
	*(counts + 3) += a3 & 0xF;
	a0 >>= 4;
	a1 >>= 4;
	a2 >>= 4;
	a3 >>= 4;
	counts += 4;
  }
}

void RSFIdeal::getVarDividesCounts(vector<size_t>& divCounts) const {
  const size_t varCount = getVarCount();
  const size_t wordCount = getWordsPerTerm();
  // We reserve BitsPerWord extra space. Otherwise we would have to
  // make sure not to index past the end of the vector of counts when
  // dealing with variables in the unused part of the last word.
  divCounts.reserve(getVarCount() + BitsPerWord);
  divCounts.resize(getVarCount());
  size_t* divCountsBasePtr = &(divCounts.front());
  size_t* divCountsEnd = divCountsBasePtr + BitsPerWord * wordCount;
  memset(divCountsBasePtr, 0, sizeof(size_t) * varCount);

  size_t generatorsToGo = getGeneratorCount();
  const_iterator blockBegin = begin();
  while (generatorsToGo > 0) {
	const size_t blockSize = generatorsToGo >= 15 ? 15 : generatorsToGo;

	size_t* counts = divCountsBasePtr;
	const Word* genOffset = *blockBegin;
	for (; counts != divCountsEnd; counts += BitsPerWord, ++genOffset)
	  countVarDividesBlockUpTo15(genOffset, blockSize, wordCount, counts);

	generatorsToGo -= blockSize;
	blockBegin = blockBegin + blockSize;
  }
}

size_t RSFIdeal::getMultiple(size_t var) const {
  ASSERT(var < getVarCount());

  const const_iterator stop = end();
  const const_iterator start = begin();
  for (const_iterator it = start; it != stop; ++it)
	if (Ops::getExponent(*it, var) == 1)
	  return it - start;
  return getGeneratorCount();
}

size_t RSFIdeal::getNonMultiple(size_t var) const {
  ASSERT(var < getVarCount());

  const const_iterator stop = end();
  const const_iterator start = begin();
  for (const_iterator it = start; it != stop; ++it)
	if (Ops::getExponent(*it, var) == 0)
	  return it - start;
  return getGeneratorCount();
}

size_t RSFIdeal::getMaxSupportGen() const {
  const_iterator it = begin();
  const const_iterator stop = end();
  if (it == stop)
	return 0;

  const size_t varCount = getVarCount();
  size_t maxSupp = Ops::getSizeOfSupport(*it, varCount);
  const_iterator maxSuppIt = it;

  for (++it; it != stop; ++it) {
	const size_t supp = Ops::getSizeOfSupport(*it, varCount);
	if (maxSupp < supp) {
		maxSupp = supp;
		maxSuppIt = it;
	}
  }
  return maxSuppIt - begin();
}

size_t RSFIdeal::getMinSupportGen() const {
  const_iterator it = begin();
  const const_iterator stop = end();
  if (it == stop)
	return 0;

  const size_t varCount = getVarCount();
  size_t minSupp = Ops::getSizeOfSupport(*it, varCount);
  const_iterator minSuppIt = it;

  for (++it; it != stop; ++it) {
	const size_t supp = Ops::getSizeOfSupport(*it, varCount);
	if (minSupp > supp) {
		minSupp = supp;
		minSuppIt = it;
	}
  }
  return minSuppIt - begin();
}

void RSFIdeal::getGcdOfMultiples(Word* gcd, size_t var) const {
  ASSERT(var < getVarCount());

  const Word* const gcdEnd = gcd + getWordsPerTerm();
  Ops::setToAllVarProd(gcd, getVarCount());

  const const_iterator stop = end();
  for (const_iterator it = begin(); it != stop; ++it)
	if (Ops::getExponent(*it, var) == 1)
	  Ops::gcdInPlace(gcd, gcdEnd, *it);
}

void RSFIdeal::getGcdOfMultiples(Word* gcd, const Word* div) const {
  const size_t varCount = getVarCount();
  const size_t wordCount = getWordsPerTerm();
  const Word* const gcdEnd = gcd + wordCount;
  const Word* divEnd = div + wordCount;
  Ops::setToAllVarProd(gcd, varCount);

  const const_iterator stop = end();
  for (const_iterator it = begin(); it != stop; ++it)
	if (Ops::divides(div, divEnd, *it))
	  Ops::gcdInPlace(gcd, gcdEnd, *it);
}

void RSFIdeal::removeGenerator(size_t gen) {
  Word* term = getGenerator(gen);
  Word* last = _memoryEnd - getWordsPerTerm();
  if (term != last)
	Ops::assign(term, term + getWordsPerTerm(), last);
  --_genCount;
  _memoryEnd -= getWordsPerTerm();
  ASSERT(isValid());
}

void RSFIdeal::insertNonMultiples(const Word* term,
								  const RawSquareFreeIdeal& ideal) {
  ASSERT(getVarCount() == ideal.getVarCount());

  const Word* termEnd = term + getWordsPerTerm();
  const_iterator stop = ideal.end();
  for (const_iterator it = ideal.begin(); it != stop; ++it)
	if (!Ops::divides(term, termEnd, *it))
	  insert(*it);
  ASSERT(isValid());
}

void RSFIdeal::insertNonMultiples(size_t var,
								  const RawSquareFreeIdeal& ideal) {
  ASSERT(getVarCount() == ideal.getVarCount());

  const_iterator stop = ideal.end();
  for (const_iterator it = ideal.begin(); it != stop; ++it)
	if (Ops::getExponent(*it, var) == 0)
	  insert(*it);
  ASSERT(isValid());
}

size_t RSFIdeal::getNotRelativelyPrime(const Word* term) {
  const size_t wordCount = getWordsPerTerm();
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
	if (!Ops::isRelativelyPrime(term, term + wordCount, getGenerator(gen)))
	  return gen;
  return getGeneratorCount();
}

size_t RSFIdeal::getExclusiveVarGenerator() {
  const size_t wordCount = getWordsPerTerm();
  for (size_t offset = 0; offset < wordCount; ++offset) {
	Word once = 0;
	Word twice = 0;
	for (size_t gen = 0; gen < _genCount; ++gen) {
	  Word word = getGenerator(gen)[offset];
	  twice |= once & word;
	  once |= word;
	}
	const Word onceOnly = once & ~twice;
	if (onceOnly != 0) {
	  for (size_t gen = 0; ; ++gen) {
		ASSERT(gen < _genCount);
		Word word = getGenerator(gen)[offset];
		if (word & onceOnly)
		  return gen;
	  }
	  ASSERT(false);
	}
  }
  return getGeneratorCount();
}

bool RSFIdeal::hasFullSupport(const Word* ignore) const {
  ASSERT(ignore != 0);
  const size_t wordCount = getWordsPerTerm();
  const Word allOnes = ~((Word)0);

  const Word* firstGenOffset = _memory;
  const Word* endGenOffset = _memoryEnd;
  size_t varsLeft = getVarCount();
  while (true) {
	const Word* gen = firstGenOffset;
	Word support = *ignore;
	while (gen != endGenOffset) {
	  support |= *gen;
	  gen += wordCount;
	}

	if (varsLeft > BitsPerWord) {
	  if (support != allOnes)
		return false;
	} else {
	  if (varsLeft == BitsPerWord)
		return support == allOnes;
	  const Word fullSupportWord = (((Word)1) << varsLeft) - 1;
	  return support == fullSupportWord;
	}

	varsLeft -= BitsPerWord;
	++ignore;
	++firstGenOffset;
	++endGenOffset;
  }
  return true;
}

bool RSFIdeal::isMinimallyGenerated() const {
  size_t wordCount = getWordsPerTerm();
  for (size_t i = 0; i < _genCount; ++i)
	for (size_t div = 0; div < _genCount; ++div)
	  if (div != i &&
		  Ops::divides(getGenerator(div), getGenerator(div) + wordCount,
					   getGenerator(i)))
		return false;
  return true;
}

void RSFIdeal::swap(size_t a, size_t b) {
  ASSERT(a < getGeneratorCount());
  ASSERT(b < getGeneratorCount());
  Ops::swap(getGenerator(a), getGenerator(b), getVarCount());
  ASSERT(isValid());
}

bool RSFIdeal::operator==(const RawSquareFreeIdeal& ideal) const {
  if (getVarCount() != ideal.getVarCount())
    return false;
  if (getGeneratorCount() != ideal.getGeneratorCount())
    return false;

  const size_t varCount = getVarCount();
  const_iterator stop = end();
  const_iterator it = begin();
  const_iterator it2 = ideal.begin();
  for (; it != stop; ++it, ++it2)
    if (!Ops::equals(*it, *it2, varCount))
      return false;
  return true;
}

namespace {
  struct CmpForSortLexAscending {
	bool operator()(size_t a, size_t b) const {
	  return Ops::lexLess(ideal->getGenerator(a), ideal->getGenerator(b),
						  ideal->getVarCount());
	}
	RawSquareFreeIdeal* ideal;
  };
}

void RSFIdeal::sortLexAscending() {
  vector<size_t> sorted(getGeneratorCount());
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
	sorted[gen] = gen;
  {
	CmpForSortLexAscending cmp;
	cmp.ideal = this;
	std::sort(sorted.begin(), sorted.end(), cmp);
  }

  RawSquareFreeIdeal* clone =
	newRawSquareFreeIdeal(getVarCount(), getGeneratorCount());
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
	clone->insert(getGenerator(gen));
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
	Ops::assign(getGenerator(gen), clone->getGenerator(sorted[gen]),
				getVarCount());
  deleteRawSquareFreeIdeal(clone);
  ASSERT(isValid());
}

void RSFIdeal::insert(const Word* term) {
  Ops::assign(_memoryEnd, _memoryEnd + getWordsPerTerm(), term);
  ++_genCount;
  _memoryEnd += getWordsPerTerm();
  ASSERT(isValid());
}

void RSFIdeal::swap01Exponents() {
  const iterator stop = end();
  const size_t varCount = getVarCount();
  for (iterator it = begin(); it != stop; ++it)
    Ops::invert(*it, varCount);
}

void RSFIdeal::insertIdentity() {
  Ops::setToIdentity(_memoryEnd, _memoryEnd + getWordsPerTerm());
  ++_genCount;
  _memoryEnd += getWordsPerTerm();
  ASSERT(isValid());
}

namespace {
  struct ColonReminimizeTermHelper {
	bool operator()(const Word* term) {
	  return !Ops::isRelativelyPrime(colon, colonEnd, term);
	}
	const Word* colon;
	const Word* colonEnd;
  };
}

void RSFIdeal::colonReminimize(const Word* by) {
  ASSERT(by != 0);
  const size_t varCount = getVarCount();
  const size_t wordCount = getWordsPerTerm();
  const iterator start = begin();
  iterator stop = end();

  ColonReminimizeTermHelper colonIsRelativelyPrime;
  colonIsRelativelyPrime.colon = by;
  colonIsRelativelyPrime.colonEnd = by + wordCount;
  iterator middle = RsfPartition(start, stop, colonIsRelativelyPrime, varCount);

  if (middle == start) {
	ASSERT(isValid());
	return; // colon is relatively prime to all generators
  }
  for (iterator it = start; it != middle; ++it)
	Ops::colonInPlace(*it, *it + wordCount, by);

  // var is not relatively prime to [start, middle) and is relatively
  // prime to [middle, end).

  iterator newMiddle = ::minimize(start, middle, getWordsPerTerm());

  iterator newEnd = newMiddle;
  for (iterator it = middle; it != stop; ++it) {
	for (const_iterator div = start; div != newMiddle; ++div)
	  if (Ops::divides(*div, *div + wordCount, *it))
		goto next;
	Ops::assign(*newEnd, *newEnd + wordCount, *it);
	++newEnd;
  next:;
  }

  _memoryEnd = *newEnd;
  _genCount = newEnd - start;
  ASSERT(isValid());
}

namespace {
  struct ColonReminimizeVarHelper {
	bool operator()(const Word* term) {
	  return Ops::getExponent(term, var) != 0;
	}
	size_t var;
  };
}

void RSFIdeal::colonReminimize(size_t var) {
  ASSERT(var < getVarCount());
  const size_t varCount = getVarCount();
  const size_t wordCount = getWordsPerTerm();
  const iterator start = begin();
  iterator stop = end();

  ColonReminimizeVarHelper varDivides;
  varDivides.var = var;
  iterator middle = RsfPartition(start, stop, varDivides, varCount);

  if (middle == start) {
	ASSERT(isValid());
	return; // var divides no generators
  }
  for (iterator it = start; it != middle; ++it)
	Ops::setExponent(*it, var, 0);
  if (middle == stop) {
	ASSERT(isValid());
	return; // var divides all
  }

  // var divided [start, middle) and did (does) not divide [middle,
  // end).  Both of these ranges are minimized on their own, and no
  // element of [middle, end) divides an element of [start, middle).
  for (iterator it = middle; it != stop;) {
	for (const_iterator div = start; div != middle; ++div) {
	  if (Ops::divides(*div, *div + wordCount, *it)) {
		--stop;
		Ops::assign(*it, *it + wordCount, *stop);
		--_genCount;
		goto next;
	  }
	}
	++it;
  next:;
  }
  _memoryEnd = *stop;

  ASSERT(isValid());
}

void RSFIdeal::getLcm(Word* lcm) const {
  Word* lcmEnd = lcm + getWordsPerTerm();
  Ops::setToIdentity(lcm, lcmEnd);
  const const_iterator stop = end();
  for (const_iterator it = begin(); it != stop; ++it)
	Ops::lcmInPlace(lcm, lcmEnd, *it);
  ASSERT(isValid());
}

bool RSFIdeal::isValid() const {
  const size_t varCount = getVarCount();
  const size_t wordCount = getWordsPerTerm();
  const size_t genCount = getGeneratorCount();
  if (varCount != _varCount)
	return false;
  if (wordCount != _wordsPerTerm)
	return false;
  if (_genCount != genCount)
	return false;

  if (wordCount != Ops::getWordCount(varCount))
	return false;
  if (_memoryEnd != _memory + wordCount * genCount)
	return false;
  if (_memoryEnd < _memory)
	return false; // happens on overflow

  for (const Word* p = _memory; p != _memoryEnd; p += wordCount)
	Ops::isValid(p, varCount);
  return true;
}

RSFIdeal* newRawSquareFreeIdeal(size_t varCount, size_t capacity) {
  size_t byteCount = RSFIdeal::getBytesOfMemoryFor(varCount, capacity);
  if (byteCount == 0)
	throw bad_alloc();
  void* buffer = new char[byteCount];
  RSFIdeal* ideal = RSFIdeal::construct(buffer, varCount);
  ASSERT(ideal->isValid());
  return ideal;
}

RawSquareFreeIdeal* newRawSquareFreeIdeal(const RawSquareFreeIdeal& ideal) {
  size_t byteCount = RSFIdeal::getBytesOfMemoryFor(ideal.getVarCount(),
												   ideal.getGeneratorCount());
  if (byteCount == 0)
	throw bad_alloc();
  void* buffer = new char[byteCount];
  RawSquareFreeIdeal* p = RSFIdeal::construct(buffer, ideal.getVarCount());
  p->insert(ideal);
  ASSERT(p->isValid());
  return p;
}

RawSquareFreeIdeal* newRawSquareFreeIdealParse(const char* str) {
  istringstream in(str);
  vector<string> lines;
  string line;

  while (getline(in, line))
	if (line != "")
	  lines.push_back(line);

  const size_t varCount = lines.empty() ? 0 : lines.front().size();
  RawSquareFreeIdeal* ideal = newRawSquareFreeIdeal(varCount, lines.size());
  for (size_t gen = 0; gen < lines.size(); ++gen) {
	ASSERT(lines[gen].size() == varCount);
	Word* term = Ops::newTermParse(lines[gen].c_str());
	ideal->insert(term);
	Ops::deleteTerm(term);
  }
  ASSERT(ideal->isValid());
  return ideal;
}

void deleteRawSquareFreeIdeal(RSFIdeal* ideal) {
  delete[] reinterpret_cast<char*>(ideal);
}
