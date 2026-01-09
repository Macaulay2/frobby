# ***** Variables


# ========================
# >>>>>  GMP set-up  <<<<<
# ========================
# For GMP set **both**  GMP_INC_DIR  **and**  ldflags_for_gmp
# If you have a system installation of GMP with C++ support use next 2 lines:
# GMP_INC_DIR=
# ldflags_for_gmp=-lgmpxx -lgmp
GMP_INC_DIR=/usr/local/include
ldflags_for_gmp=-L/usr/local/lib -lgmpxx -lgmp

# shared library version
# bump if interfaces added, removed, or changed
FROBBY_CURRENT = 1
# bump at every release, set to 0 if any interfaces added, removed, or changed
FROBBY_REVISION = 2
# bump if interfaces added, set to 0 if removed or changed
FROBBY_AGE = 0
FROBBY_SOVERSION := $(shell expr $(FROBBY_CURRENT) - $(FROBBY_AGE))
FROBBY_VERSION := $(FROBBY_SOVERSION).$(FROBBY_AGE).$(FROBBY_REVISION)

rawSources := main.cpp Action.cpp IOParameters.cpp						\
  IrreducibleDecomAction.cpp fplllIO.cpp IOHandler.cpp fourti2.cpp		\
  randomDataGenerators.cpp MonosIOHandler.cpp BigIdeal.cpp				\
  TransformAction.cpp Macaulay2IOHandler.cpp NewMonosIOHandler.cpp		\
  HelpAction.cpp DynamicFrobeniusAction.cpp					\
  dynamicFrobeniusAlgorithm.cpp GenerateIdealAction.cpp					\
  GenerateFrobeniusAction.cpp intersect.cpp FrobeniusAction.cpp			\
  Facade.cpp IOFacade.cpp DynamicFrobeniusFacade.cpp					\
  GenerateDataFacade.cpp AnalyzeAction.cpp IdealFacade.cpp				\
  Parameter.cpp ParameterGroup.cpp IntegerParameter.cpp					\
  SliceParameters.cpp BoolParameter.cpp Scanner.cpp Partition.cpp		\
  StringParameter.cpp Term.cpp TermTranslator.cpp Timer.cpp				\
  VarNames.cpp LatticeFormatAction.cpp Ideal.cpp						\
  IntersectionAction.cpp IntersectFacade.cpp							\
  AssociatedPrimesAction.cpp MsmSlice.cpp IndependenceSplitter.cpp		\
  Projection.cpp MsmStrategy.cpp lattice.cpp LatticeFacade.cpp			\
  DecomRecorder.cpp TermGrader.cpp Fourti2IOHandler.cpp					\
  NullIOHandler.cpp Minimizer.cpp AlexanderDualAction.cpp frobby.cpp	\
  BigTermConsumer.cpp TranslatingTermConsumer.cpp HilbertAction.cpp		\
  HilbertSlice.cpp Polynomial.cpp CanonicalCoefTermConsumer.cpp			\
  HilbertStrategy.cpp Slice.cpp SliceStrategyCommon.cpp					\
  DebugStrategy.cpp OptimizeStrategy.cpp SliceFacade.cpp				\
  BigTermRecorder.cpp CoCoA4IOHandler.cpp SingularIOHandler.cpp			\
  TotalDegreeCoefTermConsumer.cpp BigPolynomial.cpp						\
  CoefBigTermRecorder.cpp PolyTransformAction.cpp VarSorter.cpp			\
  TermConsumer.cpp NullTermConsumer.cpp CoefTermConsumer.cpp			\
  NullCoefTermConsumer.cpp TranslatingCoefTermConsumer.cpp				\
  PolynomialFacade.cpp HilbertBasecase.cpp								\
  HilbertIndependenceConsumer.cpp SplitStrategy.cpp						\
  CanonicalTermConsumer.cpp StatisticsStrategy.cpp TestAction.cpp		\
  NameFactory.cpp error.cpp DebugAllocator.cpp FrobbyStringStream.cpp	\
  SliceStrategy.cpp PrimaryDecomAction.cpp IdealComparator.cpp			\
  IrreducibleIdealSplitter.cpp DataType.cpp IdealConsolidator.cpp		\
  CoefBigTermConsumer.cpp PolynomialConsolidator.cpp					\
  OptimizeAction.cpp MaximalStandardAction.cpp test/Test.cpp			\
  test/TestCase.cpp test/TestQualifier.cpp test/TestRunner.cpp			\
  test/TestSuite.cpp test/TestVisitor.cpp test/macroes.cpp				\
  test/asserts.cpp IdealFactory.cpp PolynomialFactory.cpp				\
  LatticeAnalyzeAction.cpp SatBinomIdeal.cpp SatBinomConsumer.cpp		\
  SatBinomRecorder.cpp CountingIOHandler.cpp test/TestSorter.cpp		\
  DimensionAction.cpp SizeMaxIndepSetAlg.cpp TaskEngine.cpp Task.cpp	\
  BigattiHilbertAlgorithm.cpp BigattiState.cpp ObjectCache.cpp			\
  HashPolynomial.cpp BigattiBaseCase.cpp BigattiPivotStrategy.cpp		\
  BigattiFacade.cpp UniHashPolynomial.cpp CommonParams.cpp				\
  CliParams.cpp SliceParams.cpp SliceLikeParams.cpp BigattiParams.cpp	\
  IOHandlerImpl.cpp IdealWriter.cpp PolyWriter.cpp IOHandlerCommon.cpp	\
  CommonParamsHelper.cpp ActionPrinter.cpp ScarfHilbertAlgorithm.cpp	\
  ScarfFacade.cpp Deformer.cpp IdealTree.cpp TermPredicate.cpp			\
  ScarfParams.cpp IdealOrderer.cpp TermExtra.cpp display.cpp			\
  Matrix.cpp BigIntVector.cpp ColumnPrinter.cpp EulerAction.cpp			\
  RawSquareFreeTerm.cpp RawSquareFreeIdeal.cpp PivotEulerAlg.cpp		\
  EulerState.cpp PivotStrategy.cpp Arena.cpp LocalArray.cpp				\
  LatticeAlgs.cpp InputConsumer.cpp SquareFreeIdeal.cpp

