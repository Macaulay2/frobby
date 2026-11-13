/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 University of Aarhus
   Contact Bjarke Hammersholt Roune for license information (www.broune.com)

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
#ifndef UNI_HASH_POLYNOMIAL_GUARD
#define UNI_HASH_POLYNOMIAL_GUARD

#include <unordered_map>

class CoefBigTermConsumer;

/** Hash function for using an mpz_class as the key of an
 unordered_map. */
class MpzHash {
 public:
  size_t operator()(const mpz_class& i) const {
    // The constant is a prime. This method needs to be improved.
    return mpz_fdiv_ui(i.get_mpz_t(), 2106945901u);
  }
};

/** A sparse univariate polynomial represented by a hash table mapping
 terms to coefficients. This allows to avoid duplicate terms without a
 large overhead.
*/
class UniHashPolynomial {
 public:
  /** Add +t^exponent or -t^exponent to the polynomial depending on
   whether plus is true or false, respectively. */
  void add(bool plus, const mpz_class& exponent);

  /** Add coef*t^exponent to the polynomial. */
  void add(int coef, size_t exponent);

  /** Add coef*t^exponent to the polynomial. */
  void add(const mpz_class& coef, const mpz_class& exponent);

  void feedTo(CoefBigTermConsumer& consumer, bool inCanonicalOrder = false) const;

  size_t getTermCount() const;

 private:
  typedef unordered_map<mpz_class, mpz_class, MpzHash> TermMap;
  TermMap _terms;
};

#endif
