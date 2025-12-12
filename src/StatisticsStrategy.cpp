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
#include "StatisticsStrategy.h"

#include "Slice.h"

#include <cmath>

StatisticsStrategy::StatisticsStrategy(SliceStrategy* strategy, FILE* out):
  _strategy(strategy),
  _out(out),
  _internalTracker("internal nodes"),
  _leafTracker("leaf nodes") {
  ASSERT(strategy != 0);
}

StatisticsStrategy::~StatisticsStrategy() {
}

void StatisticsStrategy::run(const Ideal& ideal) {
  _strategy->run(ideal);

  fputs("**** Slice Algorithm Statistics ****\n", _out);
  _internalTracker.printReport(_out);
  _leafTracker.printReport(_out);
}

bool StatisticsStrategy::processSlice
(TaskEngine& tasks, unique_ptr<Slice> slice) {
  _internalTracker.preliminaryRecord(*slice);
  _leafTracker.preliminaryRecord(*slice);

  bool wasBaseCase = _strategy->processSlice(tasks, std::move(slice));

  if (wasBaseCase)
    _leafTracker.commitRecord();
  else
    _internalTracker.commitRecord();

  return wasBaseCase;
}

void StatisticsStrategy::setUseIndependence(bool use) {
  _strategy->setUseIndependence(use);
}

void StatisticsStrategy::setUseSimplification(bool use) {
  _strategy->setUseSimplification(use);
}

bool StatisticsStrategy::getUseSimplification() const {
  return _strategy->getUseSimplification();
}

void StatisticsStrategy::freeSlice(unique_ptr<Slice> slice) {
  _strategy->freeSlice(std::move(slice));
}

StatisticsStrategy::StatTracker::StatTracker(const string& title):
  _title(title) {
}

void StatisticsStrategy::StatTracker::preliminaryRecord(const Slice& slice) {
  _prelimIdealGenCount = slice.getIdeal().getGeneratorCount();
  _prelimSubGenCount = slice.getSubtract().getGeneratorCount();
  _prelimVarCount = slice.getVarCount();
}

void StatisticsStrategy::StatTracker::commitRecord() {
  ++_nodeCount;

  _idealGenSum += _prelimIdealGenCount;
  _subGenSum += _prelimSubGenCount;
  _varSum += _prelimVarCount;

  size_t genCountLog2 = 0;
  if (_prelimIdealGenCount > 0)
    genCountLog2 = (size_t)(log((double)_prelimIdealGenCount) / log((double)2));
  _nodesByGenCount[genCountLog2] += 1;
}

void StatisticsStrategy::StatTracker::printReport(FILE* out) const {
  fprintf(out, "|-%s:\n", _title.c_str());

  gmp_fprintf(out, " | %Zd nodes\n", getNodeCount().get_mpz_t());
  fprintf(out, " | %f generators of ideal on avg.\n", getAvgIdealGenCount());
  fprintf(out, " | %f generators of subtract on avg.\n", getAvgSubGenCount());
  fprintf(out, " | %f variables of ambient ring on avg.\n", getAvgVarCount());

  if (!_nodesByGenCount.empty()) {
    fputs(" |- nodes by log base 2 of ideal generator count:\n", out);
    size_t genCountLog2 = _nodesByGenCount.rbegin()->first;
    while (true) {
      mpz_class nodeCount = 0;
      map<size_t, mpz_class>::const_iterator it =
        _nodesByGenCount.find(genCountLog2);
      if (it != _nodesByGenCount.end())
        nodeCount = it->second;

      gmp_fprintf(out, "  | %Zd nodes has log2(#gens) = %Zd.\n",
                  nodeCount.get_mpz_t(),
                  mpz_class(genCountLog2).get_mpz_t());

      if (genCountLog2 == 0)
        break;
      --genCountLog2;
    }
  }
}

const mpz_class& StatisticsStrategy::StatTracker::getNodeCount() const {
  return _nodeCount;
}

double StatisticsStrategy::StatTracker::getAvgIdealGenCount() const {
  if (_nodeCount == 0)
    return 0.0;
  else {
    mpq_class q = mpq_class(_idealGenSum) / _nodeCount;
    return q.get_d();
  }
}

double StatisticsStrategy::StatTracker::getAvgSubGenCount() const {
  if (_nodeCount == 0)
    return 0.0;
  else {
    mpq_class q = mpq_class(_subGenSum) / _nodeCount;
    return q.get_d();
  }
}

double StatisticsStrategy::StatTracker::getAvgVarCount() const {
  if (_nodeCount == 0)
    return 0.0;
  else {
    mpq_class q = mpq_class(_varSum) / _nodeCount;
    return q.get_d();
  }
}
