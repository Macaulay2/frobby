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
#include "SliceParams.h"

#include "SplitStrategy.h"
#include "error.h"
#include "CliParams.h"

SliceParams::SliceParams():
  _split("median"),
  _useIndependence(true),
  _useBoundElimination(true),
  _useBoundSimplification(true) {
}

SliceParams::SliceParams(const CliParams& cli):
  _split("median"),
  _useIndependence(true),
  _useBoundElimination(true),
  _useBoundSimplification(true) {
  extractCliValues(*this, cli);
}

namespace {
  const char* SplitParamName = "split";
  const char* UseIndependenceName = "independence";
  const char* UseBoundElimination = "bound";
  const char* UseBoundSimplification = "boundSimplify";
}

void addSliceParams(CliParams& params) {
}

void extractCliValues(SliceParams& slice, const CliParams& cli) {
  extractCliValues(static_cast<SliceLikeParams&>(slice), cli);
  slice.setSplit(getString(cli, SplitParamName));
  if (cli.hasParam(UseIndependenceName))
    slice.useIndependenceSplits(getBool(cli, UseIndependenceName));
  if (cli.hasParam(UseBoundElimination))
    slice.useBoundElimination(getBool(cli, UseBoundElimination));
  if (cli.hasParam(UseBoundSimplification))
    slice.useBoundElimination(getBool(cli, UseBoundSimplification));
}

void validateSplit(const SliceParams& params,
                   bool allowLabel,
                   bool allowDegree) {
  unique_ptr<SplitStrategy>
    split(SplitStrategy::createStrategy(params.getSplit()));
  ASSERT(split.get() != 0)

  if (!allowLabel && split->isLabelSplit())
    reportError("Label split strategy is not appropriate "
                "in this context.");

  // TODO: implement degree when there is no grading too, so that it
  // is always appropriate.
  if (!allowDegree && params.getSplit() == "degree") {
    reportError("The split strategy degree is not appropriate "
                "in this context.");
  }

  // TODO: remove the deprecated frob.
  if (!allowDegree && params.getSplit() == "frob") {
    reportError("The split strategy frob is not appropriate "
                "in this context.");
  }
}
