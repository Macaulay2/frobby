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
#include "AnalyzeAction.h"

#include "BigIdeal.h"
#include "IOFacade.h"
#include "IdealFacade.h"
#include "Scanner.h"
#include "IOHandler.h"
#include "BigTermConsumer.h"
#include "DataType.h"

#include <algorithm>

class AnalyzeConsumer : public BigTermConsumer {
public:
  AnalyzeConsumer():
    _generatorCount(0) {
  }

  virtual void consumeRing(const VarNames& names) {
    _names = names;
    _lcm.clear();
    _lcm.resize(_names.getVarCount());
  }

  virtual void beginConsuming() {
  }

  using BigTermConsumer::consume;

  virtual void consume(const Term& term, const TermTranslator& translator) {
    BigTermConsumer::consume(term, translator);
  }

  virtual void consume(const vector<mpz_class>& term) {
    ASSERT(term.size() == _names.getVarCount());

    ++_generatorCount;
    for (size_t var = 0; var < term.size(); ++var)
      if (_lcm[var] < term[var])
        _lcm[var] = term[var];
  }

  virtual void doneConsuming() {
  }

  size_t getGeneratorCount() const {
    return _generatorCount;
  }

  const VarNames& getNames() const {
    return _names;
  }

  const vector<mpz_class>& getLcm() const {
    return _lcm;
  }

  const mpz_class& getMaximumExponent() const {
    ASSERT(_lcm.size() > 0);
    return *max_element(_lcm.begin(), _lcm.end());
  }

private:
  VarNames _names;
  size_t _generatorCount;
  vector<mpz_class> _lcm;
};

AnalyzeAction::AnalyzeAction():
  Action
(staticGetName(),
 "Display information about the input ideal.",
 "Display information about input ideal. This is useful for getting a quick\n"
 "impression of how the ideal looks like, and it can be used in scripts\n"
 "that need information about the ideal.",
 false),

  _io(DataType::getMonomialIdealType(), DataType::getMonomialIdealType()),

  _summaryLevel
  ("summaryLevel",
   "If non-zero, then print a summary of the ideal to the error output\n"
   "stream. A higher summary level results in more expensive analysis in\n"
   "order to provide more information. Currently levels 0, 1 and 2 are\n"
   "available.",
   1),

  _printLcm
  ("lcm",
   "Print the least common multiple of the generators.",
   false),

  _printVarCount
  ("varCount",
   "Print the number of variables.",
   false),

  _printGeneratorCount
  ("genCount",
   "Print the number of generators.",
   false),

  _printMaximumExponent
  ("maxExp",
   "Print the largest exponent that appears in the input file",
   false),

  _printMinimal
  ("minimal",
   "Print 1 if the ideal has no non-minimal generators. Print 0 otherwise.",
   false) {
}

void AnalyzeAction::obtainParameters(vector<Parameter*>& parameters) {
  parameters.push_back(&_summaryLevel);
  parameters.push_back(&_printLcm);
  parameters.push_back(&_printVarCount);
  parameters.push_back(&_printGeneratorCount);
  parameters.push_back(&_printMaximumExponent);
  parameters.push_back(&_printMinimal);

  _io.obtainParameters(parameters);
  Action::obtainParameters(parameters);
}

void AnalyzeAction::perform() {
  Scanner in(_io.getInputFormat(), stdin);
  _io.autoDetectInputFormat(in);
  _io.validateFormats();

  AnalyzeConsumer consumer;

  // We only read the entire ideal into memory at once if we have to.
  IOFacade ioFacade(_printActions);
  if (!requiresWholeIdeal()) {
    ioFacade.readIdeal(in, consumer);
    in.expectEOF();

    analyzeStreaming(consumer);
  } else {
    BigIdeal ideal;
    ioFacade.readIdeal(in, ideal);
    in.expectEOF();

    consumer.consume(ideal);

    analyzeStreaming(consumer);
    analyzeIdeal(ideal);
  }
}

bool AnalyzeAction::requiresWholeIdeal() const {
  return _printMinimal || _summaryLevel > 1;
}

void AnalyzeAction::analyzeIdeal(BigIdeal& ideal) const {
  IdealFacade idealFacade(_printActions);

  if (_printMinimal) {
    size_t generatorCount = ideal.getGeneratorCount();
    idealFacade.sortAllAndMinimize(ideal);
    if (generatorCount == ideal.getGeneratorCount())
      fputs("1\n", stdout);
    else
      fputs("0\n", stdout);
  }

  if (_summaryLevel >= 2) {
    idealFacade.printAnalysis(stdout, ideal);
  }
}

void AnalyzeAction::analyzeStreaming(AnalyzeConsumer& consumer) const {
  IOFacade ioFacade(_printActions);

  if (_printLcm) {
    unique_ptr<IOHandler> output = _io.createOutputHandler();
    ioFacade.writeTerm(consumer.getLcm(), consumer.getNames(),
                       output.get(), stdout);
    fputc('\n', stdout);
  }

  if (_printVarCount)
    fprintf(stdout, "%lu\n", (unsigned long)consumer.getNames().getVarCount());
  if (_printGeneratorCount)
    fprintf(stdout, "%lu\n", (unsigned long)consumer.getGeneratorCount());

  if (_printMaximumExponent) {
    if (consumer.getNames().getVarCount() == 0)
      fputs("0\n", stdout);
    else
      gmp_fprintf(stdout, "%Zd\n", consumer.getMaximumExponent().get_mpz_t());
  }

  if (_summaryLevel.getValue() == 1) {
    fprintf(stdout, "%lu generators\n",
            (unsigned long)consumer.getGeneratorCount());
    fprintf(stdout, "%lu variables\n",
            (unsigned long)consumer.getNames().getVarCount());
  }
}

const char* AnalyzeAction::staticGetName() {
  return "analyze";
}
