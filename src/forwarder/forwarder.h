/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file forwarder.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#ifndef FORWARDER_H_
#define FORWARDER_H_

#include "common/common.h"



/**
 * \brief Class for the routing table
 *
 * The routing table contains a table of different destinations
 * and the routes to use to go to it (only one route per destination)
 * @param socketRouter the socket to communicate with router to receive route updates
 * @param routerAddress the socket address associated with @socketRouter
 * @param routerIP the IP address of the router
 */


class RoutingTable {
public :
		RoutingTable(std::string routerip);
		/**
		 * @brief lookup for the route to use for the packet if not already set (the first forwarder)
		 */
        void lookupforpath(DataPacket & packet);
        /**
         * @brief This function listen to the socket connected to the router executable and when it receive a new
         * update destination-route start the function setPath to update the table
         */
		void routerComunication();
		/**
		 * @brief print routing table
		 */
		void routingTablePrint();
		/**
		 * @brief create the connection between router and forwarder
		 */
		void connectToRouter();

private :
	    void setPath(IPv4Address dest, IPRoute path);
		void readConfigurationFromFile();

		/*!
		 * @brief Routing table
		 *
		 * For each destination we associate a path
		 * A destination is an integer on 32 bits 54.92.22.35 = 5888667958
		 * A path is a table of address path[0] = 5888667958 ...
		 */
        std::map <IPv4Address, IPRoute > routingTable;

		int socketRouter;
        struct sockaddr_in routerAddress;
        IPv4Address routerIP;
};


/*!
 * @brief Forwarder for data packets
 */

class DataForwarder : public ForwarderInterface {
public :
		DataForwarder(std::string routerip=" ");
		void run();

private :
		/*!
		 * @brief this function decides whether to set the path in the packet header (if not already set)
		 * or to simply forward the packet to next forwarder
		 */
		void dispatch();
		/*!
		 * @brief this function forwards packet to next forwarder (called by dispatch)
		 */
		void forward();
		/*!
		 * @brief the waiting cycle of new packets to handle
		 */
		void process();

		DataPacket packet; /*! Variable for the received packet*/
		RoutingTable routingTable; /*! Reference towards the table of routing*/
		std::thread routingTableThread; /*! a thread for receiving router updates*/
};

#endif
