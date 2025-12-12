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
#include "TransformAction.h"

#include "BigIdeal.h"
#include "IOFacade.h"
#include "IdealFacade.h"
#include "Scanner.h"
#include "ElementDeleter.h"
#include "VarSorter.h"
#include "DataType.h"

#include <algorithm>

TransformAction::TransformAction():
  Action
(staticGetName(),
 "Change the representation of the input ideal.",
 "By default, transform simply writes the input ideals to output. A "
 "number of parameters allow one to transform the input ideal in various ways.",
 false),

  _io(DataType::getMonomialIdealType(), DataType::getMonomialIdealType()),

  _canonicalize
  ("canon",
   "Sort variables, generators and ideals to get canonical representation.",
   false),

  _sort
  ("sort",
   "Sort generators according to the reverse lexicographic order.",
   false),

  _unique
  ("unique",
   "Remove duplicate generators.",
   false),

  _minimize
  ("minimize",
  "Remove non-minimial generators.",
   false),

  _deform
  ("deform",
   "Apply a generic deformation to the input ideal.",
   false),

  _radical
  ("radical",
   "Take the radical of the generators. Combine this with -minimize to "
   "get rid of any non-minimal ones.",
   false),

  _product
  ("product",
   "Replace each ideal with the product of its generators.",
   false),

  _addPurePowers
  ("addPurePowers",
   "Adds a pure power for each variable that does not already have a pure "
   "power in the ideal. Each exponent is chosen to be one larger than the "
   "maximal exponent of that variable that appears in the ideal.",
   false),

  _trimVariables
  ("trimVariables",
   "Remove variables that divide none of the generators.",
   false),

  _transpose
  ("transpose",
   "Exchange variables and minimal generators. Let M be a matrix whose "
   "rows are labeled by minimal generators and whose columns are labeled "
   "by variables. The entry at row g and column x is the number of times "
   "that x divides g. This options transposes that matrix.",
   false),

  _swap01
  ("swap01",
   "Change all 0 exponents to 1 and vice versa.",
   false),

  _projectVar
  ("projectVar",
   "Project away the i'th variable counting from 1. No action is taken "
   "for a value of 0 or more than the number of variables in the ring.",
   0) {
}

void TransformAction::obtainParameters(vector<Parameter*>& parameters) {
  _io.obtainParameters(parameters);
  parameters.push_back(&_canonicalize);
  parameters.push_back(&_minimize);
  parameters.push_back(&_sort);
  parameters.push_back(&_unique);
  parameters.push_back(&_deform);
  parameters.push_back(&_radical);
  parameters.push_back(&_product);
  parameters.push_back(&_addPurePowers);
  parameters.push_back(&_trimVariables);
  parameters.push_back(&_swap01);
  parameters.push_back(&_projectVar);
  parameters.push_back(&_transpose);
  Action::obtainParameters(parameters);
}

void TransformAction::perform() {
  Scanner in(_io.getInputFormat(), stdin);
  _io.autoDetectInputFormat(in);
  _io.validateFormats();

  IOFacade facade(_printActions);

  vector<BigIdeal*> ideals;
  ElementDeleter<vector<BigIdeal*> > idealsDeleter(ideals);
  VarNames names;

  facade.readIdeals(in, ideals, names);
  in.expectEOF();

  IdealFacade idealFacade(_printActions);

  if (_transpose) {
    names.clear();
    for (size_t i = 0; i < ideals.size(); ++i) {
      const BigIdeal& ideal = *(ideals[i]);
      BigIdeal trans(VarNames(ideal.getGeneratorCount()));
      trans.reserve(ideal.getVarCount());
      for (size_t var = 0; var < ideal.getVarCount(); ++var) {
        trans.newLastTerm();
        for (size_t gen = 0; gen < ideal.getGeneratorCount(); ++gen)
          trans.getLastTermRef()[gen] = ideal[gen][var];
	  }
	  (*ideals[i]) = trans;
      if (i == ideals.size() - 1)
        names = ideal.getNames();
	}
  }

  if (0 < _projectVar && _projectVar <= names.getVarCount()) {
    size_t var = _projectVar - 1;
    names.projectVar(var);

    for (size_t i = 0; i < ideals.size(); ++i) {
      BigIdeal& ideal = *(ideals[i]);
      idealFacade.projectVar(ideal, var);
    }
  }

  if (_product) {
    unique_ptr<BigIdeal> ideal;
    ideal.reset(new BigIdeal(names));

    idealFacade.takeProducts(ideals, *ideal);

    idealsDeleter.deleteElements();
    exceptionSafePushBack(ideals, std::move(ideal));
  }

  for (size_t i = 0; i < ideals.size(); ++i) {
    BigIdeal& ideal = *(ideals[i]);

    if (_radical)
      idealFacade.takeRadical(ideal);

	if (_swap01)
	  idealFacade.swap01(ideal);

    if (_minimize)
      idealFacade.sortAllAndMinimize(ideal);

    if (_deform)
      idealFacade.deform(ideal);
  }

  if (_trimVariables)
	idealFacade.trimVariables(ideals, names);

  for (size_t i = 0; i < ideals.size(); ++i) {
    BigIdeal& ideal = *(ideals[i]);

    if (_addPurePowers)
      idealFacade.addPurePowers(ideal);

    if (_canonicalize)
      idealFacade.sortVariables(ideal);

    if (_unique)
      idealFacade.sortGeneratorsUnique(ideal);
    else if (_sort || _canonicalize)
      idealFacade.sortGenerators(ideal);
  }

  if (_canonicalize) {
    VarSorter sorter(names);
    sorter.getOrderedNames(names);

    sort(ideals.begin(), ideals.end(), compareIdeals);
  }

  unique_ptr<IOHandler> output(_io.createOutputHandler());
  facade.writeIdeals(ideals, names, output.get(), stdout);
}

const char* TransformAction::staticGetName() {
  return "transform";
}

bool TransformAction::compareIdeals(const BigIdeal* a, const BigIdeal* b) {
  return *a < *b;
}
