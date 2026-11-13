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
#include "MonosIOHandler.h"

#include "Scanner.h"
#include "BigTermConsumer.h"
#include "DataType.h"
#include "IdealWriter.h"
#include "error.h"
#include "InputConsumer.h"

#include <cstdio>

namespace IO {
  namespace Monos {
    void writeRing(const VarNames& names, FILE* out);
  }
  namespace M = Monos;

  class MonosIdealWriter : public IdealWriter {
  public:
    MonosIdealWriter(FILE* out): IdealWriter(out) {
    }

  private:
    virtual void doWriteHeader(bool first) {
      M::writeRing(getNames(), getFile());
      fputc('[', getFile());
    }

    virtual void doWriteTerm(const Term& term,
                             const TermTranslator& translator,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      writeTermProduct(term, translator, getFile());
    }

    virtual void doWriteTerm(const vector<mpz_class>& term,
                             bool first) {
      fputs(first ? "\n " : ",\n ", getFile());
      writeTermProduct(term, getNames(), getFile());
    }

    virtual void doWriteFooter(bool wasZeroIdeal) {
      fputs("\n];\n", getFile());
    }

    virtual void doWriteEmptyList() {
      M::writeRing(getNames(), getFile());
    }
  };

  MonosIOHandler::MonosIOHandler():
    IOHandlerCommon(staticGetName(),
                    "Older format used by the program Monos.") {
    registerInput(DataType::getMonomialIdealType());
    registerInput(DataType::getMonomialIdealListType());
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getMonomialIdealListType());
  }

  const char* MonosIOHandler::staticGetName() {
    return "monos";
  }

  BigTermConsumer* MonosIOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW MonosIdealWriter(out);
  }

  void MonosIOHandler::doWriteTerm(const vector<mpz_class>& term,
                                    const VarNames& names,
                                    FILE* out) {
    writeTermProduct(term, names, out);
  }

  void MonosIOHandler::doReadTerm(Scanner& in, InputConsumer& consumer) {
	consumer.consumeTermProductNotation(in);
  }

  void MonosIOHandler::doReadRing(Scanner& in, VarNames& names) {
    names.clear();
    in.expect("vars");
    if (!in.match(';')) {
      do {
        names.addVarSyntaxCheckUnique(in, in.readIdentifier());
      } while (in.match(','));
      in.expect(';');
    }
  }

  bool MonosIOHandler::doPeekRing(Scanner& in) {
    return in.peek('v');
  }

  void MonosIOHandler::doReadBareIdeal(Scanner& in, InputConsumer& consumer) {
    consumer.beginIdeal();

    in.expect('[');
    if (!in.match(']')) {
      do {
		consumer.consumeTermProductNotation(in);
      } while (in.match(','));
      if (!in.match(']')) {
        if (in.peekIdentifier())
          in.expect('*');
        else
          in.expect(']');
      }
    }
    in.expect(';');

    consumer.endIdeal();
  }

  void M::writeRing(const VarNames& names, FILE* out) {
    fputs("vars ", out);
    const char* pre = "";
    for (unsigned int i = 0; i < names.getVarCount(); ++i) {
      fputs(pre, out);
      fputs(names.getName(i).c_str(), out);
      pre = ", ";
    }
    fputs(";\n", out);
  }
}
