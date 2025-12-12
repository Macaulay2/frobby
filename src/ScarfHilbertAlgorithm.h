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
#ifndef SCARF_HILBERT_ALGORITHM_GUARD
#define SCARF_HILBERT_ALGORITHM_GUARD

class Consumer;
class CoefTermConsumer;
class TermTranslator;
class CoefBigTermConsumer;
class ScarfParams;
class IdealOrderer;
class IdealTree;

#include "Term.h"
#include "Ideal.h"

class ScarfHilbertAlgorithm {
 public:
  ScarfHilbertAlgorithm(const TermTranslator& translator,
                        const ScarfParams& params,
                        unique_ptr<IdealOrderer> enumerationOrder,
                        unique_ptr<IdealOrderer> deformationOrder);
  ~ScarfHilbertAlgorithm();

  void runGeneric(const Ideal& ideal,
                  CoefBigTermConsumer& consumer,
                  bool univariate,
                  bool canonical);

 private:
  struct State {
    Term term;
    Ideal::const_iterator pos;
    vector<Exponent*> face;
    bool plus;
  };
  vector<State> _states;

  void enumerateScarfComplex(const Ideal& ideal,
                             CoefTermConsumer& consumer);
  void initializeEnumeration(const Ideal& ideal,
                             size_t& activeStateCount);
  bool doEnumerationStep(const Ideal& ideal,
                         const IdealTree& tree,
                         State& state,
                         State& nextState);
  void doEnumerationBaseCase(const State& state,
                             CoefTermConsumer& consumer);

  const TermTranslator& _translator;
  const ScarfParams& _params;
  const unique_ptr<IdealOrderer> _enumerationOrder;
  const unique_ptr<IdealOrderer> _deformationOrder;

  size_t _totalStates;
  size_t _totalFaces;
};

#endif
