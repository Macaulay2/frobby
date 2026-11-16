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
#ifndef IO_FACADE_GUARD
#define IO_FACADE_GUARD

#include "Facade.h"
#include <vector>

class BigIdeal;
class Scanner;
class IOHandler;
class BigPolynomial;
class VarNames;
class BigTermConsumer;
class SatBinomIdeal;
class SatBinomConsumer;
class SquareFreeIdeal;

/** A facade for input and output of mathematical objects.

    @todo Consider letting Scanner play the role of IOFacade.
    @ingroup Facade
    @ingroup IO
*/
class IOFacade : private Facade {
 public:
  IOFacade(bool printActions);

  bool isValidMonomialIdealFormat(const string& format);

  /** Read a saturated binomial ideal from in and feed it to consumer. */
  void readSatBinomIdeal(Scanner& in, SatBinomConsumer& consumer);

  /** Read a saturated binomial ideal from in and place it in the
   parameter ideal. */
  void readSatBinomIdeal(Scanner& in, SatBinomIdeal& ideal);

  /** Read an ideal from in and feed it to consumer. */
  void readIdeal(Scanner& in, BigTermConsumer& consumer);

  /** Read an ideal from in and place it in the parameter ideal. */
  void readIdeal(Scanner& in, BigIdeal& ideal);

  /** Read a square free ideal from in and place it in the parameter
	  ideal. */
  void readSquareFreeIdeal(Scanner& in, SquareFreeIdeal& ideal);

  /** Insert the ideals that are read into the parameter ideals. The
   parameter ideals is required to be empty. Names contains the last
   ring read, even if there are no ideals.
  */
  void readIdeals(Scanner& in, vector<BigIdeal*>& ideals, VarNames& names);

  void writeIdeal(const BigIdeal& ideal, IOHandler* handler, FILE* out);

  void writeIdeals(const vector<BigIdeal*>& ideals,
                   const VarNames& names,
                   IOHandler* handler,
                   FILE* out);

  void readPolynomial(Scanner& in, BigPolynomial& polynomial);
  void writePolynomial(const BigPolynomial& polynomial,
                       IOHandler* handler,
                       FILE* out);
  void writeTerm(const vector<mpz_class>& term,
                 const VarNames& names,
                 IOHandler* handler,
                 FILE* out);

  void readFrobeniusInstance(Scanner& in, vector<mpz_class>& instance);
  void readFrobeniusInstanceWithGrobnerBasis
    (Scanner& in, BigIdeal& ideal, vector<mpz_class>& instance);
  void writeFrobeniusInstance(FILE* out, vector<mpz_class>& instance);

  bool readAlexanderDualInstance
    (Scanner& in, BigIdeal& ideal, vector<mpz_class>& term);

  void readVector(Scanner& in, vector<mpz_class>& v, size_t integerCount);

  bool isValidLatticeFormat(const string& format);
  void readLattice(Scanner& in, BigIdeal& ideal);
  void writeLattice(FILE* out, const BigIdeal& ideal, const string& format);
};

#endif
