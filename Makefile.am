SUBDIRS = src doc

EXTRA_DIST = autogen.sh README.md changelog.txt GPL-header frobgrob     \
  4ti2/README CMakeLists.txt cmake/FindGMP.cmake cmake/FindMPIR.cmake   \
  test/bench test/bigIdeals test/commonIdeals test/frob                 \
  test/idealFormats test/intersect test/irrdecom test/latticeFormats    \
  test/messages test/polyFormats test/runTestGroup test/specialIdeals   \
  test/testScripts

DOC_TARGETS = doc docPs docPdf

$(DOC_TARGETS):
	$(MAKE) -C doc $@

.PHONY: $(DOC_TARGETS)

@DX_RULES@

MOSTLYCLEANFILES = $(DX_CLEANFILES)

# ****************** Testing
# make check runs the unit tests in src and these groups of tests of
# the frobby executable, which make installcheck runs on the installed
# frobby.  Set TEST_LEVEL=few for fewer tests or TEST_LEVEL=full for
# more, including the split_* groups, which are skipped otherwise, and
# TESTARGS for options to test/testScripts/testhelper, e.g.
#  _valgrind to run under valgrind.
#  _debugAlloc to test recovery from running out of memory, which
#    needs --enable-debug.
# To run only some groups, use e.g. make check-TESTS TESTS=euler, since
# make check would also pass TESTS to src.
TESTS = euler alexdual hilbert_deform hilbert_bigatti hilbert_slice     \
  dimension primdecom intersect assoprimes minimize irrdecom radical    \
  frob maxstandard optimize idealFormats polyFormats latticeFormats     \
  messages split_hilbert_bigatti split_optimize split_irrdecom          \
  split_assoprimes split_alexdual split_hilbert_slice split_frob
LOG_COMPILER = $(srcdir)/test/runTestGroup
FROBBY = $(abs_top_builddir)/src/frobby$(EXEEXT)
AM_TESTS_ENVIRONMENT = FROBBY='$(FROBBY)' TEST_LEVEL='$(TEST_LEVEL)'    \
  TESTARGS='$(TESTARGS)'; export FROBBY TEST_LEVEL TESTARGS;

# The tests are names of groups for test/runTestGroup, not files.
$(TESTS):

installcheck-local:
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS FROBBY='$(bindir)/frobby$(EXEEXT)'

export abs_srcdir
export abs_builddir

# Run benchmarks to detect performance regressions. When MODE=profile,
# profile files for the benchmarked actions will be placed in bin/.
bench: all
	cd $(srcdir)/test/bench; ./runbench $(benchArgs)
benchHilbert: all
	cd $(srcdir)/test/bench; ./run_hilbert_bench $(benchArgs)
benchOptimize: all
	cd $(srcdir)/test/bench; ./run_optimize_bench $(benchArgs)
benchAlexdual: all
	cd $(srcdir)/test/bench; ./run_alexdual_bench $(benchArgs)
