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
#ifndef ACTION_GUARD
#define ACTION_GUARD

#include "BoolParameter.h"
#include "CliParams.h"

class Parameter;

class Action {
 public:
  Action(const char* name,
         const char* shortDescription,
         const char* description,
         bool acceptsNonParameter);
  virtual ~Action();

  const char* getName() const;
  const char* getShortDescription() const;
  const char* getDescription() const;

  // processNonParameter() can be called at most once, and only if
  // acceptsNonParameter() returns true.
  bool acceptsNonParameter() const;
  virtual void processNonParameter(const char* str);

  virtual void obtainParameters(vector<Parameter*>& parameters);

  virtual void parseCommandLine(unsigned int tokenCount,
                                const char** tokens);

  virtual void perform() = 0;

  /** Returns whether this action should be shown to the user by the
    help action.*/
  virtual bool displayAction() const;

  const Parameter& getParam(const string& name) const;

  static void getActionNames(vector<string>& names);
  static unique_ptr<Action> createActionWithPrefix(const string& prefix);

 protected:
  CliParams _params;



  const char* _name;
  const char* _shortDescription;
  const char* _description;
  bool _acceptsNonParameter;

  BoolParameter _printActions;

};

#endif
