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
#include "stdinc.h"
#include "OptimizeStrategy.h"
#include "tests.h"

#include "IdealFactory.h"
#include "Ideal.h"
#include "TermTranslator.h"
#include "TermGrader.h"
#include "SplitStrategy.h"
#include "Slice.h"

#include <vector>

// This whole thing was exceedingly hard to get right, which is why
// there are so many tests here.

TEST_SUITE(SliceStrategy)
TEST_SUITE2(SliceStrategy, OptimizeStrategy)

namespace {
  vector<mpz_class> makeVector(mpz_class a, mpz_class b, mpz_class c, mpz_class d) {
    vector<mpz_class> vec(4);
    vec[0] = a;
    vec[1] = b;
    vec[2] = c;
    vec[3] = d;
    return vec;
  }
}

TEST(OptimizeStrategy, Simplify) {
  Ideal ideal;
  TermTranslator translator(IdealFactory::xx_yy_zz_t_xz_yz(), ideal, false);
  TermGrader grader(makeVector(0, 100, 10000, mpz_class("300000000000000007")), translator);
  unique_ptr<SplitStrategy> splitStrategy = SplitStrategy::createStrategy("median");
  OptimizeStrategy strategy
    (grader, splitStrategy.get(), false,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);
  strategy.run(ideal);

  ASSERT_EQ(strategy.getMaximalSolutions(), Ideal(Term("1 1 2 1")));
  ASSERT_EQ(strategy.getMaximalValue(), mpz_class("300000000000020107"));
}

TEST(OptimizeStrategy, ChangedInWayRelevantToBound) {
  TermTranslator translator(4, 10);
  TermGrader grader(makeVector(1, -1, 0, 0), translator);
  unique_ptr<SplitStrategy> splitStrategy = SplitStrategy::createStrategy("median");
  OptimizeStrategy opt
    (grader, splitStrategy.get(), true,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);

  // Case 1 from the documentation.
  ASSERT_TRUE(opt.changedInWayRelevantToBound
              (Term("0 0 0 0"), Term("0 2 0 0"),
               Term("0 1 0 0"), Term("0 2 0 0")));

  // Case 2 from the documentation.
  ASSERT_TRUE(opt.changedInWayRelevantToBound
              (Term("0 0 0 0"), Term("0 10 0 0"),
               Term("0 0 0 0"), Term("0 9 0 0")));

  // Case 3 from the documentation.
  ASSERT_TRUE(opt.changedInWayRelevantToBound
              (Term("0 0 0 0"), Term("9 0 0 0"),
               Term("0 0 0 0"), Term("8 0 0 0")));

  // Case 4 from the documentation.
  ASSERT_TRUE(opt.changedInWayRelevantToBound
              (Term(" 9 0 0 0"), Term("10 0 0 0"),
               Term("10 0 0 0"), Term("10 0 0 0")));

  // Nothing changed.
  ASSERT_FALSE(opt.changedInWayRelevantToBound
               (Term("0 0 0 0"), Term("0 0 0 0"),
                Term("0 0 0 0"), Term("0 0 0 0")));

  // No case applies.
  ASSERT_FALSE(opt.changedInWayRelevantToBound
               (Term("1 2 3 3"), Term("10 9 10 9"),
                Term("1 2 4 4"), Term(" 9 5  9 4")));
}

#define INNER_SIMP_TEST(strat, div, dom, degree, expectPivot) \
  { \
    Term gotPivot(Term(expectPivot).getVarCount()); \
    bool expectSimplify = !Term(expectPivot).isIdentity(); \
    ASSERT_EQ(strat.getInnerSimplify \
              (Term(div), Term(dom), degree, gotPivot), \
              expectSimplify); \
    if (expectSimplify) { \
      ASSERT_EQ(gotPivot, Term(expectPivot)); \
    } \
  }

#define OUTER_SIMP_TEST(strat, div, dom, degree, expectPivot) \
  { \
    Term gotPivot(Term(expectPivot).getVarCount()); \
    bool expectSimplify = !Term(expectPivot).isIdentity(); \
    ASSERT_EQ(strat.getOuterSimplify \
              (Term(div), Term(dom), degree, gotPivot), \
              expectSimplify); \
    if (expectSimplify) { \
      ASSERT_EQ(gotPivot, Term(expectPivot)); \
    } \
  }

