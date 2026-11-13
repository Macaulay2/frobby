bin_PROGRAMS = frobby
frobby_SOURCES =  main.cpp
frobby_LDADD = libfrobby.la

unit_test_sources = LibAlexanderDualTest.cpp LibHilbertPoincareTest.cpp \
  LibIrreducibleDecomTest.cpp LibMaxStdTest.cpp LibStdProgramTest.cpp  \
  LibTest.cpp OptimizeStrategyTest.cpp TermTest.cpp                     \
  TermTranslatorTest.cpp RawSquareFreeTermTest.cpp                      \
  RawSquareFreeIdealTest.cpp LibPrimaryDecomTest.cpp                    \
  LibAssociatedPrimesTest.cpp MatrixTest.cpp IdealTest.cpp              \
  LibDimensionTest.cpp TermGraderTest.cpp ArenaTest.cpp

if DEBUG
AM_CPPFLAGS = -DDEBUG
frobby_SOURCES += $(unit_test_sources)
endif

check_PROGRAMS = frobby-unit-tests
frobby_unit_tests_SOURCES = test/testmain.cpp $(unit_test_sources)
frobby_unit_tests_LDADD = libfrobby.la
TESTS = frobby-unit-tests

EXTRA_PROGRAMS = frobby-installed-unit-tests
frobby_installed_unit_tests_SOURCES = $(frobby_unit_tests_SOURCES)
frobby_installed_unit_tests_LDADD = -L$(libdir) -lfrobby
frobby_installed_unit_tests_LDFLAGS = -Wl,-rpath,$(libdir)
frobby_installed_unit_tests_LINK = $(CXX) $(AM_CXXFLAGS) $(CXXFLAGS) \
  $(AM_LDFLAGS) $(frobby_installed_unit_tests_LDFLAGS) $(LDFLAGS) -o $@
CLEANFILES = $(EXTRA_PROGRAMS)

installcheck-local: frobby-installed-unit-tests$(EXEEXT)
	./frobby-installed-unit-tests$(EXEEXT)

libfrobby_la_LDFLAGS = -version-info @LIBFROBBY_VERSION_INFO@

