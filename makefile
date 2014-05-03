all: 
	@echo "please, specify a target"

fuzzy: 
	g++ -O2 -DZZNS=4 -m64 fuzzy.cpp -lbn -lpairs -lmiracl -o fuzzy

testbed:
	g++ -O2 -DZZNS=4 -m64 testbed.cpp -lbn -lpairs -lmiracl -o testbed

shamir.o: shamir.cpp shamir.h
	g++ -c -O2 -DZZNS=4 -m64 shamir.cpp -o shamir.o 

testshamir: testshamir.cpp shamir.o utils.o
	g++ -O2 -DZZNS=4 -m64 testshamir.cpp shamir.o utils.o -lbn -lpairs -lmiracl -o testshamir 

utils.o: utils.cpp utils.h
	g++ -O2 -DZZNS=4 -m64 -c utils.cpp -o utils.o

shaSS: utils.o shamir.o testshamir 

clean:
	rm -f testbed
	rm -f shamir.o
	rm -f testshamir
	rm -f utils.o