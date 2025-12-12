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
#ifndef HILBERT_STRATEGY_GUARD
#define HILBERT_STRATEGY_GUARD

#include <vector>
#include "IndependenceSplitter.h"
#include "SliceStrategyCommon.h"
#include "ElementDeleter.h"
#include "HilbertIndependenceConsumer.h"

class HilbertSlice;
class Ideal;
class CoefTermConsumer;
class SliceEvent;
class Slice;
class SplitStrategy;

class HilbertStrategy : public SliceStrategyCommon {
 public:
  HilbertStrategy(CoefTermConsumer* consumer,
                  const SplitStrategy* splitStrategy);

  virtual void run(const Ideal& ideal);

  virtual bool processSlice(TaskEngine& tasks, unique_ptr<Slice> slice);

  void freeConsumer(unique_ptr<HilbertIndependenceConsumer> consumer);

 private:
  unique_ptr<HilbertIndependenceConsumer> newConsumer();

  unique_ptr<HilbertSlice> newHilbertSlice();
  virtual unique_ptr<Slice> allocateSlice();
  virtual bool debugIsValidSlice(Slice* slice);

  virtual void getPivot(Term& term, Slice& slice);

  IndependenceSplitter _indepSplitter;
  void independenceSplit(unique_ptr<Slice> slice);

  vector<HilbertIndependenceConsumer*> _consumerCache;
  ElementDeleter<vector<HilbertIndependenceConsumer*> > _consumerCacheDeleter;

  CoefTermConsumer* _consumer;
  bool _useIndependence;
};

#endif
