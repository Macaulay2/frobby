/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2011 Bjarke Hammersholt Roune (www.broune.com)

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
#include "InputConsumer.h"

#include "Scanner.h"
#include "FrobbyStringStream.h"
#include "error.h"
#include "RawSquareFreeTerm.h"

#include <iostream>

InputConsumer::InputConsumer():
  _idealsDeleter(_ideals),
  _inIdeal(false),
  _requireSquareFree(false) {
}

void InputConsumer::consumeRing(const VarNames& names) {
  VarNames nameCopy(names); // exception safety: copy and swap
  if (_inIdeal) {
	ASSERT(names.getVarCount() == _names.getVarCount());
	if (_sqfIdeal.get() != 0)
	  _sqfIdeal->renameVars(names);
	else
	  _bigIdeal->renameVars(names);
  }
  _names.swap(nameCopy);
}

void InputConsumer::requireSquareFree() {
  ASSERT(!_inIdeal);
  _requireSquareFree = true;
}

void InputConsumer::beginIdeal() {
  ASSERT(!_inIdeal);
  _inIdeal = true;
  _sqfIdeal.reset(new SquareFreeIdeal(_names));
  _term.resize(_names.getVarCount());
}

void InputConsumer::hintGenCount(size_t hintGenCountParam) {
  ASSERT(_inIdeal);
  _bigIdeal->reserve(hintGenCountParam);
}

void InputConsumer::beginTerm() {
  ASSERT(_inIdeal);
  ASSERT(_term.size() == _names.getVarCount());
#ifdef DEBUG
  for (size_t var = 0; var < _term.size(); ++var) {
	ASSERT(_term[var].empty());
  }
#endif
  if (_sqfIdeal.get() != 0)
    _sqfIdeal->insertIdentity();
  else
    _bigIdeal->newLastTerm();
}

size_t InputConsumer::consumeVarNumber(Scanner& in) {
  ASSERT(_inIdeal);

  size_t var;
  in.readSizeT(var);

  if (var == 0 || var > _names.getVarCount()) {
	FrobbyStringStream errorMsg;
	errorMsg << "There is no variable number " << var << '.';
	reportSyntaxError(in, errorMsg);
  }
  return var - 1;
}

size_t InputConsumer::consumeVar(Scanner& in) {
  ASSERT(_inIdeal);
  in.readIdentifier(_tmpString);
  size_t var = _names.getIndex(_tmpString);
  if (var == VarNames::invalidIndex) {
    FrobbyStringStream errorMsg;
    errorMsg << "Unknown variable \"" << _tmpString << "\". Maybe you forgot a *.";
    reportSyntaxError(in, errorMsg);
  }
  return var;
}

void InputConsumer::consumeVarExponentOne(size_t var, const Scanner& in) {
  ASSERT(_inIdeal);
  ASSERT(var < _names.getVarCount());

  if (_sqfIdeal.get() != 0) {
    Word* back = _sqfIdeal->back();
    if (!SquareFreeTermOps::getExponent(back, var)) {
      SquareFreeTermOps::setExponent(back, var, true);
      return;
    }
  } else {
    mpz_class& exponent = _bigIdeal->getLastTermExponentRef(var);
    if (exponent == 0) {
      exponent = 1;
      return;
    }
  }
  errorVariableAppearsTwice(in, var);
}

void InputConsumer::consumeVarExponent(size_t var, Scanner& in) {
  ASSERT(_inIdeal);
  ASSERT(var < _names.getVarCount());

  if (_sqfIdeal.get() != 0) {
    in.readIntegerNoSign(_tmpString);
    Word* back = _sqfIdeal->back();
    if (!SquareFreeTermOps::getExponent(back, var)) {
      if (_tmpString == "1")
        SquareFreeTermOps::setExponent(back, var, true);
      else if (_tmpString != "0") {
        idealNotSquareFree();
        _bigIdeal->getLastTermExponentRef(var) = _tmpString;
      }
      return;
    }
  } else {
    mpz_class& exponent = _bigIdeal->getLastTermExponentRef(var);
    if (exponent == 0) {
      in.readIntegerNoSign(exponent);
      return;
    }
  }
  errorVariableAppearsTwice(in, var);
}

