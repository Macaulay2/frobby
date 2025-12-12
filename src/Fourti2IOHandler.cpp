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
#include "Fourti2IOHandler.h"

#include "Scanner.h"
#include "BigIdeal.h"
#include "Term.h"
#include "TermTranslator.h"
#include "BigPolynomial.h"
#include "BigTermConsumer.h"
#include "DataType.h"
#include "IdealConsolidator.h"
#include "PolynomialConsolidator.h"
#include "SatBinomIdeal.h"
#include "SatBinomConsumer.h"
#include "IdealWriter.h"
#include "PolyWriter.h"
#include "error.h"
#include "display.h"
#include "InputConsumer.h"

namespace IO {
  namespace Fourti2 {
    void writeRing(const VarNames& names, FILE* out);
    void writeRingWithoutHeader(const VarNames& names, FILE* out);
    void readTerm(Scanner& in, InputConsumer& consumer);
    void readRing(Scanner& in, VarNames& names);
    void readRing(Scanner& in, VarNames& names, size_t varCount);
    void writeTerm(const vector<mpz_class>& term, FILE* out);
    void writeTerm(const Term& term,
                   const TermTranslator& translator,
                   FILE* out);
    void readIdeal(Scanner& in,
                   InputConsumer& consumer,
                   size_t generatorCount,
                   size_t varCount);
    void readSatBinomIdeal(Scanner& in,
                           SatBinomConsumer& consumer,
                           size_t generatorCount,
                           size_t varCount);
    void display4ti2Warning();
  }
  namespace F = Fourti2;

  class Fourti2IdealWriter : public IdealWriter {
  public:
    Fourti2IdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      INTERNAL_ERROR_UNIMPLEMENTED();
    }

    virtual void doWriteHeader(bool first, size_t generatorCount) {
      fprintf(getFile(), "%lu %lu\n",
              (unsigned long)generatorCount,
              (unsigned long)getNames().getVarCount());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      F::writeTerm(term, translator, getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term, bool first) {
      F::writeTerm(term, getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      if (!getNames().namesAreDefault())
        F::writeRingWithoutHeader(getNames(), getFile());
    }

    virtual void doWriteEmptyList() {
      F::writeRing(getNames(), getFile());
    }
  };

  class Fourti2PolyWriter : public PolyWriter {
  public:
    Fourti2PolyWriter(FILE* out): PolyWriter(out) {
    }

  private:
    virtual void doWriteHeader() {
      INTERNAL_ERROR_UNIMPLEMENTED();
    }

    virtual void doWriteHeader(size_t termCount) {
      fprintf(getFile(), "%lu %lu\n",
              (unsigned long)termCount,
              (unsigned long)getNames().getVarCount() + 1);
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const Term& term,
                             const TermTranslator& translator,
                             bool firstGenerator) {
      ASSERT(term.getVarCount() == translator.getVarCount());

      mpz_out_str(getFile(), 10, coef.get_mpz_t());

      if (term.getVarCount() > 0) {
        fputc(' ', getFile());
        F::writeTerm(term, translator, getFile());
      } else
        fputc('\n', getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const vector<mpz_class>& term,
                             bool firstGenerator) {
      ASSERT(term.size() == getNames().getVarCount());

      mpz_out_str(getFile(), 10, coef.get_mpz_t());

      if (!term.empty()) {
        fputc(' ', getFile());
        F::writeTerm(term, getFile());
      } else
        fputc('\n', getFile());
    }

    virtual void doWriteFooter(bool wasZero) {
      fputs("(coefficient)", getFile());
      if (!getNames().namesAreDefault())
        F::writeRingWithoutHeader(getNames(), getFile());
      else
        fputc('\n', getFile());
    };
  };

  Fourti2IOHandler::Fourti2IOHandler():
    IOHandlerImpl(staticGetName(),
                  "Format used by the software package 4ti2.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerInput(DataType::getPolynomialType());
    registerInput(DataType::getSatBinomIdealType());
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getMonomialIdealListType());
    registerOutput(DataType::getPolynomialType());
  }

  const char* Fourti2IOHandler::staticGetName() {
    return "4ti2";
  }

  BigTermConsumer* Fourti2IOHandler::doCreateIdealWriter(FILE* out) {
    F::display4ti2Warning();
    unique_ptr<BigTermConsumer> writer(new Fourti2IdealWriter(out));
    return new IdealConsolidator(std::move(writer));
  }

  CoefBigTermConsumer* Fourti2IOHandler::doCreatePolynomialWriter(FILE* out) {
    F::display4ti2Warning();
    unique_ptr<CoefBigTermConsumer> writer(new Fourti2PolyWriter(out));
    return new PolynomialConsolidator(std::move(writer));
  }

  void Fourti2IOHandler::doWriteTerm(const vector<mpz_class>& term,
                                     const VarNames& names,
                                     FILE* out) {
    if (term.empty()) {
      // Otherwise we will print nothing, which would be OK inside an
      // ideal since then it is possible to see what happened from the
      // number of generators and variables. We do not have that
      // information here, so we have to print something.
      fputs("_fourtitwo_identity", out);
    }
    F::writeTerm(term, out);
  }

  void Fourti2IOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
    F::readTerm(in, consumer);
  }

