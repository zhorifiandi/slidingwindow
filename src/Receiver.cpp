#include "Buffer.cpp"

bool isXON = true;

class Receiver{
public:
	//INITIALIZATION
	Receiver(const char* arg) : port(arg){
		createSocket();

		binding(); 

		messageHandler();

		closeSocket();
	}

	~Receiver(){
		closeSocket();
	}

	/* CREATE SOCKET */
	void createSocket(){
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			throw "Error opening socket!";
		}
	}

	/* BIND SOCKET */
	void binding(){
		bzero(&Receiver_Endpoint, sizeof(Receiver_Endpoint));
		Receiver_Endpoint.sin_family = AF_INET;
		Receiver_Endpoint.sin_addr.s_addr = htonl(INADDR_ANY);
		Receiver_Endpoint.sin_port = htons(atoi(port));
		if (bind(sockfd, (struct sockaddr *)&Receiver_Endpoint, sizeof(Receiver_Endpoint)) < 0){
			throw "Error binding. Try different port!";
		} else{
			inet_ntop(AF_INET, &(Receiver_Endpoint.sin_addr), address, INET_ADDRSTRLEN);
			cout << "Binding on " << getAddress() << ":" << getPort() << " ..." << endl;
		}
	}

	/* RECEIVE DATA FROM TRANSMITTER */
	void messageHandler(){
		socklen_t addrlen = sizeof(Transmitter_Endpoint);
		char CharFrame[DATASIZE + 15];
		// Create new thread for consuming the Buffer data
		std::thread thread_consume(consumeBuffer, &buffer, &sockfd, &Transmitter_Endpoint); 
		do{
			int recvlen = recvfrom(sockfd, CharFrame, DATASIZE + 15, 0, (struct sockaddr *)&Transmitter_Endpoint, &addrlen);
			if (recvlen > 0){
				buffer.add(CharFrame);
			}	
			usleep(DELAY * 1000);
		} while(CharFrame[6] != Endfile);
		thread_consume.join();
	}

	/* CONSUME DATA IN BUFFER */
	static void consumeBuffer(Buffer *buffer, int *sockfd, sockaddr_in *transmitter){
		socklen_t addrlen = sizeof(*transmitter);
		Frame current_frame;
		while(current_frame.getData()[0] != Endfile){
			//IF EMPTY DON'T CONSUME!! CAN CAUSE SEGMENTATION FAULT :)
			if (!buffer->isEmpty()){ 
				buffer->consume(&current_frame);
				Algorithm handler(current_frame);
				if (handler.getSerialized()[0] == ACK){
					cout << "Sending ACK for frame " << handler.getNumber() << endl;
				} else if (handler.getSerialized()[0] == NAK){
					cout << "Sending NAK for frame " << handler.getNumber() << endl;
				} else{
					cout << "WHAT THE HELL!!!" << endl;
				}
				cout << "Receiving data: " << current_frame.getData() << endl;
				if (!buffer->isFull()){
					if (isXON == false){
						cout << "XON activate" << endl;
						isXON = true;char b[1];
						b[0] = XON;
						if (sendto(*sockfd, b, CHECKSUMSIZE+5, 0, (struct sockaddr *)transmitter, addrlen) < 0){
							throw "Error sending signal";
						} 
					}
					if (sendto(*sockfd, handler.getSerialized(), CHECKSUMSIZE+5, 0, (struct sockaddr *)transmitter, addrlen) < 0){
						throw "Error sending signal";
					}
				}
				else{
					isXON = false;
					char b[1];
					b[0] = XOFF;
					cout << "Sending XOFF, buffer is full" << endl;
					if (sendto(*sockfd, b, CHECKSUMSIZE+5, 0, (struct sockaddr *)transmitter, addrlen) < 0){
						throw "Error sending signal";
					}
				}
				
			}
			usleep(DELAY * 3000);
		}
		priority_queue<Frame, vector<Frame>, FrameComparator> data_all(buffer->getDataAll());

		// PRINT COMPLETE MESSAGE
		cout << endl << endl << "Here's the complete message :"<<endl << endl;
		while(!data_all.empty()) {
			Frame x = data_all.top();
			if ((x.getData())[0] == Endfile){
				data_all.pop();
				break;
			}
			cout << x.getData();
			data_all.pop();
		}
		cout << endl;
	}

	/* GETTER RECEIVER BOUND ADDRESS */
	string getAddress(){
		return address;
	}

	/* GETTER RECEIVER BOUND PORT */
	string getPort(){
		return port;
	}

	/* CLOSE SOCKET */
	void closeSocket(){
		close(sockfd);
	}
	
private:
	int sockfd; // Opened socket
	struct sockaddr_in Receiver_Endpoint; // Receiver endpoint
	struct sockaddr_in Transmitter_Endpoint; // Transmitter endpoint
	char address[INET_ADDRSTRLEN]; // Receiver bound address
	const char* port; // Receiver bound port
	Buffer buffer; // Buffer
};


int main(int argc, char const *argv[]){
	try{
		if (argc < 2){ // Parameter checking
			throw "Usage: Receiver <port>";
		} 
		
		Receiver rcvr(argv[1]); // Create Receiver object

	} catch (const char* msg) { // Exception handling
		cerr << msg << endl;
	} catch(std::exception& e){
		cerr << "Exception: " << e.what() << endl;  // Unhandled exception
	}

	return 0;
}