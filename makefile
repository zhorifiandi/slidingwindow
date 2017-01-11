all: receiver transmitter

receiver:
	g++ -std=c++11 Receiver.cpp Buffer.cpp Checksum.cpp -o receiver -lpthread

transmitter:
	g++ -std=c++11 Transmitter.cpp Checksum.cpp -o transmitter -lpthread
