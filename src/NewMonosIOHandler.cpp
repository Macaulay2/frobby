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
#include "NewMonosIOHandler.h"

#include "Scanner.h"
#include "VarNames.h"
#include "BigTermConsumer.h"
#include "DataType.h"
#include "IdealWriter.h"
#include "error.h"
#include "InputConsumer.h"

#include <cstdio>

namespace IO {
  namespace NewMonos {
    void writeRing(const VarNames& names, FILE* out);
    void readRingNoLeftParen(Scanner& in, VarNames& names);
    void readIdealNoLeftParen(Scanner& in, InputConsumer& consumer);
  }
  namespace N = NewMonos;

  class NewMonosIdealWriter : public IdealWriter {
  public:
    NewMonosIdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      fputs("(monomial-ideal-with-order\n ", getFile());
      N::writeRing(getNames(), getFile());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      fputs("\n ", getFile());
      writeTermProduct(term, translator, getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term,
                             bool first) {
      fputs("\n ", getFile());
      writeTermProduct(term, getNames(), getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      fputs("\n)\n", getFile());
    }

    virtual void doWriteEmptyList() {
      N::writeRing(getNames(), getFile());
    }
  };

  NewMonosIOHandler::NewMonosIOHandler():
    IOHandlerImpl(staticGetName(), "Newer format used by the program Monos.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerOutput(DataType::getMonomialIdealType());
  }

  const char* NewMonosIOHandler::staticGetName() {
    return "newmonos";
  }

  BigTermConsumer* NewMonosIOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW NewMonosIdealWriter(out);
  }

  void NewMonosIOHandler::doWriteTerm(const vector<mpz_class>& term,
                                      const VarNames& names,
                                      FILE* out) {
    writeTermProduct(term, names, out);
  }

  void NewMonosIOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	consumer.consumeTermProductNotation(in);
  }

  void NewMonosIOHandler::doReadIdeal(Scanner& in, InputConsumer& consumer) {
    in.expect('(');
    N::readIdealNoLeftParen(in, consumer);
  }

  void NewMonosIOHandler::doReadIdeals(Scanner& in, InputConsumer& consumer) {
    in.expect('(');
    if (in.peek('l') || in.peek('L')) {
      VarNames names;
      N::readRingNoLeftParen(in, names);
      consumer.consumeRing(names);
      return;
    }

    do {
      N::readIdealNoLeftParen(in, consumer);
    } while (in.match('('));
  }

  void N::writeRing(const VarNames& names, FILE* out) {
    fputs("(lex-order", out);
    for (unsigned int i = 0; i < names.getVarCount(); ++i) {
      putc(' ', out);
      fputs(names.getName(i).c_str(), out);
    }
    fputc(')', out);
  }

  void N::readRingNoLeftParen(Scanner& in, VarNames& names) {
    in.expect("lex-order");
    while (!in.match(')'))
      names.addVarSyntaxCheckUnique(in, in.readIdentifier());
  }

  void N::readIdealNoLeftParen(Scanner& in, InputConsumer& consumer) {
    in.expect("monomial-ideal-with-order");

    VarNames names;
    in.expect('(');
    N::readRingNoLeftParen(in, names);
    consumer.consumeRing(names);

    consumer.beginIdeal();
    while (!in.match(')'))
	  consumer.consumeTermProductNotation(in);
    consumer.endIdeal();
  }
}
