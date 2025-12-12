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
#include "MsmStrategy.h"

#include "MsmSlice.h"
#include "Term.h"
#include "Ideal.h"
#include "TermTranslator.h"
#include <vector>
#include "Projection.h"
#include "TermGrader.h"

MsmStrategy::MsmStrategy(TermConsumer* consumer,
                         const SplitStrategy* splitStrategy):
  SliceStrategyCommon(splitStrategy),
  _consumer(consumer) {
  ASSERT(consumer != 0);
}

MsmStrategy::MsmStrategy(TermConsumer* consumer,
                         const SplitStrategy* splitStrategy,
                         const Ideal& initialSubtract):
  SliceStrategyCommon(splitStrategy),
  _consumer(consumer),
  _initialSubtract(new Ideal(initialSubtract)) {
  ASSERT(consumer != 0);
}

void MsmStrategy::run(const Ideal& ideal) {
  ASSERT(_initialSubtract.get() == 0 ||
         _initialSubtract->getVarCount() == ideal.getVarCount());

  _consumer->beginConsuming();
  size_t varCount = ideal.getVarCount();
  if (_initialSubtract.get() == 0)
    _initialSubtract = unique_ptr<Ideal>(new Ideal(varCount));

  Term sliceMultiply(varCount);
  for (size_t var = 0; var < varCount; ++var)
    sliceMultiply[var] = 1;

  unique_ptr<Slice> slice
    (new MsmSlice(*this, ideal, *_initialSubtract, sliceMultiply, _consumer));
  simplify(*slice);

  _initialSubtract.reset();
  _tasks.addTask(slice.release());
  _tasks.runTasks();
  _consumer->doneConsuming();
}

bool MsmStrategy::processSlice(TaskEngine& tasks, unique_ptr<Slice> slice) {
  ASSERT(slice.get() != 0);

  if (slice->baseCase(getUseSimplification())) {
    freeSlice(std::move(slice));
    return true;
  }

  if (getUseIndependence() && _indep.analyze(*slice))
    independenceSplit(std::move(slice));
  else if (_split->isLabelSplit())
    labelSplit(std::move(slice));
  else {
    ASSERT(_split->isPivotSplit());
    pivotSplit(std::move(slice));
  }

  return false;
}

unique_ptr<MsmSlice> MsmStrategy::newMsmSlice() {
  unique_ptr<Slice> slice(newSlice());
  ASSERT(dynamic_cast<MsmSlice*>(slice.get()) != 0);
  return unique_ptr<MsmSlice>(static_cast<MsmSlice*>(slice.release()));
}

unique_ptr<Slice> MsmStrategy::allocateSlice() {
  return unique_ptr<Slice>(new MsmSlice(*this));
}

bool MsmStrategy::debugIsValidSlice(Slice* slice) {
  ASSERT(slice != 0);
  ASSERT(dynamic_cast<MsmSlice*>(slice) != 0);
  return true;
}

void MsmStrategy::labelSplit(unique_ptr<Slice> sliceParam) {
  ASSERT(sliceParam.get() != 0);
  ASSERT(debugIsValidSlice(sliceParam.get()));
  unique_ptr<MsmSlice> slice
    (static_cast<MsmSlice*>(sliceParam.release()));

  ASSERT(!slice->adjustMultiply());
  ASSERT(!slice->normalize());
  ASSERT(_split != 0);
  size_t var = _split->getLabelSplitVariable(*slice);

  Term term(slice->getVarCount());

  const Term& lcm = slice->getLcm();

  Ideal::const_iterator stop = slice->getIdeal().end();
  Ideal::const_iterator label = stop;
  bool hasTwoLabels = false;
  for (Ideal::const_iterator it = slice->getIdeal().begin();
       it != stop; ++it) {
    if ((*it)[var] == 1) {
      term = *it;
      term[var] -= 1;

      bool couldBeLabel = !slice->getSubtract().contains(term);
      if (couldBeLabel) {
        for (size_t v = 0; v < slice->getVarCount(); ++v) {
          if (term[v] == lcm[v]) {
            couldBeLabel = false;
            break;
          }
        }
      }

      if (couldBeLabel) {
        if (label == stop)
          label = it;
        else {
          hasTwoLabels = true;
          break;
        }
      }
    }
  }

  unique_ptr<Slice> hasLabelSlice;

  if (label != stop) {
    term = *label;
    term[var] -= 1;

    hasLabelSlice = newSlice();
    *hasLabelSlice = *slice;
    hasLabelSlice->innerSlice(term);

    if (hasTwoLabels)
      slice->outerSlice(term);
  }

  if (!hasTwoLabels) {
    term.setToIdentity();
    term[var] = 1;
    slice->innerSlice(term);
  }

  if (hasLabelSlice.get() != 0) {
    simplify(*hasLabelSlice);
    _tasks.addTask(hasLabelSlice.release());
  }

  simplify(*slice);
  _tasks.addTask(slice.release());
}

