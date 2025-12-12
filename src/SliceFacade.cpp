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
#include "stdinc.h"
#include "SliceFacade.h"

#include "BigTermConsumer.h"
#include "CoefBigTermConsumer.h"
#include "TermTranslator.h"
#include "BigIdeal.h"
#include "Ideal.h"
#include "Term.h"
#include "MsmStrategy.h"
#include "TranslatingTermConsumer.h"
#include "TranslatingCoefTermConsumer.h"
#include "DebugStrategy.h"
#include "DecomRecorder.h"
#include "TermGrader.h"
#include "OptimizeStrategy.h"
#include "CanonicalCoefTermConsumer.h"
#include "HilbertStrategy.h"
#include "IOHandler.h"
#include "BigPolynomial.h"
#include "TotalDegreeCoefTermConsumer.h"
#include "CoefBigTermRecorder.h"
#include "CanonicalTermConsumer.h"
#include "VarSorter.h"
#include "StatisticsStrategy.h"
#include "IrreducibleIdealSplitter.h"
#include "SizeMaxIndepSetAlg.h"
#include "SliceParams.h"
#include "error.h"
#include "display.h"

#include <iterator>

SliceFacade::SliceFacade(const SliceParams& params, const DataType& output):
  Facade(params.getPrintActions()),
  _params(params) {
  _split = SplitStrategy::createStrategy(params.getSplit().c_str());
  _common.readIdealAndSetOutput(params, output);
}

SliceFacade::SliceFacade(const SliceParams& params,
                         const BigIdeal& ideal,
                         BigTermConsumer& consumer):
  Facade(params.getPrintActions()),
  _params(params) {
  _split = SplitStrategy::createStrategy(params.getSplit().c_str());
  _common.setIdealAndIdealOutput(params, ideal, consumer);
}

SliceFacade::SliceFacade(const SliceParams& params,
                         const BigIdeal& ideal,
                         CoefBigTermConsumer& consumer):
  Facade(params.getPrintActions()),
  _params(params) {
  _split = SplitStrategy::createStrategy(params.getSplit().c_str());
  _common.setIdealAndPolyOutput(params, ideal, consumer);
}

SliceFacade::~SliceFacade() {
}

void SliceFacade::computeMultigradedHilbertSeries() {
  ASSERT(isFirstComputation());
  beginAction("Computing multigraded Hilbert-Poincare series.");

  unique_ptr<CoefTermConsumer> consumer = _common.makeTranslatedPolyConsumer();

  consumer->consumeRing(_common.getNames());
  consumer->beginConsuming();
  HilbertStrategy strategy(consumer.get(), _split.get());
  runSliceAlgorithmWithOptions(strategy);
  consumer->doneConsuming();

  endAction();
}

void SliceFacade::computeUnivariateHilbertSeries() {
  ASSERT(isFirstComputation());
  beginAction("Computing univariate Hilbert-Poincare series.");

  unique_ptr<CoefTermConsumer> consumer =
    _common.makeToUnivariatePolyConsumer();

  consumer->consumeRing(_common.getNames());
  consumer->beginConsuming();
  HilbertStrategy strategy(consumer.get(), _split.get());
  runSliceAlgorithmWithOptions(strategy);
  consumer->doneConsuming();

  endAction();
}

void SliceFacade::computeIrreducibleDecomposition(bool encode) {
  ASSERT(isFirstComputation());
  produceEncodedIrrDecom(*_common.makeTranslatedIdealConsumer(!encode));
}