TEST(OptimizeStrategy, SimplifyPositiveGrading) {
  TermTranslator translator(4, 10);
  TermGrader grader(makeVector(100, 10, 1, 0), translator);
  unique_ptr<SplitStrategy> splitStrategy = SplitStrategy::createStrategy("median");

  OptimizeStrategy all // Report all optimal solutions.
    (grader, splitStrategy.get(), true,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);
  OptimizeStrategy one // Report one optimal solution.
    (grader, splitStrategy.get(), false,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);

  all.beginConsuming();
  all.consume(Term("1 2 3 4"));
  ASSERT_EQ(all.getMaximalValue(), mpz_class("123"));

  one.beginConsuming();
  one.consume(Term("1 2 3 4"));
  ASSERT_EQ(one.getMaximalValue(), mpz_class("123"));

  // No improvement.
  INNER_SIMP_TEST(all, "1 2 3 4", "1 2 4 4", 124,  "0 0 0 0");
  INNER_SIMP_TEST(one, "1 2 4 4", "1 2 5 4", 125,  "0 0 0 0");

  // Improvement depends on reporting.
  INNER_SIMP_TEST(all, "1 2 1 1", "1 2 5 1", 125,  "0 0 2 0");
  INNER_SIMP_TEST(one, "1 2 1 1", "1 2 5 1", 125,  "0 0 3 0");

  // Improvement in more than one variable, varying with reporting.
  INNER_SIMP_TEST(all, "1 0 0 4", "1 2 4 8", 124,  "0 2 3 0");
  INNER_SIMP_TEST(one, "1 0 0 4", "1 2 4 9", 124,  "0 2 4 0");

  // Improvement due to 10 mapping to zero.
  OUTER_SIMP_TEST(all, "1 2 3 4", "1 10 3 4", 193,  "0 8 0 0");
  OUTER_SIMP_TEST(one, "1 2 3 4", "1 10 3 4", 193,  "0 8 0 0");

  // No improvement as 10 to zero does not get below bound.
  OUTER_SIMP_TEST(all, "2 2 3 4", "2 10 3 4", 293,  "0 0 0 0");
  OUTER_SIMP_TEST(one, "2 2 3 4", "2 10 3 4", 293,  "0 0 0 0");

  // Regressions, i.e. tests that capture past actual bugs.
  INNER_SIMP_TEST(one, "1 2 0 1", "1 2 10 1", 129, "0 0 4 0");
}

TEST(OptimizeStrategy, SimplifyNegativeGrading) {
  TermTranslator translator(4, 10);
  TermGrader grader(makeVector(-100, -10, -1, 0), translator);
  unique_ptr<SplitStrategy> splitStrategy = SplitStrategy::createStrategy("median");

  OptimizeStrategy all // Report all optimal solutions.
    (grader, splitStrategy.get(), true,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);
  OptimizeStrategy one // Report one optimal solution.
    (grader, splitStrategy.get(), false,
     OptimizeStrategy::UseBoundToEliminateAndSimplify);

  all.beginConsuming();
  all.consume(Term("1 2 3 4"));
  ASSERT_EQ(all.getMaximalValue(), mpz_class("-123"));

  one.beginConsuming();
  one.consume(Term("1 2 3 4"));
  ASSERT_EQ(one.getMaximalValue(), mpz_class("-123"));

  // No improvement.
  OUTER_SIMP_TEST(all, "1 2 3 4", "1 2 3 4", -123,  "0 0 0 0");
  OUTER_SIMP_TEST(one, "1 2 2 4", "1 2 2 4", -122,  "0 0 0 0");

  // Improvement depends on reporting one or all.
  OUTER_SIMP_TEST(all, "1 2 2 4", "1 2 3 5", -122,  "0 0 0 0");
  OUTER_SIMP_TEST(one, "1 2 2 4", "1 2 3 5", -122,  "0 0 1 0");

  // Improvement in more than one variable, only see the first.
  OUTER_SIMP_TEST(all, "1 2 1 4", "1 5 5 7", -121,  "0 1 0 0");
  OUTER_SIMP_TEST(one, "1 2 1 4", "1 5 5 7", -121,  "0 1 0 0");


  // Improvement due to 10 mapping to zero.
  INNER_SIMP_TEST(all, "1 5 3 4", "1 10 3 4", -103,  "0 5 0 0");
  INNER_SIMP_TEST(one, "1 5 2 4", "1 10 2 4", -103,  "0 5 0 0");

  // No improvement as 10 to zero is not necessary to get below bound.
  INNER_SIMP_TEST(all, "10 5 3 4", "10 10 3 4", -3,  "0 0 0 0");
  INNER_SIMP_TEST(one, "10 5 2 4", "10 10 2 4", -3,  "0 0 0 0");

  // No improvement due to zero both at 0 and 10.
  INNER_SIMP_TEST(all, "1 0 3 4", "1 10 3 4", -103,  "0 0 0 0");
  INNER_SIMP_TEST(one, "1 0 2 4", "1 10 2 4", -103,  "0 0 0 0");
}
