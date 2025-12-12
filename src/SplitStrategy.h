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
#ifndef SPLIT_STRATEGY_GUARD
#define SPLIT_STRATEGY_GUARD

class Term;
class Slice;
class Ideal;
class TermGrader;

/** A SplitStrategy is an implementation of a split selection strategy
 for the Slice Algorithm. Specifically, it makes a decision about what
 kind of splits to perform, and how to make any choices involved in
 performing such a split.
*/
class SplitStrategy {
 public:
  virtual ~SplitStrategy();

  /** Sets pivot to the pivot of a pivot split on slice. The slice is
   not changed mathematically, but e.g. the generators may be
   permuted.

   This method must only be called if isPivotSplit() returns true.
  */
  virtual void getPivot(Term& pivot, Slice& slice) const = 0;

  /** Sets pivot to the pivot of a pivot split on slice. Some pivot
   selection strategies make use of a grading. The slice is not
   changed mathematically, but e.g. the generators may be permuted.

   This method must only be called if isPivotSplit() returns true.
  */
  virtual void getPivot(Term& pivot, Slice& slice, const TermGrader& grader) const = 0;

  /** Returns the variable to perform a label split on.

   This method must only be called if isLabelSplit() returns true.
  */
  virtual size_t getLabelSplitVariable(const Slice& slice) const = 0;

  /** If returns true, only call getPivot. */
  virtual bool isPivotSplit() const = 0;

  /** If returns true, only call getLabelSplitVariable. */
  virtual bool isLabelSplit() const = 0;

  /** Returns the name of the strategy. */
  virtual const char* getName() const = 0;

  /** Returns the strategy whose name has the given prefix. This is
   the only way to create a SplitStrategy.
  */
  static unique_ptr<SplitStrategy> createStrategy(const string& prefix);

 protected:
  SplitStrategy();

  // To make these inaccessible. They are not implemented.
  SplitStrategy(const SplitStrategy&);
  SplitStrategy& operator=(const SplitStrategy&);
  bool operator==(const SplitStrategy&);
};

#endif
