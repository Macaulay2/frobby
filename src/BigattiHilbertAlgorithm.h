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
#ifndef BIGATTI_HILBERT_ALGORITHM_GUARD
#define BIGATTI_HILBERT_ALGORITHM_GUARD

#include "TaskEngine.h"
#include "Ideal.h"
#include "ObjectCache.h"
#include "BigattiState.h"
#include "BigattiBaseCase.h"
#include "BigattiPivotStrategy.h"
#include "BigattiParams.h"

class CoefBigTermConsumer;
class Term;

class BigattiHilbertAlgorithm {
public:
  /** Construct an object for running the Bigatti et.al. algorithm on
   ideal. Both translator and consumer must remain valid for the
   lifetime of this object.
  */
  BigattiHilbertAlgorithm(unique_ptr<Ideal> ideal,
                          const TermTranslator& translator,
                          const BigattiParams& params,
                          unique_ptr<BigattiPivotStrategy> pivot,
                          CoefBigTermConsumer& consumer);

  void setPrintStatistics(bool value);
  void setPrintDebug(bool value);
  void setUseSimplification(bool value);
  void setDoCanonicalOutput(bool value);
  void setComputeUnivariate(bool value);

  void run();

private:
    void processState(unique_ptr<BigattiState> state);
    void getPivot(BigattiState& state, size_t& var, Exponent& e);
    void simplify(BigattiState& state);

    void freeState(unique_ptr<BigattiState> state);

    size_t _varCount;
    const TermTranslator& _translator;
    CoefBigTermConsumer* _consumer;
    TaskEngine _tasks;
    ObjectCache<BigattiState> _stateCache;

    Term _tmp_processState_pivot;
    Term _tmp_simplify_gcd;

    BigattiBaseCase _baseCase;

    unique_ptr<BigattiPivotStrategy>  _pivot;

    bool _computeUnivariate;
    BigattiParams _params;

    friend class BigattiState;
};

#endif
