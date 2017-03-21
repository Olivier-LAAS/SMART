/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file daemon.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */


#ifndef DAEMON_H
#define DAEMON_H

#include "common/common.h"
#include "RoutingManagerInterface.h"
#include "PathManager.h"

/*!
 * \brief Daemon on each router connects to CU in order to communicate about configuration
 *
 */

class DaemonManager {
public :
		DaemonManager(int assignedRole);

		// Configuration
		/*!
		 * Accessor to get the Routing Manager
		 */
		void setRoutingManager(RoutingManagerInterface *  routingManager) {this->routingManager = routingManager;}

		/*!
		 * Accessor to get the Path Manager
		 */
		void setPathManager(PathManager* manager) { if (manager) pathManager = manager;}

		/*!
		 * Procedure to connect to CU
		 */
		void connectToCU();

		/*!
		 * Sending the entity information
		 */
		void sendMyInfo();

		/*!
		 * Watchdog socket to listen to CU updates
		 */
		void CUCommunication();

		/*!
		 * Get first updates about other clouds from CU
		 */
		int firstUpdate();

		/*!
		 * To be called from CUCommunication. Updates other clouds info
		 * @param newCloud the received update packet from CU
		 */
		void updateInfo(InfoCloudPacket newCloud);

		/*!
		 * @param connectCU a varaible to know whether connected or not to CU
		 */
		bool connectCU;

		void setCUIP(string ip_) { CUIP=inet_addr(ip_.c_str()); }
private :
		RoutingManagerInterface * routingManager;
		PathManager * pathManager;

		/*!
		 * @param socketCU intermediate variable for socket creation
		 * @param id to be removed
		 */
		int socketCU, id;

		/*!
		 * @param CUAddress intermediate variable to create CU socket
		 */
		struct sockaddr_in CUAddress;

		/*!
		 * @param CUIP The hardcoded IP address of CU
		 */
		IPv4Address CUIP;

		/*!
		 * @param role the node role (MASTER, SLAVE ...)
		 */
		int role;

		/*!
		 * @param newInfoCloudThread the thread used for communicating local clients information to CU (if any)
		 */
		std::thread newInfoCloudThread;
};

#endif


