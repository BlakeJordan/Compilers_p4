TESTFILES := $(wildcard *.cRAP)
TESTS := $(TESTFILES:.cRAP=.test)

.PHONY: all

all: $(TESTS)

%.test:
	@echo "Testing $*.cRAP" #The @ means don't show the command
	@rm -f $*.err
	@touch $*.err 
	@../cRAPc $*.cRAP -c 2> $*.err ;\
	PROG_EXIT_CODE=$$?;\
	echo "diff error...";\
	diff -B --ignore-all-space $*.err $*.err.expected;\
	ERR_DIFF_EXIT=$$?;\
	exit $$ERR_DIFF_EXIT

clean:
	rm -f *.out *.err
