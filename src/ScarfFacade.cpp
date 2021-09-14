/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2010 University of Aarhus
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
#include "ScarfFacade.h"

#include "DataType.h"
#include "ScarfHilbertAlgorithm.h"
#include "TranslatingCoefTermConsumer.h"
#include "TotalDegreeCoefTermConsumer.h"
#include "IdealOrderer.h"

ScarfFacade::ScarfFacade(const ScarfParams& params):
  Facade(params.getPrintActions()),
  _params(params)  {
  // Create predicate before reading the input to display errors on
  // options before potentially spending a long time reading the
  // input.
  _enumerationOrder =
    createIdealOrderer(params.getEnumerationOrder());
  _deformationOrder =
    createIdealOrderer(params.getDeformationOrder());

  _helper.readIdealAndSetPolyOutput(params);
}

ScarfFacade::~ScarfFacade() {
}

void ScarfFacade::computeMultigradedHilbertSeries() {
  beginAction("Computing multigraded Hilbert-Poincare series.");

  ScarfHilbertAlgorithm alg(_helper.getTranslator(),
                            _params,
                            std::move(_enumerationOrder),
                            std::move(_deformationOrder));

  alg.runGeneric(_helper.getIdeal(),
                 _helper.getPolyConsumer(),
                 false,
                 _params.getProduceCanonicalOutput());

  endAction();
}

void ScarfFacade::computeUnivariateHilbertSeries() {
  beginAction("Computing univariate Hilbert-Poincare series.");

  ScarfHilbertAlgorithm alg(_helper.getTranslator(),
                            _params,
                            std::move(_enumerationOrder),
                            std::move(_deformationOrder));

  alg.runGeneric(_helper.getIdeal(),
                 _helper.getPolyConsumer(),
                 true,
                 _params.getProduceCanonicalOutput());

  endAction();
}
