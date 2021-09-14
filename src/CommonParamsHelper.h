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
#ifndef COMMON_PARAMS_HELPER
#define COMMON_PARAMS_HELPER

#include "CoefTermConsumer.h"
#include "TermConsumer.h"
#include "TermTranslator.h"
#include "Ideal.h"

class CommonParams;
class Ideal;
class DataType;
class Scanner;
class CommonParams;
class BigIdeal;
class BigTermConsumer;
class CoefBigTermConsumer;
class TermTranslator;

/** Utility class for dealing with the contents of
 CommonParams. Throws an appropriate exception if given invalid data. */
class CommonParamsHelper {
 public:
  CommonParamsHelper();
  ~CommonParamsHelper();

  /** Read input ideal and support specified kind of output. */
  void readIdealAndSetOutput(const CommonParams& params,
                             const DataType& output);

  /** Read input and support polynomial output. */
  void readIdealAndSetPolyOutput(const CommonParams& params);

  /** Read input and support ideal output. */
  void readIdealAndSetIdealOutput(const CommonParams& params);

  /** Use given ideal and support ideal output. */
  void setIdealAndIdealOutput(const CommonParams& params,
                             const BigIdeal& input,
                             BigTermConsumer& output);

  /** Use given ideal and support polynomial output. */
  void setIdealAndPolyOutput(const CommonParams& params,
                             const BigIdeal& input,
                             CoefBigTermConsumer& output);

  const VarNames& getNames() {return _translator->getNames();}

  Ideal& getIdeal() {return *_ideal;}
  const Ideal& getIdeal() const {return *_ideal;}
  auto_ptr<Ideal> takeIdeal() {return std::move(_ideal);}
  bool hasIdeal() const {return _ideal.get() != 0;}

  TermTranslator& getTranslator() {return *_translator;}
  const TermTranslator& getTranslator() const {return *_translator;}
  auto_ptr<TermTranslator> takeTranslator() {return std::move(_translator);}

  BigTermConsumer& getIdealConsumer() {return *_idealConsumer;}

  auto_ptr<TermConsumer> makeTranslatedIdealConsumer(bool split = false);

  CoefBigTermConsumer& getPolyConsumer() {return *_polyConsumer;}
  auto_ptr<CoefTermConsumer> makeTranslatedPolyConsumer();
  auto_ptr<CoefTermConsumer> makeToUnivariatePolyConsumer();

  void addPurePowersAtInfinity();

 private:
  // No copies
  CommonParamsHelper(const CommonParamsHelper&);
  CommonParamsHelper& operator=(const CommonParamsHelper&);

  void readIdeal(const CommonParams& params, Scanner& in);
  void setIdeal(const CommonParams& params, const BigIdeal& ideal);

  auto_ptr<Ideal> _ideal;
  auto_ptr<TermTranslator> _translator;

  BigTermConsumer* _idealConsumer;
  auto_ptr<BigTermConsumer> _idealConsumerDeleter;

  CoefBigTermConsumer* _polyConsumer;
  auto_ptr<CoefBigTermConsumer> _polyConsumerDeleter;

  bool _produceCanonicalOutput;
};

#endif
