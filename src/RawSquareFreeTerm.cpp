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
#include "RawSquareFreeTerm.h"

#include <sstream>
#include <vector>

namespace SquareFreeTermOps {
  Word* newTermParse(const char* strParam) {
	string str(strParam);
	Word* term = newTerm(str.size());
	for (size_t var = 0; var < str.size(); ++var) {
	  ASSERT(str[var] == '0' || str[var] == '1');
	  setExponent(term, var, str[var] == '1' ? 1 : 0);
	}
	return term;
  }

  void print(FILE* file, const Word* term, size_t varCount) {
	ostringstream out;
	print(out, term, varCount);
	fputs(out.str().c_str(), file);
  }

  void print(ostream& out, const Word* term, size_t varCount) {
	ASSERT(term != 0 || varCount == 0);

	out << '(';
	for (size_t var = 0; var < varCount; ++var) {
	  out << getExponent(term, var);
	}
	out << ')';
  }

  bool isIdentity(const Word* a, Word* aEnd) {
	for (; a != aEnd; ++a)
	  if (*a != 0)
		return false;
	return true;
  }

  bool isIdentity(const Word* a, size_t varCount) {
	if (varCount == 0)
	  return true;
	while (true) {
	  if (*a != 0)
		return false;
	  if (varCount <= BitsPerWord)
		return true;
	  ++a;
	  varCount -= BitsPerWord;
	}
  }

  size_t getSizeOfSupport(const Word* a, size_t varCount) {
	if (varCount == 0)
	  return 0;
	size_t count = 0;
	while (true) {
	  Word word = *a;
	  // TODO: should be able to improve this set bit counting algorithm.
	  while (word != 0) {
		if ((word & 1) != 0)
		  ++count;
		word >>= 1;
	  }

	  if (varCount <= BitsPerWord)
		return count;
	  ++a;
	  varCount -= BitsPerWord;
	}
  }

  size_t getWordCount(size_t varCount) {
	// Compute varCount / BitsPerWord rounded up. Special case for
	// varCount == 0 as formula has underflow issue for that input.
	if (varCount == 0)
	  return 1;
	else
	  return ((varCount - 1) / BitsPerWord) + 1;
  }

  void compact(Word* compacted, const Word* term,
			   const Word* remove, size_t varCount) {
	size_t newVarCount = 0;
	for (size_t var = 0; var < varCount; ++var) {
	  if (getExponent(remove, var) != 0)
		continue;
	  setExponent(compacted, newVarCount, getExponent(term, var));
	  ++newVarCount;
	}
	for (; newVarCount % BitsPerWord != 0; ++newVarCount)
	  setExponent(compacted, newVarCount, 0);

	ASSERT(isValid(compacted, newVarCount));
  }

  void setToIdentity(Word* res, const Word* resEnd) {
	for (; res != resEnd; ++res)
	  *res = 0;
  }

  void setToIdentity(Word* res, size_t varCount) {
	for (; varCount >= BitsPerWord; ++res, varCount -= BitsPerWord)
	  *res = 0;
	if (varCount > 0)
	  *res = 0;
  }

  /** Sets all exponents of res to 1. */
  void setToAllVarProd(Word* res, size_t varCount) {
	for (; varCount >= BitsPerWord; ++res, varCount -= BitsPerWord)
	  *res = ~0;
	if (varCount > 0) {
	  const Word fullSupportWord = (((Word)1) << varCount) - 1;
	  *res = fullSupportWord;
	}
  }

  /** Returns identity term of varCount variables. Must deallocate
	  with deleteTerm(). */
  Word* newTerm(size_t varCount) {
	const size_t wordCount = getWordCount(varCount);
	Word* word = FROBBY_NEW Word[wordCount];
	setToIdentity(word, word + wordCount);
	return word;
  }

  /** Allocates and returns a term based on str. The returned term
   must be deallocated using deleteTerm. str is parsed as a list of
   exponent vector entries that must be '0' or '1'. The string can
   contain no other characters. The parsing is inefficient and is
   intended for constructing terms in testing. */
  Word* newTermParse(const char* str);

