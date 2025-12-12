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
#include "IOHandler.h"

#include "Scanner.h"
#include "VarNames.h"
#include "NameFactory.h"
#include "FrobbyStringStream.h"
#include "DataType.h"
#include "BigTermConsumer.h"
#include "CoefBigTermConsumer.h"
#include "CountingIOHandler.h"
#include "NewMonosIOHandler.h"
#include "MonosIOHandler.h"
#include "Macaulay2IOHandler.h"
#include "Fourti2IOHandler.h"
#include "NullIOHandler.h"
#include "CoCoA4IOHandler.h"
#include "SingularIOHandler.h"
#include "error.h"
#include "BigTermRecorder.h"
#include "InputConsumer.h"

IOHandler::~IOHandler() {
}

void IOHandler::readIdeal(Scanner& in, InputConsumer& consumer) {
  doReadIdeal(in, consumer);
}

void IOHandler::readIdeals(Scanner& in, InputConsumer& consumer) {
  doReadIdeals(in, consumer);
}

void IOHandler::readTerm
(Scanner& in, const VarNames& names, vector<mpz_class>& term) {
  InputConsumer consumer;
  consumer.consumeRing(names);
  consumer.beginIdeal();
  doReadTerm(in, consumer);
  consumer.endIdeal();
  ASSERT(!consumer.empty());
  unique_ptr<BigIdeal> ideal = consumer.releaseBigIdeal();
  ASSERT(consumer.empty());
  ASSERT(ideal->getGeneratorCount() == 1);
  term = (*ideal)[0];
}

void IOHandler::readPolynomial(Scanner& in, CoefBigTermConsumer& consumer) {
  doReadPolynomial(in, consumer);
}

void IOHandler::readSatBinomIdeal(Scanner& in, SatBinomConsumer& consumer) {
  doReadSatBinomIdeal(in, consumer);
}

void IOHandler::writeTerm(const vector<mpz_class>& term,
                          const VarNames& names, FILE* out) {
  doWriteTerm(term, names, out);
}

bool IOHandler::hasMoreInput(Scanner& in) const {
  return doHasMoreInput(in);
}

const char* IOHandler::getName() const {
  return doGetName();
}

const char* IOHandler::getDescription() const {
  return doGetDescription();
}

unique_ptr<BigTermConsumer> IOHandler::createIdealWriter(FILE* out) {
  if (!supportsOutput(DataType::getMonomialIdealType())) {
    throwError<UnsupportedException>
      ("The " + string(getName()) +
       " format does not support output of a monomial ideal.");
  }
  return unique_ptr<BigTermConsumer>(doCreateIdealWriter(out));
}

unique_ptr<BigTermConsumer> IOHandler::createIdealListWriter(FILE* out) {
  if (!supportsOutput(DataType::getMonomialIdealListType())) {
    throwError<UnsupportedException>
      ("The " + string(getName()) +
       " format does not support output of a list of monomial ideals.");
  }
  // This is the same kind of object as for a non-list ideal
  // writer. The only difference is that we checked for support for
  // output of lists above.
  return unique_ptr<BigTermConsumer>(doCreateIdealWriter(out));
}

unique_ptr<CoefBigTermConsumer> IOHandler::createPolynomialWriter(FILE* out) {
  if (!supportsOutput(DataType::getPolynomialType())) {
    throwError<UnsupportedException>
      ("The " + string(getName()) +
       " format does not support output of a polynomial.");
  }
  return unique_ptr<CoefBigTermConsumer>(doCreatePolynomialWriter(out));
}

bool IOHandler::supportsInput(const DataType& type) const {
  return doSupportsInput(type);
}

bool IOHandler::supportsOutput(const DataType& type) const {
  return doSupportsOutput(type);
}

namespace {
  typedef NameFactory<IOHandler> IOHandlerFactory;
  IOHandlerFactory getIOHandlerFactory() {
    IOHandlerFactory factory("format");

    nameFactoryRegister<IO::Macaulay2IOHandler>(factory);
    nameFactoryRegister<IO::CoCoA4IOHandler>(factory);
    nameFactoryRegister<IO::SingularIOHandler>(factory);
    nameFactoryRegister<IO::MonosIOHandler>(factory);
    nameFactoryRegister<IO::NewMonosIOHandler>(factory);
    nameFactoryRegister<IO::Fourti2IOHandler>(factory);
    nameFactoryRegister<IO::NullIOHandler>(factory);
    nameFactoryRegister<IO::CountingIOHandler>(factory);

    return factory;
  }
}

unique_ptr<IOHandler> createIOHandler(const string& prefix) {
  return createWithPrefix(getIOHandlerFactory(), prefix);
}

unique_ptr<IOHandler> createOHandler(const string& input, const string& output) {
  if (output == getFormatNameIndicatingToUseInputFormatAsOutputFormat())
    return createIOHandler(input);
  else
    return createIOHandler(output);
}

void getIOHandlerNames(vector<string>& names) {
  getIOHandlerFactory().getNamesWithPrefix("", names);
}

void readFrobeniusInstance(Scanner& in, vector<mpz_class>& numbers) {
  numbers.clear();

  string number;
  mpz_class n;
  while (!in.matchEOF()) {
    in.readInteger(n);

    if (n <= 1) {
      FrobbyStringStream errorMsg;
      errorMsg << "Read the number " << n
               << " while reading Frobenius instance. "
               << "Only integers strictly larger than 1 are valid.";
      reportSyntaxError(in, errorMsg);
    }

    numbers.push_back(n);
  }

  if (numbers.empty())
    reportSyntaxError
      (in, "Read empty Frobenius instance, which is not allowed.");

  mpz_class gcd = numbers[0];
  for (size_t i = 1; i < numbers.size(); ++i)
    mpz_gcd(gcd.get_mpz_t(), gcd.get_mpz_t(), numbers[i].get_mpz_t());

  if (gcd != 1) {
    // Maybe not strictly speaking a syntax error, but that category
    // of errors still fits best.
    FrobbyStringStream errorMsg;
    errorMsg << "The numbers in the Frobenius instance are not "
             << "relatively prime. They are all divisible by "
             << gcd << '.';
    reportSyntaxError(in, errorMsg);
  }
}

string autoDetectFormat(Scanner& in) {
  // We guess based on the initial non-whitespace character. We detect
  // more than the correct initial character to try to guess the
  // intended format in the face of mistakes.
  in.eatWhite();
  switch (in.peek()) {
  case 'U': // correct
  case 'u': // incorrect
    return IO::CoCoA4IOHandler::staticGetName();

  case 'r': // correct
    return IO::SingularIOHandler::staticGetName();

  case '(': // correct
  case 'l': // incorrect
  case ')': // incorrect
    return IO::NewMonosIOHandler::staticGetName();

  case '0': case '1': case '2': case '3': case '4': // correct
  case '5': case '6': case '7': case '8': case '9': // correct
  case '+': case '-': // incorrect
    return IO::Fourti2IOHandler::staticGetName();

  case 'v': // correct
    return IO::MonosIOHandler::staticGetName();

  case 'R': // correct
  default: // incorrect
    return IO::Macaulay2IOHandler::staticGetName();
  }
}

string getFormatNameIndicatingToUseInputFormatAsOutputFormat() {
  return "input";
}

string getFormatNameIndicatingToGuessTheInputFormat() {
  return "autodetect";
}
