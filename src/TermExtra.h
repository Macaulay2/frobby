/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2007 Bjarke Hammersholt Roune (www.broune.com)
   Copyright (C) 2010 University of Aarhus
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
#ifndef TERM_EXTRA_GUARD
#define TERM_EXTRA_GUARD

#include "Term.h"

/** @file
 This file contains additional operations involving Term that are
 less commonly used. This makes Term.h smaller which cuts down on
 compilation and especially recompilation time since Term.h is
 included in many places. */

/** Returns the lower median exponent of a. Returns zero if varCount
 is zero. */
Exponent median(const Exponent* a, size_t varCount);
inline Exponent median(const Term& a) {
  return median(a.begin(), a.getVarCount());
}

/** Returns the lower median of the positive exponents of a. Returns
 zero if all terms of a are zero. */
Exponent medianPositive(const Exponent* a, size_t varCount);
inline Exponent medianPositive(const Term& a) {
  return medianPositive(a.begin(), a.getVarCount());
}

/** Puts the sum of the entries of a into res. res has to be an
 mpz_class as opposed to an Exponent since the sum could cause an
 overflow. */
void totalDegree(mpz_class& res, const Exponent* a, size_t varCount);
inline void totalDegree(mpz_class& res, const Term& a) {
  totalDegree(res, a.begin(), a.getVarCount());
}

/** Returns the smallest positive exponent of a. Returns zero if all
 entries of a are zero. */
Exponent minimumPositive(const Exponent* a, size_t varCount);
inline Exponent minimumPositive(const Term& a) {
  return minimumPositive(a.begin(), a.getVarCount());
}

/** Returns the largest exponent of a. Returns zero if varCount is
 zero. */
Exponent maximum(const Exponent* a, size_t varCount);
inline Exponent maximum(const Term& a) {
  return maximum(a.begin(), a.getVarCount());
}

#endif
