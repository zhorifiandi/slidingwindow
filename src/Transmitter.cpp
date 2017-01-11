#include "Transmitter.h"
#include "Algorithm.cpp"

vector<Frame> frame_vector;
struct sockaddr_in my_address;
struct sockaddr_in remote_address;
socklen_t socket_length = sizeof(remote_address);
int* ack_status;
int sockfd;
bool isFinish = false;

void initialization(string server, int port, string filename){
	 // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket. \n");
        exit(1);
    }

    memset((char *)&my_address, 0, sizeof(my_address));
   	my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(0);

    // Bind it to local address and pick any port number
    if (bind(sockfd, (struct sockaddr *)&my_address, sizeof(my_address)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Define remote address whom we want to send messages
    memset((char *) &remote_address, 0, sizeof(remote_address));
    remote_address.sin_family = AF_INET;
    remote_address.sin_port = htons(port);

    // Convert IPV4 numbers and dots notation into binary form
    // If want to see NAK, remove this
    if (inet_aton(server.c_str(), &remote_address.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
}


void inputFile(string filename) {
    ifstream CurrentFile;
    CurrentFile.open(filename.c_str(), ios::in);

    int charCounts = 0;
    int frameNumber = 0;
    
    char current;
    char tmp_char[DATASIZE];
    memset(tmp_char, 0, sizeof(tmp_char));

    while (CurrentFile.get(current)) {
        //cout << current;
        charCounts++;
        tmp_char[(charCounts-1) % (DATASIZE - 1)] = current; 
        
        if (charCounts % (DATASIZE - 1) == 0) {
            Frame f(frameNumber, tmp_char);
            frame_vector.push_back(f);
            frameNumber++;

            memset(tmp_char, 0, sizeof(tmp_char));
        }
    }
    
    if (charCounts % (DATASIZE - 1) != 0) {
        Frame f(frameNumber, tmp_char);
        frame_vector.push_back(f);
    }

    // Create EOF
    char data_eof[DATASIZE];
    memset(data_eof, 0, sizeof(data_eof));
    data_eof[0] = Endfile;
    Frame frame_eof(frame_vector.size(), data_eof);
    frame_vector.push_back(frame_eof);

    // Initialize status table to -1
    ack_status = new int[frame_vector.size()];
    for (int i = 0; i < frame_vector.size(); i++) {
        ack_status[i] = -1;
    }
}

// if got ACK, change ack_status to 1
// if got NAK, change ack_status to-1
void acknowledgeReceiver() {
  while(!isFinish){
      char buffer[5 + CHECKSUMSIZE];
      if (!isFinish) {
          int recvlen = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_address, &socket_length);
          if (recvlen > 0) {
           	//Handling frame received
               if (buffer[0] == XOFF){
               		while (1){
               			cout << "Waiting for XON" << endl;
               			sleep(1);
               			int smtg = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&remote_address, &socket_length);
               			if (buffer[0] == XON)
               				break;
               		}
               }
               else{
               		Algorithm handler(buffer);
	               	if (handler.getSerialized()[0] == ACK) {
	                    ack_status[handler.getNumber()] = 1;
	                    cout << "ACK received for frame " << handler.getNumber() << endl;
	                } else if (handler.getSerialized()[0] == NAK) {
	                    ack_status[handler.getNumber()] = -1;
	                    cout << "NAK received for frame " << handler.getNumber() << endl;
	                } else {
	                   cout << "WHAT THE HELL???" << endl;
	                }
               }
               
           }
       }
	}
}

// Changing ack_status every time
//  1  : frame already receive ack
//  0  : frame already sent but hasn't received ack
// -1  : frame hasn't been sent or got nak 

void executeTimeOut() {
    while(!isFinish) {
        sleep(TIMEOUT);
        //IF AFTER TIMEOUR THERE'S STILL 0 VALUE, THEN MAKE IT -1 SO WE MUST SEND IT AGAIN
        for (int x = 0; x < frame_vector.size(); x++) {
            if (ack_status[x] == 0) {
            	cout <<"Selective Repeat Request !!"<< endl;
                ack_status[x] = -1;
            }
        }
    }
}


void sendMessageToRemote(string filename){
	inputFile(filename);

	int first = 0;
	int last = WINSIZE;

	int i = first;
	while (first != last){
		for (int i = first; i < last; ++i)
		{
			if (ack_status[i] == -1){
				char* msg = frame_vector[i].getSerialized();
				cout << "Sending Frame "<<frame_vector[i].getNumber() <<" ....." << endl;
				//CHANGE 32 if wanna see NAK!!
				if (sendto(sockfd, msg,DATASIZE+15,0,(struct sockaddr *)&remote_address, socket_length) == -1){
					perror("Trouble in sending message, Exit...");
					exit(1);
				}
				//Already sent message, but not yet received ACK
				ack_status[i] = 0;
			}
			//If already received message, slide window
			if(ack_status[first] == 1){
				first++;
				//For safety reason
				if (last != frame_vector.size())
					last++;
			}
			usleep(100);
		}
	}
}

void startMultiThreading(string filename){
	std::thread sendMessageThread(sendMessageToRemote, string(filename));
    std::thread receiveThread(acknowledgeReceiver);
    std::thread timeOutThread(executeTimeOut);
    sendMessageThread.join();
    receiveThread.join();
    timeOutThread.join();
}

int main(int argc, char const *argv[]) {

    if (argc < 4) {
        cout << "Usage: ./transmitter <host> <port> <filename>" << endl;
        return 0;
    }

    string server(argv[1]);
    int port = atoi(argv[2]);
    string filename(argv[3]);

    initialization(server,port,filename);

    startMultiThreading(filename);

	return 0;
}