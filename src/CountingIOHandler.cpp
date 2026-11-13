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
#include "CountingIOHandler.h"

#include "BigTermConsumer.h"
#include "CoefBigTermConsumer.h"
#include "DataType.h"
#include "BigIdeal.h"
#include "BigPolynomial.h"

namespace IO {
  namespace {
    /** This class is a helper class for CountingIOHandler. It counts
        the number of terms that it consumes. */
    class CountingConsumer :
      public BigTermConsumer, public CoefBigTermConsumer {
    public:
      CountingConsumer(FILE* out):
        _termCount(0),
        _out(out) {
      }

      /** This destructor writes the final number out.

          @todo This should not be happening in a destructor. Make a
          doneAllConsuming or something like that to do the output.
      */
      virtual ~CountingConsumer() {
        // Destructors must not throw exceptions. As far as I know,
        // bad_alloc is the only thing that could be thrown from
        // gmp_fprintf.
      }

      virtual void consumeRing(const VarNames& names) {
      }

      virtual void beginConsuming() {
        _termCount = 0;
      }

      virtual void consume(const Term& term, const TermTranslator& translator) {
        ++_termCount;
      }

      virtual void consume(const vector<mpz_class>& term) {
        ++_termCount;
      }

      virtual void consume(const BigIdeal& ideal) {
        beginConsuming();
        _termCount += ideal.getGeneratorCount();
        doneConsuming();
      }

      virtual void consume(const mpz_class& coef, const Term& term) {
        ++_termCount;
      }

      virtual void consume(const mpz_class& coef,
                           const Term& term,
                           const TermTranslator& translator) {
        ++_termCount;
      }

      virtual void consume(const mpz_class& coef,
                           const vector<mpz_class>& term) {
        ++_termCount;
      }

      virtual void consume(const BigPolynomial& poly) {
        beginConsuming();
        _termCount += poly.getTermCount();
        doneConsuming();
      }

      virtual void doneConsuming() {
        gmp_fprintf(_out, "%Zd\n", _termCount.get_mpz_t());
      }

    private:
      mpz_class _termCount;
      FILE* _out;
    };
  }

  CountingIOHandler::CountingIOHandler():
    IOHandlerImpl(staticGetName(), "Writes the number of output terms.") {
    registerOutput(DataType::getMonomialIdealType());
    registerOutput(DataType::getPolynomialType());
    registerOutput(DataType::getMonomialIdealListType());
  }

  const char* CountingIOHandler::staticGetName() {
    return "count";
  }

  BigTermConsumer* CountingIOHandler::doCreateIdealWriter(FILE* out) {
    return FROBBY_NEW CountingConsumer(out);
  }

  CoefBigTermConsumer* CountingIOHandler::doCreatePolynomialWriter(FILE* out) {
    return FROBBY_NEW CountingConsumer(out);
  }

  void CountingIOHandler::doWriteTerm(const vector<mpz_class>& term,
                                      const VarNames& names,
                                      FILE* out) {
    fputc('1', out);
  }
}
