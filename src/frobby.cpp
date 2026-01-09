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
#include "frobby.h"

#include "BigIdeal.h"
#include "SliceFacade.h"
#include "BigTermConsumer.h"
#include "TermTranslator.h"
#include "Term.h"
#include "error.h"
#include "CoefBigTermConsumer.h"
#include "IdealFacade.h"
#include "SliceParams.h"

/* also increment the version number in:
   - CMakeLists.txt
   - doc/doxygen.conf
   - doc/manual.tex
   - test/messages/help-noparam.err
   and update the library version as outlined in Makefile */
const char* const frobby_version = "0.9.7";

class ConsumerWrapper {
protected:
  ConsumerWrapper(size_t varCount):
    _varCount(varCount),
    _term(new mpz_ptr[varCount]) {
  }

  virtual ~ConsumerWrapper() {
    delete[] _term;
  }

  void setTerm(const Term& term, const TermTranslator& translator) {
    ASSERT(term.getVarCount() == _varCount);
    ASSERT(translator.getVarCount() == _varCount);

    for (size_t var = 0; var < _varCount; ++var)
      _term[var] = const_cast<mpz_ptr>
        (translator.getExponent(var, term).get_mpz_t());
  }

  void setTerm(const vector<mpz_class>& term) {
    ASSERT(term.size() == _varCount);

    for (size_t var = 0; var < _varCount; ++var)
      _term[var] = const_cast<mpz_ptr>(term[var].get_mpz_t());
  }

  size_t _varCount;
  mpz_ptr* _term;
};

class ExternalIdealConsumerWrapper : public BigTermConsumer,
                                     public ConsumerWrapper {
public:
  ExternalIdealConsumerWrapper(Frobby::IdealConsumer* consumer,
                               size_t varCount):
    ConsumerWrapper(varCount),
    _consumer(consumer) {
    ASSERT(_consumer != 0);
  }

  virtual void consumeRing(const VarNames& names) {
    ASSERT(names.getVarCount() == _varCount);
  }

  virtual void beginConsuming() {
    _consumer->idealBegin(_varCount);
  }

  virtual void consume(const Term& term, const TermTranslator& translator) {
    ASSERT(term.getVarCount() == _varCount);
    ASSERT(translator.getVarCount() == _varCount);

    setTerm(term, translator);
    _consumer->consume(_term);
  }

  virtual void consume(const vector<mpz_class>& term) {
    ASSERT(term.size() == _varCount);

    setTerm(term);
    _consumer->consume(_term);
  }

  virtual void doneConsuming() {
    _consumer->idealEnd();
  }

private:
  Frobby::IdealConsumer* _consumer;
};

class ExternalPolynomialConsumerWrapper : public CoefBigTermConsumer,
                                          public ConsumerWrapper {
public:
  ExternalPolynomialConsumerWrapper(Frobby::PolynomialConsumer* consumer,
                                    size_t varCount):
    ConsumerWrapper(varCount),
    _consumer(consumer),
    _varCount(varCount) {
    ASSERT(consumer != 0);
  }

  virtual void consumeRing(const VarNames& names) {
    ASSERT(names.getVarCount() == _varCount);
  }

  virtual void beginConsuming() {
    _consumer->polynomialBegin(_varCount);
  }

  virtual void consume(const mpz_class& coef,
                       const Term& term,
                       const TermTranslator& translator) {
    ASSERT(term.getVarCount() == _varCount);
    ASSERT(translator.getVarCount() == _varCount);

    setTerm(term, translator);
    _consumer->consume(coef.get_mpz_t(), _term);
  }

  virtual void consume(const mpz_class& coef,
                       const vector<mpz_class>& term) {
    ASSERT(term.size() == _varCount);

    for (size_t var = 0; var < _varCount; ++var)
      _term[var] = const_cast<mpz_ptr>(term[var].get_mpz_t());
    _consumer->consume(coef.get_mpz_t(), _term);
  }

  // TODO: make a note somewhere that in case of an exception,
  // polynomialEnd might not get called, this being because it is too
  // much of a burden to require it not to throw any
  // exceptions. Hmm... maybe there is an alternative solution.
  virtual void doneConsuming() {
    _consumer->polynomialEnd();
  }

private:
  Frobby::PolynomialConsumer* _consumer;
  size_t _varCount;
};

