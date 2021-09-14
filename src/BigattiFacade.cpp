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
#include "BigattiFacade.h"

#include "BigattiHilbertAlgorithm.h"
#include "BigattiParams.h"
#include "BigattiPivotStrategy.h"

BigattiFacade::BigattiFacade(const BigattiParams& params):
  Facade(params.getPrintActions()),
  _pivot(BigattiPivotStrategy::createStrategy
         (params.getPivot(), params.getWidenPivot())),
  _params(params) {
  _common.readIdealAndSetPolyOutput(params);
}

BigattiFacade::~BigattiFacade() {
  // Destructor defined so auto_ptr<T> in the header does not need
  // definition of T.
}

void BigattiFacade::computeMultigradedHilbertSeries() {
  beginAction("Computing multigraded Hilbert-Poincare series.");

  BigattiHilbertAlgorithm alg(_common.takeIdeal(),
                              _common.getTranslator(),
                              _params,
                              std::move(_pivot),
                              _common.getPolyConsumer());
  alg.setComputeUnivariate(false);
  alg.run();

  endAction();
}

void BigattiFacade::computeUnivariateHilbertSeries() {
  beginAction("Computing univariate Hilbert-Poincare series");

  BigattiHilbertAlgorithm alg(_common.takeIdeal(),
                              _common.getTranslator(),
                              _params,
                              std::move(_pivot),
                              _common.getPolyConsumer());
  alg.setComputeUnivariate(true);
  alg.run();

  endAction();
}