  void Fourti2IOHandler::doReadIdeal(Scanner& in, InputConsumer& consumer) {
    size_t generatorCount;
    in.readSizeT(generatorCount);

    size_t varCount;
    in.readSizeT(varCount);

    F::readIdeal(in, consumer, generatorCount, varCount);
  }

  void Fourti2IOHandler::doReadIdeals(Scanner& in, InputConsumer& consumer) {
    // An empty list is just a ring by itself, and this has a special
    // syntax.  So we first decipher whether we are looking at a ring or
    // an ideal.  At the point where we can tell that it is an ideal, we
    // have already read part of the ideal, so we have to do something
    // to pass the read information on to the code for reading the rest
    // of the ideal.

    size_t generatorCount;
    in.readSizeT(generatorCount);

    if (generatorCount == 42 && in.peekIdentifier()) {
      in.expect("ring");
      VarNames names;
      F::readRing(in, names);
      consumer.consumeRing(names);
      in.expectEOF();
      return;
    }

    size_t varCount;
    in.readSizeT(varCount);

    F::readIdeal(in, consumer, generatorCount, varCount);

    while (hasMoreInput(in))
      doReadIdeal(in, consumer);
  }

  void Fourti2IOHandler::doReadPolynomial(Scanner& in,
                                          CoefBigTermConsumer& consumer) {
    size_t generatorCount;
    size_t varCount;

    in.readSizeT(generatorCount);
    in.readSizeT(varCount);

    if (varCount == 0)
      reportError
        ("A polynomial has at least one column in the matrix,"
         "but this matrix has no columns.");

    // The first column is the coefficient and so does not represent a
    // variable.
    --varCount;

    BigPolynomial polynomial((VarNames(varCount)));

    for (size_t t = 0; t < generatorCount; ++t) {
      // Read a term
      polynomial.newLastTerm();
      in.readInteger(polynomial.getLastCoef());

      vector<mpz_class>& term = polynomial.getLastTerm();
      for (size_t var = 0; var < varCount; ++var) {
        ASSERT(var < term.size());
        in.readIntegerAndNegativeAsZero(term[var]);
      }
    }

    if (!in.match('(')) {
      // This expect will fail which improves the error message compared
      // to just expect('(').
      in.expect("(coefficient)");
    }
    in.expect("coefficient");
    in.expect(')');

    if (in.peekIdentifier()) {
      VarNames names;
      for (size_t var = 0; var < varCount; ++var)
        names.addVar(in.readIdentifier());
      polynomial.renameVars(names);
    }

    consumer.consume(polynomial);
  }

  void Fourti2IOHandler::doReadSatBinomIdeal(Scanner& in,
                                             SatBinomConsumer& consumer) {
    size_t generatorCount;
    in.readSizeT(generatorCount);

    size_t varCount;
    in.readSizeT(varCount);

    F::readSatBinomIdeal(in, consumer, generatorCount, varCount);
  }

  void F::writeRing(const VarNames& names, FILE* out) {
    fputs("42 ring\n", out);
    writeRingWithoutHeader(names, out);
  }

