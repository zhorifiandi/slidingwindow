#include <iostream>
#include <cstdio>
#include <cstring>

#include "dcomm.h"
#include "Checksum.h"

using namespace std;

class Frame {
public:
	
	Frame() : soh(SOH), stx(STX), etx(ETX) {
		FrameNumber = -1;
		data[0] = '\0';
		//setChecksum();
	}

	Frame(int i, char s[DATASIZE]) : soh(SOH), stx(STX), etx(ETX) {
		// Set FrameNumber
		FrameNumber = i;

		// Set data
		strcpy(data, s);

		// Serialize SOH, STX, and ETX
		serialized[0] = soh;
		serialized[5] = stx;
		serialized[6 + DATASIZE] = etx;

		// Serialize FrameNumber
		serialized[1] = i & 0xFF;
		serialized[2] = (i >> 8) & 0xFF;
		serialized[3] = (i >> 16) & 0xFF;
		serialized[4] = (i >> 24) & 0xFF;

		// Serialize data
		for (int i = 0; i < DATASIZE; i++) {
			serialized[6 + i] = data[i];
		}

		// SetChecksum from SOH to ETX
		string tmp;
		for (int i = 0; i < DATASIZE + 6; i++) {
			tmp.push_back(serialized[i]);
		}
		setChecksum(tmp);

		// Serialize checksum
		for (int i = 0; i < CHECKSUMSIZE; i++) {
			serialized[7 + DATASIZE + i] = checksum[i];
		}

	}

	Frame (char source[7 + DATASIZE + CHECKSUMSIZE]) {
		// Set SOH, STX, and EXT
		soh = source[0];
		stx = source[5];
		etx = source[6 + DATASIZE];

		// Set FrameNumber
		char tmp[sizeof(int)];
		for (int i = 0; i < sizeof(int); i++) {
			tmp[i] = source[i + 1];
		}
		FrameNumber = *(int *)tmp;

		// Set data
		for (int i = 0; i < DATASIZE; i++) {
			data[i] = source[6 + i];
		}

		// Set checksum
		string checksum_tmp;
		for (int i = 0; i < CHECKSUMSIZE; i++) {
			checksum_tmp.push_back(source[7 + DATASIZE + i]);
		}
		checksum = checksum_tmp;

		// Set serialized
		for (int i = 0; i < 7 + DATASIZE + CHECKSUMSIZE; i++) {
			serialized[i] = source[i];
		}
	}

	~Frame(){
	}

	void empty() {
		FrameNumber = -1;
	}
	

	void setSOH(char c) {
		soh = c;
	}
	void setNumber(int i) {
		FrameNumber = i;
	}
	void setSTX(char c)  {
		stx = c;
	}
	void setData(string c) {
		
	}
	void setETX(char c) {
		etx = c;
	}
	void setChecksum(string s) {
		Checksum crc(s);
		checksum = crc.hexCS;
	}

	char getSOH() {
		return soh;
	}
	int getNumber() {
		return FrameNumber;
	}
	char getSTX() {
		return stx;
	}
	char* getData() {
		return data;
	}
	char getETX() {
		return etx;
	}
	string getChecksum() {
		return checksum;
	}

	char* getSerialized() {
		return serialized;
	}

private:
	char soh;
	int FrameNumber;
	char stx;
	char data[DATASIZE];
	char etx;
	string checksum;
	char serialized[7 + DATASIZE + CHECKSUMSIZE];
};