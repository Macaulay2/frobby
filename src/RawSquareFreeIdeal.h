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
#ifndef RAW_SQUARE_FREE_IDEAL_GUARD
#define RAW_SQUARE_FREE_IDEAL_GUARD

#include <ostream>
#include <vector>

class Ideal;
class BigIdeal;

/** A bit packed square free ideal placed in a pre-allocated buffer.

	Use SquareFreeIdeal for a more pleasant interface.

	This class allocates none of its own memory. It must be
	initialized using the member function construct() in a buffer of
	sufficient size. What is sufficient memory for a given ideal can
	be determined by getBytesOfMemoryFor(). The buffer must be
	correctly aligned to support the data members that this class has.
*/
class RawSquareFreeIdeal {
 public:
  static RawSquareFreeIdeal* construct(void* buffer, size_t varCount = 0);
  static RawSquareFreeIdeal* construct(void* buffer, const Ideal& ideal);
  static RawSquareFreeIdeal* construct(void* buffer,
    const RawSquareFreeIdeal& ideal);

  /** Returns the number of bytes of memory necessary to contain an
	  ideal with the given parameters. This size is padded to ensure
	  proper alignment if ideals are placed one after the other.

	  This function return zero if the number of bytes cannot be
	  represented in a size_t, or nearly cannot be represented in a
	  size_t. The returned value is otherwise positive.
  */
  static size_t getBytesOfMemoryFor(size_t varCount, size_t generatorCount);

  /** Resets this object to be a copy of ideal. */
  RawSquareFreeIdeal& operator=(const RawSquareFreeIdeal& ideal)
    {return *construct(this, ideal);}

  /** Resets this object to the transpose of ideal.

   Let M be a matrix with variable-labeled columns and
   generator-labeled rows. Entry (g,v) is 1 if and only if
   v divides g and otherwise it is 0. There is a bijection
   between such matrices and generating sets of ideals. Take
   a generating set, construct its matric and transpose the
   matrix. The generating set corresponding to the transposed
   matrix generates the transpose ideal.

   The eraseVars parameter is used as for compact(). So the
   variables who have a 1 at their index into eraseVars (as bits)
   will not be taken into the matrix that is transposed. A null
   value indicates to take along all variables. */
  void setToTransposeOf(const RawSquareFreeIdeal& ideal, Word* eraseVars = 0);

  /** Equivalent to setToTransposeOf(this, eraseVars). */
  void transpose(Word* eraseVars = 0);

  /** Removes the variables that divide remove. Unless remove is the
   identity this will decrease varCount. The operation is much like
   colon, except that the exponents are remove entirely instead of
   just set to zero. The relative order of the remaining variables is
   preserved. */
  void compact(const Word* remove);

  /** Print a debug-suitable representation of this object to file. */
  void print(FILE* file) const;

  /** Print a debug-suitable representation of this object to out. */
  void print(ostream& out) const;

  /** Inserts the generators of ideal from index 0 onward until
	  reaching a non-squarefree generator or all generators have been
	  inserted. Returns the number of generators that has been
	  inserted. There must be enough capacity for all the generators
	  even if not all are actually inserted. */
  size_t insert(const Ideal& ideal);
  size_t insert(const BigIdeal& ideal);

  /** Inserts all the generators of ideal. */
  void insert(const RawSquareFreeIdeal& ideal);

  void insert(const Word* term);
  void insertIdentity();

  /** Returns false if the strings in term are not all "", "1" or "0". */
  bool insert(const std::vector<std::string>& term);
  void minimize();
  void colon(const Word* by);
  void colon(size_t var);

  /** Performs a colon and minimize. Object must be already minimized. */
  void colonReminimize(const Word* colon);

  /** Performs a colon by var and minimize. Object must be already
   minimized. */
  void colonReminimize(size_t var);

  /** Change 0 exponents into 1 and vice versa. */
  void swap01Exponents();

  /** Puts the least common multiple of the generators of the ideal
	  into lcm. */
  void getLcm(Word* lcm) const;

  size_t getGeneratorCount() const {return _genCount;}
  size_t getVarCount() const {return _varCount;}
  size_t getWordsPerTerm() const {return _wordsPerTerm;}

