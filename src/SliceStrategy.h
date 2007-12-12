#ifndef SLICE_STRATEGY_GUARD
#define SLICE_STRATEGY_GUARD

#include <string>
#include "DecomConsumer.h"

class Slice;
class Term;
class TermTranslator;
class Projection;
class Ideal;

class SliceStrategy : public DecomConsumer {
 public:
  virtual ~SliceStrategy();

  // Methods for handling independence splits
  virtual void doingIndependenceSplit(const Slice& slice,
				      Ideal* mixedProjectionSubtract) = 0;
  virtual void doingIndependentPart(const Projection& projection,
				    bool last) = 0;
  virtual bool doneWithIndependentPart() = 0;
  virtual void doneWithIndependenceSplit() = 0;

  // Methods to inform debug strategies when the algorothm starts
  // processing the content of a slice and when it stops.
  virtual void startingContent(const Slice& slice);
  virtual void endingContent();

  // Methods for handling pivot and label splits

  enum SplitType {
    LabelSplit = 1,
    PivotSplit = 2
  };
  virtual SplitType getSplitType(const Slice& slice) = 0;

  virtual void getPivot(Term& pivot, const Slice& slice);
  virtual size_t getLabelSplitVariable(const Slice& slice);

  // report a msm to the strategy.
  virtual void consume(const Term& term) = 0;

  // Simplifies the slice prior to a split.
  virtual void simplify(Slice& slice);


  // Static methods to create strategies.

  static SliceStrategy* newDecomStrategy(const string& name,
					 DecomConsumer* consumer);

  static SliceStrategy* newFrobeniusStrategy(const string& name,
					     const vector<mpz_class>& instance,
					     const TermTranslator* translator,
					     mpz_class& frobeniusNumber);

  static SliceStrategy* addStatistics(SliceStrategy* strategy);
  static SliceStrategy* addDebugOutput(SliceStrategy* strategy);
};

#endif