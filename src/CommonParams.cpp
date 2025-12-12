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
#include "CommonParams.h"

#include "CliParams.h"
#include "IOHandler.h"
#include "BoolParameter.h"

CommonParams::CommonParams():
  _idealIsMinimal(false),
  _printActions(false),
  _produceCanonicalOutput(false),
  _printDebug(false),
  _printStatistics(false),
  _inputFormat(getFormatNameIndicatingToGuessTheInputFormat()),
  _outputFormat(getFormatNameIndicatingToUseInputFormatAsOutputFormat()) {
}

namespace {
  static const char* IdealIsMinimalName = "minimal";
  static const char* PrintActionsName = "time";
  static const char* CanonicalName = "canon";
  static const char* InputFormatParamName = "iformat";
  static const char* OutputFormatParamName = "oformat";
  static const char* PrintDebugName = "debug";
  static const char* PrintStatisticsName = "stats";
}

void addDebugParam(CliParams& params) {
  ASSERT(!params.hasParam(PrintDebugName));
  params.add
    (unique_ptr<Parameter>
     (new BoolParameter
      ("debug",
       "Print what the algorithm does at each step.",
       false)));
}

void addCommonParams(CliParams& params) {
  addDebugParam(params);
}

void extractCliValues(CommonParams& common, const CliParams& cli) {
  common.idealIsMinimal(getBool(cli, IdealIsMinimalName));
  common.printActions(getBool(cli, PrintActionsName));
  common.produceCanonicalOutput(getBool(cli, CanonicalName));

  if (cli.hasParam(InputFormatParamName))
    common.setInputFormat(getString(cli, InputFormatParamName));
  if (cli.hasParam(OutputFormatParamName))
    common.setOutputFormat(getString(cli, OutputFormatParamName));
  if (cli.hasParam(PrintDebugName))
    common.printDebug(getBool(cli, PrintDebugName));
  if (cli.hasParam(PrintStatisticsName))
    common.printStatistics(getBool(cli, PrintStatisticsName));
}
