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
#ifndef BIGATTI_FACADE_GUARD
#define BIGATTI_FACADE_GUARD

#include "Facade.h"
#include "CommonParamsHelper.h"
#include "BigattiParams.h"

class BigattiPivotStrategy;

/** A facade for computing Hilbert series of monomial ideals using the
 divide-and-conquer algorithm by Bigatti et.al.

 @ingroup Facade
*/
class BigattiFacade : public Facade {
 public:
  BigattiFacade(const BigattiParams& params);
  ~BigattiFacade();

  /** Computes the numerator of the multigraded Hilbert-Poincare
   series with no cancellation of common terms in numerator and
   denominator. */
  void computeMultigradedHilbertSeries();

  /** Computes the numerator of the univariate Hilbert-Poincare series
   with no cancellation of common terms in numerator and
   denominator. */
  void computeUnivariateHilbertSeries();

 private:
  unique_ptr<BigattiPivotStrategy> _pivot;
  BigattiParams _params;
  CommonParamsHelper _common;
};

#endif
