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
#include "PolyTransformAction.h"

#include "Scanner.h"
#include "IOFacade.h"
#include "BigPolynomial.h"
#include "PolynomialFacade.h"
#include "DataType.h"

PolyTransformAction::PolyTransformAction():
  Action
(staticGetName(),
 "Change the representation of the input polynomial.",
 "By default, transform simply writes the input polynomial to output. A\n"
 "number of parameters allow one to transform the input polynomial in various\n"
 "ways.",
 false),

  _io(DataType::getPolynomialType(), DataType::getPolynomialType()),

  _canonicalize
  ("canon",
   "Sort variables and generators to get a canonical representation.",
   false),

  _sortTerms
  ("sort",
   "Sort the terms.",
   false) {
}

void PolyTransformAction::obtainParameters(vector<Parameter*>& parameters) {
  Action::obtainParameters(parameters);
  _io.obtainParameters(parameters);
  parameters.push_back(&_canonicalize);
  parameters.push_back(&_sortTerms);
}

void PolyTransformAction::perform() {
  Scanner in(_io.getInputFormat(), stdin);
  _io.autoDetectInputFormat(in);
  _io.validateFormats();

  IOFacade facade(_printActions);

  BigPolynomial polynomial;
  facade.readPolynomial(in, polynomial);
  in.expectEOF();

  PolynomialFacade polyFacade(_printActions);
  if (_canonicalize)
    polyFacade.sortVariables(polynomial);
  if (_sortTerms || _canonicalize)
    polyFacade.sortTerms(polynomial);

  unique_ptr<IOHandler> output = _io.createOutputHandler();
  facade.writePolynomial(polynomial, output.get(), stdout);
}

const char* PolyTransformAction::staticGetName() {
  return "ptransform";
}
