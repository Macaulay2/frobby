cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(frobby VERSION 0.9.5 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(WITH_MPIR "Build with MPIR or not" off)

if(WITH_MPIR)
  find_package(MPIR 3.0.0 REQUIRED)
  set(GMP_LIBRARIES ${MPIR_LIBRARIES})
  set(GMP_INCLUDE_DIRS ${MPIR_INCLUDE_DIRS})
else()
  find_package(GMP 6.0.0 REQUIRED)
endif()

add_compile_options(
  -DPACKAGE_NAME="${PROJECT_NAME}"
  -DPACKAGE_TARNAME="${PROJECT_NAME}"
  -DPACKAGE_VERSION="${PROJECT_VERSION}"
  -DPACKAGE_STRING="${PROJECT_NAME} ${PROJECT_VERSION}"
  -DPACKAGE_BUGREPORT=""
  -DPACKAGE_URL=""
  -DPACKAGE="${PROJECT_NAME}"
  -DVERSION="${PROJECT_VERSION}"
  -Wno-uninitialized -Wno-unused-parameter
  )

add_library(frobby STATIC
  src/main.cpp
  src/display.cpp
  src/dynamicFrobeniusAlgorithm.cpp
  src/error.cpp
  src/fourti2.cpp
  src/fplllIO.cpp
  src/frobby.cpp
  src/intersect.cpp
  src/lattice.cpp
  src/randomDataGenerators.cpp
  src/stdinc.h

  src/Action.cpp
  src/ActionPrinter.cpp
  src/AlexanderDualAction.cpp
  src/AnalyzeAction.cpp
  src/Arena.cpp
  src/AssociatedPrimesAction.cpp
  src/BigIdeal.cpp
  src/BigIntVector.cpp
  src/BigPolynomial.cpp
  src/BigTermConsumer.cpp
  src/BigTermRecorder.cpp
  src/BigattiBaseCase.cpp
  src/BigattiFacade.cpp
  src/BigattiHilbertAlgorithm.cpp
  src/BigattiParams.cpp
  src/BigattiPivotStrategy.cpp
  src/BigattiState.cpp
  src/BoolParameter.cpp
  src/CanonicalCoefTermConsumer.cpp
  src/CanonicalTermConsumer.cpp
  src/CliParams.cpp
  src/CoCoA4IOHandler.cpp
  src/CoefBigTermConsumer.cpp
  src/CoefBigTermRecorder.cpp
  src/CoefTermConsumer.cpp
  src/ColumnPrinter.cpp
  src/CommonParams.cpp
  src/CommonParamsHelper.cpp
  src/CountingIOHandler.cpp
  src/DataType.cpp
  src/DebugAllocator.cpp
  src/DebugStrategy.cpp
  src/DecomRecorder.cpp
  src/Deformer.cpp
  src/DimensionAction.cpp
  src/DynamicFrobeniusAction.cpp
  src/DynamicFrobeniusFacade.cpp
  src/EulerAction.cpp
  src/EulerState.cpp
  src/Facade.cpp
  src/Fourti2IOHandler.cpp
  src/FrobbyStringStream.cpp
  src/FrobeniusAction.cpp
  src/GenerateDataFacade.cpp
  src/GenerateFrobeniusAction.cpp
  src/GenerateIdealAction.cpp
  src/HashPolynomial.cpp
  src/HelpAction.cpp
  src/HilbertAction.cpp
  src/HilbertBasecase.cpp
  src/HilbertIndependenceConsumer.cpp
  src/HilbertSlice.cpp
  src/HilbertStrategy.cpp
  src/IOFacade.cpp
  src/IOHandler.cpp
  src/IOHandlerCommon.cpp
  src/IOHandlerImpl.cpp
  src/IOParameters.cpp
  src/Ideal.cpp
  src/IdealComparator.cpp
  src/IdealConsolidator.cpp
  src/IdealFacade.cpp
  src/IdealFactory.cpp
  src/IdealOrderer.cpp
  src/IdealTree.cpp
  src/IdealWriter.cpp
  src/IndependenceSplitter.cpp
  src/InputConsumer.cpp
  src/IntegerParameter.cpp
  src/IntersectFacade.cpp
  src/IntersectionAction.cpp
  src/IrreducibleDecomAction.cpp
  src/IrreducibleIdealSplitter.cpp
  src/LatticeAlgs.cpp
  src/LatticeAnalyzeAction.cpp
  src/LatticeFacade.cpp
  src/LatticeFormatAction.cpp
  src/LocalArray.cpp
  src/Macaulay2IOHandler.cpp
  src/Matrix.cpp
  src/MaximalStandardAction.cpp
  src/Minimizer.cpp
  src/MonosIOHandler.cpp
  src/MsmSlice.cpp
  src/MsmStrategy.cpp
  src/NameFactory.cpp
  src/NewMonosIOHandler.cpp
  src/NullCoefTermConsumer.cpp
  src/NullIOHandler.cpp
  src/NullTermConsumer.cpp
  src/ObjectCache.cpp
  src/OptimizeAction.cpp
  src/OptimizeStrategy.cpp
  src/Parameter.cpp
  src/ParameterGroup.cpp
  src/Partition.cpp
  src/PivotEulerAlg.cpp
  src/PivotStrategy.cpp
  src/PolyTransformAction.cpp
  src/PolyWriter.cpp
  src/Polynomial.cpp
  src/PolynomialConsolidator.cpp
  src/PolynomialFacade.cpp
  src/PolynomialFactory.cpp
  src/PrimaryDecomAction.cpp
  src/Projection.cpp
  src/RawSquareFreeIdeal.cpp
  src/RawSquareFreeTerm.cpp
  src/SatBinomConsumer.cpp
  src/SatBinomIdeal.cpp
  src/SatBinomRecorder.cpp
  src/Scanner.cpp
  src/ScarfFacade.cpp
  src/ScarfHilbertAlgorithm.cpp
  src/ScarfParams.cpp
  src/SingularIOHandler.cpp
  src/SizeMaxIndepSetAlg.cpp
  src/Slice.cpp
  src/SliceFacade.cpp
  src/SliceLikeParams.cpp
  src/SliceParameters.cpp
  src/SliceParams.cpp
  src/SliceStrategy.cpp
  src/SliceStrategyCommon.cpp
  src/SplitStrategy.cpp
  src/SquareFreeIdeal.cpp
  src/StatisticsStrategy.cpp
  src/StringParameter.cpp
  src/Task.cpp
  src/TaskEngine.cpp
  src/Term.cpp
  src/TermConsumer.cpp
  src/TermExtra.cpp
  src/TermGrader.cpp
  src/TermPredicate.cpp
  src/TermTranslator.cpp
  src/TestAction.cpp
  src/Timer.cpp
  src/TotalDegreeCoefTermConsumer.cpp
  src/TransformAction.cpp
  src/TranslatingCoefTermConsumer.cpp
  src/TranslatingTermConsumer.cpp
  src/UniHashPolynomial.cpp
  src/VarNames.cpp
  src/VarSorter.cpp

  src/test/Test.cpp
  src/test/TestCase.cpp
  src/test/TestQualifier.cpp
  src/test/TestRunner.cpp
  src/test/TestSorter.cpp
  src/test/TestSuite.cpp
  src/test/TestVisitor.cpp
  src/test/asserts.cpp
  src/test/macroes.cpp
  )

target_link_libraries(frobby ${GMP_LIBRARIES})
target_include_directories(frobby PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${GMP_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:include>
  )

set_target_properties(frobby PROPERTIES PUBLIC_HEADER src/frobby.h)
install(TARGETS frobby LIBRARY DESTINATION lib)
install(FILES GPL-header COPYING DESTINATION licenses/frobby)

add_executable(frobby-executable src/main.cpp)
set_target_properties(frobby-executable PROPERTIES OUTPUT_NAME frobby)
target_link_libraries(frobby-executable frobby)
install(TARGETS frobby-executable)

include(CTest)
if(BUILD_TESTING)
  add_executable(frobby-tests
    src/ArenaTest.cpp
    src/IdealTest.cpp
    src/LibAlexanderDualTest.cpp
    src/LibAssociatedPrimesTest.cpp
    src/LibDimensionTest.cpp
    src/LibHilbertPoincareTest.cpp
    src/LibIrreducibleDecomTest.cpp
    src/LibMaxStdTest.cpp
    src/LibPrimaryDecomTest.cpp
    src/LibStdProgramTest.cpp
    src/LibTest.cpp
    src/MatrixTest.cpp
    src/OptimizeStrategyTest.cpp
    src/RawSquareFreeIdealTest.cpp
    src/RawSquareFreeTermTest.cpp
    src/TermGraderTest.cpp
    src/TermTest.cpp
    src/TermTranslatorTest.cpp
    )
  target_link_libraries(frobby-tests frobby)

  # TODO: see testing section in Makefile
  # add_test()
endif()