  /** Deletes term previously returned by newTerm(). Term can be null. */
  void deleteTerm(Word* term) {
	delete[] term;
  }

  bool lexLess(const Word* a, const Word* b, size_t varCount) {
	if (varCount == 0)
	  return false;
	while (true) {
	  if (*a != *b) {
		Word xorAB = (*a) ^ (*b);
		ASSERT(xorAB != 0);
		Word leastSignificantBit = xorAB & (-xorAB);
		return (*a & leastSignificantBit) == 0;
	  }
	  if (varCount <= BitsPerWord)
		return false; // a and b are equal
	  ++a;
	  ++b;
	  varCount -= BitsPerWord;
	}
  }

  void colon(Word* res, const Word* resEnd, const Word* a, const Word* b) {
	for (; res != resEnd; ++res, ++a, ++b)
	  *res = (*a) & (~*b);
  }

  void colonInPlace(Word* res, const Word* resEnd, const Word* b) {
	for (; res != resEnd; ++res,  ++b)
	  *res &= ~*b;
  }

  void assign(Word* a, const Word* b, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++b, varCount -= BitsPerWord)
	  *a = *b;
	if (varCount > 0)
	  *a = *b;
  }

  bool encodeTerm(Word* encoded, const Exponent* term, const size_t varCount) {
	size_t var = 0;
	while (var < varCount) {
	  Word bit = 1;
	  *encoded = 0;
	  do {
		if (term[var] == 1)
		  *encoded |= bit;
		else if (term[var] != 0)
		  return false;
		bit <<= 1;
		++var;
	  } while (bit != 0 && var < varCount);
	  ++encoded;
	}
	return true;
  }

  bool encodeTerm(Word* encoded, const std::vector<mpz_class>& term, const size_t varCount) {
	size_t var = 0;
	while (var < varCount) {
	  Word bit = 1;
	  *encoded = 0;
	  do {
		if (term[var] == 1)
		  *encoded |= bit;
		else if (term[var] != 0)
		  return false;
		bit <<= 1;
		++var;
	  } while (bit != 0 && var < varCount);
	  ++encoded;
	}
	return true;
  }

  bool encodeTerm(Word* encoded, const std::vector<std::string>& term, const size_t varCount) {
	size_t var = 0;
	while (var < varCount) {
	  Word bit = 1;
	  *encoded = 0;
	  do {
        if (!term[var].empty()) {
          if (term[var].size() > 1)
            return false;
          if (term[var][0] == '1')
            *encoded |= bit;
          else if (term[var][0] != '0')
            return false;
        }
		bit <<= 1;
		++var;
	  } while (bit != 0 && var < varCount);
	  ++encoded;
	}
	return true;
  }

  void lcm(Word* res, const Word* resEnd,
				  const Word* a, const Word* b) {
	for (; res != resEnd; ++a, ++b, ++res)
	  *res = (*a) | (*b);
  }

  void lcm(Word* res, const Word* a, const Word* b, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++b, ++res, varCount -= BitsPerWord)
	  *res = (*a) | (*b);
	if (varCount != 0)
	  *res = (*a) | (*b);
  }

  void lcmInPlace(Word* res, const Word* resEnd, const Word* a) {
	for (; res != resEnd; ++a, ++res)
	  *res |= *a;
  }

  void lcmInPlace(Word* res, const Word* a, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++res, varCount -= BitsPerWord)
	  *res |= *a;
	if (varCount != 0)
	  *res |= *a;
  }

  void gcd(Word* res, const Word* resEnd, const Word* a, const Word* b) {
	for (; res != resEnd; ++a, ++b, ++res)
	  *res = (*a) & (*b);
  }

  void gcd(Word* res, const Word* a, const Word* b, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++b, ++res, varCount -= BitsPerWord)
	  *res = (*a) & (*b);
	if (varCount != 0)
	  *res = (*a) & (*b);
  }

  void gcdInPlace(Word* res, const Word* resEnd, const Word* a) {
	for (; res != resEnd; ++a, ++res)
	  *res &= *a;
  }

  void gcdInPlace(Word* res, const Word* a, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++res, varCount -= BitsPerWord)
	  *res &= *a;
	if (varCount != 0)
	  *res &= *a;
  }

  bool isRelativelyPrime(const Word* a, const Word* b, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++b, varCount -= BitsPerWord)
	  if ((*a) & (*b))
		return false;
	if (varCount != 0)
	  if ((*a) & (*b))
		return false;
	return true;
  }

  /** Make 0 exponents 1 and make 1 exponents 0. */
  void invert(Word* a, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, varCount -= BitsPerWord)
	  *a = ~*a;
	if (varCount > 0) {
	  const Word fullSupportWord = (((Word)1) << varCount) - 1;
	  *a = (~*a) & fullSupportWord;
	}
  }

  /** Returns var if a equals var. If a is the identity or the product
	  of more than one variable then returns varCount. */
  size_t getVarIfPure(const Word* const a, size_t varCount) {
	const Word* hit = 0;
	size_t varsToGo = varCount;
	const Word* it = a;
	for (; varsToGo >= BitsPerWord; ++it, varsToGo -= BitsPerWord) {
	  if (*it != 0) {
		if (hit != 0)
		  return varCount;
		hit = it;
	  }
	}
	if (varsToGo != 0) {
	  if (*it != 0) {
		if (hit != 0)
		  return varCount;
		hit = it;
	  }
	}
	if (hit == 0)
	  return varCount;
	size_t hitVar = (hit - a) * BitsPerWord;
	Word word = *hit;
	while ((word & 1) == 0) {
	  ASSERT(word != 0);
	  ++hitVar;
	  word >>= 1;
	}
	word >>= 1;
	if (word != 0)
	  return varCount;
	return hitVar;
  }

  /** For every variable var that divides a, decrement inc[var] by one. */
  void decrementAtSupport(const Word* a, size_t* inc, size_t varCount) {
	if (varCount == 0)
	  return;
	while (true) {
	  Word word = *a;
	  size_t* intraWordInc = inc;
	  while (word != 0) {
		*intraWordInc -= word & 1; // -1 if bit set
		++intraWordInc;
		word = word >> 1;
	  }
	  if (varCount <= BitsPerWord)
		return;
	  varCount -= BitsPerWord;
	  inc += BitsPerWord;
	  ++a;
	}
  }

  /** For every variable var that divides a, set inc[var] to zero. */
  void toZeroAtSupport(const Word* a, size_t* inc, size_t varCount) {
	if (varCount == 0)
	  return;
	while (true) {
	  Word word = *a;
	  size_t* intraWordInc = inc;
	  while (word != 0) {
		if (word & 1)
		  *intraWordInc = 0;
		++intraWordInc;
		word = word >> 1;
	  }
	  if (varCount <= BitsPerWord)
		return;
	  varCount -= BitsPerWord;
	  inc += BitsPerWord;
	  ++a;
	}
  }

  /** Returns true if a equals b. */
  bool equals(const Word* a, const Word* b, size_t varCount) {
	if (varCount == 0)
	  return true;
	while (true) {
	  if (*a != *b)
		return false;
	  if (varCount <= BitsPerWord)
		return true;
	  ++a;
	  ++b;
	  varCount -= BitsPerWord;
	}
  }

  /** The unused bits at the end of the last word must be zero for the
	  functions here to work correctly. They should all maintain this
	  invariant. This function is useful in tests to ensure that that
	  is true. */
  bool isValid(const Word* a, size_t varCount) {
	size_t offset = varCount / BitsPerWord;
	a += offset;
	varCount -= BitsPerWord * offset;
	if (varCount == 0)
	  return true;
	const Word fullSupportWord = (((Word)1) << varCount) - 1;
	return ((*a) & ~fullSupportWord) == 0;
  }

  void swap(Word* a, Word* b, size_t varCount) {
	for (; varCount >= BitsPerWord; ++a, ++b, varCount -= BitsPerWord)
	  std::swap(*a, *b);
	if (varCount > 0)
	  std::swap(*a, *b);
  }
}
