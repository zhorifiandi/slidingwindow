#ifndef _CHECKSUM_H
#define _CHECKSUM_H

#include <cstdio>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

#define nPoly 9 

class Checksum
{
public:
	Checksum(string chf);
	void axor();
	void crc();
	
	/* Getter */	
	char getElmtFrame(int idx);
	char getElmtCheckSum(int idx);
	char getElmtPolynomial(int idx);
	/* Setter */
	void setElmtFrame(int idx, char ch);
	void setElmtCheckSum(int idx, char ch);

	/* Convert char-binary */
	void convertFrame();
	void hexingCheckSum();

	/* Error Detection */
	bool isError();

	~Checksum();

	/* data */
	char* Frame; // Frame representasi biner
	char CheckSum[nPoly]; // Checksum representasi biner
	string SFrame;
	string hexCS;
	char Polynomial[nPoly];
	int a;
	bool Error;
};

#endif