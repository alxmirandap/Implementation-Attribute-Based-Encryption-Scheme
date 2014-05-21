all: sham abe

utils.o: utils.cpp utils.h
	g++ -O2 -DZZNS=4 -m64 -c utils.cpp -o utils.o

shamir.o: shamir.cpp shamir.h
	g++ -c -O2 -DZZNS=4 -m64 shamir.cpp -o shamir.o 

testshamir: testshamir.cpp shamir.o utils.o
	g++ -O2 -DZZNS=4 -m64 testshamir.cpp shamir.o utils.o -lbn -lpairs -lmiracl -o testshamir 

sham: utils.o shamir.o testshamir 

kpabe.o: kpabe.cpp kpabe.h shamir.h
	g++ -c -O2 -DZZNS=4 -m64 kpabe.cpp -o kpabe.o 

testkpabe: testkpabe.cpp shamir.o utils.o kpabe.o
	g++ -O2 -DZZNS=4 -m64 testkpabe.cpp kpabe.o utils.o shamir.o -lbn -lpairs -lmiracl -o testkpabe 

abe: shamir.o utils.o testkpabe


testbed:
	g++ -O2 -DZZNS=4 -m64 testbed.cpp -lbn -lpairs -lmiracl -o testbed

clean:
	rm -f testbed
	rm -f utils.o
	rm -f shamir.o
	rm -f testshamir
	rm -f kpabe.o
	rm -f testkpabe

fuzzy: 
	g++ -O2 -DZZNS=4 -m64 fuzzy.cpp -lbn -lpairs -lmiracl -o fuzzy
