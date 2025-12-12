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
#ifndef INTERSECT_FACADE_GUARD
#define INTERSECT_FACADE_GUARD

#include <vector>
#include "Facade.h"

class BigIdeal;
class VarNames;

/** A facade for intersecting monomial ideals.

    @todo Merge this into IdealFacade.
    @ingroup Facade
 */
class IntersectFacade : Facade {
 public:
  IntersectFacade(bool printActions);

  /** Returns the intersection of ideals. If ideals is empty, then the
      whole ring is the intersection, which requires knowing the ring,
      and this is what names contains.
  */
  unique_ptr<BigIdeal> intersect(const vector<BigIdeal*>& ideals,
                               const VarNames& names);
};

#endif
