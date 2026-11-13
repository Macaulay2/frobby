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
#include "CoCoA4IOHandler.h"

#include "Scanner.h"
#include "VarNames.h"
#include "Term.h"
#include "TermTranslator.h"
#include "FrobbyStringStream.h"
#include "DataType.h"
#include "IdealWriter.h"
#include "PolyWriter.h"
#include "error.h"
#include "InputConsumer.h"

#include <cstdio>

namespace IO {
  namespace CoCoA4 {
    void writeRing(const VarNames& names, FILE* out);
    void writeTermProduct(const Term& term,
                          const TermTranslator& translator,
                          FILE* out);
    void writeTermProduct(const vector<mpz_class>& term,
                          const VarNames& names,
                          FILE* out);
    void readTerm(Scanner& in, vector<mpz_class>& term);
    void readTerm(Scanner& in, InputConsumer& consumer);
    void readVarPower(vector<mpz_class>& term, Scanner& in);
    void readVarPower(Scanner& in, InputConsumer& consumer);
    void readCoefTerm(mpz_class& coef,
                      vector<mpz_class>& term,
                      bool firstTerm,
                      Scanner& in);
  }
  namespace C = CoCoA4;

  class CoCoA4IdealWriter : public IdealWriter {
  public:
    CoCoA4IdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      C::writeRing(getNames(), getFile());
      fputs("I := Ideal(", getFile());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      C::writeTermProduct(term, translator, getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      C::writeTermProduct(term, getNames(), getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      fputs("\n);\n", getFile());
    }

    virtual void doWriteEmptyList() {
      C::writeRing(getNames(), getFile());
    }
  };

  class CoCoA4PolyWriter : public PolyWriter {
  public:
    CoCoA4PolyWriter(FILE* out): PolyWriter(out) {
    }

    virtual void doWriteHeader() {
      C::writeRing(getNames(), getFile());
      fputs("p :=", getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const Term& term,
                             const TermTranslator& translator,
                             bool firstGenerator) {
      fputs("\n ", getFile());

      if (coef >= 0 && !firstGenerator)
        fputc('+', getFile());

      if (term.isIdentity()) {
        gmp_fprintf(getFile(), "%Zd", coef.get_mpz_t());
        return;
      }

      if (coef == -1)
        fputc('-', getFile());
      else if (coef != 1)
        gmp_fprintf(getFile(), "%Zd", coef.get_mpz_t());

      C::writeTermProduct(term, translator, getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const vector<mpz_class>& term,
                             bool firstGenerator) {
      fputs("\n ", getFile());
      if (coef >= 0 && !firstGenerator)
        fputc('+', getFile());

      bool isIdentity = true;
      for (size_t var = 0; var < term.size(); ++var)
        if (term[var] != 0)
          isIdentity = false;

      if (isIdentity) {
        gmp_fprintf(getFile(), "%Zd", coef.get_mpz_t());
        return;
      }

      if (coef == -1)
        fputc('-', getFile());
      else if (coef != 1)
        gmp_fprintf(getFile(), "%Zd", coef.get_mpz_t());

      C::writeTermProduct(term, getNames(), getFile());
    }

    virtual void doWriteFooter(bool wasZero) {
      if (wasZero)
        fputs("\n 0", getFile());
      fputs(";\n", getFile());
    }
  };

  IO::CoCoA4IOHandler::CoCoA4IOHandler():
    IOHandlerCommon(staticGetName(),
                    "Format understandable by the program CoCoA 4.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerInput(DataType::getPolynomialType());
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getPolynomialType());
  }

  const char* CoCoA4IOHandler::staticGetName() {
    return "cocoa4";
  }

  BigTermConsumer* CoCoA4IOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW CoCoA4IdealWriter(out);
  }

  CoefBigTermConsumer* CoCoA4IOHandler::doCreatePolynomialWriter(FILE* out) {
    return FROBBY_NEW CoCoA4PolyWriter(out);
  }

  void CoCoA4IOHandler::doWriteTerm(const vector<mpz_class>& term,
                                    const VarNames& names,
                                    FILE* out) {
    C::writeTermProduct(term, names, out);
  }

  void CoCoA4IOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	C::readTerm(in, consumer);
  }

  void CoCoA4IOHandler::doReadRing(Scanner& in, VarNames& names) {
    names.clear();

    in.expect("Use");
    in.expect('R');
    in.expect("::=");
    in.expect('Q');
    in.expect('[');
    in.expect('x');

    size_t varCount = 0;
    if (in.match('[')) {
      in.expect('1');
      in.expect("..");
      in.readSizeT(varCount);
      in.expect(']');
    }
    in.expect(']');
    in.expect(';');

    in.expect("Names");
    in.expect(":=");
    in.expect('[');

    for (size_t var = 0; var < varCount; ++var) {
      in.expect('\"');
      if (in.peekWhite())
        reportSyntaxError(in, "Variable name contains space.");

      names.addVarSyntaxCheckUnique(in, in.readIdentifier());

      if (in.peekWhite())
        reportSyntaxError(in, "Variable name contains space.");

      in.expect('\"');
      if (var < varCount - 1)
        in.expect(',');
    }

    in.expect(']');
    in.expect(';');
  }

  bool CoCoA4IOHandler::doPeekRing(Scanner& in) {
    return in.peek('U') || in.peek('u');
  }

