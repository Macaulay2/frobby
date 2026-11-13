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
#include "NullIOHandler.h"

#include "BigIdeal.h"
#include "NullCoefTermConsumer.h"
#include "NullTermConsumer.h"
#include "BigPolynomial.h"
#include "VarNames.h"
#include "DataType.h"
#include "SatBinomConsumer.h"
#include "SatBinomIdeal.h"
#include "InputConsumer.h"

namespace IO {
  NullIOHandler::NullIOHandler():
    IOHandlerImpl(staticGetName(),
                  "Ignores input and produces no output.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getPolynomialType());
    registerInput(DataType::getMonomialIdealListType());

    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getPolynomialType());
    registerOutput(DataType::getMonomialIdealListType());
  }

  const char* NullIOHandler::staticGetName() {
    return "null";
  }

  BigTermConsumer* NullIOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW NullTermConsumer();
  }

  CoefBigTermConsumer* NullIOHandler::doCreatePolynomialWriter(FILE* out) {
    return FROBBY_NEW NullCoefTermConsumer();
  }

  void NullIOHandler::doWriteTerm(const vector<mpz_class>& term,
                                  const VarNames& names,
                                  FILE* out) {
  }

  void NullIOHandler::doReadIdeal(Scanner& in, InputConsumer& consumer) {
	consumer.consumeRing(VarNames());
	consumer.beginIdeal();
	consumer.endIdeal();
  }

  void NullIOHandler::doReadIdeals(Scanner& in, InputConsumer& consumer) {
    consumer.consumeRing(VarNames());
  }

  void NullIOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	consumer.beginTerm();
	consumer.endTerm();
  }

  void NullIOHandler::doReadPolynomial(Scanner& in, CoefBigTermConsumer& consumer) {
    consumer.consume(BigPolynomial());
  }

  void NullIOHandler::doReadSatBinomIdeal(Scanner& in, SatBinomConsumer& consumer) {
    consumer.consume(SatBinomIdeal());
  }

  bool NullIOHandler::doHasMoreInput(Scanner& in) const {
    return false;
  }
}
