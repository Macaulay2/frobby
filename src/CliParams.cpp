/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 University of Aarhus
   Contact Bjarke Hammersholt Roune for license information (www.broune.com)

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
#include "CliParams.h"

#include "error.h"
#include "ParameterGroup.h"
#include "IntegerParameter.h"
#include "BoolParameter.h"
#include "StringParameter.h"
#include "NameFactory.h"

namespace {
  typedef void* Dummy;
  typedef NameFactory<Dummy> ParamNames;

  ParamNames getParamNames(vector<Parameter*> params) {
    struct HoldsFunction {
      static unique_ptr<Dummy> dummyCreate() {
        return unique_ptr<Dummy>();
      }
    };

    ParamNames names("option");
    for (size_t i = 0; i < params.size(); ++i)
      names.registerProduct(params[i]->getName(), HoldsFunction::dummyCreate);
    return names;
  }
}

CliParams::CliParams():
  _paramsDeleter(_ownedParams) {
}

void CliParams::parseCommandLine(unsigned int tokenCount, const char** tokens) {
  ParamNames names = getParamNames(_params);

  unsigned int i = 0;
  while (i < tokenCount) {
    if (tokens[i][0] != '-')
      reportError(string("Expected an option when reading \"") +
                  tokens[i] + "\", but options start with a dash (-).\n");
    string prefix(tokens[i] + 1); // +1 to skip the '-'

    string name = getUniqueNameWithPrefix(names, prefix);

    // Parse list of arguments to option.
    size_t argCount = 0;
    while (i + 1 + argCount < tokenCount &&
           tokens[i + 1 + argCount][0] != '-')
      ++argCount;

    processOption(name, tokens + i + 1, argCount);

    i += argCount + 1;
  }
}

void CliParams::processOption(const string& optionName,
                              const char** args,
                              unsigned int argCount) {
  for (vector<Parameter*>::iterator it = _params.begin();
    it != _params.end(); ++it) {
    if ((*it)->getName() == optionName) {
      (*it)->processArguments(args, argCount);
      return;
    }
  }

  reportInternalError("Processing non-existent option \"" + optionName + "\".");
}

void CliParams::add(unique_ptr<Parameter> param) {
  ASSERT(!hasParam(param->getName()));
  Parameter& paramRef = *param;
  exceptionSafePushBack(_ownedParams, std::move(param));
  add(paramRef);
}

void CliParams::add(Parameter& param) {
  ASSERT(!hasParam(param.getName()));
  _params.push_back(&param);
}

void CliParams::add(ParameterGroup& params) {
  for (ParameterGroup::iterator it = params.begin(); it != params.end(); ++it)
    add(**it);
}

bool CliParams::hasParam(const string& name) const {
  for (const_iterator it = _params.begin(); it != _params.end(); ++it)
    if ((*it)->getName() == name)
      return true;
  return false;
}

const Parameter& CliParams::getParam(const string& name) const {
  return const_cast<CliParams*>(this)->getParam(name);
}

Parameter& CliParams::getParam(const string& name) {
  for (const_iterator it = _params.begin(); it != _params.end(); ++it)
    if ((*it)->getName() == name)
      return **it;

  INTERNAL_ERROR("Unknown parameter " + name + ".");
}

bool getBool(const CliParams& params, const string& name) {
  const Parameter& param = params.getParam(name);
  ASSERT(dynamic_cast<const BoolParameter*>(&param) != 0);
  return static_cast<const BoolParameter&>(param);
}

const string& getString(const CliParams& params, const string& name) {
  const Parameter& param = params.getParam(name);
  ASSERT(dynamic_cast<const StringParameter*>(&param) != 0);
  return static_cast<const StringParameter&>(param);
}

unsigned int getInt(const CliParams& params, const string& name) {
  const Parameter& param = params.getParam(name);
  ASSERT(dynamic_cast<const IntegerParameter*>(&param) != 0);
  return static_cast<const IntegerParameter&>(param);
}

void setBool(CliParams& params, const string& name, bool newValue) {
  Parameter& param = params.getParam(name);
  ASSERT(dynamic_cast<BoolParameter*>(&param) != 0);
  static_cast<BoolParameter&>(param) = newValue;
}
