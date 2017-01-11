#include "Checksum.h"
using namespace std;

Checksum::Checksum(string chf) {
	Error = false;
	strcpy(Polynomial, "100110001");
	memset(CheckSum,nPoly-1,0);
	a = chf.length();
	a = a * 8;
	SFrame = chf;
	Frame = new char[a+9];
	for (int i = 0; i < a+9; ++i)
	{
		Frame[i] = '0';
	}
	convertFrame();
	crc();
	for (int i = a; i < a+nPoly-1; ++i)
	{
		setElmtFrame(i, getElmtCheckSum(i-a));
	}
	hexingCheckSum();
}

Checksum::~Checksum() {
	delete [] Frame;
}

void Checksum::axor() {
	for (int i = 1; i < nPoly; ++i)
	{
		if (getElmtCheckSum(i) == getElmtPolynomial(i))
		{
			setElmtCheckSum(i,'0');
		}
		else {
			setElmtCheckSum(i,'1');
		}
	}
}

void Checksum::crc() {
	int i, j;
	for (i = 0; i < nPoly; ++i)
	{
		setElmtCheckSum(i,(getElmtFrame(i)));
	}
	int k = nPoly;
	do {
		if (getElmtCheckSum(0) == '1')
		{
			axor();
		}
		for (j = 0; j < nPoly-1; ++j)
		{
			setElmtCheckSum(j, getElmtCheckSum(j+1));
		}
		char tmp = Frame[k];
		CheckSum[j] = tmp;
		k++;
	} while (k <= a+nPoly-1);
}

char Checksum::getElmtFrame(int idx){
	return Frame[idx];
}

char Checksum::getElmtCheckSum(int idx){
	return CheckSum[idx];
}

char Checksum::getElmtPolynomial(int idx){
	return Polynomial[idx];
}

void Checksum::setElmtFrame(int idx, char ch) {
	Frame[idx] = ch;
}

void Checksum::setElmtCheckSum(int idx, char ch) {
	CheckSum[idx] = ch;
}

void Checksum::convertFrame() {
	for (int i = 0; i < SFrame.length(); ++i)
	{
		int ctemp = (int) SFrame[i];
		int count = 7;
		while(count >= 0) {
			if (ctemp % 2 == 0)
			{
				setElmtFrame(count+i*8, '0');
			}
			else {
				setElmtFrame(count+i*8, '1');
			}
			ctemp = ctemp / 2;
			count--;
		}
	}
}

void Checksum::hexingCheckSum() {
	int temp = 0; int nPow = 0; char res;
	for (int i = nPoly-2; i >= 0; --i)
	{
		int power = 1;
		for (int j = 0; j < nPow; ++j)
		{
			power *= 2;
		}
		nPow = (nPow + 1) % 4;
		temp += (CheckSum[i] - '0') * power;
		if (nPow == 0)
		{
			if (temp < 10)
			{
				res = temp + '0';
			}
			else {
				switch (temp) {
					case 10 : res = 'A'; break;
					case 11 : res = 'B'; break;
					case 12 : res = 'C'; break;
					case 13 : res = 'D'; break;
					case 14 : res = 'E'; break;
					case 15 : res = 'F'; break;
					default : break;
				}
			}
			temp = 0;
			hexCS = res + hexCS;
		}
	}
}

void isError() {

}