void InputConsumer::consumeTermProductNotation(Scanner& in) {
  ASSERT(_inIdeal);

  beginTerm();
  if (!in.match('1')) {
    do {
      const size_t var = consumeVar(in);
      if (in.match('^'))
        consumeVarExponent(var, in);
      else
        consumeVarExponentOne(var, in);
    } while (in.match('*'));
  }
  endTerm();
}

void InputConsumer::endTerm() {
  ASSERT(_inIdeal);
/*  const size_t varCount = _names.getVarCount();
  if (_sqfIdeal.get() != 0) {
    if (_sqfIdeal->insert(_term)) {
      for (size_t var = 0; var < varCount; ++var)
        _term[var].clear();
      return;
    }
    if (_requireSquareFree)
      reportError("Expected square free term.");
    toBigIdeal(_sqfIdeal, _bigIdeal);
  }
  ASSERT(!_requireSquareFree);

  ASSERT(_bigIdeal.get() != 0);
  _bigIdeal->newLastTerm();
  for (size_t var = 0; var < varCount; ++var) {
    std::string& str = _term[var];
    if (str.empty())
      continue;
    mpz_class& integer = _bigIdeal->getLastTermExponentRef(var);
    mpz_set_str(integer.get_mpz_t(), str.c_str(), 10);
    str.clear();
  }*/
}

void InputConsumer::endIdeal() {
  ASSERT(_inIdeal);
  _inIdeal = false;
  auto_ptr<Entry> entry(new Entry());
  entry->_big = std::move(_bigIdeal);
  entry->_sqf = std::move(_sqfIdeal);
  exceptionSafePushBack(_ideals, std::move(entry));
}

void InputConsumer::releaseIdeal(auto_ptr<SquareFreeIdeal>& sqf, auto_ptr<BigIdeal>& big) {
  ASSERT(!_inIdeal);
  ASSERT(!empty());
  Entry entry;
  releaseIdeal(entry);
  sqf = std::move(entry._sqf);
  big = std::move(entry._big);
}

auto_ptr<BigIdeal> InputConsumer::releaseBigIdeal() {
  ASSERT(!_inIdeal);
  ASSERT(!empty());
  Entry entry;
  releaseIdeal(entry);
  toBigIdeal(entry._sqf, entry._big);
  return std::move(entry._big);
}

auto_ptr<SquareFreeIdeal> InputConsumer::releaseSquareFreeIdeal() {
  ASSERT(!_inIdeal);
  ASSERT(!empty());
  ASSERT(_ideals.front()->_sqf.get() != 0);
  Entry entry;
  releaseIdeal(entry);
  return std::move(entry._sqf);
}

void InputConsumer::releaseIdeal(Entry& entry) {
  ASSERT(!_inIdeal);
  ASSERT(!empty());
  entry = std::move(*_ideals.front());
  _ideals.pop_front();
}

void InputConsumer::errorVariableAppearsTwice(const Scanner& in, size_t var) {
  FrobbyStringStream errorMsg;
  errorMsg << "The variable " << _names.getName(var)
    << " appears twice in the same monomial.";
  reportSyntaxError(in, errorMsg);
}

void InputConsumer::idealNotSquareFree() {
  if (_requireSquareFree)
    reportError("Expected square free term.");
  toBigIdeal(_sqfIdeal, _bigIdeal);
}

void InputConsumer::toBigIdeal(std::auto_ptr<SquareFreeIdeal>& sqf, std::auto_ptr<BigIdeal>& big) {
  if (big.get() != 0)
    return;
  ASSERT(sqf.get() != 0);
  big.reset(new BigIdeal(sqf->getNames()));
  big->insert(*sqf);
  sqf.reset(0);
}
