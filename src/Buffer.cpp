#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <queue>
#include "dcomm.h"
#include "Algorithm.cpp"

using namespace std;



#define DELAY 500 // Delay to adjust speed of consuming Buffer, in milliseconds
#define UPLIMIT 6 // Define minimum upper limit
#define LOWLIMIT 2 // Define maximum lower limit


struct FrameComparator
{
  bool operator()(Frame& a, Frame& b)
  {
    return a.getNumber() > b.getNumber();
  }
};

class Buffer{
public:
	Buffer() : maxsize(RXQSIZE){}

	~Buffer(){
	}

	void add(char tx_result[DATASIZE + 15]){
		Frame Frame_(tx_result);
		if (!isFull()){
			body_element.push_back(Frame_);
			body_element_all.push(Frame_);
		}
	}

	void consume(Frame *Frame_){
		if (!isEmpty()){
			*Frame_ = body_element[0];
			body_element.erase(body_element.begin());
		} 
	}
	int getCount(){
		return body_element.size();
	}

	bool isFull(){
		return (body_element.size() == maxsize);
	}

	bool isEmpty(){
		return body_element.empty();
	}

	priority_queue<Frame, vector<Frame>, FrameComparator> getDataAll() {
		return body_element_all;
	}

private:
	vector<Frame> body_element; // Buffer memory region
	priority_queue<Frame, vector<Frame>, FrameComparator> body_element_all;
	int maxsize; 
};