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
#include "OptimizeAction.h"

#include "DataType.h"
#include "IOFacade.h"
#include "SliceFacade.h"
#include "Scanner.h"
#include "BigIdeal.h"
#include "BigTermConsumer.h"
#include "NullTermConsumer.h"
#include "SliceParams.h"
#include "error.h"

#include <algorithm>
#include <iterator>

OptimizeAction::OptimizeAction():
  Action
(staticGetName(),
 "Solve optimization problems related to the input ideal.",
 "Solves an optimization program defined by the input monomial ideal I, and "
 "an\ninput vector of integers v. The optimization program is \n"
 "\n"
 "  maximize v * e such that e encodes an irreducible component of I,\n"
 "\n"
 "where * is dot product and e is a vector of integers that uniquely encodes "
 "an\nirreducible ideal by being the exponent vector of the product of the\n"
 "minimal generators.\n"
 "\n"
 "The input is composed of the ideal I in any format, optionally followed by "
 "the\nentries of v in a space separated list. If v is not explicitly "
 "specified,\nthen every entry is assumed to 1, i.e. then v is of the form "
 "(1, ..., 1).\n"
 "\n"
 "This action has options for displaying the optimal value or not and for\n"
 "displaying zero, one or all of the optimal solutions. The algorithm used "
 "to\nsolve the optimization program is the Slice Algorithm using the bound\n"
 "optimization. Thus this action also has options related to that.",
 false),

  _sliceParams(true, false),

  _displayLevel
  ("displayLevel",
   "Controls how many optimal solutions to display. If the value is 0 or "
   "1,\nFrobby displays 0 or 1 solutions respectively. If the value is 2 or "
   "more,\nall solutions are displayed. The output is presented as generators "
   "of a\nmonomial ideal.",
   0),

  _displayValue
  ("displayValue",
   "Display the optimal value of the optimization program.",
   true),

  _maxStandard
  ("maxStandard",
   "Solve the optimization program for maximal standard monomials instead "
   "of\nfor monomials representing irreducible components.",
   false),

  _chopFirstAndSubtract
  ("chopFirstAndSubtract",
   "Remove the first variable from generators, from the ring and from v, "
   "and\nsubtract the value of the first entry of v from the reported "
   "optimal value.\nThis is useful for Frobenius number calculations.",
   false),

  _minimizeValue
  ("minValue",
   "Minimize the value of v * e above. If this option is not set, maximize "
   "v * e\ninstead, as is the stated default above.",
   false),

  _io(DataType::getMonomialIdealType(), DataType::getMonomialIdealType()) {
  _sliceParams.setSplit("degree");
}

void OptimizeAction::obtainParameters(vector<Parameter*>& parameters) {
  parameters.push_back(&_displayLevel);
  parameters.push_back(&_displayValue);
  parameters.push_back(&_maxStandard);
  parameters.push_back(&_chopFirstAndSubtract);
  parameters.push_back(&_minimizeValue);
  _io.obtainParameters(parameters);
  _sliceParams.obtainParameters(parameters);
  Action::obtainParameters(parameters);
}

void OptimizeAction::perform() {
  SliceParams params(_params);
  validateSplit(params, true, true);

  BigIdeal ideal;
  vector<mpz_class> v;
  {
    Scanner in(_io.getInputFormat(), stdin);
    _io.autoDetectInputFormat(in);
    _io.validateFormats();

    IOFacade ioFacade(_printActions);
    ioFacade.readIdeal(in, ideal);
    if (!in.matchEOF())
      ioFacade.readVector(in, v, ideal.getVarCount());
    else
      fill_n(back_inserter(v), ideal.getVarCount(), 1);
    in.expectEOF();
  }

  mpz_class subtract = 0;
  if (_chopFirstAndSubtract) {
    if (v.empty()) {
      _chopFirstAndSubtract = false;
    } else {
      subtract = v[0];

      v.erase(v.begin());
      ideal.eraseVar(0);
    }
  }

  if (_minimizeValue) {
    for (size_t var = 0; var < v.size(); ++var)
      v[var] = -v[var];
  }

  unique_ptr<IOHandler> handler;
  unique_ptr<BigTermConsumer> output;
  if (_displayLevel > 0) {
    handler = _io.createOutputHandler();
    output = handler->createIdealWriter(stdout);
  } else
    output.reset(new NullTermConsumer());

  SliceFacade facade(params, ideal, *output);

  mpz_class optimalValue = 0;

  bool displayAll = (_displayLevel >= 2);
  bool anySolution;
  if (_maxStandard)
    anySolution = facade.solveStandardProgram
      (v, optimalValue, displayAll);
  else
    anySolution = facade.solveIrreducibleDecompositionProgram
      (v, optimalValue, displayAll);

  if (_displayValue) {
    if (!anySolution)
      fputs("no solution.\n", stdout);
    else {
      if (_minimizeValue) {
        // We flipped the sign of the vector to optimize before, so we
        // need to flip the sign of the value again.
        optimalValue = -optimalValue;
      }
      if (_chopFirstAndSubtract)
        optimalValue -= subtract;
      gmp_fprintf(stdout, "%Zd\n", optimalValue.get_mpz_t());
    }
  }
}

const char* OptimizeAction::staticGetName() {
  return "optimize";
}
