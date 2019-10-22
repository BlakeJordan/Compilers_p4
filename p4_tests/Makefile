TESTFILES := $(wildcard *.lake)
TESTS := $(TESTFILES:.lake=.test)

.PHONY: all

all: $(TESTS)

%.test:
	@echo "Testing $*.lake" #The @ means don't show the command
	@rm -f $*.err
	@touch $*.err 
	@../lakec $*.lake -c 2> $*.err ;\
	PROG_EXIT_CODE=$$?;\
	echo "Checking expected error output for $*.lake...";\
	diff -B --ignore-all-space $*.err $*.err.expected;\
	ERR_DIFF_EXIT=$$?;\
	exit $$ERR_DIFF_EXIT

clean:
	rm -f *.out *.err
