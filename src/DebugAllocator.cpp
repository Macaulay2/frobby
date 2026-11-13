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
#include "DebugAllocator.h"

#include "main.h"
#include "error.h"
#include "Ideal.h"
#include "test/TestCase.h"
#include "IOHandler.h"
#include <limits>

// Must not include code below if not debugging. For one, then Frobby
// cannot be built as a library as this code calls main.
// The code only applies to debug builds anyway.
#ifdef DEBUG

static const size_t AllocationLimitsTryFirst = 500;
static const size_t AllocationLimitsTryLast = 200;
static const size_t AllocationLimitsStepRatio = 1000;

DebugAllocator& DebugAllocator::getSingleton() {
  static DebugAllocator singleton;
  return singleton;
}

void DebugAllocator::rewindInput() {
  if (_inputFile != "")
    if (!freopen(_inputFile.c_str(), "r", stdin))
      reportError("Could not open file \"" + _inputFile + "\" for input.");
}

/** @todo consider off-by-one conditions on the allocation limit
    conditions in this method.

    @todo at the end, make sure each identifiable allocation site has
    been hit. Some can be missed now when the stepsize is larger than
    1. This would require having a set of sites that have been hit.
*/
int DebugAllocator::runDebugMain(int argc, const char** argv) {
  processDebugOptions(argc, argv);

  // Run all the way through once to count the number of allocations
  // and to produce the correct output. Note that the number of
  // allocations on subsequent runs can be a bit less due to static
  // caches.
  rewindInput();
  _allocationCount = 0;
  int exitCode = frobbyMain(argc, argv);
  size_t maxAllocations = _allocationCount;
  if (_actionIsTest || !_debugAllocation)
    return exitCode;

  fclose(stdout); // Output has already been produced.

  fputs("DEBUG: Now debugging out-of-memory conditions.\n", stderr);
  fprintf(stderr, "DEBUG: There are %i allocations in total on this input.\n",
          (int)maxAllocations);

  // If maxAllocations is small then just try every possible limit.
  if (maxAllocations < AllocationLimitsTryFirst + AllocationLimitsTryLast) {
    fputs("DEBUG: This is few, so am now running through "
          "every possible condition.\n", stderr);
    for (size_t limit = 0; limit < maxAllocations; ++limit)
      runWithLimit(argc, argv, limit);
    return ExitCodeSuccess;
  }

  fprintf(stderr, "DEBUG: Trying the first %i allocations.\n",
          (int)AllocationLimitsTryFirst);
  for (size_t limit = 0; limit < AllocationLimitsTryFirst; ++limit)
    runWithLimit(argc, argv, limit);

  fprintf(stderr, "DEBUG: Trying the last %i allocations.\n",
          (int)AllocationLimitsTryLast);
  for (size_t limit = 0; limit < AllocationLimitsTryLast; ++limit)
    runWithLimit(argc, argv, maxAllocations - limit);

  size_t limitsLeft =
    maxAllocations - AllocationLimitsTryFirst - AllocationLimitsTryLast;
  size_t stepSize = limitsLeft / AllocationLimitsStepRatio + 1;
  fprintf(stderr,
          "DEBUG: Going through the %i remaining allocations "
          "with steps of size %i.\n",
          (int)limitsLeft, (int)stepSize);

  for (size_t limit = AllocationLimitsTryFirst;
       limit < maxAllocations - AllocationLimitsTryLast; limit += stepSize)
    runWithLimit(argc, argv, limit);

  return ExitCodeSuccess;
}

void DebugAllocator::runWithLimit(int argc, const char** argv, size_t limit) {
  if (_detailAllocation)
    fprintf(stderr, "DEBUG: Trying allocation limit of %i\n", (int)limit);

  // To make each run more similar.
  Ideal::clearStaticCache();

  _limitAllocation = true;
  _allocationLimit = limit;
  _allocationCount = 0;

  // We use this to distinguish genuinely running out of memory from
  // our artificial limit-induced throwing of bad_alloc.
  _expectBadAllocException = false;

  rewindInput();

  try {
    frobbyMain(argc, argv);
  } catch (const bad_alloc&) {
    if (!_expectBadAllocException)
      throw;
  }

  _limitAllocation = false;
}