rawTests := LibAlexanderDualTest.cpp LibHilbertPoincareTest.cpp			\
  LibIrreducibleDecomTest.cpp LibMaxStdTest.cpp LibStdProgramTest.cpp	\
  LibTest.cpp OptimizeStrategyTest.cpp TermTest.cpp						\
  TermTranslatorTest.cpp RawSquareFreeTermTest.cpp						\
  RawSquareFreeIdealTest.cpp LibPrimaryDecomTest.cpp					\
  LibAssociatedPrimesTest.cpp MatrixTest.cpp IdealTest.cpp				\
  LibDimensionTest.cpp TermGraderTest.cpp ArenaTest.cpp

ifndef CXX
  CXX      = "g++"
endif
ifndef STRIP
  STRIP    = strip
endif

cxxflags = $(CXXFLAGS) $(CPPFLAGS) -I $(GMP_INC_DIR) -Wno-uninitialized -Wno-unused-parameter
program = frobby
library = libfrobby.a
benchArgs = $(FROBBYARGS)

ifndef MODE
 MODE=release
endif

ifndef ldflags
  ldflags = $(LDFLAGS) -lgmpxx -lgmp
endif

MATCH=false
ifeq ($(MODE), release)
  outdir = bin/release/
  cxxflags += -O2
  MATCH=true
endif
ifeq ($(MODE), debug)
  rawSources := $(rawSources) $(rawTests)
  outdir = bin/debug/
  cxxflags += -g -D DEBUG -fno-inline -Werror -Wextra -Wno-uninitialized \
            -Wno-unused-parameter
  MATCH=true
endif
ifeq ($(MODE), shared)
  outdir = bin/shared/
  cxxflags += -O2 -fPIC
  library = libfrobby.so
  MATCH=true
endif
ifeq ($(MODE), profile)
  outdir = bin/profile/
  cxxflags += -g -pg -O2 -D PROFILE
  ldflags += -pg
  MATCH=true
  benchArgs = _profile $(FROBBYARGS)
endif
ifeq ($(MODE), analysis)
  rawSources := $(rawSources) $(rawTests)
  outdir = bin/analysis/
  cxxflags += -Wextra -fsyntax-only -O1 -Wfloat-equal -Wundef			\
      -Wno-endif-labels -Wshadow -Wlarger-than-1000 -Wpointer-arith		\
      -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wsign-compare	\
      -Waggregate-return -Wmissing-noreturn -Wmissing-format-attribute		\
      -Wno-multichar -Wno-deprecated-declarations -Wpacked			\
      -Wno-redundant-decls -Wunreachable-code -Winline				\
      -Wno-invalid-offsetof -Winvalid-pch -Wlong-long				\
      -Wdisabled-optimization -D DEBUG -Werror
  MATCH=true
endif

ifeq ($(MATCH), false)
  $(error Unknown value of MODE: "$(MODE)")
endif

