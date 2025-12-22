# Frobby - Computations With Monomial Ideals

Frobby is a software system and project for computations with monomial ideals. Frobby is free software and it is intended as a vehicle for computational and mathematical research on monomial ideals.

The current functionality includes Euler characteristic, Hilbert series, maximal standard monomials, combinatorial optimization on monomial ideals, primary decomposition, irreducible decomposition, Alexander dual, associated primes, minimization and intersection of monomial ideals as well as the computation of Frobenius problems (using [4ti2](https://4ti2.github.io/)) with very large numbers. Frobby is also able to translate between formats that can be used with several different computer systems, such as Macaulay2, Monos, 4ti2, CoCoA4 and Singular. Thus Frobby can be used with any of those systems.

The systems [Macaulay2](https://macaulay2.com/) and [CoCoA](https://sites.google.com/view/cocoa-cocoalib) include Frobby and use it to perform some of their computations on monomial ideals. Frobby is also available as an optional package for [Sage](https://www.sagemath.org/).

## Download And Installation

Download Frobby from the [GitHub releases page](https://github.com/Macaulay2/frobby/releases).

Frobby works with Linux and MacOS X, as well as with Windows through Cygwin. The short description for getting a Frobby executable is to download Frobby, unpack it with tar and type `make` and then `make install`. This requires that you have installed [GMP](https://gmplib.org/) with C++ support. There are more detailed installation instructions in the [Frobby manual](https://macaulay2.com/Downloads/OtherSourceCode/manual.pdf).

To try Frobby out after installation, you might generate a random monomial ideal and decompose it into irreducible components by typing

    frobby genideal|frobby irrdecom

See the [Frobby manual](https://macaulay2.com/Downloads/OtherSourceCode/manual.pdf) for a tutorial that shows how to do most of what Frobby can do. Frobby also has a built-in help system which contains complete information about what Frobby can do, though in a more compact and perhaps less accessible way. You access the built-in help system by typing

    frobby help

*NOTE: Frobby is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.*

## Papers
Some of the algorithms currently in Frobby are described in the following two papers.

- [Solving Thousand Digit Frobenius Problems Using Grobner Bases](https://doi.org/10.1016/j.jsc.2007.06.002)
- [The Slice Algorithm For Irreducible Decomposition of Monomial Ideals](https://doi.org/10.1016/j.jsc.2008.08.002)
