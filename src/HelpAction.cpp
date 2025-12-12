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
#include "frobby.h"
#include "stdinc.h"
#include "HelpAction.h"

#include "Parameter.h"
#include "IOHandler.h"
#include "DataType.h"
#include "error.h"
#include "display.h"
#include "FrobbyStringStream.h"
#include "ColumnPrinter.h"

#include <algorithm>

HelpAction::HelpAction():
  Action
(staticGetName(),
 "Display this help screen.",
 "Typing `frobby help' displays a list of the available actions.\n"
 "Typing `frobby help ACTION' displays a detailed description of that "
 "action.\n\n"
 "As an example, typing `frobby help irrdecom' will yield detailed "
 "information\n"
 "about the irrdecom action.",
 true) {
}

void HelpAction::obtainParameters(vector<Parameter*>& parameters) {
}

void HelpAction::processNonParameter(const char* str) {
  _topic = str;
}

namespace {
  // Helper function for displayActionHelp().
  bool paramCmp(Parameter* a, Parameter* b) {
    ASSERT(a != 0);
    ASSERT(b != 0);
    return string(a->getName()) < b->getName();
  }
}

void HelpAction::displayActionHelp(Action& action) {
  FrobbyStringStream out;
  out << "Displaying information on action: " << action.getName() << "\n\n";
  out << action.getDescription() << "\n";

  vector<Parameter*> parameters;
  action.obtainParameters(parameters);
  sort(parameters.begin(), parameters.end(), paramCmp);

  display(out);

  if (!parameters.empty()) {
    fprintf(stderr, "\nThe parameters accepted by %s are as follows.\n",
            action.getName());

    typedef vector<Parameter*>::const_iterator cit;
    for (cit it = parameters.begin(); it != parameters.end(); ++it) {
      string defaultValue = (*it)->getValueAsString();
      fprintf(stderr, "\n -%s %s   (default is %s)\n",
              (*it)->getName().c_str(),
              (*it)->getArgumentType().c_str(),
              (*it)->getValueAsString().c_str());
      display((*it)->getDescription(), "   ");
    }
  }
}

void HelpAction::displayIOHelp() {
  display
    ("Displaying information on topic: io\n"
     "\n"
     "Frobby understands several file formats. These are not documented, "
     "but they are simple enough that seeing an example should be enough "
     "to figure them out. Getting an example is as simple as making "
     "Frobby produce output in that format. "
     "\n\n"
     "It is true of all the formats that white-space is insignificant, "
     "but other than that Frobby is quite fuzzy about how the input "
     "must look. E.g. a Macaulay 2 file containing a monomial ideal "
     "must start with \"R = \", so writing \"r = \" with a lower-case r "
     "is an error. To help with this, Frobby tries to say what is wrong "
     "if there is an error."
     "\n\n"
     "If no input format is specified, Frobby will guess at the format, "
     "and it will guess correctly if there are no errors in the input. "
     "If no output format is specified, Frobby will use the same format "
     "for output as for input. If you want to force Frobby to use a "
     "specific format, use the -iformat and -oformat options. Using "
     "these with the transform action allows translation between formats. "
     "\n\n"
     "The formats available in Frobby and the types of data they "
     "support are as follows. "
     "\n");

  vector<string> names;
  getIOHandlerNames(names);
  for (vector<string>::const_iterator name = names.begin();
       name != names.end(); ++name) {
    unique_ptr<IOHandler> handler = createIOHandler(*name);
    ASSERT(handler.get() != 0);

    fprintf(stderr, "\n* The format %s: %s\n",
            handler->getName(),
            handler->getDescription());


    vector<const DataType*> types = DataType::getTypes();
    for (vector<const DataType*>::const_iterator typeIt = types.begin();
         typeIt != types.end(); ++typeIt) {
      const DataType& type = **typeIt;

      bool input = handler->supportsInput(type);
      bool output = handler->supportsOutput(type);

      const char* formatStr = "";
      if (input && output)
        formatStr = "  - supports input and output of %s.\n";
      else if (input)
        formatStr = "  - supports input of %s.\n";
      else if (output)
        formatStr = "  - supports output of %s.\n";

      fprintf(stderr, formatStr, type.getName());
    }
  }
}

void HelpAction::perform() {
  if (_topic != "") {
    if (_topic == "io")
      displayIOHelp();
    else
      displayActionHelp(*Action::createActionWithPrefix(_topic));

    return;
  }

  FrobbyStringStream out;

  out << "Frobby version " << frobby_version
      << " Copyright (C) 2007 Bjarke Hammersholt Roune\n";
  out <<
    "Frobby performs a number of computations related to monomial "
    "ideals.\nYou run it by typing `frobby ACTION', where ACTION is "
    "one of the following. "
    "\n\n";

  ColumnPrinter printer;
  printer.addColumn(false, " ");
  printer.addColumn(true, " - ");

  vector<string> names;
  Action::getActionNames(names);
  for (vector<string>::const_iterator it = names.begin();
       it != names.end(); ++it) {
    unique_ptr<Action> action(Action::createActionWithPrefix(*it));
    if (action->displayAction()) {
	  printer[0] << action->getName() << '\n';
	  printer[1] << action->getShortDescription() << '\n';
	}
  }
  printer.print(out);

  out <<
    "\nType 'frobby help ACTION' to get more details on a specific action.\n"
    "Note that all input and output is done via the standard streams.\n"
    "Type 'frobby help io' for more information on input and output formats.\n"
    "See www.broune.com for further information and new versions of Frobby.\n"
    "\n"
    "Frobby is free software and you are welcome to redistribute it under certain "
    "conditions. Frobby comes with ABSOLUTELY NO WARRANTY. See the GNU General "
    "Public License version 2.0 in the file COPYING for details.\n";
  display(out);
}

const char* HelpAction::staticGetName() {
  return "help";
}

bool HelpAction::displayAction() const {
  return false;
}