mpz_class SliceFacade::computeDimension(bool codimension) {
  ASSERT(isFirstComputation());

  if (_common.getIdeal().containsIdentity()) {
    if (codimension) {
      // convert to mpz_class before increment to ensure no overflow.
      return mpz_class(_common.getIdeal().getVarCount()) + 1;
    } else
      return -1;
  }

  // todo: inline?
  takeRadical();

  beginAction("Preparing to compute dimension.");

  vector<mpz_class> v;
  fill_n(back_inserter(v), _common.getIdeal().getVarCount(), -1);

  endAction();

  mpz_class minusCodimension;
#ifdef DEBUG
  // Only define hasComponents when DEBUG is defined since otherwise
  // GCC will warn about hasComponents not being used.
  bool hasComponents =
#endif
    solveIrreducibleDecompositionProgram(v, minusCodimension, false);
  ASSERT(hasComponents);

  if (codimension)
    return -minusCodimension;
  else
    return v.size() + minusCodimension;
}

void SliceFacade::computePrimaryDecomposition() {
  ASSERT(isFirstComputation());

  size_t varCount = _common.getIdeal().getVarCount();

  Ideal irreducibleDecom(varCount);
  {
    DecomRecorder recorder(&irreducibleDecom);
    produceEncodedIrrDecom(recorder);
  }

  beginAction
    ("Computing primary decomposition from irreducible decomposition.");

  // Do intersection of each component also using irreducible
  // decomposition of the dual. We can't use the Alexander dual
  // methods, since those switch around the translator to emit altered
  // big integers, while keeping the small integers the same, but we
  // want to keep this in small integers. So we have to do the dual
  // thing here.

  // To get actual supports.
  _common.getTranslator().setInfinityPowersToZero(irreducibleDecom);

  // To collect same-support vectors together.
  irreducibleDecom.sortReverseLex();

  Term lcm(varCount);
  irreducibleDecom.getLcm(lcm);

  Term tmp(varCount);
  Term support(varCount);

  _common.getIdeal().clear();
  Ideal& primaryComponentDual = _common.getIdeal();
  Ideal primaryComponent(varCount);

  DecomRecorder recorder(&primaryComponent);

  unique_ptr<TermConsumer> consumer = _common.makeTranslatedIdealConsumer();
  consumer->consumeRing(_common.getNames());
  consumer->beginConsumingList();

  Ideal::const_iterator stop = irreducibleDecom.end();
  Ideal::const_iterator it = irreducibleDecom.begin();
  while (it != stop) {
    // Get all vectors with same support.
    support = *it;
    do {
      tmp.encodedDual(*it, lcm);
      primaryComponentDual.insert(tmp);
      ++it;
    } while (it != stop && support.hasSameSupport(*it));
    ASSERT(!primaryComponentDual.isZeroIdeal());

    _common.getTranslator().addPurePowersAtInfinity(primaryComponentDual);
    {
      MsmStrategy strategy(&recorder, _split.get());
      runSliceAlgorithmWithOptions(strategy);
    }
    _common.getTranslator().setInfinityPowersToZero(primaryComponent);

    consumer->beginConsuming();
    for (Ideal::const_iterator dualTerm = primaryComponent.begin();
         dualTerm != primaryComponent.end(); ++dualTerm) {
      tmp.encodedDual(*dualTerm, lcm);
      consumer->consume(tmp);
    }
    consumer->doneConsuming();

    primaryComponent.clear();
    primaryComponentDual.clear();
  }

  consumer->doneConsumingList();

  endAction();
}

void SliceFacade::computeMaximalStaircaseMonomials() {
  ASSERT(isFirstComputation());
  beginAction("Computing maximal staircase monomials.");

  unique_ptr<TermConsumer> consumer = _common.makeTranslatedIdealConsumer();
  consumer->consumeRing(_common.getNames());
  MsmStrategy strategy(consumer.get(), _split.get());
  runSliceAlgorithmWithOptions(strategy);

  endAction();
}

void SliceFacade::computeMaximalStandardMonomials() {
  ASSERT(isFirstComputation());

  beginAction("Preparing to compute maximal standard monomials.");
  _common.getTranslator().decrement();
  endAction();
  computeMaximalStaircaseMonomials();
}

