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
#include "GenerateIdealAction.h"

#include "BigIdeal.h"
#include "GenerateDataFacade.h"
#include "IOFacade.h"
#include "error.h"
#include "DataType.h"

GenerateIdealAction::GenerateIdealAction():
  Action
(staticGetName(),
 "Generate a random monomial ideal.",
 "Generate a monomial ideal. The default is to generate a random one,\n"
 "and in certain rare cases the generated ideal may have less than the "
 "requested\n"
 "number of minimial generators. This can be alleviated by increasing the\n"
 "exponent range or the number of variables.\n\n"
 "The other types of ideals are not random, and they use the number of "
 "variables\n"
 "as a parameter and may thus have more generators, depending on the type.",
 false),

  _type
("type",
 "The supported types of ideals are random, edge, list, king, knight, rook, matching and tree.",
 "random"),
  _variableCount("varCount", "The number of variables.", 3),
  _generatorCount("genCount", "The number of minimal generators.", 5),
  _exponentRange
  ("expRange",
   "Exponents are chosen uniformly in the range [0,INTEGER].", 9),

  _io(DataType::getNullType(), DataType::getMonomialIdealType()) {
}

void GenerateIdealAction::obtainParameters(vector<Parameter*>& parameters) {
  Action::obtainParameters(parameters);
  _io.obtainParameters(parameters);

  parameters.push_back(&_exponentRange);
  parameters.push_back(&_variableCount);
  parameters.push_back(&_generatorCount);
  parameters.push_back(&_type);
}

void GenerateIdealAction::perform() {
  _io.validateFormats();

  BigIdeal ideal;

  GenerateDataFacade generator(_printActions);
  const string& type = _type;
  if (type == "random")
    generator.generateIdeal(ideal,
                            _exponentRange,
                            _variableCount,
                            _generatorCount);
  else if (type == "list")
    generator.generateListIdeal(ideal, _variableCount);
  else if (type == "edge")
    generator.generateEdgeIdeal(ideal, _variableCount, _generatorCount);
  else if (type == "king")
    generator.generateKingChessIdeal(ideal, _variableCount);
  else if (type == "knight")
    generator.generateKnightChessIdeal(ideal, _variableCount);
  else if (type == "rook")
    generator.generateRookChessIdeal(ideal, _variableCount, _generatorCount);
  else if (type == "matching")
    generator.generateMatchingIdeal(ideal, _variableCount);
  else if (type == "tree")
    generator.generateTreeIdeal(ideal, _variableCount);
  else
    reportError("Unknown ideal type \"" + type + "\".");

  IOFacade ioFacade(_printActions);
  unique_ptr<IOHandler> output = _io.createOutputHandler();

  ioFacade.writeIdeal(ideal, output.get(), stdout);
}

const char* GenerateIdealAction::staticGetName() {
  return "genideal";
}
