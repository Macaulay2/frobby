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
#ifndef IO_PARAMETERS_GUARD
#define IO_PARAMETERS_GUARD

#include "ParameterGroup.h"
#include "StringParameter.h"
#include "IOHandler.h"

class Scanner;
class DataType;

class IOParameters : public ParameterGroup {
 public:
  IOParameters(const DataType& input, const DataType& output);

  void setOutputFormat(const string& format);
  void setInputFormat(const string& format);

  const string& getInputFormat() const;
  const string& getOutputFormat() const;

  unique_ptr<IOHandler> createInputHandler() const;
  unique_ptr<IOHandler> createOutputHandler() const;

  /** If using the input format, this must be called before validating
   the ideals, since the auto detect format is not a valid format other
   than as a place holder for the auto detected format. If the format
   on in is auto detect, it will (also) be set.
  */
  void autoDetectInputFormat(Scanner& in);

  // Exits with an error message if the input or output format is not
  // known.
  void validateFormats() const;

 private:
  const DataType& _inputType;
  const DataType& _outputType;

  unique_ptr<StringParameter> _inputFormat;
  unique_ptr<StringParameter> _outputFormat;
};

#endif
