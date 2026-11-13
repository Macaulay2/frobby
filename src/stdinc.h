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
/** \file stdinc.h
 This header file includes common definitions and is included as the
 first line of code in every implementation file (.cpp). Thus it is
 never necessary to include it in headers, and anything defined here
 is defined everywhere throughout the code base.
*/

// Some versions of GMP do not define gmp_fprintf unless cstdio is
// included first, so we have to include it here.
#include <cstdio>
#include <cstddef>

#include <gmp.h>
#include <gmpxx.h>

#include <cstddef>
#include <memory>
using namespace std;

#ifdef DEBUG
#include <iostream> // Useful for debugging.
#define PRINT
#define IF_DEBUG(X) X
#include <cassert>
#define ASSERT(X) assert(X);
void* operator new(size_t s, const char* file, size_t line);
void* operator new[](size_t s, const char* file, size_t line);
void operator delete(void* s, const char* file, size_t line);
void operator delete[](void* s, const char* file, size_t line);
#define FROBBY_NEW new (__FILE__, __LINE__)
#else
#define IF_DEBUG(X)
#define ASSERT(X)
#define FROBBY_NEW new
#endif

typedef unsigned int Exponent;

/// The native unsigned type for the CPU. An incorrect type can result in
/// worse performance but all computations must still be correct.
typedef unsigned long Word;
static const size_t BitsPerWord = 8 * sizeof(Word);

/// The alignment that memory allocators must ensure. In other words
/// allocators must return pointer addresses that are divisible by
/// MemoryAlignment. MemoryAlignment must be a power of 2.
static const size_t MemoryAlignment = sizeof(long);