Frobby::IdealConsumer::~IdealConsumer() {
}

void Frobby::IdealConsumer::idealBegin(size_t varCount) {
}

void Frobby::IdealConsumer::idealEnd() {
}

Frobby::PolynomialConsumer::~PolynomialConsumer() {
}

void Frobby::PolynomialConsumer::polynomialBegin(size_t varCount) {
}

void Frobby::PolynomialConsumer::polynomialEnd() {
}

namespace FrobbyImpl {
  using ::BigIdeal;

  class FrobbyIdealHelper {
  public:
    FrobbyIdealHelper(size_t variableCount):
      _ideal(VarNames(variableCount)),
      _atVariable(variableCount) {
    }

    static const BigIdeal& getIdeal(const Frobby::Ideal& ideal) {
      return ideal._data->_ideal;
    }

  private:
    friend class Frobby::Ideal;

    BigIdeal _ideal;
    size_t _atVariable;
  };
}

Frobby::Ideal::Ideal(size_t variableCount) {
  _data = new FrobbyImpl::FrobbyIdealHelper(variableCount);
}

Frobby::Ideal::Ideal(const Ideal& ideal) {
  _data = new FrobbyImpl::FrobbyIdealHelper(*ideal._data);
}

Frobby::Ideal::~Ideal() {
  delete _data;
}

Frobby::Ideal& Frobby::Ideal::operator=(const Ideal& ideal) {
  // Allocate new object before deleting old object to leave *this
  // in a valid state in case of new throwing an exception.
  FrobbyImpl::FrobbyIdealHelper* newValue =
    new FrobbyImpl::FrobbyIdealHelper(*ideal._data);

  delete _data;
  _data = newValue;

  return *this;
}

void Frobby::Ideal::addExponent(const mpz_t exponent) {
  ASSERT(_data->_atVariable <= _data->_ideal.getVarCount());

  if (_data->_atVariable == _data->_ideal.getVarCount()) {
    _data->_ideal.newLastTerm();
    _data->_atVariable = 0;
    if (_data->_ideal.getVarCount() == 0)
      return;
  }

  mpz_class& ref = _data->_ideal.getLastTermExponentRef(_data->_atVariable);
  mpz_set(ref.get_mpz_t(), exponent);
  ++_data->_atVariable;
}

void Frobby::Ideal::addExponent(int exponent) {
  mpz_class tmp(exponent);
  addExponent(tmp.get_mpz_t());
}

void Frobby::Ideal::addExponent(unsigned int exponent) {
  mpz_class tmp(exponent);
  addExponent(tmp.get_mpz_t());
}

bool Frobby::alexanderDual(const Ideal& ideal,
                           const mpz_t* reflectionMonomial,
                           IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  ExternalIdealConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());

  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  if (reflectionMonomial == 0)
    facade.computeAlexanderDual();
  else {
    vector<mpz_class> point;
    point.resize(bigIdeal.getVarCount());
    for (size_t var = 0; var < bigIdeal.getVarCount(); ++var)
      mpz_set(point[var].get_mpz_t(), reflectionMonomial[var]);

    // We guarantee not to retain a reference to reflectionMonomial
    // when providing terms to the consumer.
    reflectionMonomial = 0;

    try {
      facade.computeAlexanderDual(point);
    } catch (const FrobbyException&) {
      return false;
    }
  }

  return true;
}

bool Frobby::alexanderDual(const Ideal& ideal,
                           const Ideal& reflectionMonomial,
                           IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);
  const BigIdeal& reflectionIdeal =
    FrobbyImpl::FrobbyIdealHelper::getIdeal(reflectionMonomial);

  if (reflectionIdeal.getGeneratorCount() != 1)
    return false;
  if (reflectionIdeal.getVarCount() != bigIdeal.getVarCount())
    return false;

  const vector<mpz_class>& monomial = reflectionIdeal.getTerm(0);
  const mpz_t* monomialPtr = 0;
  if (reflectionIdeal.getVarCount() > 0)
    monomialPtr = (const mpz_t*)&(monomial[0]);

  return alexanderDual(ideal, monomialPtr, consumer);
}

void Frobby::multigradedHilbertPoincareSeries(const Ideal& ideal,
                                              PolynomialConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  ExternalPolynomialConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computeMultigradedHilbertSeries();
}

