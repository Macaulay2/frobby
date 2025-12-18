SUBDIRS = src doc

DOC_TARGETS = doc docPs docPdf

$(DOC_TARGETS):
	$(MAKE) -C doc $@

.PHONY: $(DOC_TARGETS)

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

export abs_srcdir
export abs_builddir

test: all
	$(srcdir)/test/runTests

# Run all tests that it makes sense to run.
fullTest: all
	$(srcdir)/test/runTests _full
	$(srcdir)/test/runSplitTests _full

# Good for testing Frobby after a small change.
microTest: all
	$(srcdir)/test/runTests _few $(TESTARGS)
miniTest: all
	$(srcdir)/test/runTests $(TESTARGS)

# Runs all tests and allows full control over the arguments.
bareTest: all
	$(srcdir)/test/runTests $(TESTARGS)
	$(srcdir)/test/runSplitTests $(TESTARGS)

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