class MsmIndependenceSplit : public TermConsumer, public Task {
public:
  Task* getLeftEvent() {
    return this;
  }

  TermConsumer* getLeftConsumer() {
    return this;
  }

  TermConsumer* getRightConsumer() {
    return &_rightConsumer;
  }

  const Projection& getLeftProjection() {
    return _leftProjection;
  }

  const Projection& getRightProjection() {
    return _rightProjection;
  }

  void reset(TermConsumer* consumer,
             IndependenceSplitter& splitter) {
    _consumer = consumer;
    _tmpTerm.reset(splitter.getVarCount());

    splitter.getBigProjection(_leftProjection);
    splitter.getRestProjection(_rightProjection);

    _rightConsumer._decom.clearAndSetVarCount
      (_rightProjection.getRangeVarCount());
  }

private:
  virtual void run(TaskEngine& engine) {
    dispose();
  }

  virtual void dispose() {
    delete this;
  }

  virtual void beginConsuming() {
  }

  virtual void doneConsuming() {
  }

  virtual void consume(const Term& term) {
    _leftProjection.inverseProject(_tmpTerm, term);
    Ideal::const_iterator stop = _rightConsumer._decom.end();
    for (Ideal::const_iterator it = _rightConsumer._decom.begin();
         it != stop; ++it) {
      _rightProjection.inverseProject(_tmpTerm, *it);
      _consumer->consume(_tmpTerm);
    }
  }

  struct RightConsumer : public TermConsumer {
    virtual void beginConsuming() {
    }

    virtual void doneConsuming() {
    }

    virtual void consume(const Term& term) {
      _decom.insert(term);
    }

    Ideal _decom;
  } _rightConsumer;

  TermConsumer* _consumer;

  Projection _leftProjection;
  Projection _rightProjection;

  Term _tmpTerm;
};

void MsmStrategy::independenceSplit(unique_ptr<Slice> sliceParam) {
  ASSERT(sliceParam.get() != 0);
  ASSERT(debugIsValidSlice(sliceParam.get()));
  unique_ptr<MsmSlice> slice
    (static_cast<MsmSlice*>(sliceParam.release()));

  // Construct split object
  unique_ptr<MsmIndependenceSplit> autoSplit(new MsmIndependenceSplit());
  autoSplit->reset(slice->getConsumer(), _indep);
  MsmIndependenceSplit* split = autoSplit.release();
  _tasks.addTask(split); // Runs when we are done with all of this split.

  // Construct left slice.
  unique_ptr<MsmSlice> leftSlice(new MsmSlice(*this));
  leftSlice->setToProjOf(*slice, split->getLeftProjection(), split);
  _tasks.addTask(leftSlice.release());

  // Construct right slice.
  unique_ptr<MsmSlice> rightSlice(new MsmSlice(*this));
  rightSlice->setToProjOf(*slice, split->getRightProjection(),
                          split->getRightConsumer());
  _tasks.addTask(rightSlice.release());

  // Deal with slice.
  freeSlice(std::move(slice));
}

void MsmStrategy::getPivot(Term& pivot, Slice& slice) {
  ASSERT(_split != 0);
  ASSERT(_split->isPivotSplit());

  _split->getPivot(pivot, slice);
}

void MsmStrategy::getPivot(Term& pivot, Slice& slice, const TermGrader& grader) {
  ASSERT(_split != 0);
  ASSERT(_split->isPivotSplit());

  _split->getPivot(pivot, slice, grader);
}
