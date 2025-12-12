/* Frobby: Software for monomial ideal computations.
   Copyright (C) 2010 University of Aarhus
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
#include "IdealOrderer.h"

#include "Ideal.h"
#include "TermPredicate.h"
#include "IdealOrderer.h"
#include "NameFactory.h"
#include "TermExtra.h"
#include "ElementDeleter.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <map>

IdealOrderer::~IdealOrderer() {
}

namespace {
  template<class Pred>
  class IdealSorter : public IdealOrderer {
  public:
    static const char* staticGetName() {
      return Pred::staticGetName();
    }

  private:
    virtual void doOrder(Ideal& ideal) const {
      Pred pred;
      pred.setVarCount(ideal.getVarCount());
      stable_sort(ideal.begin(), ideal.end(), pred);
    }
  };

  class RandomOrderer : public IdealOrderer {
  public:
    static const char* staticGetName() {return "random";}
  private:
    struct URBG {
        using result_type = int;
        static constexpr int min() { return 0; }
        static constexpr int max() { return RAND_MAX; }
        int operator()() const { return std::rand(); }
    };
    void doOrder(Ideal& ideal) const {
      URBG g;
      std::shuffle(ideal.begin(), ideal.end(), g);
    }
  };

  class TotalDegreeComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "tdeg";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      totalDegree(_degA, a, getVarCount());
      totalDegree(_degB, b, getVarCount());
      return _degA < _degB;
    }
    mutable mpz_class _degA; // member to avoid repeated allocation
    mutable mpz_class _degB;
  };

  class MedianComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "median";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      return median(a, getVarCount()) < median(b, getVarCount());
    }
  };

  class MedianPositiveComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "posMedian";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      return medianPositive(a, getVarCount()) <
        medianPositive(b, getVarCount());
    }
  };

  class MinimumPositiveComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "minPos";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      return minimumPositive(a, getVarCount()) <
        minimumPositive(b, getVarCount());
    }
  };

  class MaximumComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "max";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      return maximum(a, getVarCount()) < maximum(b, getVarCount());
    }
  };

  class SupportComparator : public TermPredicate {
  public:
    static const char* staticGetName() {return "support";}
  private:
    virtual bool doPredicate(const Exponent* a,
                             const Exponent* b) const {
      return Term::getSizeOfSupport(a, getVarCount()) <
        Term::getSizeOfSupport(b, getVarCount());
    }
  };

  class StrongGenericityOrderer : public IdealOrderer {
  public:
    static const char* staticGetName() {return "strongGenericity";}

  protected:
    void orderGenericity(Ideal& ideal, bool strong) const {
      // Using a map here is the prettier of several ugly
      // alternaties. Only trade more ugly for efficiency if this
      // method turns up as a significant consumer of time in a
      // profiler.
      UnGenMap degeneracy;

      // Make degeneracy[gen] be the number of other generators that
      // shares a positive exponent with gen.
      Term tmp(ideal.getVarCount());
      for (cit a = ideal.begin(); a != ideal.end(); ++a) {
        for (cit b = a + 1; b != ideal.end(); ++b) {
          if (Term::sharesNonZeroExponent(*a, *b, ideal.getVarCount())) {
            if (!strong) {
              tmp.lcm(*a, *b);
              if (ideal.strictlyContains(tmp))
                continue;
            }
            ++degeneracy[*a];
            ++degeneracy[*b];
          }
        }
      }

      Pred pred(degeneracy);
      stable_sort(ideal.begin(), ideal.end(), pred);
    }

  private:
    typedef Ideal::const_iterator cit;
    typedef map<const Exponent*, size_t> UnGenMap;

    virtual void doOrder(Ideal& ideal) const {
      orderGenericity(ideal, true);
    }

    class Pred {
    public:
      Pred(UnGenMap& degeneracy): _degeneracy(degeneracy) {}

      bool operator()(const Exponent* a, const Exponent* b) const {
        return _degeneracy[a] < _degeneracy[b];
      }

    private:
      UnGenMap& _degeneracy;
    };
  };

  class NullOrderer : public IdealOrderer {
  public:
    static const char* staticGetName() {return "null";}
  private:
    virtual void doOrder(Ideal& ideal) const {}
  };

  /** @todo: do this more elegantly than with inheritance. */
  class WeakGenericityOrderer : public StrongGenericityOrderer {
  public:
    static const char* staticGetName() {return "weakGenericity";}
  private:
    virtual void doOrder(Ideal& ideal) const {
      orderGenericity(ideal, false);
    }
  };

  /** Sorts in the reverse order of the orderer passed to the
   constructor. */
  class ReverseOrderer : public IdealOrderer {
  public:
    ReverseOrderer(unique_ptr<IdealOrderer> orderer): _orderer(std::move(orderer)) {}

  private:
    virtual void doOrder(Ideal& ideal) const {
      // Could probably be done more efficiently by trying to interact
      // with the orderer, but that would be so much more trouble. The
      // first reverse is necessary to ensure the ordering is stable.
      reverse(ideal.begin(), ideal.end());
      _orderer->order(ideal);
      reverse(ideal.begin(), ideal.end());
    }
    unique_ptr<IdealOrderer> _orderer;
  };

  class CompositeOrderer : public IdealOrderer {
  public:
    CompositeOrderer(): _orderersDeleter(_orderers) {}

    void refineOrderingWith(unique_ptr<IdealOrderer> orderer) {
      exceptionSafePushBack(_orderers, std::move(orderer));
    }

  private:
    typedef vector<IdealOrderer*> Container;
    typedef Container::const_reverse_iterator rev_cit;

    virtual void doOrder(Ideal& ideal) const {
      // This works because orderes that define a non-total order
      // (i.e. those that can be interestingly refined) use a stable
      // sorting algorithm.
      rev_cit rbegin(_orderers.end());
      rev_cit rend(_orderers.begin());
      for (rev_cit it = rbegin; it != rend; ++it)
        (*it)->order(ideal);
    }

    Container _orderers;
    ElementDeleter<Container> _orderersDeleter;
  };

  typedef NameFactory<IdealOrderer> OrdererFactory;
  OrdererFactory getOrdererFactory() {
    OrdererFactory factory("ordering of terms");

    nameFactoryRegister<RandomOrderer>(factory);
    nameFactoryRegister<NullOrderer>(factory);
    nameFactoryRegister<IdealSorter<LexComparator> >(factory);
    nameFactoryRegister<IdealSorter<ReverseLexComparator> >(factory);
    nameFactoryRegister<IdealSorter<TotalDegreeComparator> >(factory);
    nameFactoryRegister<IdealSorter<MedianComparator> >(factory);
    nameFactoryRegister<IdealSorter<MedianPositiveComparator> >(factory);
    nameFactoryRegister<IdealSorter<MinimumPositiveComparator> >(factory);
    nameFactoryRegister<IdealSorter<MaximumComparator> >(factory);
    nameFactoryRegister<IdealSorter<SupportComparator> >(factory);
    nameFactoryRegister<StrongGenericityOrderer>(factory);
    nameFactoryRegister<WeakGenericityOrderer>(factory);

    return factory;
  }

  unique_ptr<IdealOrderer> createNonCompositeOrderer(const string& prefix) {
    if (prefix.substr(0, 3) == "rev") {
      unique_ptr<IdealOrderer> orderer =
        createWithPrefix(getOrdererFactory(), prefix.substr(3));
      return unique_ptr<IdealOrderer>(new ReverseOrderer(std::move(orderer)));
    } else
      return createWithPrefix(getOrdererFactory(), prefix);
  }
}

unique_ptr<IdealOrderer> createIdealOrderer(const string& prefix) {
  if (prefix.find('_') == string::npos)
    return createNonCompositeOrderer(prefix);

  unique_ptr<CompositeOrderer> composite(new CompositeOrderer());
  size_t pos = 0;
  while (true) {
    size_t nextUnderscore = prefix.find('_', pos);
    string subPrefix = prefix.substr(pos, nextUnderscore - pos);
    composite->refineOrderingWith(createNonCompositeOrderer(subPrefix));

    if (nextUnderscore == string::npos)
      break;
    pos = nextUnderscore + 1;
  }
  return unique_ptr<IdealOrderer>(composite.release());
}