  void CoCoA4IOHandler::doReadBareIdeal
  (Scanner& in, InputConsumer& consumer) {
    consumer.beginIdeal();

    in.expect('I');
    in.expect(":=");
    in.expect("Ideal");
    in.expect('(');

    if (!in.match(')')) {
      do {
		C::readTerm(in, consumer);
      } while (in.match(','));
      in.expect(')');
    }
    in.match(';');

    consumer.endIdeal();
  }

  void CoCoA4IOHandler::doReadBarePolynomial(Scanner& in,
                                             const VarNames& names,
                                             CoefBigTermConsumer& consumer) {
    consumer.consumeRing(names);
    vector<mpz_class> term(names.getVarCount());
    mpz_class coef;

    in.expect('p');
    in.expect(":=");

    consumer.beginConsuming();
    bool first = true;
    do {
      C::readCoefTerm(coef, term, first, in);
      consumer.consume(coef, term);
      first = false;
    } while (!in.match(';'));
    consumer.doneConsuming();
  }

  void C::writeRing(const VarNames& names, FILE* out) {
    if (names.getVarCount() ==  0) {
      fputs("Use R ::= Q[x];\nNames := [];\n", out);
      return;
    }

    fprintf(out, "Use R ::= Q[x[1..%lu]];\n",
            (unsigned long)names.getVarCount());

    fputs("Names := [", out);

    const char* pre = "\"";
    for (size_t i = 0; i < names.getVarCount(); ++i) {
      fputs(pre, out);
      fputs(names.getName(i).c_str(), out);
      pre = "\", \"";
    }
    fputs("\"];\n", out);
  }

  void C::writeTermProduct(const Term& term,
                           const TermTranslator& translator,
                           FILE* out) {
    bool seenNonZero = false;
    size_t varCount = term.getVarCount();
    for (size_t var = 0; var < varCount; ++var) {
      const char* exp = translator.getExponentString(var, term[var]);
      if (exp == 0)
        continue;
      seenNonZero = true;

      fprintf(out, "x[%lu]", (unsigned long)(var + 1));
      if (exp[0] != '1' || exp[1] != '\0') {
        fputc('^', out);
        fputs(exp, out);
      }
    }

    if (!seenNonZero)
      fputc('1', out);
  }

  void C::writeTermProduct(const vector<mpz_class>& term,
                           const VarNames& names,
                           FILE* out) {
    bool seenNonZero = false;
    size_t varCount = term.size();
    for (size_t var = 0; var < varCount; ++var) {
      if (term[var] == 0)
        continue;
      seenNonZero = true;

      fprintf(out, "x[%lu]", (unsigned long)(var + 1));
      if (term[var] != 1) {
        fputc('^', out);
        mpz_out_str(out, 10, term[var].get_mpz_t());
      }
    }

    if (!seenNonZero)
      fputc('1', out);
  }

  void C::readTerm(Scanner& in, vector<mpz_class>& term) {
    for (size_t var = 0; var < term.size(); ++var)
      term[var] = 0;

    if (in.match('1'))
      return;

    do {
      C::readVarPower(term, in);
      in.eatWhite();
    } while (in.peek() == 'x');
  }

  void C::readTerm(Scanner& in, InputConsumer& consumer) {
	consumer.beginTerm();
    if (!in.match('1')) {
      do {
        C::readVarPower(in, consumer);
        in.eatWhite();
      } while (in.peek() == 'x');
    }
	consumer.endTerm();
  }

  void C::readVarPower(vector<mpz_class>& term, Scanner& in) {
    in.expect('x');
    in.expect('[');

    size_t var;
    in.readSizeT(var);
    if (var == 0 || var > term.size()) {
      FrobbyStringStream errorMsg;
      errorMsg << "There is no variable x[" << var << "].";
      reportSyntaxError(in, errorMsg);
    }
    --var;

    in.expect(']');

    if (term[var] != 0) {
      FrobbyStringStream errorMsg;
      errorMsg << "The variable x["
               << (var + 1)
               << "] appears twice in the same monomial.";
      reportSyntaxError(in, errorMsg);
    }

    if (in.match('^')) {
      in.readInteger(term[var]);
      if (term[var] <= 0) {
        FrobbyStringStream errorMsg;
        errorMsg << "Expected positive integer as exponent but got "
                 << term[var] << '.';
        reportSyntaxError(in, errorMsg);
      }
    } else
      term[var] = 1;
  }

  void C::readVarPower(Scanner& in, InputConsumer& consumer) {
    in.expect('x');
    in.expect('[');
    size_t var = consumer.consumeVarNumber(in);
    in.expect(']');
    if (in.match('^'))
	  consumer.consumeVarExponent(var, in);
    else
	  consumer.consumeVarExponentOne(var, in);
  }

  void C::readCoefTerm(mpz_class& coef,
                       vector<mpz_class>& term,
                       bool firstTerm,
                       Scanner& in) {
    for (size_t var = 0; var < term.size(); ++var)
      term[var] = 0;

    bool positive = true;
    if (!firstTerm && in.match('+'))
      positive = !in.match('-');
    else if (in.match('-'))
      positive = false;
    else if (!firstTerm) {
      in.expect('+');
      return;
    }
    if (in.match('+') || in.match('-'))
      reportSyntaxError(in, "Too many adjacent signs.");

    if (in.peekIdentifier()) {
      coef = 1;
      C::readVarPower(term, in);
    } else
      in.readInteger(coef);

    in.eatWhite();
    while (in.peek() == 'x') {
      C::readVarPower(term, in);
      in.eatWhite();
    }

    if (!positive)
      coef = -coef;
  }
}