sources = $(patsubst %.cpp, src/%.cpp, $(rawSources))
objs    = $(patsubst %.cpp, $(outdir)%.o, $(rawSources))

RANLIB = ranlib

# ***** Compilation

.PHONY: all depend clean bin/$(program) test library distribution clear fixspace

all: bin/$(program) $(outdir)$(program)

# ****************** Testing
# use TESTARGS of
#  _valgrind to run under valgrind.
#  _debugAlloc to test recovery when running out of memory.
#  _full to obtain extra tests by verifying relations
#    between outputs of different actions, and generally testing
#    everything that can be tested.
# _full cannot follow the other options because it is picked up at an earlier
# point in the test system than they are. There are more options - see
# test/testScripts/testhelper for a full list.
#
# Only miniTest and bareTest support TESTARGS, and some options are not
# available unless MODE=debug.

# The correct choice to do a reasonably thorough test of an
# installation of Frobby.
test: all
	test/runTests

# Run all tests that it makes sense to run.
fullTest: all
	test/runTests _full
	test/runSplitTests _full

# Good for testing Frobby after a small change.
microTest: all
	test/runTests _few $(TESTARGS)
miniTest: all
	test/runTests $(TESTARGS)

# Runs all tests and allows full control over the arguments.
bareTest: all
	test/runTests $(TESTARGS) 
	test/runSplitTests $(TESTARGS)

# Run benchmarks to detect performance regressions. When MODE=profile,
# profile files for the benchmarked actions will be placed in bin/.
bench: all
	cd test/bench; ./runbench $(benchArgs)
benchHilbert: all
	cd test/bench; ./run_hilbert_bench $(benchArgs)
benchOptimize: all
	cd test/bench; ./run_optimize_bench $(benchArgs)
benchAlexdual: all
	cd test/bench; ./run_alexdual_bench $(benchArgs)

bin/:
	mkdir bin/;

# Make symbolic link to program from bin/
bin/$(program): $(outdir)$(program) bin/
ifneq ($(MODE), analysis)
	cd bin; rm -f $(program); ln -s ../$(outdir)$(program) $(program); cd ..
endif

# Link object files into executable
$(outdir)$(program): $(objs) | $(outdir)
	@mkdir -p $(dir $@)
ifeq ($(MODE), analysis)
	echo > $@
endif
ifneq ($(MODE), analysis)
	$(CXX) $(objs) $(ldflags) -o $@
	if [ -f $@.exe ]; then \
      mv -f $@.exe $@; \
	fi
endif
ifeq ($(MODE), release)
	$(STRIP) $@
endif

# Link object files into library
library: bin/$(library)
bin/$(library): $(objs) | bin/
	rm -f bin/$(library)
ifeq ($(MODE), shared)
	$(CXX) -shared -Wl,-soname,$(library).$(FROBBY_SOVERSION) \
	-o bin/$(library).$(FROBBY_VERSION) $(ldflags) \
	  $(patsubst $(outdir)main.o,,$(objs)) -lgmp -lgmpxx
else
	ar crs bin/$(library) $(patsubst $(outdir)main.o,,$(objs))
	$(RANLIB) bin/$(library)
endif

# Compile and output object files.
# In analysis mode no file is created, so create one
# to allow dependency analysis to work.
$(outdir)%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) ${cxxflags} -c $< -o $@
	$(CXX) $(cxxflags) -MM -c $< > $(@:.o=.d).tmp
# using /usr/bin/env echo to get the non-built-in echo on OS X, since
# the built-in one does not understand the parameter -n.
	@/usr/bin/env echo -n "$(dir $@)" > $(@:.o=.d)
	@cat $(@:.o=.d).tmp >> $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp
ifeq ($(MODE), analysis)
	  echo > $@
endif

-include $(objs:.o=.d)

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MAN1DIR ?= $(PREFIX)/share/man/man1
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

# Installation.
install:
ifneq ($(wildcard bin/frobby),)
	install -d $(DESTDIR)$(BINDIR)
	install bin/frobby $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(MAN1DIR)
	install -m 644 doc/frobby.1 $(DESTDIR)$(MAN1DIR)
endif
ifneq ($(wildcard bin/libfrobby*),)
	install -d $(DESTDIR)$(LIBDIR)
	install -d $(DESTDIR)$(INCLUDEDIR)
	install src/frobby.h $(DESTDIR)$(INCLUDEDIR)
ifneq ($(wildcard bin/libfrobby.a),)
	install bin/libfrobby.a $(DESTDIR)$(LIBDIR)
