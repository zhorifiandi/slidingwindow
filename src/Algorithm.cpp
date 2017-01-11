#include <iostream>
#include "dcomm.h"
#include "Frame.cpp"
#include "Checksum.h"

using namespace std;

class Algorithm {
public:
	char sign;
	int frameNumber;
	string checksum;
	char serialized[5 + CHECKSUMSIZE];

	Algorithm() {
		sign = NAK;
		frameNumber = -1;
		checksum = "";
	}

	Algorithm(char _sign, int _frameNumber, string _checksum) {
		sign = _sign;
		frameNumber = _frameNumber;
		checksum = _checksum;
	}

	Algorithm (char source[5 + CHECKSUMSIZE]) {
		// Set sign
		sign = source[0];
		
		// Set frameNumber
		char tmp[sizeof(int)];
		for (int i = 0; i < sizeof(int); i++) {
			tmp[i] = source[i + 1];
		}
		frameNumber = *(int *)tmp;

		// Set checksum
		string checksum_tmp;
		for (int i = 0; i < CHECKSUMSIZE; i++) {
			checksum_tmp.push_back(source[5 + i]);
		}
		checksum = checksum_tmp;

		// Set serialized
		for (int i = 0; i < 5 + CHECKSUMSIZE; i++) {
			serialized[i] = source[i];
		}
	}

	Algorithm(Frame frame_) {
		frameNumber = frame_.getNumber();
		string checksum_tmp; 
		for (int j = 0; j < DATASIZE + 6; j++) { 
			checksum_tmp.push_back(frame_.getSerialized()[j]); 
		} 
		Checksum crcFrame(checksum_tmp); 
		string cs = crcFrame.hexCS;

		if (cs[0] == frame_.getChecksum()[0] && cs[1] == frame_.getChecksum()[1]){
			sign = ACK;
		} else{
			sign = NAK;
		}	

		serialized[0] = sign;
			
		// Serialize frameNumber
		serialized[1] = frameNumber & 0xFF;
		serialized[2] = (frameNumber >> 8) & 0xFF;
		serialized[3] = (frameNumber >> 16) & 0xFF;
		serialized[4] = (frameNumber >> 24) & 0xFF;

		string tmp;
		for (int i = 0; i < 5; ++i){
			tmp[i] = serialized[i];
		}
		Checksum crcResponse(tmp);
		checksum = crcResponse.hexCS;


//		checksum = crc32(tmp);
		
		// Serialize checksum
		for (int i = 0; i < CHECKSUMSIZE; i++) {
			serialized[5 + i] = checksum[i];
		}		

	}
	
	~Algorithm() {

	}

	char getSign() {
		return sign;
	}

	void setSign(char _sign) {
		sign = _sign;
	}

	int getNumber() {
		return frameNumber;
	}

	void setNumber(int _frameNumber) {
		frameNumber = _frameNumber;
	}

	string getChecksum() {
		return checksum;
	}
	void setChecksum(string s) {
		Checksum crc(s);
		checksum = crc.hexCS;
	}
	char* getSerialized() {
		return serialized;
	}
	
};