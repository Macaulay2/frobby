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
#include "SingularIOHandler.h"

#include "Scanner.h"
#include "VarNames.h"
#include "BigTermConsumer.h"
#include "FrobbyStringStream.h"
#include "DataType.h"
#include "CoefBigTermConsumer.h"
#include "IdealWriter.h"
#include "PolyWriter.h"
#include "error.h"
#include "InputConsumer.h"

#include <cstdio>

namespace IO {
  namespace Singular {
    void writeRing(const VarNames& names, FILE* out);
  }
  namespace S = Singular;

  class SingularIdealWriter : public IdealWriter {
  public:
    SingularIdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      S::writeRing(getNames(), getFile());
      fputs("ideal I =", getFile());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      IO::writeTermProduct(term, translator, getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      writeTermProduct(term, getNames(), getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      if (wasZeroIdeal)
        fputs("\n 0", getFile());
      fputs(";\n", getFile());
    }

    virtual void doWriteEmptyList() {
      S::writeRing(getNames(), getFile());
    }
  };

  class SingularPolyWriter : public PolyWriter {
  public:
    SingularPolyWriter(FILE* out): PolyWriter(out) {
    }

    virtual void doWriteHeader() {
      S::writeRing(getNames(), getFile());
      fputs("poly p =", getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const Term& term,
                             const TermTranslator& translator,
                             bool firstGenerator) {
      fputs("\n ", getFile());
      writeCoefTermProduct(coef, term, translator, firstGenerator, getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const vector<mpz_class>& term,
                             bool firstGenerator) {
      fputs("\n ", getFile());
      writeCoefTermProduct(coef, term, getNames(), firstGenerator, getFile());
    }

    virtual void doWriteFooter(bool wasZero) {
      if (wasZero)
        fputs("\n 0", getFile());
      fputs(";\n", getFile());
    }
  };

  SingularIOHandler::SingularIOHandler():
    IOHandlerCommon(staticGetName(),
                    "Format understandable by the program Singular.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerInput(DataType::getPolynomialType());
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getPolynomialType());
  }

  const char* SingularIOHandler::staticGetName() {
    return "singular";
  }

  BigTermConsumer* SingularIOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW SingularIdealWriter(out);
  }

  CoefBigTermConsumer* SingularIOHandler::
  doCreatePolynomialWriter(FILE* out) {
    return FROBBY_NEW SingularPolyWriter(out);
  }

  void SingularIOHandler::doWriteTerm(const vector<mpz_class>& term,
                                      const VarNames& names,
                                      FILE* out) {
    writeTermProduct(term, names, out);
  }

  void SingularIOHandler::doReadTerm(Scanner& in,
                                     const VarNames& names,
                                     vector<mpz_class>& term) {
    readTermProduct(in, names, term);
  }

  void SingularIOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	consumer.consumeTermProductNotation(in);
  }

  void SingularIOHandler::doReadRing(Scanner& in, VarNames& names) {
    names.clear();

    in.expect("ring");
    in.expect('R');
    in.expect('=');
    in.expect('0');
    in.expect(',');
    in.expect('(');

    do {
      names.addVarSyntaxCheckUnique(in, in.readIdentifier());
    } while (in.match(','));

    in.expect(')');
    in.expect(',');
    in.expect("lp");
    in.expect(';');

    in.expect("int");
    in.expect("noVars");
    in.expect("=");
    if (in.match('1')) {
      if (names.getVarCount() != 1 ||
          names.getName(0) != string("dummy")) {
        FrobbyStringStream errorMsg;
        errorMsg <<
          "A singular ring with no actual variables must have a single "
          "place-holder variable named \"dummy\", and in this case ";
        if (names.getVarCount() != 1)
          errorMsg << "there are " << names.getVarCount()
                   << " place-holder variables.";
        else
          errorMsg << "it has the name \"" << names.getName(0) << "\".";

        reportSyntaxError(in, errorMsg);
      }
      names.clear();
    } else if (!in.match('0')) {
      // TODO: Replace following line with: in.expect('0', '1');
      reportSyntaxError(in, "noVars must be either 0 or 1.");
    }

    in.expect(';');
  }

  bool SingularIOHandler::doPeekRing(Scanner& in) {
    return in.peek('r') || in.peek('R');
  }

  void SingularIOHandler::doReadBareIdeal(Scanner& in, const VarNames& names,
                                          BigTermConsumer& consumer) {
    consumer.beginConsuming(names);
    vector<mpz_class> term(names.getVarCount());

    in.expect("ideal");
    in.expect('I');
    in.expect('=');

    if (!in.match('0')) {
      do {
        readTerm(in, names, term);
        consumer.consume(term);
      } while (in.match(','));
    }
    in.expect(';');

    consumer.doneConsuming();
  }

  void SingularIOHandler::doReadBareIdeal
  (Scanner& in, InputConsumer& consumer) {
    consumer.beginIdeal();

    in.expect("ideal");
    in.expect('I');
    in.expect('=');

    if (!in.match('0')) {
      do {
		consumer.consumeTermProductNotation(in);
      } while (in.match(','));
    }
    in.expect(';');

    consumer.endIdeal();
  }

  void SingularIOHandler::doReadBarePolynomial(Scanner& in,
                                               const VarNames& names,
                                               CoefBigTermConsumer& consumer) {
    consumer.consumeRing(names);
    vector<mpz_class> term(names.getVarCount());
    mpz_class coef;

    in.expect("poly");
    in.expect('p');
    in.expect('=');

    consumer.beginConsuming();
    bool first = true;
    do {
      readCoefTerm(coef, term, names, first, in);
      consumer.consume(coef, term);
      first = false;
    } while (!in.match(';'));
    consumer.doneConsuming();
  }

  void S::writeRing(const VarNames& names, FILE* out) {
    if (names.getVarCount() == 0)
      fputs("ring R = 0, (dummy), lp;\nint noVars = 1;\n", out);
    else {
      fputs("ring R = 0, (", out);

      const char* pre = "";
      for (unsigned int i = 0; i < names.getVarCount(); ++i) {
        fputs(pre, out);
        fputs(names.getName(i).c_str(), out);
        pre = ", ";
      }
      fputs("), lp;\nint noVars = 0;\n", out);
    }
  }
}
