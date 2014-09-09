WARNINGS=-W -Wall -Wextra -pedantic
CVERS=-std=gnu++0x
DEBUG=-g
OPT=-O2
#OPT=
MIRACL=-DZZNS=4 -m64
LIBS=-lbn -lpairs -lmiracl

all: testutils testtree testBLcanonical testShTree testkpabe1 testkpabe2 benchmark_bl_1 benchmark_bl_2 benchmark_sh_2 benchmark_sh_1

utils.o: utils.cpp utils.h utils_impl.tcc
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c utils.cpp -o utils.o

testutils: utils.o testutils.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testutils.cpp utils.o $(LIBS) -o testutils

secretsharing.o: secretsharing.cpp secretsharing.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c secretsharing.cpp -o secretsharing.o

tree.o:	tree.cpp tree.h utils.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c tree.cpp -o tree.o

testtree: utils.o testtree.cpp tree.o tree.h tree.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testtree.cpp utils.o tree.o $(LIBS) -o testtree

BLcanonical.o: BLcanonical.h BLcanonical.cpp utils.o secretsharing.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c BLcanonical.cpp -o BLcanonical.o

testBLcanonical: BLcanonical.o testBLcanonical.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testBLcanonical.cpp BLcanonical.o utils.o secretsharing.o $(LIBS) -o testBLcanonical

ShTree.o: ShTree.h ShTree.cpp utils.o tree.o secretsharing.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c ShTree.cpp -o ShTree.o

testShTree: ShTree.o testShTree.cpp 
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testShTree.cpp ShTree.o tree.o utils.o secretsharing.o $(LIBS) -o testShTree

kpabe1.o: kpabe.cpp kpabe.h 
	cp atts.h_1 atts.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c kpabe.cpp -o kpabe1.o 

kpabe2.o: kpabe.cpp kpabe.h 
	cp atts.h_2 atts.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c kpabe.cpp -o kpabe2.o 

testkpabe1: testkpabe.cpp utils.o kpabe1.o secretsharing.o BLcanonical.o ShTree.o tree.o
	cp atts.h_1 atts.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testkpabe.cpp kpabe1.o utils.o secretsharing.o BLcanonical.o ShTree.o tree.o $(LIBS) -o testkpabe1 

testkpabe2: testkpabe.cpp utils.o kpabe2.o secretsharing.o BLcanonical.o ShTree.o tree.o
	cp atts.h_2 atts.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testkpabe.cpp kpabe2.o utils.o secretsharing.o BLcanonical.o ShTree.o tree.o $(LIBS) -o testkpabe2 


bbench: basic-benchmark.cpp 
	g++ $(WARNINGS) $(CVERS) $(MIRACL) basic-benchmark.cpp $(LIBS) -o bbench # no optimization!!!



benchmark_bl_1: benchmark.cpp utils.o kpabe1.o secretsharing.o BLcanonical.o 
	@echo "target: " $@
	@echo "============="
	cp benchmark_defs_bl.h benchmark_defs.h
	cp atts.h_1 atts.h
	g++ $(WARNINGS) $(CVERS) $(MIRACL) benchmark.cpp kpabe1.o utils.o secretsharing.o BLcanonical.o  $(LIBS) -o benchmark_bl_1 # no optimization!!!

benchmark_bl_2: benchmark.cpp utils.o kpabe2.o secretsharing.o BLcanonical.o 
	@echo "target: " $@
	@echo "============="
	cp benchmark_defs_bl.h benchmark_defs.h
	cp atts.h_2 atts.h
	g++ $(WARNINGS) $(CVERS) $(MIRACL) benchmark.cpp kpabe2.o utils.o secretsharing.o BLcanonical.o  $(LIBS) -o benchmark_bl_2 # no optimization!!!

