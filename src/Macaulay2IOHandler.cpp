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
#include "Macaulay2IOHandler.h"

#include "Scanner.h"
#include "VarNames.h"
#include "TermTranslator.h"
#include "DataType.h"
#include "Term.h"
#include "IdealWriter.h"
#include "PolyWriter.h"
#include "error.h"
#include "display.h"
#include "InputConsumer.h"
#include <cstdio>

namespace IO {
  namespace {
    string m2GetRingName(const VarNames& names);
    void m2WriteRing(const VarNames& names, FILE* out);
  }

  class M2IdealWriter : public IdealWriter {
  public:
    M2IdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      if (first)
        m2WriteRing(getNames(), getFile());
      fputs("I = monomialIdeal(", getFile());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      writeTermProduct(term, translator, getFile());

      const size_t varCount = translator.getVarCount();
      for (size_t var = 0; var < varCount; ++var)
        if (translator.getExponent(var, term) != 0)
          return;

      fputc('_', getFile());
      fputs(m2GetRingName(translator.getNames()).c_str(), getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      writeTermProduct(term, getNames(), getFile());

      const size_t varCount = term.size();
      for (size_t var = 0; var < varCount; ++var)
        if (term[var] != 0)
          return;

      fputc('_', getFile());
      fputs(m2GetRingName(getNames()).c_str(), getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      if (wasZeroIdeal) {
        // Macaulay 2's monomialIdeal reports an error if we give it an
        // empty list, so to get the zero ideal we have to explicitly
        // specify zero as a generator.
        fprintf(getFile(), "0_%s);\n", m2GetRingName(getNames()).c_str());
      } else
        fputs("\n);\n", getFile());
    }

    virtual void doWriteEmptyList() {
      m2WriteRing(getNames(), getFile());
    }
  };

  class M2PolyWriter : public PolyWriter {
  public:
    M2PolyWriter(FILE* out): PolyWriter(out) {
    }

    virtual void doWriteHeader() {
      m2WriteRing(getNames(), getFile());
      fputs("p =", getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const Term& term,
                             const TermTranslator& translator,
                             bool firstGenerator) {
      if (firstGenerator)
        fputs("\n ", getFile());
      else
        fputs(" +\n ", getFile());

      writeCoefTermProduct(coef, term, translator, true, getFile());
    }

    virtual void doWriteTerm(const mpz_class& coef,
                             const vector<mpz_class>& term,
                             bool firstGenerator) {
      if (firstGenerator)
        fputs("\n ", getFile());
      else
        fputs(" +\n ", getFile());

      writeCoefTermProduct(coef, term, getNames(), true, getFile());
    }

    virtual void doWriteFooter(bool wasZero) {
      if (wasZero)
        fputs("\n 0", getFile());
      fputs(";\n", getFile());
    }
  };

  Macaulay2IOHandler::Macaulay2IOHandler():
    IOHandlerCommon(staticGetName(),
                    "Format understandable by the program Macaulay 2.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerInput(DataType::getPolynomialType());
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getMonomialIdealListType());
    registerOutput(DataType::getPolynomialType());
  }

  const char* Macaulay2IOHandler::staticGetName() {
    return "m2";
  }

  BigTermConsumer* Macaulay2IOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW M2IdealWriter(out);
  }

  CoefBigTermConsumer* Macaulay2IOHandler::doCreatePolynomialWriter(FILE* out) {
    return FROBBY_NEW M2PolyWriter(out);
  }

  void Macaulay2IOHandler::doWriteTerm(const vector<mpz_class>& term,
                                       const VarNames& names,
                                       FILE* out) {
    writeTermProduct(term, names, out);
  }

  void Macaulay2IOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	consumer.consumeTermProductNotation(in);
	if (in.match('_'))
	  in.readIdentifier();
  }

  void Macaulay2IOHandler::doReadRing(Scanner& in, VarNames& names) {
    names.clear();
    const char* ringName = in.readIdentifier();
    ASSERT(ringName != 0 && string(ringName) != "");
    if (ringName[0] != 'R') {
      reportSyntaxError
        (in, "Expected name of ring to start with an upper case R.");
      ASSERT(false); // shouldn't reach here.
    }

    in.expect('=');

    in.eatWhite();
    if (in.peek() == 'Z') {
      displayNote("In the Macaulay 2 format, writing ZZ as the ground field "
                  "instead of QQ is deprecated and may not work in future "
                  "releases of Frobby.");
      in.expect("ZZ");
    } else
      in.expect("QQ");
    in.expect('[');

    // The enclosing braces are optional, but if the start brace is
    // there, then the end brace should be there too.
    bool readBrace = in.match('{');
    if (readBrace) {
      displayNote("In the Macaulay 2 format, putting braces { } around the "
                  "variables is deprecated and may not work in future "
                  "releases of Frobby.");
    }

    if (in.peekIdentifier()) {
      do {
        names.addVarSyntaxCheckUnique(in, in.readIdentifier());
      } while (in.match(','));
    }

    if (readBrace)
      in.expect('}');
    in.expect(']');
    in.expect(';');
  }

  bool Macaulay2IOHandler::doPeekRing(Scanner& in) {
    return in.peek('R') || in.peek('r');
  }

  void Macaulay2IOHandler::doReadBareIdeal
  (Scanner& in, InputConsumer& consumer) {
    consumer.beginIdeal();

    in.expect('I');
    in.expect('=');
    in.expect("monomialIdeal");
    in.expect('(');

    if (in.match('0')) {
      if (in.match('_'))
        in.readIdentifier();
    } else {
      do {
		consumer.consumeTermProductNotation(in);
        if (in.match('_'))
          in.readIdentifier();
      } while (in.match(','));
    }
    in.expect(')');
    in.expect(';');
    consumer.endIdeal();
  }

  void Macaulay2IOHandler::doReadBarePolynomial(Scanner& in,
                                                const VarNames& names,
                                                CoefBigTermConsumer& consumer) {
    consumer.consumeRing(names);
    vector<mpz_class> term(names.getVarCount());
    mpz_class coef;

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

  namespace {
    string m2GetRingName(const VarNames& names) {
      string name = "R";
      if (!names.contains(name))
        return name;

      for (mpz_class i = 1; true; ++i) {
        name = "R" + i.get_str();
        if (!names.contains(name))
          return name;
      }
    }

    void m2WriteRing(const VarNames& names, FILE* out) {
      fputs(m2GetRingName(names).c_str(), out);
      fputs(" = QQ[", out);

      const char* pre = "";
      for (unsigned int i = 0; i < names.getVarCount(); ++i) {
        fputs(pre, out);
        if (names.getName(i) == "R") {
          string msg =
            "The name of the ring in Macaulay 2 format is usually named R,\n"
            "but in this case there is already a variable named R. Thus,\n"
            "the ring has been renamed to " + m2GetRingName(names) + '.';
          displayNote(msg);
        }
        fputs(names.getName(i).c_str(), out);
        pre = ", ";
      }
      fputs("];\n", out);
    }
  }
}
