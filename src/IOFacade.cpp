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
#include "IOFacade.h"

#include "IOHandler.h"
#include "fourti2.h"
#include "BigIdeal.h"
#include "fplllIO.h"
#include "Scanner.h"
#include "ElementDeleter.h"
#include "error.h"
#include "FrobbyStringStream.h"
#include "BigTermRecorder.h"
#include "CoefBigTermConsumer.h"
#include "CoefBigTermRecorder.h"
#include "SatBinomIdeal.h"
#include "SatBinomRecorder.h"
#include "InputConsumer.h"
#include "SquareFreeIdeal.h"
#include "RawSquareFreeIdeal.h"
#include <iterator>

IOFacade::IOFacade(bool printActions):
  Facade(printActions) {
}

bool IOFacade::isValidMonomialIdealFormat(const string& format) {
  beginAction("Validating monomial ideal format name.");

  bool valid = true;
  try {
    (void)createIOHandler(format);
  } catch (const UnknownNameException&) {
    valid = false;
  }

  endAction();

  return valid;
}

void IOFacade::readSatBinomIdeal(Scanner& in, SatBinomConsumer& consumer) {
  beginAction("Reading saturated binomial ideal.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  handler->readSatBinomIdeal(in, consumer);

  endAction();
}

void IOFacade::readSatBinomIdeal(Scanner& in, SatBinomIdeal& ideal) {
  beginAction("Reading saturated binomial ideal.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  ideal.clear();
  SatBinomRecorder recorder(ideal);
  handler->readSatBinomIdeal(in, recorder);

  endAction();
}

void IOFacade::readIdeal(Scanner& in, BigTermConsumer& consumer) {
  beginAction("Reading monomial ideal.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  InputConsumer middleman;
  handler->readIdeal(in, middleman);
  // todo: find a way to generate the input as it comes in rather than
  // storing it and only then letting it go on.
  ASSERT(!middleman.empty());
  consumer.consume(middleman.releaseBigIdeal());
  ASSERT(middleman.empty());

  endAction();
}

void IOFacade::readIdeal(Scanner& in, BigIdeal& ideal) {
  beginAction("Reading monomial ideal.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  InputConsumer recorder;
  handler->readIdeal(in, recorder);

  ASSERT(!recorder.empty());
  ideal.swap(*(recorder.releaseBigIdeal()));
  ASSERT(recorder.empty());

  endAction();
}

/** Read a square free ideal from in and place it in the parameter
	ideal. */
void IOFacade::readSquareFreeIdeal(Scanner& in, SquareFreeIdeal& ideal) {
  beginAction("Reading square free ideal.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  InputConsumer consumer;
  consumer.requireSquareFree();
  handler->readIdeal(in, consumer);
  ASSERT(!consumer.empty());
  ideal.swap(*consumer.releaseSquareFreeIdeal());
  ASSERT(consumer.empty());

  endAction();
}

void IOFacade::readIdeals(Scanner& in,
                          vector<BigIdeal*>& ideals,
                          VarNames& names) {
  beginAction("Reading monomial ideals.");

  // To make it clear what needs to be deleted in case of an exception.
  ASSERT(ideals.empty());
  ElementDeleter<vector<BigIdeal*> > idealsDeleter(ideals);

  unique_ptr<IOHandler> handler(in.createIOHandler());

  //BigTermRecorder recorder;
  InputConsumer recorder;
  handler->readIdeals(in, recorder);

  names = recorder.getRing();
  while (!recorder.empty())
    exceptionSafePushBack(ideals, recorder.releaseBigIdeal());

  idealsDeleter.release();

  endAction();
}

void IOFacade::writeIdeal(const BigIdeal& ideal,
                          IOHandler* handler,
                          FILE* out) {
  ASSERT(handler != 0);

  beginAction("Writing monomial ideal.");

  handler->createIdealWriter(out)->consume(ideal);

  endAction();
}

void IOFacade::writeIdeals(const vector<BigIdeal*>& ideals,
                           const VarNames& names,
                           IOHandler* handler,
                           FILE* out) {
  ASSERT(handler != 0);

  beginAction("Writing monomial ideals.");

  {
    unique_ptr<BigTermConsumer> consumer = handler->createIdealWriter(out);

    consumer->beginConsumingList();
    consumer->consumeRing(names);

    for (vector<BigIdeal*>::const_iterator it = ideals.begin();
         it != ideals.end(); ++it)
      consumer->consume(**it);

    consumer->doneConsumingList();
  }

  endAction();
}

void IOFacade::readPolynomial(Scanner& in, BigPolynomial& polynomial) {

  beginAction("Reading polynomial.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  CoefBigTermRecorder recorder(&polynomial);
  handler->readPolynomial(in, recorder);

  endAction();
}

void IOFacade::writePolynomial(const BigPolynomial& polynomial,
                               IOHandler* handler,
                               FILE* out) {
  ASSERT(handler != 0);
  ASSERT(out != 0);

  beginAction("Writing polynomial.");

  handler->createPolynomialWriter(out)->consume(polynomial);

  endAction();
}

void IOFacade::writeTerm(const vector<mpz_class>& term,
                         const VarNames& names,
                         IOHandler* handler,
                         FILE* out) {
  beginAction("Writing monomial.");

  handler->writeTerm(term, names, out);

  endAction();
}

bool IOFacade::readAlexanderDualInstance
(Scanner& in, BigIdeal& ideal, vector<mpz_class>& term) {
  beginAction("Reading Alexander dual input.");

  unique_ptr<IOHandler> handler(in.createIOHandler());
  ASSERT(handler.get() != 0);

  InputConsumer recorder;
  handler->readIdeal(in, recorder);

  // TODO: return value instead of this copy.
  ASSERT(!recorder.empty());
  ideal = *(recorder.releaseBigIdeal());
  ASSERT(recorder.empty());

  bool pointSpecified = false;
  if (handler->hasMoreInput(in)) {
    handler->readTerm(in, ideal.getNames(), term);
    pointSpecified = true;
  }

  endAction();

  return pointSpecified;
}

void IOFacade::readVector
(Scanner& in, vector<mpz_class>& v, size_t integerCount) {
  beginAction("Reading vector.");

  v.resize(integerCount);
  for (size_t i = 0; i < integerCount; ++i)
    in.readInteger(v[i]);

  endAction();
}

void IOFacade::
readFrobeniusInstance(Scanner& in, vector<mpz_class>& instance) {
  beginAction("Reading Frobenius instance.");

  ::readFrobeniusInstance(in, instance);

  endAction();
}

void IOFacade::readFrobeniusInstanceWithGrobnerBasis
(Scanner& in, BigIdeal& ideal, vector<mpz_class>& instance) {
  beginAction("Reading frobenius instance with Grobner basis.");

  fourti2::readGrobnerBasis(in, ideal);
  ::readFrobeniusInstance(in, instance);

  if (instance.size() != ideal.getVarCount() + 1) {
    if (instance.empty())
      reportSyntaxError
        (in, "The Grobner basis is not followed by a Frobenius instance.");
    else {
      // Note that we add one since the first entry of the rows encoding
      // the Grobner basis is chopped off.
      FrobbyStringStream errorMsg;
      errorMsg << "The Grobner basis has "
               << ideal.getVarCount() + 1
               << " entries, and the Frobenius instance should then also have "
               << ideal.getVarCount() + 1
               << " entries, but in fact it has "
               << instance.size()
               << " entries.";
        reportSyntaxError(in, errorMsg);
    }
  }

  endAction();
}

void IOFacade::
writeFrobeniusInstance(FILE* out, vector<mpz_class>& instance) {
  beginAction("Writing Frobenius instance.");

  for (size_t i = 0; i < instance.size(); ++i) {
    if (i > 0)
      fputc(' ', out);
    gmp_fprintf(out, "%Zd", instance[i].get_mpz_t());
  }
  fputc('\n', out);

  endAction();
}

bool IOFacade::isValidLatticeFormat(const string& format) {
  beginAction("Validating lattice format name.");

  bool valid = (format == "4ti2" || format == "fplll");

  endAction();

  return valid;
}

void IOFacade::
readLattice(Scanner& in, BigIdeal& ideal) {
  beginAction("Reading lattice basis.");

  if (in.getFormat() == "4ti2")
    fourti2::readLatticeBasis(in, ideal);
  else if (in.getFormat() == "fplll")
    fplll::readLatticeBasis(in, ideal);
  else {
    ASSERT(false);
  }

  endAction();
}

void IOFacade::
writeLattice(FILE* out, const BigIdeal& ideal, const string& format) {
  beginAction("Writing lattice basis.");

  if (format == "4ti2")
    fourti2::writeLatticeBasis(out, ideal);
  else if (format == "fplll")
    fplll::writeLatticeBasis(out, ideal);
  else {
    ASSERT(false);
  }

  endAction();
}
