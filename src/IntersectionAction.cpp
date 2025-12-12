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
#include "IntersectionAction.h"

#include "BigIdeal.h"
#include "IOFacade.h"
#include "IntersectFacade.h"
#include "Scanner.h"
#include "IdealFacade.h"
#include "VarSorter.h"
#include "ElementDeleter.h"
#include "DataType.h"

IntersectionAction::IntersectionAction():
  Action
(staticGetName(),
 "Intersect the input ideals.",
 "Computes the intersection of the input ideals. Simply concatenate "
 "the textual\n"
 "representations of the ideals in order to intersect them.\n\n"
 "Note that this operation is currently implemented in a rather slow way.",
 false),

  _io(DataType::getMonomialIdealListType(), DataType::getMonomialIdealType()),

  _canonical
  ("canon",
   "Sort the generators and variables to get a canonical output.",
   false) {
}

void IntersectionAction::obtainParameters(vector<Parameter*>& parameters) {
  _io.obtainParameters(parameters);
  parameters.push_back(&_canonical);
  Action::obtainParameters(parameters);
}

void IntersectionAction::perform() {
  Scanner in(_io.getInputFormat(), stdin);
  _io.autoDetectInputFormat(in);
  _io.validateFormats();

  vector<BigIdeal*> ideals;
  ElementDeleter<vector<BigIdeal*> > idealsDeleter(ideals);

  IOFacade ioFacade(_printActions);
  VarNames names;
  ioFacade.readIdeals(in, ideals, names);
  in.expectEOF();

  IntersectFacade facade(_printActions);
  unique_ptr<BigIdeal> intersection(facade.intersect(ideals, names));
  idealsDeleter.deleteElements();

  if (_canonical) {
    IdealFacade idealFacade(_printActions);
    idealFacade.sortVariables(*intersection);
    idealFacade.sortGenerators(*intersection);
  }

  unique_ptr<IOHandler> output = _io.createOutputHandler();
  ioFacade.writeIdeal(*intersection, output.get(), stdout);
}

const char* IntersectionAction::staticGetName() {
  return "intersection";
}
