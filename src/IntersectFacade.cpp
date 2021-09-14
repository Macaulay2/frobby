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
#include "stdinc.h"
#include "IntersectFacade.h"

#include "Term.h"
#include "BigIdeal.h"
#include "Ideal.h"
#include "intersect.h"
#include "Ideal.h"
#include "TermTranslator.h"
#include "ElementDeleter.h"

IntersectFacade::IntersectFacade(bool printActions):
  Facade(printActions) {
}

auto_ptr<BigIdeal> IntersectFacade::intersect(const vector<BigIdeal*>& ideals,
                                              const VarNames& emptyNames) {
  beginAction("Intersecting ideals.");

  if (ideals.empty()) {
    auto_ptr<BigIdeal> entireRing(new BigIdeal(emptyNames));
    entireRing->newLastTerm();
    return entireRing;
  }

  vector<Ideal*> ideals2;
  ElementDeleter<vector<Ideal*> > ideals2Deleter(ideals2);
  TermTranslator translator(ideals, ideals2);

  const VarNames& names = translator.getNames();
  size_t variableCount = names.getVarCount();

  auto_ptr<Ideal> intersection(new Ideal(variableCount));
  Term identity(variableCount);
  intersection->insert(identity);

  for (size_t i = 0; i < ideals2.size(); ++i) {
    ideals2[i]->minimize();

    // Compute intersection
    auto_ptr<Ideal> tmp(new Ideal(variableCount));
    ::intersect(tmp.get(), intersection.get(), ideals2[i]);

    // Handle bookkeeping
    intersection = std::move(tmp);
  }

  auto_ptr<BigIdeal> bigIdeal(new BigIdeal(names));
  bigIdeal->insert(*intersection, translator);

  endAction();
  return bigIdeal;
}
