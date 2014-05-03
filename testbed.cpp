/*
  Testbed for empirical evaluation of KP-ABE schemes.
  Alexandre Miranda Pinto

  Based on sample file fuzzy.cpp, of the MIRACL distribution.


  Using BN curves for a security level of AES-128. k = 12, for Type 3 pairings.

  Compile with modules as specified below:

  Library Preparation:

  - After compiling miracl.a as per MIRACL's installation instructions, compile pairs.a and bn_pair.o in the following way:

  	pairs.a:
		g++ -O2 -DZZNS=4 -m64 -c zzn12a.cpp zzn4.cpp ecn2.cpp zzn2.cpp big.cpp zzn.cpp ecn.cpp 
		ar rc pairs.a zzn12a.o zzn4.o ecn2.o zzn2.o big.o zzn.o ecn.o

	bn_pair.o:
		g++ -O2 -DZZNS=4 -m64 -c bn_pair.cpp

	libs: pairs.a bn_pair.o
		rm -f libpairs.a
		rm -f libbn.a
		cp pairs.a libpairs.a
		ar rc libbn.a bn_pair.o
		
  - Copy the resulting libraries libpairs.a and libbn.a to /usr/local/lib. Also, copy miracl.a for the same directory and rename it libmiracl.a

  - Compile this file as

  	g++ -O2 -m64 -DZZNS=4 testbed.cpp -lbn -lmiracl -lpairs -o testbed

*/

#include <iostream>
#include <ctime>

//********* choose just one of these pairs **********
#define MR_PAIRING_BN    // AES-128 security
#define AES_SECURITY 128
//*********************************************

#include "pairing_3.h"

// Global Declarations

#define NATTR 20

PFC pfc(AES_SECURITY);  // initialise pairing-friendly curve
Big y;	//private exponent for the keys, and for the blinding factor
GT Y;	//blinding factor for the message

G1 P;	//generator of G1
G2 Q;	//generator of G2


// private and public values for each attribute
Big t[NATTR];
G1 T[NATTR];


// The objective of this method is to generate the basic parameters of the system. These are not public parameters yet, just the framework values:
// G_1, G_2, G_T, g_1, g_2, p.
// Note that indeed G_1, G_2, G_T are fixed by the specific curve library that we use. Therefore, we only have to generate the other values, besides setting the
// environment itself to run.
// The correspondence to code is like this:
// 	order: p
//	g_1: P
//	g_2: Q
int params_gen(){
	miracl *mip=get_mip();  // get handle on mip (Miracl Instance Pointer)
	Big order=pfc.order();  // get pairing-friendly group order

	//random is polymorphic. It can take Big, G1 or G2 as arguments. It selects a random group element
	pfc.random(P);	
	pfc.random(Q);
	// I don't understand what is being precomputed, but I am not going to care for now
	pfc.precomp_for_mult(Q);  // Q is fixed, so precompute on it
}


// The objective of this method is to setup the parameters of the system
// That means generating public and private keys
int setup(){
	// The following finally make sense. y is a Big so can be passed to PFC.random. Then, pfc.pairing executes the pairing e(g1,g2) and the result is raised to y.
	// The outcome is store in Y, thus making Y = e(g1,g2)^y.
	// This makes it obvious that Q and P are standing for the generators of G1 and G2. Notice that the first element of the pairing is in fact G2 and not G1.
	pfc.random(y);
	Y=pfc.power(pfc.pairing(Q,P),y);

	// The following block initializes the values for all attributes
	// Note the following: the several T_i are actually components of G1. In the original paper, both groups are equal. 
	// In our implementation, I defaulted to having T_i built on G2 instead. I will have to keep that in mind.
	// Important: The definition is T_i = g_1^t_i. Notice that this is done by a multiplication operation! --- this is just a different interpretation: 
	// it looks at the group operation as an addition, whereas I am looking at it as an exponentiation. Keep in mind!

	for (int i=0;i<NATTR;i++)
	{
		pfc.random(t[i]);                 // Note t[i] will be 2*AES_SECURITY bits long
		T[i]=pfc.mult(P,t[i]);            // which may be less than the group order.
		pfc.precomp_for_mult(T[i],TRUE);  // T[i] are system params, so precompute on them
	                                          // Note second parameter indicates that all multipliers 
						  // must  be <=2*AES_SECURITY bits, which may be shorter
						  // than the full group size. 
	}
}


  /*
    This function generates a key for a given policy
    A policy is a set of attributes. 
   */

  int key_generation(int[] attribs, int length){
    


		BD[j]=pfc.mult(Q,moddiv(qi,t[i],order));
		pfc.precomp_for_pairing(BD[j]);   // Bob precomputes on his private key



	poly[0]=y;
	for (i=1;i<Nd;i++)
		pfc.random(poly[i]);
	
	for (j=0;j<NALICE;j++)
	{
		i=Alice[j];
		qi=y; ik=i;
		for (k=1;k<Nd;k++)
		{ // evaluate polynomial a0+a1*x+a2*x^2... for x=i;
			qi+=modmult(poly[k],(Big)ik,order);
			ik*=i;
			qi%=order;
		}

		AD[j]=pfc.mult(Q,moddiv(qi,t[i],order));  // exploits precomputation
	}

}

int main()
{   
  params_gen();
  setup();
  return 0;
}




int pp_gen() {
  /* 

     poly[Nd]: coefficients for the polynomial: these are numbers in Z_p
     S[NBOB]: not really sure, these are numbers for Bob's attributes. But are they the secret exponents?
     t[NATTR]: these are still integers, and so are the secret values for each attribute
     T[NATTR]: these are the public values of every attribute, and so are elements of G1
     E[Nd]: I am not sure of this type, nor of what these should mean
     AE[NALICE], BD[NBOB]: equally I am not sure of what this type means

     Y, DB: Y is the pair e(g1,g2)^y and is an element of GT. Not sure what DB is.

     We need the following public parameters (not the public key): G_1, G_2, G_T, p, g_1, g_2
     The relevant definitions in the code below are:

     G_1, G_2, G_T are fixed by the construction, they are predetermined curves. But 
  */

  /*
	int i,j,k,n,ik,S[NBOB];
	Big s,y,qi,M,ED,t[NATTR];
	Big poly[Nd];
	G1 P,T[NATTR],E[Nd],AE[NALICE];
	G2 Q,AD[NALICE],BD[NBOB];
	GT Y,DB;

	pfc.random(P);
	pfc.random(Q);
	pfc.precomp_for_mult(Q);  // Q is fixed, so precompute on it

	pfc.random(y);
	Y=pfc.power(pfc.pairing(Q,P),y);
	for (i=0;i<NATTR;i++)
	{
		pfc.random(t[i]);                 // Note t[i] will be 2*AES_SECURITY bits long
		T[i]=pfc.mult(P,t[i]);            // which may be less than the group order.
		pfc.precomp_for_mult(T[i],TRUE);  // T[i] are system params, so precompute on them
                                          // Note second parameter indicates that all multipliers 
										  // must  be <=2*AES_SECURITY bits, which may be shorter
										  // than the full group size. 
	}
  */
}
