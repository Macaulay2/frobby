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
#include "BigIdeal.h"

#include "VarNames.h"
#include "TermTranslator.h"
#include "Ideal.h"
#include "VarSorter.h"
#include "RawSquareFreeTerm.h"
#include "SquareFreeIdeal.h"
#include <sstream>

class OffsetTermCompare {
public:
  OffsetTermCompare(const BigIdeal& ideal): _ideal(ideal) {
  }
  OffsetTermCompare(const OffsetTermCompare&) = default;

  bool operator()(size_t aa, size_t bb) const {
    const vector<mpz_class>& a = _ideal.getTerm(aa);
    const vector<mpz_class>& b = _ideal.getTerm(bb);

    ASSERT(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) {
      if (a[i] > b[i])
        return true;
      if (a[i] < b[i])
        return false;
    }
    return false;
  }

private:
  void operator=(const OffsetTermCompare&); // To make this inaccessible.

  const BigIdeal& _ideal;
};

BigIdeal::BigIdeal() {
}

BigIdeal::BigIdeal(const VarNames& names):
  _names(names) {
}

void BigIdeal::insert(const Ideal& ideal) {
  reserve(getGeneratorCount() + ideal.getGeneratorCount());

  Ideal::const_iterator it = ideal.begin();
  for (; it != ideal.end(); ++it) {
    newLastTerm();
    for (size_t var = 0; var < _names.getVarCount(); ++var)
      getLastTermExponentRef(var) = (*it)[var];
  }
}

void BigIdeal::insert(const Ideal& ideal,
                      const TermTranslator& translator) {
  reserve(getGeneratorCount() + ideal.getGeneratorCount());

  Ideal::const_iterator it = ideal.begin();
  for (; it != ideal.end(); ++it) {
    newLastTerm();
    for (size_t var = 0; var < _names.getVarCount(); ++var)
      getLastTermExponentRef(var) = translator.getExponent(var, (*it)[var]);
  }
}

void BigIdeal::insert(const SquareFreeIdeal& ideal) {
  reserve(getGeneratorCount() + ideal.getGeneratorCount());

  SquareFreeIdeal::const_iterator it = ideal.begin();
  for (; it != ideal.end(); ++it) {
    newLastTerm();
    for (size_t var = 0; var < _names.getVarCount(); ++var)
      getLastTermExponentRef(var) = SquareFreeTermOps::getExponent(*it, var);
  }
}

void BigIdeal::insert(const vector<mpz_class>& term) {
  newLastTerm();
  getLastTermRef() = term;
}

void BigIdeal::renameVars(const VarNames& names) {
  ASSERT(names.getVarCount() == _names.getVarCount());
  _names = names;
}

void BigIdeal::newLastTerm() {
  if (_terms.size() == _terms.capacity())
    reserve(getVarCount() * _terms.size());

  _terms.resize(_terms.size() + 1);
  _terms.back().resize(_names.getVarCount());
}

void BigIdeal::reserve(size_t capacity) {
  // std::vector can do reallocations by itself, but the version here
  // is much faster.
  if (capacity <= _terms.capacity())
    return;

  // We grow the capacity at a rate of getVarCount() instead of a
  // doubling because each *used* entry allocates at least
  // getVarCount() memory anyway, so we will still only use at most
  // double the memory than we need.
  //
  // We make tmp have the capacity we need, then we move the data
  // entry by entry to tmp, and then we swap tmp and _terms. This
  // will also swap the excess capacity into _terms. If allowed to
  // reallocate by itself, the implementation of STL (GCC 3.4.4) I'm
  // using will *copy* the data instead of swapping it, which is
  // very bad.
  vector<vector<mpz_class> > tmp;
  size_t newCapacity = getVarCount() * _terms.size();
  if (capacity > newCapacity)
    newCapacity = capacity;

  tmp.reserve(newCapacity);
  tmp.resize(_terms.size());

  size_t size = _terms.size();
  for (size_t i = 0; i < size; ++i)
    tmp[i].swap(_terms[i]);
  tmp.swap(_terms);
}

void BigIdeal::getLcm(vector<mpz_class>& lcm) const {
  lcm.clear();
  lcm.resize(getVarCount());

  for (vector<vector<mpz_class> >::const_iterator it = _terms.begin();
       it != _terms.end(); ++it)
    for (size_t var = 0; var < getVarCount(); ++var)
      if (lcm[var] < (*it)[var])
        lcm[var] = (*it)[var];
}

bool BigIdeal::operator==(const BigIdeal& b) const {
  return _terms == b._terms;
}

void BigIdeal::projectVar(size_t var) {
  ASSERT(var < getVarCount());

  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
    _terms[gen].erase(_terms[gen].begin() + var);
  _names.projectVar(var);
}

bool BigIdeal::operator<(const BigIdeal& ideal) const {
  if (getNames() < ideal.getNames())
    return true;
  if (ideal.getNames() < getNames())
    return false;

  for (size_t t = 0; t < _terms.size(); ++t) {
    if (t == ideal._terms.size())
      return true;

    const vector<mpz_class>& a = _terms[t];
    const vector<mpz_class>& b = ideal._terms[t];

    ASSERT(a.size() == b.size());

    for (size_t i = 0; i < a.size(); ++i) {
      if (a[i] > b[i])
        return true;
      if (a[i] < b[i])
        return false;
    }
  }

  return false;
}

