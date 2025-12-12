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
#include "Action.h"

#include "error.h"
#include "NameFactory.h"

#include "IrreducibleDecomAction.h"
#include "TransformAction.h"
#include "HelpAction.h"
#include "DynamicFrobeniusAction.h"
#include "GenerateIdealAction.h"
#include "GenerateFrobeniusAction.h"
#include "FrobeniusAction.h"
#include "AnalyzeAction.h"
#include "LatticeFormatAction.h"
#include "IntersectionAction.h"
#include "AssociatedPrimesAction.h"
#include "AlexanderDualAction.h"
#include "HilbertAction.h"
#include "PolyTransformAction.h"
#include "HelpAction.h"
#include "TestAction.h"
#include "PrimaryDecomAction.h"
#include "OptimizeAction.h"
#include "MaximalStandardAction.h"
#include "LatticeAnalyzeAction.h"
#include "DimensionAction.h"
#include "EulerAction.h"

Action::Action(const char* name,
               const char* shortDescription,
               const char* description,
               bool acceptsNonParameterParam):
  _name(name),
  _shortDescription(shortDescription),
  _description(description),
  _acceptsNonParameter(acceptsNonParameterParam),
  _printActions("time", "Display and time each subcomputation.", false) {

  _params.add(_printActions);
}

Action::~Action() {
}

namespace {
  typedef NameFactory<Action> ActionFactory;

  ActionFactory getActionFactory() {
    ActionFactory factory("action");

    nameFactoryRegister<HilbertAction>(factory);
    nameFactoryRegister<EulerAction>(factory);
    nameFactoryRegister<IrreducibleDecomAction>(factory);
    nameFactoryRegister<PrimaryDecomAction>(factory);
    nameFactoryRegister<AlexanderDualAction>(factory);
    nameFactoryRegister<AssociatedPrimesAction>(factory);
    nameFactoryRegister<MaximalStandardAction>(factory);
    nameFactoryRegister<DimensionAction>(factory);
    nameFactoryRegister<OptimizeAction>(factory);

    nameFactoryRegister<TransformAction>(factory);
    nameFactoryRegister<PolyTransformAction>(factory);

    nameFactoryRegister<IntersectionAction>(factory);
    nameFactoryRegister<GenerateIdealAction>(factory);
    nameFactoryRegister<FrobeniusAction>(factory);
    nameFactoryRegister<DynamicFrobeniusAction>(factory);
    nameFactoryRegister<GenerateFrobeniusAction>(factory);
    nameFactoryRegister<AnalyzeAction>(factory);
    nameFactoryRegister<LatticeFormatAction>(factory);
    nameFactoryRegister<LatticeAnalyzeAction>(factory);

    nameFactoryRegister<HelpAction>(factory);
    nameFactoryRegister<TestAction>(factory);

    return factory;
  }
}

bool Action::displayAction() const {
  return true;
}

const Parameter& Action::getParam(const string& name) const {
  return _params.getParam(name);
}

void Action::getActionNames(vector<string>& names) {
  getActionFactory().getNamesWithPrefix("", names);
}

unique_ptr<Action> Action::createActionWithPrefix(const string& prefix) {
  return createWithPrefix(getActionFactory(), prefix);
}

const char* Action::getName() const {
  return _name;
}

const char* Action::getShortDescription() const {
  return _shortDescription;
}

const char* Action::getDescription() const {
  return _description;
}

bool Action::acceptsNonParameter() const {
  return _acceptsNonParameter;
}

void Action::processNonParameter(const char*) {
  reportInternalError("Action::processNonParameter called.");
}

void Action::obtainParameters(vector<Parameter*>& parameters) {
  parameters.insert(parameters.end(), _params.begin(), _params.end());
}

void Action::parseCommandLine(unsigned int tokenCount, const char** tokens) {
  vector<Parameter*> tmp;
  obtainParameters(tmp);
  for (size_t i = 0; i < tmp.size(); ++i)
    if (!_params.hasParam(tmp[i]->getName()))
      _params.add(*tmp[i]);

  if (acceptsNonParameter() && tokenCount > 0 && tokens[0][0] != '-') {
    processNonParameter(tokens[0]);
    --tokenCount;
    ++tokens;
  }

  _params.parseCommandLine(tokenCount, tokens);
}
