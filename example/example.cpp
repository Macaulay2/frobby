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

// This file is an example of how to use the public interface of Frobby.
// It also serves as a test of the public interface of Frobby.

#include "../src/frobby.h"

#include <vector>
#include <algorithm>
#include <gmp.h>

#include "MyIdeal.h"
#include "MyPolynomial.h"
#include "MyTermConsumer.h"
#include "MyIdealCreators.h"
#include "MyPolynomialCreators.h"

#include "AlexanderDualTests.h"
#include "HilbertPoincareTests.h"

void testFrobbyLibraryInterface() {
  testAlexanderDual();
  testHilbertPoincareSeries();
}
