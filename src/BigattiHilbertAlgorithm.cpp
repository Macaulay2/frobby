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
#include "BigattiHilbertAlgorithm.h"

#include "Ideal.h"
#include "CoefBigTermConsumer.h"
#include "BigattiState.h"

BigattiHilbertAlgorithm::
BigattiHilbertAlgorithm
(auto_ptr<Ideal> ideal,
 const TermTranslator& translator,
 const BigattiParams& params,
 auto_ptr<BigattiPivotStrategy> pivot,
 CoefBigTermConsumer& consumer):
 _translator(translator),
 _consumer(&consumer),
 _baseCase(translator),
 _pivot(std::move(pivot)),
 _computeUnivariate(false),
 _params(params) {

  ASSERT(ideal.get() != 0);
  ASSERT(ideal->isMinimallyGenerated());
  _varCount = ideal->getVarCount();
  _tmp_simplify_gcd.reset(_varCount);

  _baseCase.setPrintDebug(_params.getPrintDebug());

  // TODO: use swap to avoid copy of ideal.
  _tasks.addTask(new BigattiState(this, *ideal, Term(_varCount)));
}

void BigattiHilbertAlgorithm::setComputeUnivariate(bool value) {
  _computeUnivariate = value;
}

void BigattiHilbertAlgorithm::run() {
  if (_pivot.get() == 0)
    _pivot = BigattiPivotStrategy::createStrategy("median", true);

  _baseCase.setComputeUnivariate(_computeUnivariate);
  _tasks.runTasks();
  _baseCase.feedOutputTo(*_consumer, _params.getProduceCanonicalOutput());

  if (_params.getPrintStatistics()) {
    fputs("*** Statistics for run of Bigatti algorithm ***\n", stderr);
    fprintf(stderr, " %u states processed.\n",
            (unsigned int)_tasks.getTotalTasksEver());
    fprintf(stderr, " %u base cases.\n",
            (unsigned int)_baseCase.getTotalBaseCasesEver());
    fprintf(stderr, " %u terms output.\n",
            (unsigned int)_baseCase.getTotalTermsOutputEver());
    fprintf(stderr, " %u terms in final output.\n",
            (unsigned int)_baseCase.getTotalTermsInOutput());
  }
}

void BigattiHilbertAlgorithm::processState(auto_ptr<BigattiState> state) {
  if (_params.getUseSimplification())
    simplify(*state);

  if (_params.getPrintDebug()) {
    fputs("Debug: Processing state.\n", stderr);
    state->print(stderr);
  }

  bool isBaseCase = _params.getUseGenericBaseCase() ?
    _baseCase.genericBaseCase(*state) :
    _baseCase.baseCase(*state);
  if (isBaseCase) {
    freeState(std::move(state));
    return;
  }

  const Term& pivot = _pivot->getPivot(*state);
  if (_params.getPrintDebug()) {
    fputs("Debug: Performing pivot split on ", stderr);
    pivot.print(stderr);
    fputs(".\n", stderr);
  }
  ASSERT(!pivot.isIdentity());
  ASSERT(!state->getIdeal().contains(pivot));

  auto_ptr<BigattiState> colonState(_stateCache.newObjectCopy(*state));
  colonState->colonStep(pivot);
  _tasks.addTask(colonState.release());

  state->addStep(pivot);
  _tasks.addTask(state.release());
}

void BigattiHilbertAlgorithm::simplify(BigattiState& state) {
  Term& gcd = _tmp_simplify_gcd;
  ASSERT(gcd.getVarCount() == _varCount);

  state.getIdeal().getGcd(gcd);
  if (!gcd.isIdentity()) {
    // Do colon and output multiply-gcd*multiply.
    _baseCase.output(true, state.getMultiply());
    state.colonStep(gcd);
    _baseCase.output(false, state.getMultiply());
  }

  IF_DEBUG(state.getIdeal().getGcd(gcd));
  ASSERT(gcd.isIdentity());
}

void BigattiHilbertAlgorithm::freeState(auto_ptr<BigattiState> state) {
  state->getIdeal().clear(); // To preserve memory
  _stateCache.freeObject(std::move(state));
}
