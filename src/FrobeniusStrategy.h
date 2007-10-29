#ifndef FROBENIUS_ACTION_GUARD
#define FROBENIUS_ACTION_GUARD

#include "Strategy.h"

class TermTree;
class TermTranslator;
class Partition;

class FrobeniusStrategy : public Strategy {
public:
  FrobeniusStrategy(const vector<mpz_class>& degrees,
		    mpz_class* frobeniusNumber,
		    unsigned int dimension,
		    const TermTranslator* translator,
		    bool useBound);

  virtual ~FrobeniusStrategy();

  virtual void getName(string& name) const;

  virtual bool consideringCall(const Term& b,
			       bool sameExponentAsNext,
			       const TermTree& tree);

  virtual bool startingCall(const Term& b,
			    const TermTree& tree,
			    bool startingPartition);

  virtual void endingCall(const Term& b,
			  const TermTree& tree);

  virtual void foundSolution(const Term& b,
			     bool startingPartition);

  virtual void startingPartitioning(const Term& b,
				    const Partition& partition,
				    const TermTree& tree);

  virtual void doingPartitionSet(int position,
				 const Term& b,
				 const Term& compressedB,
				 const Partition& partition,
				 vector<Exponent> compressor,
				 const TermTree& tree);

  virtual void doneDoingPartitionSet(int position,
				     const vector<Exponent>& compressor);

  virtual void endingPartitioning(int position,
				  const Term& b);

  void print(ostream& out);
  
private:
  bool canSkipDueToUpperBound(const TermTree& tree, const Degree& degree);

  void createDegreeMultiples(const vector<mpz_class>& degrees);

  bool _startingPartition;
  vector<Degree> _storedMaximumDegreeSeen;
  vector<Degree**> _storedDegreeMultiples;
  bool _onlyTwoDimensional;
  const TermTranslator* _translator;
  unsigned int _dimension;
  vector<Degree> _partialDegrees;
  Degree** _degreeMultiples;
  Degree _initialDegree;
  Degree _maximumDegreeSeen;
  mpz_class* _frobeniusNumber;
  bool _useBound;
};

#endif