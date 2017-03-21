/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RA.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#ifndef RA_H_
#define RA_H_

#include "common/common.h"


/*
 * Please look at RA.cpp to have more comments on following attributes
 */

ushort checksum(unsigned short *buffer, int size);

u_int16_t tcpChecksum(const void *buff, size_t len, IPv4Address src_addr, IPv4Address dest_addr);

void updateChecksumICMP(DataPacket &p);

void disableChecksumUDP(DataPacket &p);

void updateChecksumTCP(DataPacket &p);

/*!
 * @brief The reception agent class, it is used to desencapsulate packets and deliver them to destination
 * @param dataPacket SMART packet with additional header
 * @param packet original packet to be delivered to destination
 * @param socketDescription the socket used to receive dataPackets
 * @param raw the socket used to communicate with destination application
 * @param serverAddress a structure to create dataPacket socket
 * @param the private IP address of the reception client. It is important for NAT handling.
 * If only public IP addresses are used the private address is equal to the public address
 */
class ReceptionAgent {
public :
		ReceptionAgent();
		/*!
		 * the function that desencapsulates SMART dataPacket
		 */
		void desencapsulate();
		/*!
		 * Only used to read @myIPPrivate
		 */
		void readConfigurationFromFile();
		/*!
		 * Tha main process function
		 */
		void run();

		DataPacket dataPacket;
		Packet packet;

		int socketDescription, raw;
		struct sockaddr_in serverAddress;

        IPv4Address  myIPPrivate;

};


#endif