endif
ifneq ($(wildcard bin/libfrobby.so.$(FROBBY_VERSION)),)
	install bin/libfrobby.so.$(FROBBY_VERSION) $(DESTDIR)$(LIBDIR)
	cd $(DESTDIR)$(LIBDIR) && \
		rm -f libfrobby.so.$(FROBBY_SOVERSION) && \
		ln -s libfrobby.so.$(FROBBY_VERSION) \
			libfrobby.so.$(FROBBY_SOVERSION) && \
		rm -f libfrobby.so && \
		ln -s libfrobby.so.$(FROBBY_VERSION) libfrobby.so
endif
endif

# ***** Documentation

# We need to run latex three times to make sure that references are done
# properly in the output.
doc: docPs docPdf
docPs:
	rm -rf bin/doc
	mkdir bin/doc
	for i in 1 2 3; do latex -output-directory=bin/doc/ doc/manual.tex; done
	cd bin; dvips doc/manual.dvi
docPdf:
	rm -rf bin/doc
	mkdir bin/doc
	for i in 1 2 3; do pdflatex -output-directory=bin/doc/ doc/manual.tex; done
	mv bin/doc/manual.pdf bin
docDviOnce: # Useful to view changes when writing the manual
	latex -output-directory=bin/doc doc/manual.tex

# It may seem wasteful to run doxygen three times to generate three
# kinds of output. However, the latex output for creating a pdf file
# and for creating a PostScript file is different, and so at least two
# runs are necessary. Making the HTML output a third run is cleaner
# than tacking it onto one or both of the other two targets.
bin/develDoc/: bin/
	mkdir bin/develDoc
develDoc: develDocHtml develDocPdf develDocPs
develDocHtml: bin/develDoc/
	cat doc/doxygen.conf doc/doxHtml|doxygen -
develDocPdf: bin/develDoc/
	rm -rf bin/develDoc/latexPdf bin/develDoc/warningLog
	cat doc/doxygen.conf doc/doxPdf|doxygen -
	cd bin/develDoc/latexPdf; for f in `ls *.eps`; do epstopdf $$f; done # Cygwin fix
	cd bin/develDoc/latexPdf/; make refman.pdf; mv refman.pdf ../develDoc.pdf
develDocPs: bin/develDoc/
	rm -rf bin/develDoc/latexPs bin/develDoc/warningLog
	cat doc/doxygen.conf doc/doxPs|doxygen -
	cd bin/develDoc/latexPs/; make refman.ps; mv refman.ps ../develDoc.ps

clean: tidy
	rm -rf bin

# ***** Miscellaneous

tidy:
	find .|grep -x ".*~\|.*/\#.*\#|.*\.stackdump\|gmon\.out\|.*\.orig\|.*/core\|core"|xargs rm -f

# Fixes various white space related issues.
fixspace:
	find src/ doc/ -type f|xargs ./fixspace;

commit: test
	echo
	hg commit -m "$(MSG)"

# ***** Distribution

remoteUrl = ssh://daimi/projs/frobby
pull:
	hg pull $(remoteUrl)
push:
	hg push $(remoteUrl)

distribution:
ifndef VER
	echo "Please specify version of Frobby distribution using VER=x.y.z";
	exit 1;
endif
	rm -fr frobby_v$(VER).tar.gz frobby_v$(VER)
	mkdir frobby_v$(VER)
	cp -r changelog.txt frobgrob COPYING Makefile src test doc \
		cmake CMakeLists.txt GPL-header frobby_v$(VER)
	mkdir frobby_v$(VER)/4ti2
	tar --create --gzip --file=frobby_v$(VER).tar.gz frobby_v$(VER)/
	rm -fr frobby_v$(VER)	
	ls -l frobby_v$(VER).tar.gz

spkg: tidy depend
ifndef VER
	echo "Please specify version of Frobby spkg using VER=x.y.z";
	exit 1;
endif
	if [ "$$SAGE_LOCAL" = "" ]; then \
	  echo "SAGE_LOCAL undefined ... exiting"; \
	  echo "Maybe run 'sage -sh?'" \
	  exit 1; \
	fi

	if [ ! -d sage/ ]; then echo "sage/ directory not found."; exit 1; fi
# Ensure that previous builds have been cleaned up
	rm -rf bin/sagetmp bin/frobby-$(VER) bin/frobby-$(VER).spkg

	hg clone sage bin/sagetmp

	mkdir bin/sagetmp/src
	cp -r COPYING Makefile src test bin/sagetmp/src

	mv bin/sagetmp bin/frobby-$(VER)
	cd bin/; $(SAGE_ROOT)/sage -pkg `pwd`/frobby-$(VER)
	rm -rf bin/frobby-$(VER)
