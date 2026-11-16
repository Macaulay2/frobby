/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2009 Bjarke Hammersholt Roune (www.broune.com)

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

/** @file doxygen.cpp This file contains content for the Doxygen-generated
    documentation that does not fit elsewhere.

    @todo Add all Frobby code to a FrobbyImpl namespace or some
    such. Perhaps do it by starting the namespace in stdinc.h, and
    then adding an include to the bottom of every .cpp file that
    closes the namespace.

    @todo Do something to automatically get a Doxygen module of all
    things not in a Doxygen module. This is a good way to capture
    those things falling through the cracks by forgetting an ingroup
    directive. Perhaps do this as for the idea with a namespace above,
    but instead start a weakgroup in stdinc.h and end it in the bottom
    include.

    @todo Go through the documentation in frobby.h and properly Doxify
    it and use LaTeX. Explain the mathematical background a bit,
    including definitions of each operation.

    @todo Add a gradedHilbertPoincareSeries function to library
    interface which dots each multigraded piece with some given
    vector, rather than (1,...,1) as for
    univariateHilbertPointcareSeries.

    @todo Make a Slice Algorithm Module. Look into this set of classes
    and remove any smeels. Then document those classes that need it.

    @todo Add examples sections to library interface documentation. I
    believe there is a command for this in Doxygen. Especially specify
    what happens with the zero ideal, the whole ring ideal and when
    there are no variables.

    @todo Take a look at MsmSlice::baseCase for the case where the
    ideal is not simplified. Some of the base cases for the simplified
    case should apply as well for the non-simplified case, or at least
    it should be possible to modify them to apply.

    @todo See if this idea works: If m is a minimal generator of I,
    and m has maximal exponent in some var, and has the unique minimum
    exponent in some other var, than we can do an inner split on that
    other var.

    @todo Grep for TODO in upper case and add everything found by that
    as Doxygen todo's.
*/

/** \defgroup Facade The Facade Module

    The facades follow the %Facade Pattern, which is to say that they
    put a convenient interface on an underlying system whose interface
    is more difficult to use.

    Apart from implementing this pattern, the facades also take on the
    responsibility for printing out actions that are being carried
    out, and how long each step took, if asked to do so. This is a
    convenient way to do this, since each call to a facade corresponds
    to some concrete action to carry out, and the printing code does
    not belong in the code doing the actual computations and it would
    be much duplicated if it resided in the public interface code.

    That the facades take on this responsibility implies that public
    interface code must never call computational code without using a
    facade, as then that computation will not be printed. This has the
    occasional result that facades are put on code whose interface is
    simple enough that it otherwise would not need a facade.

    @todo Document each facade.
*/

/** \defgroup IO The Input/Output Module

    @todo Describe the IO module.

    @todo Add the IO classes to the IO module.

    @todo Take a look at the IO module, change it if it needs it, and
    then document it.
*/

/** \mainpage

%Frobby is a program and library for performing computations related to
monomial ideals, such as Hilbert-Poincare series and irreducible
decomposition.

%Frobby is loosely structured into a number of layers. These are the
bottom layer which performs the actual computations, the middle layer
that gives the functionality in the bottom layer a more convenient
interface, and the top layer which consists of Frobby's interface to
the world as a library or console program.

The interface to %Frobby as a library is in the file frobby.h. The
console layer has more code to it, where the central classes are those
derived from the classes Action and IOHandler.

The middle layer is composed of a number of classes following the
%Facade design pattern, and they all derive from the class
Facade.

The bottom layer consists of various classes and functions that
perform calculations on monomial ideals. The main functionality is
accessed by using the method SliceStrategy::run, which runs the Slice
Algorithm on a monomial ideal. The Slice Algorithm can be specialized
to compute a number of things, and these specializations are
implemented as derived classes of SliceStrategy.
*/