  /** Returns the generator at index. */
  Word* getGenerator(size_t index);

  /** Returns the generator at index. */
  const Word* getGenerator(size_t index) const;

  /** Returns a pointer to the memory where a generator at index would
	  be, even if index is equal to or greater than
	  getGeneratorCount(). */
  Word* getGeneratorUnsafe(size_t index);

  /** Returns a pointer to the memory where a generator at index would
	  be, even if index is equal to or greater than
	  getGeneratorCount(). */
  const Word* getGeneratorUnsafe(size_t index) const;

  /** Sets lcm to be the least common multple of those generators that
	  var does not divide. */
  void getLcmOfNonMultiples(Word* lcm, size_t var) const;

  /** Sets gcd to be the greatest common denominator of those
	  generators that are divisible by var. */
  void getGcdOfMultiples(Word* gcd, size_t var) const;

  /** Sets gcd to be the greatest common denominator of those
	  generators that are divisible by div. */
  void getGcdOfMultiples(Word* gcd, const Word* div) const;

  /** Sets counts[var] to the number of generators that var divides. */
  void getVarDividesCounts(vector<size_t>& counts) const;

  /** Returns the index of the first generator that var divides or
	  getGeneratorCount() if no such generator exists. */
  size_t getMultiple(size_t var) const;

  /** Returns the index of the first generator that var does not
	  divide or getGeneratorCount() if no such generator exists. */
  size_t getNonMultiple(size_t var) const;

  /** Returns the index of a generator with maximum support. Returns 0
	  if there are no generators. */
  size_t getMaxSupportGen() const;

  /** Returns the index of a generator with minimum support. Returns 0
	  if there are no generators. */
  size_t getMinSupportGen() const;

  /** Removes the generator at index. */
  void removeGenerator(size_t index);

  /** Insert those generators of ideal that are not multiples of term. */
  void insertNonMultiples(const Word* term, const RawSquareFreeIdeal& ideal);

  /** Insert those generators of ideal that are not multiples of var. */
  void insertNonMultiples(size_t var, const RawSquareFreeIdeal& ideal);

  /** Returns the index of the first generator that is not relatively
	  prime with term. Returns getGeneratorCount() if no such
	  generator exists. */
  size_t getNotRelativelyPrime(const Word* term);

  /** Returns the index of a generator that is the only one to be
	  divisible by some variable. Returns getGeneratorCount() if there
	  is no such generator. */
  size_t getExclusiveVarGenerator();

  /** Returns true if for every variable it either divides ignore or
	  it divides some (not necessarily minimal) generator. */
  bool hasFullSupport(const Word* ignore) const;

  /** Returns true if no generator divides another. */
  bool isMinimallyGenerated() const;

  void swap(size_t a, size_t b);

  /** Returns true if *this equals ideal. This comparison takes
   non-minimal generators and the order of the generators into
   account. */
  bool operator==(const RawSquareFreeIdeal& ideal) const;
  bool operator!=(const RawSquareFreeIdeal& ideal) const {
	return !(*this == ideal);
  }

  Word* back() {iterator e = end(); --e; return *e;}
  const Word* back() const {const_iterator e = end(); --e; return *e;}

  /** Sorts the generators in ascending lex order. */
  void sortLexAscending();

  /** const_iterator doesn't have all it needs to be a proper STL
	  iterator. Extend it if that becomes necessary. */
  class const_iterator {
  public:
  const_iterator(const Word* term, size_t wordsPerTerm):
	_term(term), _wordsPerTerm(wordsPerTerm) {
	}
	const_iterator(const const_iterator&) = default;

	const Word* operator*() const {return _term;}
	const_iterator operator++() {_term += _wordsPerTerm; return *this;}
	const_iterator operator--() {_term -= _wordsPerTerm; return *this;}

	bool operator==(const const_iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return _term == it._term;
	}
	bool operator!=(const const_iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return _term != it._term;
	}

	ptrdiff_t operator-(const const_iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return (_term - it._term) / _wordsPerTerm;
	}
	const_iterator operator+(ptrdiff_t i) const {
	  return const_iterator(_term + i * _wordsPerTerm, _wordsPerTerm);
	}

