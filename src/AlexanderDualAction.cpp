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
#include "AlexanderDualAction.h"

#include "BigIdeal.h"
#include "IOFacade.h"
#include "Scanner.h"
#include "SliceFacade.h"
#include "SliceParams.h"
#include "DataType.h"
#include "BigTermConsumer.h"

AlexanderDualAction::AlexanderDualAction():
  Action
(staticGetName(),
 "Compute the Alexander dual of the input ideal.",
 "Compute the alexander dual of the input monomial ideal.\n\n"
 "The computation is done using irreducible decomposition, which is why "
 "there are\n"
 "a number of options related to that. See the help topic on irrdecom for "
 "details.",
 false),

  _io(DataType::getMonomialIdealType(), DataType::getMonomialIdealType()) {
}

void AlexanderDualAction::obtainParameters(vector<Parameter*>& parameters) {
  Action::obtainParameters(parameters);
  _sliceParams.obtainParameters(parameters);
  _io.obtainParameters(parameters);
}

void AlexanderDualAction::perform() {
  SliceParams params(_params);
  validateSplit(params, true, false);

  BigIdeal ideal;
  vector<mpz_class> point;
  bool pointSpecified;

  {
    Scanner in(_io.getInputFormat(), stdin);
    _io.autoDetectInputFormat(in);
    _io.validateFormats();

    IOFacade ioFacade(_printActions);
    pointSpecified = ioFacade.readAlexanderDualInstance(in, ideal, point);
    in.expectEOF();
  }

  unique_ptr<BigTermConsumer> output =
    _io.createOutputHandler()->createIdealWriter(stdout);
  SliceFacade facade(params, ideal, *output);

  if (pointSpecified)
    facade.computeAlexanderDual(point);
  else
    facade.computeAlexanderDual();
}

const char* AlexanderDualAction::staticGetName() {
  return "alexdual";
}