lib_LTLIBRARIES = libfrobby.la
include_HEADERS = frobby.h
noinst_HEADERS = Action.h ActionPrinter.h AlexanderDualAction.h         \
  AnalyzeAction.h Arena.h AssociatedPrimesAction.h BigIdeal.h           \
  BigIntVector.h BigPolynomial.h BigTermConsumer.h BigTermRecorder.h    \
  BigattiBaseCase.h BigattiFacade.h BigattiHilbertAlgorithm.h           \
  BigattiParams.h BigattiPivotStrategy.h BigattiState.h                 \
  BoolParameter.h CanonicalCoefTermConsumer.h CanonicalTermConsumer.h   \
  CliParams.h CoCoA4IOHandler.h CoefBigTermConsumer.h                   \
  CoefBigTermRecorder.h CoefTermConsumer.h ColumnPrinter.h              \
  CommonParams.h CommonParamsHelper.h CountingIOHandler.h DataType.h    \
  DebugAllocator.h DebugStrategy.h DecomRecorder.h Deformer.h           \
  DimensionAction.h DynamicFrobeniusAction.h DynamicFrobeniusFacade.h   \
  ElementDeleter.h EulerAction.h EulerState.h Facade.h                  \
  Fourti2IOHandler.h FrobbyStringStream.h FrobeniusAction.h             \
  GenerateDataFacade.h GenerateFrobeniusAction.h GenerateIdealAction.h  \
  HashPolynomial.h HelpAction.h HilbertAction.h HilbertBasecase.h       \
  HilbertIndependenceConsumer.h HilbertSlice.h HilbertStrategy.h        \
  IOFacade.h IOHandler.h IOHandlerCommon.h IOHandlerImpl.h              \
  IOParameters.h Ideal.h IdealComparator.h IdealConsolidator.h          \
  IdealFacade.h IdealFactory.h IdealOrderer.h IdealTree.h               \
  IdealWriter.h IndependenceSplitter.h InputConsumer.h                  \
  IntegerParameter.h IntersectFacade.h IntersectionAction.h             \
  IrreducibleDecomAction.h IrreducibleIdealSplitter.h LatticeAlgs.h     \
  LatticeAnalyzeAction.h LatticeFacade.h LatticeFormatAction.h          \
  LibTest.h LocalArray.h Macaulay2IOHandler.h Matrix.h                  \
  MaximalStandardAction.h Minimizer.h MonosIOHandler.h MsmSlice.h       \
  MsmStrategy.h NameFactory.h NewMonosIOHandler.h                       \
  NullCoefTermConsumer.h NullIOHandler.h NullTermConsumer.h             \
  ObjectCache.h OptSliceParams.h OptimizeAction.h OptimizeStrategy.h    \
  Parameter.h ParameterGroup.h Partition.h PivotEulerAlg.h              \
  PivotStrategy.h PolyTransformAction.h PolyWriter.h Polynomial.h       \
  PolynomialConsolidator.h PolynomialFacade.h PolynomialFactory.h       \
  PrimaryDecomAction.h Projection.h RawSquareFreeIdeal.h                \
  RawSquareFreeTerm.h SatBinomConsumer.h SatBinomIdeal.h                \
  SatBinomRecorder.h Scanner.h ScarfFacade.h ScarfHilbertAlgorithm.h    \
  ScarfParams.h SingularIOHandler.h SizeMaxIndepSetAlg.h Slice.h        \
  SliceFacade.h SliceLikeParams.h SliceParameters.h SliceParams.h       \
  SliceStrategy.h SliceStrategyCommon.h SplitStrategy.h                 \
  SquareFreeIdeal.h StatisticsStrategy.h StringParameter.h Task.h       \
  TaskEngine.h Term.h TermConsumer.h TermExtra.h TermGrader.h           \
  TermIgnorer.h TermPredicate.h TermTranslator.h TestAction.h Timer.h   \
  TotalDegreeCoefTermConsumer.h TransformAction.h                       \
  TranslatingCoefTermConsumer.h TranslatingTermConsumer.h               \
  UniHashPolynomial.h VarNames.h VarSorter.h display.h                  \
  dynamicFrobeniusAlgorithm.h error.h fourti2.h fplllIO.h               \
  hash_map/hash_fun.h hash_map/hashtable.h intersect.h lattice.h        \
  main.h randomDataGenerators.h stdinc.h test/Test.h test/TestCase.h    \
  test/TestQualifier.h test/TestRunner.h test/TestSorter.h              \
  test/TestSuite.h test/TestVisitor.h test/all.h test/asserts.h         \
  test/macroes.h test/stdinc.h tests.h

