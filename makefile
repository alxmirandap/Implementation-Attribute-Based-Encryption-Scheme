WARNINGS=-W -Wall -Wextra -pedantic
CVERS=-std=gnu++0x
DEBUG=-g
#OPT=-O2
OPT=
MIRACL=-DZZNS=4 -m64
LIBS=-lbn -lpairs -lmiracl

all: testutils testBLcanonical testkpabe

utils.o: utils.cpp utils.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c utils.cpp -o utils.o

testutils: utils.o testutils.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testutils.cpp utils.o $(LIBS) -o testutils

secretsharing.o: secretsharing.cpp secretsharing.h
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c secretsharing.cpp -o secretsharing.o

BLcanonical.o: BLcanonical.h BLcanonical.cpp utils.o secretsharing.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c BLcanonical.cpp -o BLcanonical.o

testBLcanonical: BLcanonical.o testBLcanonical.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testBLcanonical.cpp BLcanonical.o utils.o secretsharing.o $(LIBS) -o testBLcanonical

BLCanonkpabe.o: BLCanonkpabe.cpp BLCanonkpabe.h 
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c BLCanonkpabe.cpp -o BLCanonkpabe.o 

kpabe.o: kpabe.cpp kpabe.h 
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) -c kpabe.cpp -o kpabe.o 

testkpabe: testkpabe.cpp utils.o kpabe.o secretsharing.o BLcanonical.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testkpabe.cpp kpabe.o utils.o secretsharing.o BLcanonical.o $(LIBS) -o testkpabe 


tree.o:	tree.cpp tree.h utils.o
	g++ $(DEBUG) $(WARNINGS) $(CVERS) -c $(OPT) $(MIRACL) tree.cpp -o tree.o

testtree: utils.o testtree.cpp tree.o tree.h tree.cpp
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testtree.cpp utils.o tree.o $(LIBS) -o testtree


bbench: basic-benchmark.cpp 
	g++ $(WARNINGS) $(CVERS) $(MIRACL) basic-benchmark.cpp $(LIBS) -o bbench # no optimization!!!

benchmark: benchmark.cpp utils.o kpabe.o secretsharing.o BLcanonical.o
	g++ $(WARNINGS) $(CVERS) $(MIRACL) benchmark.cpp kpabe.o utils.o secretsharing.o BLcanonical.o $(LIBS) -o benchmark # no optimization!!!



testBL: testBL.cpp utils.o tree.o BL.h BL.cpp BL.o secretsharing.o 
	g++ $(DEBUG) $(WARNINGS) $(CVERS) $(OPT) $(MIRACL) testBL.cpp secretsharing.o utils.o tree.o BL.o $(LIBS) -o testBL

BL.o: BL.cpp BL.h tree.o utils.o secretsharing.o
	g++ -W -Wall -Wextra -pedantic -std=gnu++0x -c -O2 -DZZNS=4 -m64 BL.cpp -o BL.o 



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
	rm -f utils.o
	rm -f shamir.o
	rm -f shamir2.o
	rm -f kpabe.o
	rm -f tree.o
	rm -f secretsharing.o
	rm -f BLcanonical.o
	rm -f BL.o

	rm -f testkpabe
	rm -f testshamir
	rm -f testshamir2
	rm -f testBLcanonical
	rm -f testutils
	rm -f testtree
	rm -f testBL


fuzzy: 
	g++ -O2 -DZZNS=4 -m64 fuzzy.cpp -lbn -lpairs -lmiracl -o fuzzy

