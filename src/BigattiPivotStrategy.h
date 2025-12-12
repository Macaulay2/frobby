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
#ifndef BIGATTI_PIVOT_STRATEGY_GUARD
#define BIGATTI_PIVOT_STRATEGY_GUARD

class Term;
class BigattiState;

/** A BigattiPivotStrategy is an implementation of a pivot selection
 strategy for the Hilbert series algorithm by Bigatti
 et.al..
*/
class BigattiPivotStrategy {
 public:
  virtual ~BigattiPivotStrategy();

  /** Returns the pivot of a pivot split of state. The state is not
      changed mathematically, but e.g. the generators of the ideal may be
      permuted. The returned object will be valid until the next
      non-const method on this object is called, such as getPivot or a
      destructor. */
  virtual const Term& getPivot(BigattiState& state) = 0;

  /** Returns the name of the strategy. */
  virtual const char* getName() const = 0;

  /** Returns the strategy whose name has the given prefix. This
      function is the only way to create a BigattiPivotStrategy.

      A pivot p is widened by replacing it by
      \f$\gcd(\min(I)\cap\langle p\rangle/f$. This may be an
      improvement for the state with colon while being almost as good
      for the state with add.
  */
  static unique_ptr<BigattiPivotStrategy> createStrategy
    (const string& prefix, bool widen);

 protected:
  BigattiPivotStrategy();

  // To make these inaccessible. They are not implemented.
  BigattiPivotStrategy(const BigattiPivotStrategy&);
  BigattiPivotStrategy& operator=(const BigattiPivotStrategy&);
  bool operator==(const BigattiPivotStrategy&);
};

#endif
