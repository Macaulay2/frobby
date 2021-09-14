/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2010 Bjarke Hammersholt Roune (www.broune.com)

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
#include "EulerAction.h"

#include "DataType.h"
#include "IOFacade.h"
#include "Scanner.h"
#include "BigIdeal.h"
#include "BigTermConsumer.h"
#include "error.h"
#include "PivotEulerAlg.h"
#include "Ideal.h"
#include "HilbertBasecase.h"
#include "PivotStrategy.h"
#include "SquareFreeIdeal.h"
#include "ActionPrinter.h"

#include <algorithm>
#include <cstdio>
#include <limits>

EulerAction::EulerAction():
  Action
(staticGetName(),
 "Compute the Euler characteristic.",
 "Compute the Euler characteristic of a monomial ideal I. This is defined as "
 "the Euler characteristic of the simplicial complex D where I is the dual of "
 "the Stanley-Reisner ideal of D. The translation between I and D is "
 "computationally efficient. Define f by\n"
 "\n"
 "  f(v) = product of all variables not in the set v\n"
 "\n"
 "Then f is a bijection from the facets of D to the minimal generators of I. "
 "So this action can easily be used to compute Euler characteristics of "
 "abstract simplicial complexes given by their facets. If you have an input "
 "file where the 0-1 exponents are opposite of what you need for this "
 "action, use the -swap01 option.",
 false),

  _pivot
  ("pivot",
   "Which kind of pivots to use. Options are\n"
   "  std: Use standard pivots only.\n"
   "  gen: Use generator pivots only.\n"
   "  hybrid: Use a heuristic to choose at each split.\n",
   "gen"),

  _stdPivot
  ("stdPivot",
   "Which kind of standard pivots to use. The options are\n"
   "  popvar: Use a popular variable as pivot.\n"
   "  rarevar: Use a rare variable as pivot.\n"
   "  popgcd: Use the gcd of 3 generators divisible by a popular variable.\n"
   "  any: Use some variable in a way that does not vary between runs.\n"
   "  random: Use a random variable. Choices may vary between runs.\n"
   "A rare variable is a variable that divides a minimum number of "
   "generators. A popular variable is a variable that divides a "
   "maximum number of generators.\n"
   "\n"
   "In addition, widen_X where X is one of the strategies above will "
   "compute a preliminary pivot according to X, and then select the actual "
   "pivot to be the gcd of all generators that the preliminary pivot divides.",
   "popvar"),

  _genPivot
  ("genPivot",
   "Which kind of generator pivots to use. The options are\n"
   "  rarevar: Pick a generator divisible by a rare variable.\n"
   "  popvar: Pick a generator divisible by a popular variable.\n"
   "  maxsupp: Pick a generator with maximum support.\n"
   "  minsupp: Pick a generator with minimum support.\n"
   "  any: Pick some generator in a way that does not vary between runs.\n"
   "  random: Pick a random generator. Choices may vary between runs.\n"
   "  rarest: Pick a generator that is divisible by a maximum number of\n"
   "    rare variables. Break ties by picking the generator that is divisible\n"
   "    by the maximum number of second-most-rare variables and so on.\n"
   "  raremax: as rarevar_maxsupp.\n"
   "A rare variable is a variable that divides a minimum number of "
   "generators. A popular variable is a variable that divides a "
   "maximum number of generators.\n"
   "\n"
   "All of these strategies except any and random can have ties. Combine "
   "strategies A and B by writing A_B. If A has a tie then A_B will use "
   "B to break the tie. For example rarevar_minsupp will pick some rare "
   "variable "
   "and select the generator with maximum support divisible by that variable. "
   "For another example, rarevar_minsupp_random will do the same thing, but "
   "if two generators divisible by the rare variable has the same "
   "maximal support "
   "then it will pick one at random instead of deterministically.\n"
   "\n"
   "All choices implicitly have _any appended to them, so any remaining "
   "ties are broken arbitrarily in a deterministic way. If a strategy would "
   "eliminate all candidates for a pivot it will instead preserve all the "
   "candidates. This can happen for example in minsupp_rarevar where the "
   "minsupp strategy might have eliminated all generators that are divisible "
   "by the rare variable that rarevar selects. Then rarevar cannot make a "
   "choice so it will refrain from doing so.",
   "raremax"),

  _autoTranspose
  ("autotranspose",
   "The two algorithms prefer more variables and more generators "
   "respectively. Transposing the variable-generator divides "
   "matrix swaps the number of variables and generators without "
   "changing the Euler characteristic. If this option is on it "
   "will transpose at each step if the preferred one of "
   "variables and generators is not larger. If this option is "
   "set to \"once\", it will do this but only at the first step. "
   "If this option is off, no transposes are done.",
   "on"),

  _printDebug
  ("debug",
   "Print what the algorithm does at each step.",
   false),

  _printStatistics
  ("stats",
   "Print statistics on what the algorithm did.",
   false),

  _useUniqueDivSimplify
  ("uniqueDiv",
   "Simplify ideals at each step where a variable divides only one generator.",
   true),

  _useManyDivSimplify
  ("manyDiv",
   "Simplify ideals at each step where a variable divides all generators "
   "except up to 2.",
   true),

  _useAllPairsSimplify
  ("impliedDiv",
   "Simplify ideals at each step with variables X and Y such that all "
   "generators divisible by A are also divisible by B.",
   false),

  _swap01
  ("swap01",
   "Change all 0 exponents to 1 and vice versa.",
   false),

  _io(DataType::getMonomialIdealType(), DataType::getNullType()) {
}