  void F::writeRingWithoutHeader(const VarNames& names, FILE* out) {
    if (names.getVarCount() == 0)
      return;

    fputc(' ', out);
    for (size_t var = 0; var < names.getVarCount(); ++var) {
      if (var > 0)
        fputc(' ', out);
      fputs(names.getName(var).c_str(), out);
    }
    fputc('\n', out);
  }

  void F::readTerm(Scanner& in, InputConsumer& consumer) {
    consumer.beginTerm();
    const size_t varCount = consumer.getRing().getVarCount();
	if (varCount == 0)
      in.expect("_fourtitwo_identity");
    else {
      for (size_t var = 0; var < varCount; ++var) {
        if (in.match('-'))
          in.expectIntegerNoSign();
        else
          consumer.consumeVarExponent(var, in);
      }
    }
    consumer.endTerm();
  }

  void F::readRing(Scanner& in, VarNames& names) {
    names.clear();
    while (in.peekIdentifier())
      names.addVarSyntaxCheckUnique(in, in.readIdentifier());
  }

  void F::readRing(Scanner& in, VarNames& names, size_t varCount) {
    names.clear();
    for (size_t var = 0; var < varCount; ++var)
      names.addVarSyntaxCheckUnique(in, in.readIdentifier());
  }

  void F::writeTerm(const vector<mpz_class>& term, FILE* out) {
    size_t varCount = term.size();
    for (size_t var = 0; var < varCount; ++var) {
      fputc(' ', out);
      mpz_out_str(out, 10, term[var].get_mpz_t());
    }

    if (varCount != 0)
      fputc('\n', out);
  }

  void F::writeTerm(const Term& term,
                    const TermTranslator& translator,
                    FILE* out) {
    ASSERT(term.getVarCount() == translator.getVarCount());

    size_t varCount = term.getVarCount();
    for (size_t var = 0; var < varCount; ++var) {
      fputc(' ', out);
      const char* exp = translator.getExponentString(var, term[var]);
      if (exp == 0)
        exp = "0";
      fputs(exp, out);
    }

    if (varCount != 0)
      fputc('\n', out);
  }

  /** The parsing of the header with the number of generators and
      variables has to be separate from the code below reading the rest
      of the ideal, since in some contexts what looks like the header
      of an ideal could be a ring description, while in other contexts
      this cannot happen. The code below is the common code for these
      two cases. */
  void F::readIdeal(Scanner& in,
                    InputConsumer& consumer,
                    size_t generatorCount,
                    size_t varCount) {
	consumer.consumeRing(VarNames(varCount));
	consumer.beginIdeal();

    if (varCount == 0) {
      for (size_t t = 0; t < generatorCount; ++t) {
        consumer.beginTerm();
        consumer.endTerm();
      }
    } else {
      for (size_t t = 0; t < generatorCount; ++t)
        F::readTerm(in, consumer);
    }

    if (in.peekIdentifier()) {
      VarNames names;
      F::readRing(in, names, varCount);
	  consumer.consumeRing(names);
    }
	consumer.endIdeal();
  }

  void F::readSatBinomIdeal(Scanner& in,
                            SatBinomConsumer& consumer,
                            size_t generatorCount,
                            size_t varCount) {
    // We have to read the entire ideal before we can tell whether there is
    // a ring associated to it, so we have to store the ideal here until
    // that time.

    SatBinomIdeal ideal((VarNames(varCount)));
    ideal.reserve(generatorCount);
    for (size_t t = 0; t < generatorCount; ++t) {
      // Read a term
      ideal.newLastTerm();
      vector<mpz_class>& binom = ideal.getLastBinomRef();
      for (size_t var = 0; var < varCount; ++var)
        in.readInteger(binom[var]);
    }

    if (in.peekIdentifier()) {
      VarNames names;
      F::readRing(in, names, varCount);
      ideal.renameVars(names);
    }

    consumer.consume(ideal);
  }

  void F::display4ti2Warning() {
    string msg = "Using the format ";
    msg += Fourti2IOHandler::staticGetName();
    msg += " makes it necessary to store all of the output in "
      "memory before writing it out. This increases "
      "memory consumption and decreases performance.";
    displayNote(msg);
  }
}
