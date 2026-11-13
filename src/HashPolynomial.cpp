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
#include "HashPolynomial.h"

#include "CoefBigTermConsumer.h"
#include "TermTranslator.h"
#include "TermPredicate.h"
#include <vector>
#include <algorithm>

HashPolynomial::HashPolynomial(size_t varCount):
  _varCount(varCount) {
}

void HashPolynomial::clearAndSetVarCount(size_t varCount) {
  _terms.clear();
  _varCount = varCount;
}

void HashPolynomial::add(const mpz_class& coef, const Term& term) {
  ASSERT(_varCount == term.getVarCount());

  if (coef == 0)
    return;

  // Doing it this way incurs the penalty of looking up term twice if
  // ref ends up zero. I don't know how to avoid two look-ups in all
  // cases, especially when the interface of _terms is not fixed,
  // e.g. lowerbound doesn't exist for GCC's hash_map, so we can't use
  // that.
  mpz_class& ref = _terms[term];
  ref += coef;
  if (ref == 0)
    _terms.erase(term);
}

void HashPolynomial::add(bool plus, const Term& term) {
  ASSERT(_varCount == term.getVarCount());

  mpz_class& ref = _terms[term];
  if (plus)
    ++ref;
  else
    --ref;
  if (ref == 0)
    _terms.erase(term);
}

namespace {
  /** Helper class for feedTo. */
  class RefCompare {
  public:
    typedef unordered_map<Term, mpz_class, TermHash> TermMap;
    bool operator()(TermMap::const_iterator a, TermMap::const_iterator b) {
      return lexCompare(a->first, b->first) > 0;
    }
  };
}

void HashPolynomial::feedTo
(const TermTranslator& translator,
 CoefBigTermConsumer& consumer,
 bool inCanonicalOrder) const {

  consumer.consumeRing(translator.getNames());
  consumer.beginConsuming();

  if (!inCanonicalOrder) {
    // Output the terms in whatever order _terms is storing them.
    TermMap::const_iterator termsEnd = _terms.end();
    TermMap::const_iterator it = _terms.begin();
    for (; it != termsEnd; ++it)
      consumer.consume(it->second, it->first, translator);
  } else {

    // Fill refs with references to the terms in order to sort
    // them. We can't sort _terms since unordered_map doesn't support that,
    // so we have to sort references into _terms instead.
    vector<TermMap::const_iterator> refs;
    refs.reserve(_terms.size());

    TermMap::const_iterator termsEnd = _terms.end();
    TermMap::const_iterator it = _terms.begin();
    for (; it != termsEnd; ++it)
      refs.push_back(it);

    // Sort the references.
    sort(refs.begin(), refs.end(), RefCompare());

    // Output the terms in the sorted order specified by refs.

    vector<TermMap::const_iterator>::const_iterator refsEnd = refs.end();
    vector<TermMap::const_iterator>::const_iterator refIt = refs.begin();
    for (; refIt != refsEnd; ++refIt)
      consumer.consume((*refIt)->second, (*refIt)->first, translator);
  }

  consumer.doneConsuming();
}

size_t HashPolynomial::getTermCount() const {
  return _terms.size();
}
