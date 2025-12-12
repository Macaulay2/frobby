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
#ifndef STATISTICS_STRATEGY
#define STATISTICS_STRATEGY

#include "SliceStrategy.h"

#include <map>

/** A wrapper for a SliceStrategy that collects statistics on what is
 going on, while delegating everything to the strategy being wrapped.
*/
class StatisticsStrategy : public SliceStrategy {
 public:
  /** Statistics are written to out, and every call is delegated to
   strategy. */
  StatisticsStrategy(SliceStrategy* strategy, FILE* out);
  virtual ~StatisticsStrategy();

  virtual void run(const Ideal& ideal);

  virtual bool processSlice(TaskEngine& tasks, unique_ptr<Slice> slice);

  virtual void setUseIndependence(bool use);
  virtual void setUseSimplification(bool use);
  virtual bool getUseSimplification() const;

  virtual void freeSlice(unique_ptr<Slice> slice);

 private:
  SliceStrategy* _strategy;
  FILE* _out;

  /** Tracks statistics on slices. */
  struct StatTracker {
    /** The title parameter indicates what is to be printed when
     calling printReport().
    */
    StatTracker(const string& title);

    /** Record information about slice, but store it only until this
     method is next called on this object.
     */
    void preliminaryRecord(const Slice& slice);

    /** Commit the most recent argument to preliminaryTrack
        permanently to the record. */
    void commitRecord();

    /** Print a report on statistics of the recorded slices to the
      file out. */
    void printReport(FILE* out) const;

    const mpz_class& getNodeCount() const;
    double getAvgIdealGenCount() const;
    double getAvgSubGenCount() const;
    double getAvgVarCount() const;

  private:
    string _title;

    size_t _prelimIdealGenCount;
    size_t _prelimSubGenCount;
    size_t _prelimVarCount;

    mpz_class _nodeCount;
    mpz_class _idealGenSum;
    mpz_class _subGenSum;
    mpz_class _varSum;

    /** _nodesByGenCount[l] records how many slices have been recorded
     whose ideal has approximately 2^l generators. */
    map<size_t, mpz_class> _nodesByGenCount;
  };

  StatTracker _internalTracker;
  StatTracker _leafTracker;
};

#endif
