bin_PROGRAMS = frobby
frobby_SOURCES =  main.cpp
frobby_LDADD = libfrobby.la

if DEBUG
AM_CPPFLAGS = -DDEBUG
frobby_SOURCES += LibAlexanderDualTest.cpp LibHilbertPoincareTest.cpp  \
  LibIrreducibleDecomTest.cpp LibMaxStdTest.cpp LibStdProgramTest.cpp  \
  LibTest.cpp OptimizeStrategyTest.cpp TermTest.cpp                     \
  TermTranslatorTest.cpp RawSquareFreeTermTest.cpp                      \
  RawSquareFreeIdealTest.cpp LibPrimaryDecomTest.cpp                    \
  LibAssociatedPrimesTest.cpp MatrixTest.cpp IdealTest.cpp              \
  LibDimensionTest.cpp TermGraderTest.cpp ArenaTest.cpp
endif

libfrobby_la_LDFLAGS = -version-info @LIBFROBBY_VERSION_INFO@

lib_LTLIBRARIES = libfrobby.la
include_HEADERS = frobby.h
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
