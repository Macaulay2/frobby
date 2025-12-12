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
#include "BigattiState.h"

#include "BigattiHilbertAlgorithm.h"
#include <sstream>

BigattiState::BigattiState(BigattiHilbertAlgorithm* algorithm,
                           const Ideal& ideal, const Term& multiply):
  _algorithm(algorithm),
  _ideal(ideal),
  _multiply(multiply) {
  ASSERT(_algorithm != 0);
  ASSERT(_ideal.getVarCount() == _multiply.getVarCount());
}

const Ideal& BigattiState::getIdeal() const {
  return _ideal;
}

const Term& BigattiState::getMultiply() const {
  return _multiply;
}

Ideal& BigattiState::getIdeal() {
  return _ideal;
}

Term& BigattiState::getMultiply() {
  return _multiply;
}

size_t BigattiState::getVarCount() const {
  ASSERT(_multiply.getVarCount() == _ideal.getVarCount());
  return _ideal.getVarCount();
}

Exponent BigattiState::getMedianPositiveExponentOf(size_t var) {
  ASSERT(var < getVarCount());

  _ideal.singleDegreeSort(var);
  Ideal::const_iterator end = _ideal.end();
  Ideal::const_iterator begin = _ideal.begin();
  while ((*begin)[var] == 0) {
    ++begin;
    ASSERT(begin != end);
  }

  // This picks the lower median in case of a tie.
  Exponent median = (*(begin + (distance(begin, end) - 1) / 2))[var];
  ASSERT(median > 0);
  return median;
}

size_t BigattiState::getTypicalExponent(size_t& var, Exponent& exp) {
  return _ideal.getTypicalExponent(var, exp);
}

void BigattiState::singleDegreeSort(size_t var) {
  _ideal.singleDegreeSort(var);
}

void BigattiState::colonStep(const Term& pivot) {
  ASSERT(pivot.getVarCount() == getVarCount());
  _ideal.colonReminimize(pivot);
  _multiply.product(_multiply, pivot);
}

void BigattiState::addStep(const Term& pivot) {
  ASSERT(pivot.getVarCount() == getVarCount());
  _ideal.insertReminimize(pivot);
}

void BigattiState::run(TaskEngine& tasks) {
  _algorithm->processState(unique_ptr<BigattiState>(this));
}

void BigattiState::dispose() {
  _algorithm->freeState(unique_ptr<BigattiState>(this));
}

void BigattiState::print(FILE* out) {
  ostringstream str;
  print(str);
  fputs(str.str().c_str(), out);
}

void BigattiState::print(ostream& out) {
  out << "BigattiState(multiply: " << _multiply << "\n"
      << _ideal << ")\n";
}