	const_iterator& operator=(const const_iterator& it) {
	  ASSERT(it._wordsPerTerm == _wordsPerTerm);
	  _term = it._term;
	  return *this;
	}

  private:
	const Word* _term;
	const size_t _wordsPerTerm;
  };

  /** iterator doesn't have all it needs to be a proper STL
	  iterator. Extend it if that becomes necessary. */
  class iterator {
  public:
  iterator(Word* term, size_t wordsPerTerm):
	_term(term), _wordsPerTerm(wordsPerTerm) {
	}
	iterator(const iterator&) = default;

	operator const_iterator() const {
	  return const_iterator(_term, _wordsPerTerm);
	}

	Word* operator*() const {return _term;}
	iterator operator++() {_term += _wordsPerTerm; return *this;}
	iterator operator--() {_term -= _wordsPerTerm; return *this;}

	bool operator==(const iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return _term == it._term;
	}
	bool operator!=(const iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return _term != it._term;
	}

	ptrdiff_t operator-(const iterator& it) const {
	  ASSERT(_wordsPerTerm == it._wordsPerTerm);
	  return (_term - it._term) / _wordsPerTerm;
	}
	iterator operator+(ptrdiff_t i) const {
	  return iterator(_term + i * _wordsPerTerm, _wordsPerTerm);
	}
	iterator& operator=(const iterator& it) {
	  ASSERT(it._wordsPerTerm == _wordsPerTerm);
	  _term = it._term;
	  return *this;
	}

  private:
	Word* _term;
	const size_t _wordsPerTerm;
  };

  iterator begin()
  {return iterator(_memory, getWordsPerTerm());}
  const_iterator begin() const
  {return const_iterator(_memory, getWordsPerTerm());}

  iterator end()
  {return iterator(_memoryEnd, getWordsPerTerm());}
  const_iterator end() const
  {return const_iterator(_memoryEnd, getWordsPerTerm());}

  /** Returns true if the internal invariants of ideal are
   satisfied. Useful for debugging and tests. */
  bool isValid() const;

 private:
  RawSquareFreeIdeal(); // Not available
  RawSquareFreeIdeal(const RawSquareFreeIdeal&); // Not available

  size_t _varCount;
  size_t _wordsPerTerm;
  size_t _genCount;
  Word* _memoryEnd;
  Word _memory[1]; // variable size array
};

/** Allocates object with enough memory for capacity generators in
	varCount variables. Pointer must be deallocated using
	deleteRawSquareFreeIdeal. */
RawSquareFreeIdeal* newRawSquareFreeIdeal(size_t varCount, size_t capacity);

/** Allocates a copy of ideal with no extra capacity. */
RawSquareFreeIdeal* newRawSquareFreeIdeal(const RawSquareFreeIdeal& ideal);

/** Allocates and returns an ideal based on str. The returned ideal
  must be deallocated using deleteRawSquareFreeIdeal. str is parsed
  with a generator on each line as parsed by newTermParse. Results are
  undefined if str does not have that format.

  The parsing is inefficient and is intended for constructing ideals in
  testing. */
RawSquareFreeIdeal* newRawSquareFreeIdealParse(const char* str);

/** Deallocates memory returned by newRawSquareFreeIdeal(). */
void deleteRawSquareFreeIdeal(RawSquareFreeIdeal* ideal);

inline ostream& operator<<(ostream& out, const RawSquareFreeIdeal& ideal) {
  ideal.print(out);
  return out;
}

inline Word* RawSquareFreeIdeal::getGenerator(size_t index) {
  ASSERT(index < getGeneratorCount());
  return _memory + index * getWordsPerTerm();
}

inline const Word* RawSquareFreeIdeal::getGenerator(size_t index) const {
  ASSERT(index < getGeneratorCount());
  return _memory + index * getWordsPerTerm();
}

inline Word* RawSquareFreeIdeal::getGeneratorUnsafe(size_t index) {
  // no assert to check index is valid as this method specifically
  // allows out-of-bounds access.
  return _memory + index * getWordsPerTerm();
}

inline const Word* RawSquareFreeIdeal::getGeneratorUnsafe(size_t index) const {
  // no assert to check index is valid as this method specifically
  // allows out-of-bounds access.
  return _memory + index * getWordsPerTerm();
}

#endif
