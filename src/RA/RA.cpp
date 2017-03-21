/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RA.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "RA.h"


/**
 * @brief Classic checksum algorithm (used in IP headers and ICMP headers)
 * @param buffer
 * @param size
 * @return
 */
ushort checksum(unsigned short *buffer, int size) {
	unsigned long checksum = 0;
	while (size > 1) {
		checksum += *buffer++;
		size -= sizeof(ushort);
	}
	if (size) {
		checksum += *(unsigned char*) buffer;
	}
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);
	return (unsigned short) (~checksum);
}

//! \brief Calculate the TCP checksum.
//! \param buff The TCP packet.
//! \param len The size of the TCP packet.
//! \param src_addr The IP source address (in network format).
//! \param dest_addr The IP destination address (in network format).
//! \return The result of the checksum.
u_int16_t tcpChecksum(const void *buff, size_t len, IPv4Address src_addr, IPv4Address dest_addr)
{
    const u_int16_t *buf= (const u_int16_t*)buff;
    u_int16_t *ip_src=(u_int16_t *)&src_addr, *ip_dst=(u_int16_t *)&dest_addr;
    u_int32_t sum;
    size_t length=len;

    //! Calculate the sum
    sum = 0;
    while (len > 1)
    {
        sum += *buf++;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        len -= 2;
    }

    if ( len & 1 )
        //! Add the padding if the packet lenght is odd
        sum += *((u_int8_t *)buf);

    //! Add the pseudo-header
    sum += *(ip_src++);
    sum += *ip_src;
    sum += *(ip_dst++);
    sum += *ip_dst;
    sum += htons(IPPROTO_TCP);
    sum += htons(length);

    //! Add the carries
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    //! Return the one's complement of sum
    return ( (u_int16_t)(~sum)  );
}

/**
 * @brief Update the ICMP checksum
 * @param p : the packet
 */
void updateChecksumICMP(DataPacket &p) {
    struct icmphdr * icmpheader = (icmphdr *) (p.msg + sizeof(PanaceaHeader) + sizeof(iphdr));
    icmpheader->checksum = 0;
    icmpheader->checksum = checksum((unsigned short*)icmpheader, p.lenght - sizeof(PanaceaHeader) - sizeof(iphdr));
}

/**
 * @brief Disable the UDP checksum (checksum = 0)
 * @param p : the packet
 */
void disableChecksumUDP(DataPacket &p) {
    struct udphdr * udpheader = (udphdr *) (p.msg + sizeof(PanaceaHeader) + sizeof(iphdr));
    udpheader->check = 0; // désactivation du checksum UDP
}

/**
 * @brief Update the TCP checksum
 * @param p : the packet
 */
void updateChecksumTCP(DataPacket &p) {
    struct tcphdr * tcpheader = (tcphdr *) (p.msg + sizeof(PanaceaHeader) + sizeof(iphdr));
    tcpheader->check = 0; //! Necessary before calculating the true checksum TCP
    tcpheader->check = tcpChecksum(tcpheader, p.lenght - sizeof(PanaceaHeader) - sizeof(iphdr), p.originaliphdr->saddr, p.originaliphdr->daddr);
}

//-----------------------------------------------------------------------------------------------


ReceptionAgent::ReceptionAgent(){
	// create the socket to receive data packet
	socketDescription = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketDescription == -1) {
		std::cout << "Error creating UDP socket\n";
		exit(1);
	}

	bzero(&serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(DATA_PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY );

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

	// create the socket toward the application
	raw = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (raw == -1) {
		std::cout << "Error creating RAW socket\n";
		exit(1);
	}

	const int on = 1;
	if (setsockopt(raw, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		std::cerr << "setsockopt() failed to set IP_HDRINCL fd" << std::endl;
	}
	
	if (setsockopt(raw, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) == -1) {
	  std::cout << "Error changing UDP socket buffer size\n";
		exit(1);
	}
	

	readConfigurationFromFile();

}

//! Initialize the private IP from a file
void ReceptionAgent::readConfigurationFromFile()
{
    std::string line;
    std::ifstream in("mylocalip");
    if (in.is_open()){
        getline(in, line);
        myIPPrivate = inet_addr(line.c_str());
        in.close();
    }

}

//! desencapsulate the data packet and send the original message to the application
void ReceptionAgent::desencapsulate(){
	dataPacket.lenght = recvfrom(socketDescription, dataPacket.msg, BUFLENGTH /*sizeof(packet) - 1*/, 0, NULL, 0);
    dataPacket.originaliphdr = dataPacket.getIPv4Header();
    dataPacket.originaliphdr->saddr = dataPacket.getPanaceaHeader()->sourceAddress;
	dataPacket.originaliphdr->daddr = myIPPrivate; // insertion de l'adresse locale pour redirection correcte
	dataPacket.originaliphdr->check = checksum((unsigned short*) dataPacket.originaliphdr, sizeof(iphdr));

    //std::cout << "Received Packet on RA" << std::endl;
	switch (dataPacket.originaliphdr->protocol) {
		case IPPROTO_ICMP:
				updateChecksumICMP(dataPacket);
				break;
		case IPPROTO_UDP :
				disableChecksumUDP(dataPacket);
				break;
		case IPPROTO_TCP :
				updateChecksumTCP(dataPacket);
				break;
		default:
				break;
	}
}

void ReceptionAgent::run() {
	while (true) {

		desencapsulate();

		int res = htons(dataPacket.getIPv4Header()->tot_len);\
		struct sockaddr_in clientAddress;
		bzero(&clientAddress, sizeof(clientAddress));
		clientAddress.sin_family = AF_INET;
		clientAddress.sin_addr.s_addr = dataPacket.getIPv4Header()->daddr;

		memcpy(packet.msg, dataPacket.msg + sizeof(PanaceaHeader), res);
		packet.lenght = res;

		if (sendto(raw, (char*) packet.msg, packet.lenght, 0,
				(struct sockaddr*) &clientAddress, sizeof(sockaddr_in)) < 0) {
			std::cerr << "sendto() failed fd" << std::endl;
		}
	
	}
}

