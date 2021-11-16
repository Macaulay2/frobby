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
#include "stdinc.h"
#include "CommonParamsHelper.h"

#include "CommonParams.h"
#include "IOFacade.h"
#include "BigIdeal.h"
#include "Ideal.h"
#include "TermTranslator.h"
#include "VarSorter.h"
#include "Scanner.h"
#include "IOHandler.h"
#include "DataType.h"
#include "error.h"
#include "BigTermConsumer.h"
#include "CoefBigTermConsumer.h"
#include "ActionPrinter.h"
#include "NullTermConsumer.h"
#include "NullCoefTermConsumer.h"
#include "IrreducibleIdealSplitter.h"
#include "TranslatingTermConsumer.h"
#include "TranslatingCoefTermConsumer.h"
#include "CanonicalTermConsumer.h"
#include "CanonicalCoefTermConsumer.h"
#include "TotalDegreeCoefTermConsumer.h"

CommonParamsHelper::CommonParamsHelper():
  _idealConsumer(0),
  _polyConsumer(0),
  _produceCanonicalOutput(false) {
}

CommonParamsHelper::~CommonParamsHelper() {
  // Constructor defined so auto_ptr<T> in the header does not need
  // definition of T.
}

void CommonParamsHelper::readIdealAndSetOutput(const CommonParams& params,
                                               const DataType& output) {
  _produceCanonicalOutput = params.getProduceCanonicalOutput();

  Scanner in(params.getInputFormat(), stdin);
  auto_ptr<IOHandler> outputHandler =
    createOHandler(in.getFormat(), params.getOutputFormat());
  if (output == DataType::getPolynomialType()) {
    _polyConsumerDeleter = outputHandler->createPolynomialWriter(stdout);
    _polyConsumer = _polyConsumerDeleter.get();
  } else if (output == DataType::getMonomialIdealType()) {
    _idealConsumerDeleter = outputHandler->createIdealWriter(stdout);
    _idealConsumer = _idealConsumerDeleter.get();
  } else if (output == DataType::getMonomialIdealListType()) {
    _idealConsumerDeleter = outputHandler->createIdealListWriter(stdout);
    _idealConsumer = _idealConsumerDeleter.get();
  } else if (output == DataType::getNullType()) {
    _idealConsumerDeleter.reset(new NullTermConsumer());
    _idealConsumer = _idealConsumerDeleter.get();
    _polyConsumerDeleter.reset(new NullCoefTermConsumer());
    _polyConsumer = _polyConsumerDeleter.get();
  } else {
    string msg = "Unsupported output type of ";
    msg += output.getName();
    msg += '.';
    INTERNAL_ERROR(msg);
  }

  readIdeal(params, in);
}

void CommonParamsHelper::readIdealAndSetPolyOutput(const CommonParams& params) {
  _produceCanonicalOutput = params.getProduceCanonicalOutput();

  Scanner in(params.getInputFormat(), stdin);
  auto_ptr<IOHandler> outputHandler =
    createOHandler(in.getFormat(), params.getOutputFormat());
  _polyConsumerDeleter = outputHandler->createPolynomialWriter(stdout);
  _polyConsumer = _polyConsumerDeleter.get();

  readIdeal(params, in);
}

void CommonParamsHelper::readIdealAndSetIdealOutput
(const CommonParams& params) {
  _produceCanonicalOutput = params.getProduceCanonicalOutput();

  Scanner in(params.getInputFormat(), stdin);
  auto_ptr<IOHandler> outputHandler =
    createOHandler(in.getFormat(), params.getOutputFormat());
  _idealConsumerDeleter = outputHandler->createIdealWriter(stdout);
  _idealConsumer = _idealConsumerDeleter.get();

  readIdeal(params, in);
}

void CommonParamsHelper::setIdealAndIdealOutput(const CommonParams& params,
                                                const BigIdeal& input,
                                                BigTermConsumer& output) {
  _produceCanonicalOutput = params.getProduceCanonicalOutput();

  _idealConsumer = &output;
  setIdeal(params, input);
}

void CommonParamsHelper::setIdealAndPolyOutput(const CommonParams& params,
                                               const BigIdeal& input,
                                               CoefBigTermConsumer& output) {
  _produceCanonicalOutput = params.getProduceCanonicalOutput();

  _polyConsumer = &output;
  setIdeal(params, input);
}

auto_ptr<TermConsumer> CommonParamsHelper::
makeTranslatedIdealConsumer(bool split) {
  auto_ptr<TermConsumer> translated;
  if (split) {
    auto_ptr<BigTermConsumer> splitter
      (new IrreducibleIdealSplitter(*_idealConsumer));
    translated.reset
      (new TranslatingTermConsumer(std::move(splitter), getTranslator()));
  } else
    translated.reset
      (new TranslatingTermConsumer(*_idealConsumer, getTranslator()));

  if (_produceCanonicalOutput) {
    return auto_ptr<TermConsumer>
      (new CanonicalTermConsumer(std::move(translated),
                                 getIdeal().getVarCount(),
                                 &getTranslator()));
  } else
    return translated;
}

auto_ptr<CoefTermConsumer> CommonParamsHelper::makeTranslatedPolyConsumer() {
  auto_ptr<CoefTermConsumer> translated
    (new TranslatingCoefTermConsumer(*_polyConsumer, getTranslator()));
  if (_produceCanonicalOutput)
    return auto_ptr<CoefTermConsumer>
      (new CanonicalCoefTermConsumer(std::move(translated)));
  else
    return translated;
}

auto_ptr<CoefTermConsumer> CommonParamsHelper::makeToUnivariatePolyConsumer() {
  return auto_ptr<CoefTermConsumer>
    (new TotalDegreeCoefTermConsumer(*_polyConsumer, getTranslator()));
}

void CommonParamsHelper::addPurePowersAtInfinity() {
  _translator->addPurePowersAtInfinity(*_ideal);
}

void CommonParamsHelper::readIdeal(const CommonParams& params, Scanner& in) {
  BigIdeal bigIdeal;
  IOFacade facade(params.getPrintActions());
  facade.readIdeal(in, bigIdeal);
  in.expectEOF();
  setIdeal(params, bigIdeal);
}

void CommonParamsHelper::setIdeal(const CommonParams& params,
                                  const BigIdeal& bigIdeal) {
  ActionPrinter printer(params.getPrintActions());

  printer.beginAction("Translating ideal to internal data structure.");
  _ideal.reset(new Ideal());
  _translator.reset(new TermTranslator(bigIdeal, *_ideal, false));
  printer.endAction();

  if (!params.getIdealIsMinimal()) {
    printer.beginAction("Minimizing ideal.");
    _ideal->minimize();
    printer.endAction();
  }

  if (params.getProduceCanonicalOutput()) {
    printer.beginAction("Sorting variables for canonical representation.");

    VarSorter sorter(_translator->getNames());
    sorter.permute(_translator.get());

    Ideal::iterator stop = _ideal->end();
    for (Ideal::iterator it = _ideal->begin(); it != stop; ++it)
      sorter.permute(*it);

    printer.endAction();
  }
}
