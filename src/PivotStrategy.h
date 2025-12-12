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
#ifndef PIVOT_EULER_STRATEGY_GUARD
#define PIVOT_EULER_STRATEGY_GUARD

#include <vector>
#include <ostream>
#include <string>

class EulerState;
class PivotEulerAlg;

/** A pivot selection strategy for the Euler algorithm. */
class PivotStrategy {
public:
  virtual ~PivotStrategy() {}

  /** Split state. Returns null if state was a base case. Otherwise
   state and the return value become the two substates. divCounts[var]
   must be the number of generators that div divides. */
  virtual EulerState* doPivot(EulerState& state, const size_t* divCounts) = 0;

  /** Prints the name of the strategy to out. */
  virtual void getName(ostream& out) const = 0;

  /** Call this function once when computation is complete. */
  virtual void computationCompleted(const PivotEulerAlg& alg) = 0;

  /** Returns true if transposing state is adviced. */
  virtual bool shouldTranspose(const EulerState& state) const = 0;

 protected:
  PivotStrategy() {}

 private:
  PivotStrategy(const PivotStrategy&); // not available
  void operator=(const PivotStrategy&); // not available
};

unique_ptr<PivotStrategy> newDefaultPivotStrategy();

unique_ptr<PivotStrategy> newStdPivotStrategy(const string& name);
unique_ptr<PivotStrategy> newGenPivotStrategy(const string& name);
unique_ptr<PivotStrategy> newHybridPivotStrategy
  (unique_ptr<PivotStrategy> stdStrat, unique_ptr<PivotStrategy> genStrat);
unique_ptr<PivotStrategy> newDebugPivotStrategy(unique_ptr<PivotStrategy> strat,
											  FILE* out);
unique_ptr<PivotStrategy> newStatisticsPivotStrategy
(unique_ptr<PivotStrategy> strat, FILE* out);

#endif
