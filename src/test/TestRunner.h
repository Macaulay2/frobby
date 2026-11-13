/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 Bjarke Hammersholt Roune (www.broune.com)

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
#ifndef TEST_RUNNER_GUARD
#define TEST_RUNNER_GUARD

#include "TestQualifier.h"

class TestCase;

/** Runs every test in forward order. */
class TestRunner : public TestQualifier {
  virtual bool visit(TestCase& test);
};

/** Sorts and runs every registered test. Returns false after printing
    the error to standard error if a test fails. */
bool runAllTests();

#endif