void EulerAction::obtainParameters(vector<Parameter*>& parameters) {
  _io.obtainParameters(parameters);
  parameters.push_back(&_pivot);
  parameters.push_back(&_stdPivot);
  parameters.push_back(&_genPivot);
  parameters.push_back(&_autoTranspose);
  parameters.push_back(&_printDebug);
  parameters.push_back(&_printStatistics);
  parameters.push_back(&_useUniqueDivSimplify);
  parameters.push_back(&_useManyDivSimplify);
  parameters.push_back(&_useAllPairsSimplify);
  parameters.push_back(&_swap01);
  Action::obtainParameters(parameters);
}

void EulerAction::perform() {
  auto_ptr<PivotStrategy> stdStrat = newStdPivotStrategy(_stdPivot.getValue());
  auto_ptr<PivotStrategy> genStrat = newGenPivotStrategy(_genPivot.getValue());
  auto_ptr<PivotStrategy> strat;
  if (_pivot == "std")
	strat = std::move(stdStrat);
  else if (_pivot == "gen")
	strat = std::move(genStrat);
  else if (_pivot == "hybrid")
	strat = newHybridPivotStrategy(std::move(stdStrat), std::move(genStrat));
  else
    reportError("Unknown kind of pivot strategy \"" +
				_pivot.getValue() + "\".");

  if (_printDebug)
	strat = newDebugPivotStrategy(std::move(strat), stderr);
  if (_printStatistics)
	strat = newStatisticsPivotStrategy(std::move(strat), stderr);

  PivotEulerAlg alg;
  alg.setPivotStrategy(std::move(strat));
  alg.setUseUniqueDivSimplify(_useUniqueDivSimplify);
  alg.setUseManyDivSimplify(_useManyDivSimplify);
  alg.setUseAllPairsSimplify(_useAllPairsSimplify);

  IOFacade ioFacade(_printActions);
  SquareFreeIdeal ideal;
  {
    Scanner in(_io.getInputFormat(), stdin);
    _io.autoDetectInputFormat(in);
    _io.validateFormats();
    ioFacade.readSquareFreeIdeal(in, ideal);
    in.expectEOF();
  }
  if (_swap01) {
    ActionPrinter pr(_printActions, "Inverting ideal.");
    ideal.swap01Exponents();
  }

  {
    ActionPrinter pr(_printActions, "Minimizing ideal.");
    ideal.minimize();
  }

  if (_autoTranspose == "on") {
    alg.setAutoTranspose(true);
    alg.setInitialAutoTranspose(true);
  } else if (_autoTranspose == "once") {
    alg.setAutoTranspose(false);
    alg.setInitialAutoTranspose(true);
  } else if (_autoTranspose == "off") {
    alg.setAutoTranspose(false);
    alg.setInitialAutoTranspose(false);
  } else
    reportError("Unknown setting for -autoTranspose of \"" +
				_autoTranspose.getValue() + "\".");

  mpz_class euler;
  {
    ActionPrinter pr(_printActions, "Computing Euler characteristic.");
    euler = alg.computeEulerCharacteristic(*ideal.getRawIdeal());
  }
  gmp_fprintf(stdout, "%Zd\n", euler.get_mpz_t());
}

const char* EulerAction::staticGetName() {
  return "euler";
}