void SliceFacade::computeAlexanderDual(const vector<mpz_class>& point) {
  ASSERT(isFirstComputation());
  ASSERT(point.size() == _common.getIdeal().getVarCount());

  beginAction("Ensuring specified point is divisible by lcm.");
  vector<mpz_class> lcm;
  getLcmOfIdeal(lcm);

  for (size_t var = 0; var < lcm.size(); ++var) {
    if (lcm[var] > point[var]) {
      endAction();
      reportError
        ("The specified point to dualize on is not divisible by the "
         "least common multiple of the minimal generators of the ideal.");
    }
  }
  endAction();

  beginAction("Preparing to compute Alexander dual.");
  _common.getTranslator().dualize(point);
  endAction();

  produceEncodedIrrDecom(*_common.makeTranslatedIdealConsumer());
}

void SliceFacade::computeAlexanderDual() {
  ASSERT(isFirstComputation());

  beginAction("Computing lcm for Alexander dual.");
  vector<mpz_class> lcm;
  getLcmOfIdeal(lcm);
  endAction();

  computeAlexanderDual(lcm);
}

void SliceFacade::computeAssociatedPrimes() {
  ASSERT(isFirstComputation());

  size_t varCount = _common.getIdeal().getVarCount();

  // Obtain generators of radical from irreducible decomposition.
  Ideal radical(varCount);
  {
    Ideal decom(varCount);
    DecomRecorder recorder(&decom);
    produceEncodedIrrDecom(recorder);

    beginAction("Computing associated primes from irreducible decomposition.");

    Term tmp(varCount);
    Ideal::const_iterator stop = decom.end();
    for (Ideal::const_iterator it = decom.begin(); it != stop; ++it) {
      for (size_t var = 0; var < varCount; ++var) {
        // We cannot just check whether (*it)[var] == 0, since the
        // added fake pure powers map to zero but are not themselves
        // zero.
        if (_common.getTranslator().getExponent(var, (*it)[var]) == 0)
          tmp[var] = 0;
        else
          tmp[var] = 1;
      }
      radical.insert(tmp);
    }
  }

  radical.removeDuplicates();


  // Output associated primes.
  setToZeroOne(_common.getTranslator());
  unique_ptr<TermConsumer> consumer = _common.makeTranslatedIdealConsumer();

  consumer->consumeRing(_common.getNames());
  consumer->beginConsuming();
  Term tmp(varCount);
  Ideal::const_iterator stop = radical.end();
  for (Ideal::const_iterator it = radical.begin(); it != stop; ++it) {
    tmp = *it;
    consumer->consume(tmp);
  }
  consumer->doneConsuming();

  endAction();
}

bool SliceFacade::solveIrreducibleDecompositionProgram
(const vector<mpz_class>& grading,
 mpz_class& optimalValue,
 bool reportAllSolutions) {
  ASSERT(isFirstComputation());
  ASSERT(grading.size() == _common.getIdeal().getVarCount());

  beginAction("Preparing to solve optimization program.");
  if (!_common.getIdeal().containsIdentity())
    _common.addPurePowersAtInfinity();
  endAction();

  return solveProgram(grading, optimalValue, reportAllSolutions);
}

bool SliceFacade::solveStandardProgram
(const vector<mpz_class>& grading,
 mpz_class& optimalValue,
 bool reportAllSolutions) {
  ASSERT(isFirstComputation());
  ASSERT(grading.size() == _common.getIdeal().getVarCount());

  _common.getTranslator().decrement();
  return solveProgram(grading, optimalValue, reportAllSolutions);
}

void SliceFacade::produceEncodedIrrDecom(TermConsumer& consumer) {
  ASSERT(isFirstComputation());
  beginAction("Computing irreducible decomposition.");

  _common.addPurePowersAtInfinity();
  MsmStrategy strategy(&consumer, _split.get());

  consumer.consumeRing(_common.getNames());
  runSliceAlgorithmWithOptions(strategy);

  endAction();
}