benchmark_sh_1: benchmark.cpp utils.o kpabe1.o secretsharing.o ShTree.o tree.o
	@echo "target: " $@
	@echo "============="
	cp benchmark_defs_sh.h benchmark_defs.h
	cp atts.h_1 atts.h
	g++ $(WARNINGS) $(CVERS) $(MIRACL) benchmark.cpp kpabe1.o utils.o secretsharing.o ShTree.o tree.o $(LIBS) -o benchmark_sh_1 # no optimization!!!

benchmark_sh_2: benchmark.cpp utils.o kpabe2.o secretsharing.o ShTree.o tree.o
	@echo "target: " $@
	@echo "============="
	cp benchmark_defs_sh.h benchmark_defs.h
	cp atts.h_2 atts.h
	g++ $(WARNINGS) $(CVERS) $(MIRACL) benchmark.cpp kpabe2.o utils.o secretsharing.o ShTree.o tree.o $(LIBS) -o benchmark_sh_2 # no optimization!!!



#BLCanonkpabe.o: BLCanonkpabe.cpp BLCanonkpabe.h 
#	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c BLCanonkpabe.cpp -o BLCanonkpabe.o 

#testBL: testBL.cpp utils.o tree.o BL.h BL.cpp BL.o secretsharing.o 
#	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testBL.cpp secretsharing.o utils.o tree.o BL.o $(LIBS) -o testBL

#ShTreekpabe.o: ShTreekpabe.cpp ShTreekpabe.h 
#	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c ShTreekpabe.cpp -o ShTreekpabe.o 

#BL.o: BL.cpp BL.h tree.o utils.o secretsharing.o
#	g++ -W -Wall -Wextra -pedantic -std=gnu++0x -c -O2 -DZZNS=4 -m64 BL.cpp -o BL.o 



#shamir.o: shamir.cpp shamir.h
#	g++ -c -O2 -DZZNS=4 -m64 shamir.cpp -o shamir.o 

#testshamir: testshamir.cpp shamir.o utils.o
#	g++ -O2 -DZZNS=4 -m64 testshamir.cpp shamir.o utils.o -lbn -lpairs -lmiracl -o testshamir 

#testshamir2: testshamir2.cpp shamir2.o utils.o secretsharing.o 
#	g++ -O2 -DZZNS=4 -m64 testshamir2.cpp secretsharing.o shamir2.o utils.o -lbn -lpairs -lmiracl -o testshamir2 

#sham: utils.o shamir.o testshamir 

#abe: shamir.o utils.o testkpabe

#secret: secretsharing.o shamir2.o secret_sharing.cpp secret_sharing.h shamir2.cpp shamir2.h


#shamir2.o: shamir2.cpp shamir2.h secretsharing.o
#	g++ -W -Wall -Wextra -pedantic -std=c++0x -c -O2 -DZZNS=4 -m64 shamir2.cpp -o shamir2.o 


testbed:
	g++ -O2 -DZZNS=4 -m64 testbed.cpp -lbn -lpairs -lmiracl -o testbed

clean:
	rm -f testbed
	rm -f bbench
	rm -f benchmark_bl_1
	rm -f benchmark_bl_2
	rm -f benchmark_sh_1
	rm -f benchmark_sh_2

	rm -f atts.h

	rm -f utils.o
	rm -f secretsharing.o
	rm -f tree.o
	rm -f BLcanonical.o
	rm -f ShTree.o
	rm -f kpabe1.o
	rm -f kpabe2.o
#	rm -f shamir.o
#	rm -f shamir2.o
#	rm -f BLCanonkpabe.o
#	rm -f ShTreekpabe.o

	rm -f testutils
	rm -f testtree
	rm -f testBLcanonical
	rm -f testShTree
	rm -f testkpabe1
	rm -f testkpabe2
#	rm -f testshamir
#	rm -f testshamir2
#	rm -f testBL
#	rm -f BL.o


#fuzzy: 
#	g++ -O2 -DZZNS=4 -m64 fuzzy.cpp -lbn -lpairs -lmiracl -o fuzzy