EXTRA_DIST = doxygen.cpp
libfrobby_la_SOURCES = Action.cpp IOParameters.cpp                      \
  IrreducibleDecomAction.cpp fplllIO.cpp IOHandler.cpp fourti2.cpp      \
  randomDataGenerators.cpp MonosIOHandler.cpp BigIdeal.cpp              \
  TransformAction.cpp Macaulay2IOHandler.cpp NewMonosIOHandler.cpp      \
  HelpAction.cpp DynamicFrobeniusAction.cpp                             \
  dynamicFrobeniusAlgorithm.cpp GenerateIdealAction.cpp                 \
  GenerateFrobeniusAction.cpp intersect.cpp FrobeniusAction.cpp         \
  Facade.cpp IOFacade.cpp DynamicFrobeniusFacade.cpp                    \
  GenerateDataFacade.cpp AnalyzeAction.cpp IdealFacade.cpp              \
  Parameter.cpp ParameterGroup.cpp IntegerParameter.cpp                 \
  SliceParameters.cpp BoolParameter.cpp Scanner.cpp Partition.cpp       \
  StringParameter.cpp Term.cpp TermTranslator.cpp Timer.cpp             \
  VarNames.cpp LatticeFormatAction.cpp Ideal.cpp                        \
  IntersectionAction.cpp IntersectFacade.cpp                            \
  AssociatedPrimesAction.cpp MsmSlice.cpp IndependenceSplitter.cpp      \
  Projection.cpp MsmStrategy.cpp lattice.cpp LatticeFacade.cpp          \
  DecomRecorder.cpp TermGrader.cpp Fourti2IOHandler.cpp                 \
  NullIOHandler.cpp Minimizer.cpp AlexanderDualAction.cpp frobby.cpp    \
  BigTermConsumer.cpp TranslatingTermConsumer.cpp HilbertAction.cpp     \
  HilbertSlice.cpp Polynomial.cpp CanonicalCoefTermConsumer.cpp         \
  HilbertStrategy.cpp Slice.cpp SliceStrategyCommon.cpp                 \
  DebugStrategy.cpp OptimizeStrategy.cpp SliceFacade.cpp                \
  BigTermRecorder.cpp CoCoA4IOHandler.cpp SingularIOHandler.cpp         \
  TotalDegreeCoefTermConsumer.cpp BigPolynomial.cpp                     \
  CoefBigTermRecorder.cpp PolyTransformAction.cpp VarSorter.cpp         \
  TermConsumer.cpp NullTermConsumer.cpp CoefTermConsumer.cpp            \
  NullCoefTermConsumer.cpp TranslatingCoefTermConsumer.cpp              \
  PolynomialFacade.cpp HilbertBasecase.cpp                              \
  HilbertIndependenceConsumer.cpp SplitStrategy.cpp                     \
  CanonicalTermConsumer.cpp StatisticsStrategy.cpp TestAction.cpp       \
  NameFactory.cpp error.cpp DebugAllocator.cpp FrobbyStringStream.cpp   \
  SliceStrategy.cpp PrimaryDecomAction.cpp IdealComparator.cpp          \
  IrreducibleIdealSplitter.cpp DataType.cpp IdealConsolidator.cpp       \
  CoefBigTermConsumer.cpp PolynomialConsolidator.cpp                    \
  OptimizeAction.cpp MaximalStandardAction.cpp test/Test.cpp            \
  test/TestCase.cpp test/TestQualifier.cpp test/TestRunner.cpp          \
  test/TestSuite.cpp test/TestVisitor.cpp test/macroes.cpp              \
  test/asserts.cpp IdealFactory.cpp PolynomialFactory.cpp               \
  LatticeAnalyzeAction.cpp SatBinomIdeal.cpp SatBinomConsumer.cpp       \
  SatBinomRecorder.cpp CountingIOHandler.cpp test/TestSorter.cpp        \
  DimensionAction.cpp SizeMaxIndepSetAlg.cpp TaskEngine.cpp Task.cpp    \
  BigattiHilbertAlgorithm.cpp BigattiState.cpp ObjectCache.cpp          \
  HashPolynomial.cpp BigattiBaseCase.cpp BigattiPivotStrategy.cpp       \
  BigattiFacade.cpp UniHashPolynomial.cpp CommonParams.cpp              \
  CliParams.cpp SliceParams.cpp SliceLikeParams.cpp BigattiParams.cpp   \
  IOHandlerImpl.cpp IdealWriter.cpp PolyWriter.cpp IOHandlerCommon.cpp  \
  CommonParamsHelper.cpp ActionPrinter.cpp ScarfHilbertAlgorithm.cpp    \
  ScarfFacade.cpp Deformer.cpp IdealTree.cpp TermPredicate.cpp          \
  ScarfParams.cpp IdealOrderer.cpp TermExtra.cpp display.cpp            \
  Matrix.cpp BigIntVector.cpp ColumnPrinter.cpp EulerAction.cpp         \
  RawSquareFreeTerm.cpp RawSquareFreeIdeal.cpp PivotEulerAlg.cpp        \
  EulerState.cpp PivotStrategy.cpp Arena.cpp LocalArray.cpp             \
  LatticeAlgs.cpp InputConsumer.cpp SquareFreeIdeal.cpp
