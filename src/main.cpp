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
#include "main.h"

#include "Action.h"
#include "DebugAllocator.h"
#include "error.h"
#include "display.h"

#include <ctime>
#include <cstdlib>
#include <exception>
#include <sys/types.h>
#include <unistd.h>

/** This function runs the Frobby console interface. the ::main
    function calls this function after having set up DEBUG-specific
    things, catching exceptions, setting the random seed and so on.
*/
int frobbyMain(int argc, const char** argv) {
  string prefix;
  if (argc > 1) {
    prefix = argv[1];
    --argc;
    ++argv;
  } else
    prefix = "help";

  const unique_ptr<Action> action(Action::createActionWithPrefix(prefix));
  action->parseCommandLine(argc - 1, argv + 1);
  action->perform();

  return ExitCodeSuccess;
}

/** A replacement for the default C++ built-in terminate() function. Do
 not call this method or cause it to be called.
*/
void frobbyTerminate() {
  fputs("INTERNAL ERROR: Something caused terminate() to be called. "
        "This should never happen.\nPlease contact the Frobby developers.\n",
        stderr);
  fflush(stderr);
  ASSERT(false);
  abort();
}

/** This function is the entry point for Frobby as a console
    program. It does some DEBUG-specific things, sets the random seed
    and so on before calling ::frobbyMain.
*/
int main(int argc, const char** argv) {
  try {
    std::set_terminate(frobbyTerminate);

    srand((unsigned int)time(0) +
#ifdef __GNUC__ // Only GCC defines this macro.
          (unsigned int)getpid() +
#endif
          (unsigned int)clock());

#ifdef PROFILE
    fputs("This is a PROFILE build of Frobby. It is therefore SLOW.\n",
          stderr);
#endif
#ifdef DEBUG
    fputs("This is a DEBUG build of Frobby. It is therefore SLOW.\n",
          stderr);
#endif

#ifdef DEBUG
    return DebugAllocator::getSingleton().runDebugMain(argc, argv);
#else
    return frobbyMain(argc, argv);
#endif
  } catch (const bad_alloc&) {
    displayError("Ran out of memory.");
    return ExitCodeOutOfMemory;
  } catch (const InternalFrobbyException& e) {
    displayException(e);
    return ExitCodeInternalError;
  } catch (const FrobbyException& e) {
    displayException(e);
    return ExitCodeError;
  } catch (...) {
    try {
      throw;
    } catch (const exception& e) {
      try {
        displayError(e.what());
      } catch (...) {
      }
    } catch (...) {
    }
    return ExitCodeUnknownError;
  }
}