void Frobby::univariateHilbertPoincareSeries(const Ideal& ideal,
                                             PolynomialConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  ExternalPolynomialConsumerWrapper wrappedConsumer(&consumer, 1);
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computeUnivariateHilbertSeries();
}

/**
 @todo: This seems redundant with IdealSplitter. Investigate.
*/
class IrreducibleIdealDecoder : public Frobby::IdealConsumer {
public:
  IrreducibleIdealDecoder(IdealConsumer* consumer):
    _varCount(0),
    _consumer(consumer),
    _term(0) {
    ASSERT(_consumer != 0);
  }

  ~IrreducibleIdealDecoder() {
  }

  virtual void idealBegin(size_t varCount) {
    _varCount = varCount;
    _term.resize(varCount);
    for (size_t var = 0; var < _varCount; ++var)
      _term[var] = _zero.get_mpz_t();
  }

  virtual void idealEnd() {
    _term.clear();
  }

  virtual void consume(mpz_ptr* exponentVector) {
    _consumer->idealBegin(_varCount);

    bool isIdentity = true;
    for (size_t var = 0; var < _varCount; ++var) {
      if (mpz_cmp_ui(exponentVector[var], 0) != 0) {
        isIdentity = false;
        _term[var] = exponentVector[var];
        _consumer->consume(&*(_term.begin()));
        _term[var] = _zero.get_mpz_t();
      }
    }

    _consumer->idealEnd();
  }

private:
  size_t _varCount;
  IdealConsumer* _consumer;
  vector<mpz_ptr> _term;
  mpz_class _zero;
};

void Frobby::irreducibleDecompositionAsIdeals(const Ideal& ideal,
                                              IdealConsumer& consumer) {
  IrreducibleIdealDecoder wrappedConsumer(&consumer);
  if (!irreducibleDecompositionAsMonomials(ideal, wrappedConsumer)) {
    const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);
    consumer.idealBegin(bigIdeal.getVarCount());
    consumer.idealEnd();
  }
}

bool Frobby::irreducibleDecompositionAsMonomials(const Ideal& ideal,
                                                 IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);
  if (bigIdeal.getGeneratorCount() == 0)
    return false;

  ExternalIdealConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computeIrreducibleDecomposition(true);
  return true;
}

void Frobby::primaryDecomposition(const Ideal& ideal,
								  IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  ExternalIdealConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computePrimaryDecomposition();
}

void Frobby::maximalStandardMonomials(const Ideal& ideal,
                                      IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  ExternalIdealConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computeMaximalStandardMonomials();
}

bool Frobby::solveStandardMonomialProgram(const Ideal& ideal,
                                          const mpz_t* l,
                                          IdealConsumer& consumer) {
  ASSERT(l != 0);

  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  vector<mpz_class> grading;
  for (size_t var = 0; var < bigIdeal.getVarCount(); ++var)
    grading.push_back(mpz_class(l[var]));

  ExternalIdealConsumerWrapper wrappedConsumer
    (&consumer, bigIdeal.getVarCount());
  SliceParams params;
  params.useIndependenceSplits(false); // not supported
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  mpz_class dummy;
  return facade.solveStandardProgram(grading, dummy, false);
}

void Frobby::codimension(const Ideal& ideal, mpz_t codim) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);
  dimension(ideal, codim);
  mpz_ui_sub(codim, bigIdeal.getVarCount(), codim);
}

void Frobby::dimension(const Ideal& ideal, mpz_t dim) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);

  IdealFacade facade(false);
  mpz_class dimen = facade.computeDimension(bigIdeal, false);
  mpz_set(dim, dimen.get_mpz_t());
}

void Frobby::associatedPrimes(const Ideal& ideal, IdealConsumer& consumer) {
  const BigIdeal& bigIdeal = FrobbyImpl::FrobbyIdealHelper::getIdeal(ideal);
  IrreducibleIdealDecoder decodingConsumer(&consumer);

  ExternalIdealConsumerWrapper wrappedConsumer
    (&decodingConsumer, bigIdeal.getVarCount());
  SliceParams params;
  SliceFacade facade(params, bigIdeal, wrappedConsumer);

  facade.computeAssociatedPrimes();
}
