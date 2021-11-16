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
#include "DebugStrategy.h"

#include "Slice.h"

DebugStrategy::DebugStrategy(SliceStrategy* strategy, FILE* out):
  _strategy(strategy),
  _out(out) {
  ASSERT(strategy != 0);
  fputs("DEBUG: Starting slice computation.\n", _out);
}

DebugStrategy::~DebugStrategy() {
}

void DebugStrategy::run(const Ideal& ideal) {
  fputs("DEBUG: Starting Slice Algorithm. Input ideal is:\n", _out);
  ideal.print(_out);

  _strategy->run(ideal);

  fputs("DEBUG: Slice computation done.\n", _out);
}

bool DebugStrategy::processSlice(TaskEngine& tasks, auto_ptr<Slice> slice) {
  fputs("DEBUG: Processing slice.\n", _out);
  slice->print(stderr);
  bool wasBaseCase = _strategy->processSlice(tasks, std::move(slice));
  if (wasBaseCase)
    fputs("DEBUG: Determined that slice is base case.\n", _out);
  else
    fputs("DEBUG: Determined that slice is not base case.\n", _out);
  return wasBaseCase;
}

void DebugStrategy::setUseIndependence(bool use) {
  if (use)
    fputs("DEBUG: Turning on independence splits.", _out);
  else
    fputs("DEBUG: Turning off independence splits.", _out);
  _strategy->setUseIndependence(use);
}

void DebugStrategy::setUseSimplification(bool use) {
  if (use)
    fputs("DEBUG: Turning on simplification.", _out);
  else
    fputs("DEBUG: Turning off simplification.", _out);
  _strategy->setUseSimplification(use);
}

bool DebugStrategy::getUseSimplification() const {
  return _strategy->getUseSimplification();
}

void DebugStrategy::freeSlice(auto_ptr<Slice> slice) {
  fputs("DEBUG: Freeing slice.\n", _out);
  _strategy->freeSlice(std::move(slice));
}
