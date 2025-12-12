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
#ifndef TERM_ORDER_GUARD
#define TERM_ORDER_GUARD

#include "Term.h"

class TermPredicate {
 public:
  TermPredicate(size_t varCount = 0);
  virtual ~TermPredicate();

  bool operator()(const Term& a, const Term& b) const {
    ASSERT(a.getVarCount() == getVarCount());
    ASSERT(b.getVarCount() == getVarCount());
    return doPredicate(a.begin(), b.begin());
  }

  bool operator()(const Term& a, const Exponent* b) const {
    ASSERT(a.getVarCount() == getVarCount());
    ASSERT(b != 0 || getVarCount() == 0);
    return doPredicate(a.begin(), b);
  }

  bool operator()(const Exponent* a, const Term& b) const {
    ASSERT(b.getVarCount() == getVarCount());
    ASSERT(a != 0 || getVarCount() == 0);
    return doPredicate(a, b.begin());
  }

  bool operator()(const Exponent* a, const Exponent* b) const {
    ASSERT(a != 0 || getVarCount() == 0);
    ASSERT(b != 0 || getVarCount() == 0);
    return doPredicate(a, b);
  }

  size_t getVarCount() const {return _varCount;}
  void setVarCount(size_t varCount) {_varCount = varCount;}

 private:
  virtual bool doPredicate(const Exponent* a, const Exponent* b) const = 0;

  size_t _varCount;
};

/** Adapter for TermPredicate which allows it to be used as a
 predicate in STL. */
class StlTermPredicate {
 public:
  StlTermPredicate(const TermPredicate& pred): _pred(pred) {}
  template<class T>
  bool operator()(const T& a, const T& b) const {return _pred(a, b);}
 private:
  const TermPredicate& _pred;
};

/** Returns the predicate whose name has the given prefix. */
unique_ptr<TermPredicate> createTermPredicate(const string& prefix,
                                            size_t varCount = 0);


/** Indicates how a relates to b according to the lexicographic term
 order where \f$x_1>\cdots>x_n\f$. Returns -1 if a < b, returns 0 if a
 = b and returns 1 if a > b. As an example \f$x^(0,0) < x^(0,1) <
 x^(1,0)\f$. */
int lexCompare(const Exponent* a, const Exponent* b, size_t varCount);
int lexCompare(const Term& a, const Term& b);

/** Indicates how a relates to b according to the reverse
 lexicographic term order where \f$x_1<\cdots<x_n\f$. Returns -1 if a
 < b, returns 0 if a = b and returns 1 if a > b. As an example
 \f$x^(0,0) < x^(1,0) < x^(0,1)\f$. */
int reverseLexCompare(const Exponent* a, const Exponent* b, size_t varCount);

/** Returns whether the entries of a are equal to the entries of b. */
bool equals(const Exponent* a, const Exponent* b, size_t varCount);



/** A predicate that sorts terms according to lexicographic order. */
class LexComparator : public TermPredicate {
 public:
  LexComparator(size_t varCount = 0);

  static const char* staticGetName();

  private:
  virtual bool doPredicate(const Exponent* a,
                           const Exponent* b) const {
    return lexCompare(a, b, getVarCount()) < 0;
  }
};

/** A predicate that sorts according to reverse lexicographic order. */
class ReverseLexComparator : public TermPredicate {
 public:
  ReverseLexComparator(size_t varCount = 0);

  static const char* staticGetName();

  private:
  virtual bool doPredicate(const Exponent* a,
                           const Exponent* b) const {
    return reverseLexCompare(a, b, getVarCount()) < 0;
  }
};

/** A predicate that sorts terms in weakly ascending order according
 to degree of the specified variable. */
class SingleDegreeComparator : public TermPredicate {
 public:
  SingleDegreeComparator(size_t var, size_t varCount = 0);

 private:
  virtual bool doPredicate(const Exponent* a,
                           const Exponent* b) const {
    return a[_var] < b[_var];
  }

  size_t _var;
};

/** A predicate that sorts terms in weakly descending order according
 to degree of the specified variable. */
class ReverseSingleDegreeComparator : public TermPredicate {
 public:
  ReverseSingleDegreeComparator(size_t var, size_t varCount = 0);

 private:
  virtual bool doPredicate(const Exponent* a,
                           const Exponent* b) const {
    return a[_var] > b[_var];
  }

  size_t _var;
};

/** A predicate that compares for equality. */
class EqualsPredicate : public TermPredicate {
 public:
  EqualsPredicate(size_t varCount = 0);

 private:
  virtual bool doPredicate(const Exponent* a,
                           const Exponent* b) const {
    return equals(a, b, getVarCount());
  }
};

#endif
