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

#ifdef DEBUG
#include "../error.h"

// A DEBUG build of libfrobby calls frobbyMain, which only main.cpp
// defines, so the unit tests need one to link.
int frobbyMain(int, const char**) {
  reportInternalError("frobbyMain called from the unit tests.");
  return 1;
}
#endif

int main() {
  return runAllTests() ? 0 : 1;
}
