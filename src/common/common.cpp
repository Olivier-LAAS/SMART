/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file common.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */


#include "common.h"



Packet::Packet() : lenght(0) {

}


/*!
 *  \brief Print the value of the packet. The format used is the same as the wireshark display.
 */
void Packet::display() {
	int j = 1;
	for (unsigned int i = 0; i < lenght; i++) {
		if ((j % 17) == 0) {
			j = 1;
			printf("\n");
		}

		if ((j + 1) % 2 == 0)
			printf(" ");

		if (msg[i] <= 15)
			printf("0%1X", msg[i]);
		else
			printf("%1X", msg[i]);
		j++;
	}
	std::cout << std::endl << std::endl;
}

//----------------------------------------------------------------------

DataPacket::DataPacket() : Packet(),originaliphdr(NULL) {

}


//! Initialization Method for the pointer PanaceaHeader which point to the panacea header of data packet
PanaceaHeader * DataPacket::getPanaceaHeader() {
    return (PanaceaHeader *) msg;
}

//! Initialization Method for the pointer IPv4Header which point to the original IP header of data packet
IPv4Header * DataPacket::getIPv4Header() {
    return (IPv4Header *) (msg + sizeof(PanaceaHeader));
}

//----------------------------------------------------------------------

/**
 *
 * Create the network interface for the sending of packet in transit
 * and initialize the static variables of the destination address.
 *
 */
ForwarderInterface::ForwarderInterface(int PORT){
	socketDescription = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketDescription == -1) {
		std::cout << "Error creating UDP socket " << std::endl;
		exit(1);
	}

    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    int err = bind(socketDescription, (struct sockaddr *) &serverAddress, sizeof(sockaddr));
	if (err < 0) {
		std::cout << "Error binding socket" << err << std::endl;
		exit(1);
	}
	
	int bufferSize = 1024*4096;
	if (setsockopt(socketDescription, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize)) == -1) {
	  std::cout << "Error changing UDP socket buffer size\n";
		exit(1);
	}
	
	if (setsockopt(socketDescription, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) == -1) {
	  std::cout << "Error changing UDP socket buffer size\n";
		exit(1);
	}
}

//----------------------------------------------------------------------



/*!
* Get time difference in ms
*/
long time_diff(struct timeval start , struct timeval finish)
{
    struct timeval res;
    timersub(&finish, &start, &res);
    return (long) (res.tv_sec*1000+res.tv_usec/1000);
}


std::string IPToString(const IPv4Address addr) {
    char strbuff[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, (char*)&strbuff[0], INET_ADDRSTRLEN);
    return std::string(strbuff);
}
