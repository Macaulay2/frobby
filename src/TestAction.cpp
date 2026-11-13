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
#include "TestAction.h"

#include "error.h"
#include "test/TestRunner.h"

TestAction::TestAction():
  Action
(staticGetName(),
 "Perform tests of C++ Frobby library interface.",
 "Perform tests of C++ Frobby library interface and other internal tests.",
 false) {
}

void TestAction::obtainParameters(vector<Parameter*>& parameters) {
}

void TestAction::perform() {
  if (!runAllTests())
    reportError("A unit test failed.");
}

const char* TestAction::staticGetName() {
  return "test";
}

bool TestAction::displayAction() const {
  return false;
}
