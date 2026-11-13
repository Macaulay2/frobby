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
#include "stdinc.h"
#include "TestRunner.h"

#include "TestCase.h"
#include "TestSorter.h"
#include "all.h"
#include "../DebugAllocator.h"
#include "../error.h"

bool TestRunner::visit(TestCase& test) {
  string qualifiedName = getPath() + test.getName();
  fputs(qualifiedName.c_str(), stdout);
  fputs(": ", stdout);

#ifdef DEBUG
  DebugAllocator::getSingleton().runTest(test, qualifiedName);
#else
  test.run(qualifiedName.c_str(), true);
#endif

  fputc('\n', stdout);
  fflush(stdout);
  return true;
}

bool runAllTests() {
  try {
    TestSorter sorter;
    GET_TEST_SUITE(root).accept(sorter);

    TestRunner runner;
    GET_TEST_SUITE(root).accept(runner);
    return true;
  } catch (const FrobbyException& e) {
    fputs(e.what(), stderr);
  } catch (const AssertException& e) {
    fputs(e.what(), stderr);
  }
  return false;
}
