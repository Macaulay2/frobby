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
#ifndef SLICE_STRATEGY_GUARD
#define SLICE_STRATEGY_GUARD

class Slice;
class Term;
class SliceEvent;
class Ideal;
class TaskEngine;

/** This class describes the interface of a strategy object for the
 Slice Algorithm. It determines what goes on when the algorithm runs,
 allowing to specialize the algorithm to do several different
 computations.

 @todo Inspect comments.
*/
class SliceStrategy {
 public:
  virtual ~SliceStrategy();

  /** Run the Slice algorithm. */
  virtual void run(const Ideal& ideal) = 0;

  /** Process the parameter slice. Returns true if this is a base
   case and false otherwise. */
  virtual bool processSlice(TaskEngine& tasks, unique_ptr<Slice> slice) = 0;

  /** This method should only be called before calling run(). */
  virtual void setUseIndependence(bool use) = 0;

  /** This method should only be called before calling run(). */
  virtual void setUseSimplification(bool use) = 0;

  virtual bool getUseSimplification() const = 0;

  /** It is allowed to delete returned slices directly, but it is
   better to use freeSlice. freeSlice can only be called on slices
   obtained from a method of the same strategy. This allows caching of
   slices to avoid frequent allocation and deallocation.
  */
  virtual void freeSlice(unique_ptr<Slice> slice) = 0;
};

#endif
