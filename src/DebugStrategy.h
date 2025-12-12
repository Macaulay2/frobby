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
#ifndef DEBUG_STRATEGY
#define DEBUG_STRATEGY

#include "SliceStrategy.h"

/** A wrapper for a SliceStrategy that prints out what is going out
 for debugging purposes, while delegating everything to the strategy
 being wrapped.
*/
class DebugStrategy : public SliceStrategy {
 public:
  /** Debug information is written to out, and every call is delegated
   to strategy. */
  DebugStrategy(SliceStrategy* strategy, FILE* out);
  virtual ~DebugStrategy();

  virtual void run(const Ideal& ideal);

  bool processSlice(TaskEngine& tasks, unique_ptr<Slice> slice);

  virtual void setUseIndependence(bool use);
  virtual void setUseSimplification(bool use);
  virtual bool getUseSimplification() const;

  virtual void freeSlice(unique_ptr<Slice> slice);

 private:
  SliceStrategy* _strategy;
  FILE* _out;
};

#endif
