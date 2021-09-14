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
#ifndef PIVOT_EULER_ALG_GUARD
#define PIVOT_EULER_ALG_GUARD

#include "PivotStrategy.h"

#include <vector>
#include <cstdio>

class Ideal;
class RawSquareFreeIdeal;
class EulerState;

class PivotEulerAlg {
 public:
  PivotEulerAlg();

  const mpz_class& computeEulerCharacteristic(const Ideal& ideal);
  const mpz_class& computeEulerCharacteristic(const RawSquareFreeIdeal& ideal);
  const mpz_class& getComputedEulerCharacteristic() const {return _euler;}

  void setPivotStrategy(auto_ptr<PivotStrategy> strategy) {
	_pivotStrategy = std::move(strategy);
  }

  void setInitialAutoTranspose(bool value) {_initialAutoTranspose = value;}
  bool getInitialAutoTranspose() const {return _initialAutoTranspose;}

  void setAutoTranspose(bool value) {_autoTranspose = value;}
  bool getAutoTranspose() const {return _autoTranspose;}

  void setUseUniqueDivSimplify(bool value) {_useUniqueDivSimplify = value;}
  bool getUseUniqueDivSimplify() const {return _useUniqueDivSimplify;}

  void setUseManyDivSimplify(bool value) {_useManyDivSimplify = value;}
  bool getUseManyDivSimplify() const {return _useManyDivSimplify;}

  void setUseAllPairsSimplify(bool value) {_useAllPairsSimplify = value;}
  bool getUseAllPairsSimplify() const {return _useAllPairsSimplify;}

 private:
  void computeEuler(EulerState* state);
  bool autoTranspose(EulerState& state);

  EulerState* processState(EulerState& state);
  void getPivot(const EulerState& state, Word* pivot);

  mpz_class _euler;
  Word* _termTmp;
  vector<size_t> _divCountsTmp;

  bool _useUniqueDivSimplify;
  bool _useManyDivSimplify;
  bool _useAllPairsSimplify;
  bool _autoTranspose;
  bool _initialAutoTranspose;
  auto_ptr<PivotStrategy> _pivotStrategy;
};

#endif
