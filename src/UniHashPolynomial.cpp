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
#include "stdinc.h"
#include "UniHashPolynomial.h"

#include "CoefBigTermConsumer.h"
#include "VarNames.h"
#include <algorithm>

void UniHashPolynomial::add(bool plus, const mpz_class& exponent) {
  mpz_class& ref = _terms[exponent];
  if (plus)
    ++ref;
  else
    --ref;
  if (ref == 0)
    _terms.erase(exponent);
}

void UniHashPolynomial::add(int coef, size_t exponent) {
  if (coef == 0)
    return;
  mpz_class& ref = _terms[exponent];
  ref += coef;
  if (ref == 0)
    _terms.erase(exponent);
}

void UniHashPolynomial::add(const mpz_class& coef, const mpz_class& exponent) {
  if (coef == 0)
    return;
  mpz_class& ref = _terms[exponent];
  ref += coef;
  if (ref == 0)
    _terms.erase(exponent);
}

namespace {
  /** Helper class for feedTo. */
  class RefCompare {
  public:
    typedef unordered_map<mpz_class, mpz_class, MpzHash> TermMap;
    bool operator()(TermMap::const_iterator a, TermMap::const_iterator b) {
      return a->first > b->first;
    }
  };
}

void UniHashPolynomial::feedTo(CoefBigTermConsumer& consumer, bool inCanonicalOrder) const {
  VarNames names;
  names.addVar("t");
  consumer.consumeRing(names);
  vector<mpz_class> term(1);

  consumer.beginConsuming();

  if (!inCanonicalOrder) {
    // Output the terms in whatever order _terms is storing them.
    TermMap::const_iterator termsEnd = _terms.end();
    TermMap::const_iterator it = _terms.begin();
    for (; it != termsEnd; ++it) {
      ASSERT(it->second != 0);
      term[0] = it->first;
      consumer.consume(it->second, term);
    }
  } else {

    // Fill refs with references in order to sort them. We can't sort
    // _terms since unordered_map doesn't support that, so we have to sort
    // references into _terms instead.
    vector<TermMap::const_iterator> refs;
    refs.reserve(_terms.size());

    TermMap::const_iterator termsEnd = _terms.end();
    for (TermMap::const_iterator it = _terms.begin();
         it != termsEnd; ++it)
      refs.push_back(it);

    sort(refs.begin(), refs.end(), RefCompare());

    // Output the terms in the sorted order specified by refs.
    vector<TermMap::const_iterator>::const_iterator refsEnd = refs.end();
    vector<TermMap::const_iterator>::const_iterator refIt = refs.begin();
    for (; refIt != refsEnd; ++refIt) {
      TermMap::const_iterator it = *refIt;
      ASSERT(it->second != 0);
      term[0] = it->first;
      consumer.consume(it->second, term);
    }
  }

  consumer.doneConsuming();
}

size_t UniHashPolynomial::getTermCount() const {
  return _terms.size();
}
