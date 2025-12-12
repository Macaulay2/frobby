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
#include "TermPredicate.h"

#include "NameFactory.h"

TermPredicate::TermPredicate(size_t varCount):
  _varCount(varCount) {
}

TermPredicate::~TermPredicate() {
}

namespace {
  typedef NameFactory<TermPredicate> PredFactory;
  PredFactory getPredFactory() {
    PredFactory factory("Term ordering");

    nameFactoryRegister<LexComparator>(factory);
    nameFactoryRegister<ReverseLexComparator>(factory);

    return factory;
  }
}

unique_ptr<TermPredicate> createTermPredicate(const string& prefix, size_t varCount) {
  unique_ptr<TermPredicate> pred = createWithPrefix(getPredFactory(), prefix);
  ASSERT(pred.get() != 0);
  pred->setVarCount(varCount);
  return pred;
}

int lexCompare(const Exponent* a, const Exponent* b, size_t varCount) {
  ASSERT(a != 0 || varCount == 0);
  ASSERT(b != 0 || varCount == 0);

  for (size_t var = 0; var < varCount; ++var) {
    if (a[var] == b[var])
      continue;

    if (a[var] < b[var])
      return -1;
    else
      return 1;
  }
  return 0;
}

int lexCompare(const Term& a, const Term& b) {
  ASSERT(a.getVarCount() == b.getVarCount());
  return lexCompare(a.begin(), b.begin(), a.getVarCount());
}

int reverseLexCompare(const Exponent* a, const Exponent* b, size_t varCount) {
  ASSERT(a != 0 || varCount == 0);
  ASSERT(b != 0 || varCount == 0);
  return -lexCompare(a, b, varCount);
}

bool equals(const Exponent* a, const Exponent* b, size_t varCount) {
  ASSERT(a != 0 || varCount == 0);
  ASSERT(b != 0 || varCount == 0);

  for (size_t var = 0; var < varCount; ++var)
    if (a[var] != b[var])
      return false;
  return true;
}

LexComparator::LexComparator(size_t varCount):
  TermPredicate(varCount) {
}

const char* LexComparator::staticGetName() {
  return "lex";
}

ReverseLexComparator::ReverseLexComparator(size_t varCount):
  TermPredicate(varCount) {
}

const char* ReverseLexComparator::staticGetName() {
  return "revlex";
}

SingleDegreeComparator::SingleDegreeComparator(size_t var, size_t varCount):
  TermPredicate(varCount),
  _var(var) {
}

ReverseSingleDegreeComparator::ReverseSingleDegreeComparator(size_t var,
                                                             size_t varCount):
  TermPredicate(varCount),
  _var(var) {
}

EqualsPredicate::EqualsPredicate(size_t varCount):
  TermPredicate(varCount) {
}