bool BigIdeal::empty() const {
  return _terms.empty();
}

bool BigIdeal::containsIdentity() const {
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen) {
    for (size_t var = 0; var < getVarCount(); ++var)
      if (_terms[gen][var] != 0)
        goto notIdentity;
    return true;
  notIdentity:;
  }
  return false;
}

bool BigIdeal::contains(const vector<mpz_class>& term) const {
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen) {
    for (size_t var = 0; var < getVarCount(); ++var)
      if (_terms[gen][var] > term[var])
        goto notDivisor;
    return true;
  notDivisor:;
  }
  return false;
}

void BigIdeal::clear() {
  _terms.clear();
}

void BigIdeal::clearAndSetNames(const VarNames& names) {
  clear();
  _names = names;
}

bool BigIdeal::addVarToClearedIdeal(const char* var) {
  ASSERT(getGeneratorCount() == 0);

  return _names.addVar(var);
}

void BigIdeal::eraseVar(size_t varToErase) {
  ASSERT(varToErase < getVarCount());

  VarNames newNames;
  for (size_t var = 0; var < getVarCount(); ++var)
    if (var != varToErase)
      newNames.addVar(_names.getName(var));

  try {
    _names = newNames;
    for (size_t term = 0; term < getGeneratorCount(); ++term)
      _terms[term].erase(_terms[term].begin() + varToErase);
  } catch (...) {
    // To leave in valid state, which requires that _names has the same
    // number of variables as each generator.
    clear();
    throw;
  }
}

const VarNames& BigIdeal::getNames() const {
  return _names;
}

void BigIdeal::deform() {
  for (size_t gen = 0; gen < getGeneratorCount(); ++gen)
    for (size_t var = 0; var < getVarCount(); ++var)
      if (_terms[gen][var] > 0)
        _terms[gen][var] = _terms[gen][var] * getGeneratorCount() + gen;
}

void BigIdeal::takeRadical() {
  vector<vector<mpz_class> >::iterator end = _terms.end();
  for (vector<vector<mpz_class> >::iterator it = _terms.begin();
       it != end; ++it)
    for (size_t var = 0; var < getVarCount(); ++var)
      if ((*it)[var] > 1)
        (*it)[var] = 1;
}

void BigIdeal::sortGeneratorsUnique() {
  sortGenerators();
  vector<vector<mpz_class> >::iterator newEnd =
    unique(_terms.begin(), _terms.end());
  _terms.erase(newEnd, _terms.end());
}

void BigIdeal::sortGenerators() {
  size_t size = _terms.size();
  vector<size_t> sortedOffsets(size);
  for (size_t term = 0; term < size; ++term)
    sortedOffsets[term] = term;

  std::sort(sortedOffsets.begin(), sortedOffsets.end(),
            OffsetTermCompare(*this));

  vector<vector<mpz_class> > sorted;
  sorted.reserve(_terms.capacity());
  sorted.resize(size);
  for (size_t term = 0; term < size; ++term)
    sorted[term].swap(_terms[sortedOffsets[term]]);

  _terms.swap(sorted);
}

void BigIdeal::sortVariables() {
  VarSorter sorter(_names);
  sorter.getOrderedNames(_names);
  for (size_t i = 0; i < _terms.size(); ++i)
    sorter.permute(_terms[i]);
}

void BigIdeal::swap(BigIdeal& ideal) {
  _terms.swap(ideal._terms);
  _names.swap(ideal._names);
}

void BigIdeal::print(FILE* file) const {
  ostringstream out;
  out << *this;
  fputs(out.str().c_str(), file);
}

void BigIdeal::print(ostream& out) const {
  out << "/---- BigIdeal of " << _terms.size() << " terms:\n";
  for (vector<vector<mpz_class> >::const_iterator it = _terms.begin();
       it != _terms.end(); ++it) {
    for (vector<mpz_class>::const_iterator entry = it->begin();
         entry != it->end(); ++entry)
      out << *entry << ' ';
    out << '\n';
  }
  out << "----/ End of list.\n";
}

const mpz_class& BigIdeal::getExponent(size_t term, size_t var) const {
  ASSERT(term < _terms.size());
  ASSERT(var < _names.getVarCount());

  return _terms[term][var];
}

mpz_class& BigIdeal::getExponent(size_t term, size_t var) {
  ASSERT(term < _terms.size());
  ASSERT(var < _names.getVarCount());

  return _terms[term][var];
}

void BigIdeal::setExponent(size_t term, size_t var, const mpz_class& exp) {
  ASSERT(term < _terms.size());
  ASSERT(var < _names.getVarCount());

  _terms[term][var] = exp;
}

bool BigIdeal::bigTermCompare(const vector<mpz_class>& a,
                  const vector<mpz_class>& b) {
  ASSERT(a.size() == b.size());
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i] > b[i])
      return true;
    if (a[i] < b[i])
      return false;
  }
  return false;
}

ostream& operator<<(ostream& out, const BigIdeal& ideal) {
  ideal.print(out);
  return out;
}

ostream& operator<<(ostream& out, const vector<BigIdeal>& ideals) {
  out << "List of " << ideals.size() << " ideals:\n";
  for (size_t i = 0; i < ideals.size(); ++i)
    out << ideals[i];
  return out;
}