bool SliceFacade::solveProgram(const vector<mpz_class>& grading,
                               mpz_class& optimalValue,
                               bool reportAllSolutions) {
  ASSERT(isFirstComputation());
  ASSERT(grading.size() == _common.getIdeal().getVarCount());

  if (_params.getUseIndependenceSplits()) {
    displayNote
      ("Turning off Independence splits as they are not supported\n"
       "for optimization.");
    _params.useIndependenceSplits(false);
  }

  if (_params.getUseBoundSimplification() &&
      !_params.getUseBoundElimination()) {
    displayNote
      ("Bound simplification requires using the bound to eliminate\n"
       "non-improving slices, which has been turned off. Am now turning\n"
       "this on.");
    _params.useBoundElimination(true);
  }

  beginAction("Solving optimization program.");

  OptimizeStrategy::BoundSetting boundSetting;
  if (_params.getUseBoundSimplification()) {
    ASSERT(_params.getUseBoundElimination());
    boundSetting = OptimizeStrategy::UseBoundToEliminateAndSimplify;
  } else if (_params.getUseBoundElimination())
    boundSetting = OptimizeStrategy::UseBoundToEliminate;
  else
    boundSetting = OptimizeStrategy::DoNotUseBound;

  TermGrader grader(grading, _common.getTranslator());
  OptimizeStrategy strategy
    (grader, _split.get(), reportAllSolutions, boundSetting);
  runSliceAlgorithmWithOptions(strategy);

  endAction();

  const Ideal& solution = strategy.getMaximalSolutions();

  unique_ptr<TermConsumer> consumer = _common.makeTranslatedIdealConsumer();
  consumer->consumeRing(_common.getNames());
  consumer->consume(solution);

  if (solution.isZeroIdeal())
    return false;
  else {
    optimalValue = strategy.getMaximalValue();
    return true;
  }
}

bool SliceFacade::isFirstComputation() const {
  return _common.hasIdeal();
}

void SliceFacade::takeRadical() {
  ASSERT(isFirstComputation());

  beginAction("Taking radical of ideal.");

  bool skip = false;
  Term lcm(_common.getIdeal().getVarCount());
  _common.getIdeal().getLcm(lcm);
  if (lcm.isSquareFree())
    skip = true;

  if (!skip) {
    _common.getTranslator().setInfinityPowersToZero(_common.getIdeal());
    _common.getIdeal().takeRadicalNoMinimize();
    _common.getIdeal().minimize();
  }

  setToZeroOne(_common.getTranslator());

  endAction();
}

void SliceFacade::getLcmOfIdeal(vector<mpz_class>& bigLcm) {
  ASSERT(isFirstComputation());

  Term lcm(_common.getIdeal().getVarCount());
  _common.getIdeal().getLcm(lcm);

  bigLcm.clear();
  bigLcm.reserve(_common.getIdeal().getVarCount());
  for (size_t var = 0; var < _common.getIdeal().getVarCount(); ++var)
    bigLcm.push_back(_common.getTranslator().getExponent(var, lcm));
}

void SliceFacade::runSliceAlgorithmWithOptions(SliceStrategy& strategy) {
  ASSERT(isFirstComputation());
  strategy.setUseIndependence(_params.getUseIndependenceSplits());
  strategy.setUseSimplification(_params.getUseSimplification());

  SliceStrategy* strategyWithOptions = &strategy;

  unique_ptr<SliceStrategy> debugStrategy;
  if (_params.getPrintDebug()) {
    debugStrategy.reset
      (new DebugStrategy(strategyWithOptions, stderr));
    strategyWithOptions = debugStrategy.get();
  }

  unique_ptr<SliceStrategy> statisticsStrategy;
  if (_params.getPrintStatistics()) {
    statisticsStrategy.reset
      (new StatisticsStrategy(strategyWithOptions, stderr));
    strategyWithOptions = statisticsStrategy.get();
  }

  ASSERT(strategyWithOptions != 0);
  strategyWithOptions->run(_common.getIdeal());
}