void DebugAllocator::processDebugOptions(int& argc, const char**& argv) {
  const char* originalArgvZero = argv[0];

  while (true) {
    if (argc <= 1 || argv[1][0] != '_')
      break;

    string option(argv[1] + 1);
    ++argv;
    --argc;

    if (option == "debugAlloc")
      _debugAllocation = true;
    else if (option == "detailAlloc")
      _detailAllocation = true;
    else if (option == "input") {
      if (argc <= 1)
        reportError("Debug option _input requries an argument.");

      _inputFile = argv[1];
      ++argv;
      --argc;
    } else
      reportError("Unknown debug option \"" + option + "\".");
  }

  if (argc >= 2 && string(argv[1]) == "test")
    _actionIsTest = true;

  argv[0] = originalArgvZero;
}

void DebugAllocator::runTest(TestCase& test, const string& name) {
  test.run(name.c_str(), true);

  if (!_debugAllocation)
    return;

  _limitAllocation = true;

  for (_allocationLimit = 0; _allocationLimit < numeric_limits<size_t>::max();
       ++_allocationLimit) {
    if (_detailAllocation)
      fprintf(stderr, "DEBUG: Trying test allocation limit of %i\n",
              (int)_allocationLimit);

    // To make each run more similar.
    Ideal::clearStaticCache();

    _allocationCount = 0;
    _expectBadAllocException = false;

    rewindInput();

    try {
      test.run(name.c_str(), false);

      // At this point test has completed within allocation limit.
      break;
    } catch (const bad_alloc&) {
      if (!_expectBadAllocException)
        throw;
      // Continue and try next limit.
    }
  }

  _limitAllocation = false;
}

void* DebugAllocator::allocate(size_t size) {
  return allocate(size, 0, 0);
}

void* DebugAllocator::allocate(size_t size,
                               const char* file,
                               size_t lineNumber) {
  if (_detailAllocation) {
    if (file != 0)
      fprintf(stderr, "DEBUG: Allocating %i bytes at %s:%i.\n",
              (int)size, file, (int)lineNumber);
    if (file == 0)
      fprintf(stderr, "DEBUG: Allocating %i bytes at an unknown point.\n",
              (int)size);
  }

  if (_limitAllocation && _allocationCount >= _allocationLimit) {
    if (_detailAllocation)
      fputs("DEBUG: Throwing bad_alloc due to artifically imposed limit.\n",
            stderr);
    _expectBadAllocException = true;
    throw bad_alloc();
  }

  ++_allocationCount;
  void* p = malloc(size);
  if (p == 0)
	throw bad_alloc();
  return p;
}

DebugAllocator::DebugAllocator():
  _debugAllocation(false),
  _detailAllocation(false),
  _limitAllocation(false),
  _expectBadAllocException(false),
  _actionIsTest(false),
  _allocationCount(0),
  _allocationLimit(0) {
}

void* operator new(size_t size, const char* file, size_t lineNumber) {
  return DebugAllocator::getSingleton().allocate(size, file, lineNumber);
}

void* operator new[](size_t size, const char* file, size_t lineNumber) {
  return DebugAllocator::getSingleton().allocate(size, file, lineNumber);
}

void* operator new(size_t size) {
  return DebugAllocator::getSingleton().allocate(size);
}

void* operator new[](size_t size) {
  return DebugAllocator::getSingleton().allocate(size);
}

void operator delete(void* buffer) noexcept {
  free(buffer);
}

void operator delete[](void* buffer) noexcept {
  free(buffer);
}

/** Note that these overloads to operator delete are strictly
 necessary, even though they will only be called in the very special
 circumstance where an exception gets thrown while an object
 allocated using the new above has been *partially* constructed.

 However, when that happens, this overload gets called. The compiler
 will happily substitute its own automatically generated version,
 which does *not* deallocate the memory, leading to a memory leak.

 It is also important that these overloads are declared in each
 translation unit, as otherwise the compiler generated do-nothing
 version will be used in those translation units that have no such
 declaration. To me this all falls into the category of "Insane but
 true".

 @todo Make sure this doc. is displayed and apply it to the array
 operator delete too.
*/
void operator delete(void* buffer, const char* file, size_t line) {
  free(buffer);
}

void operator delete[](void* buffer, const char* file, size_t line) {
  free(buffer);
}

#endif
