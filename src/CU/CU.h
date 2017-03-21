/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file DC.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#include "common/common.h"
#include <pthread.h>
#include <algorithm>



/*!
 * \brief This structure contains cloud information. By cloud we mean Router, Monitor, Forwarder and clients entities per cloud.
 * @param role Cloud role (Master, Salve...)
 * @param monitor IPv4 Monitor address
 * @param forwarder IPv4 Forwarder address
 * @param clients its a vector of all clients IPv4 addresses
 */
typedef struct {
	int role;
	IPv4Address monitor;
	IPv4Address forwarder;
	std::vector<IPv4Address> clients;
}Cloud;


/*!
 * \brief CU central unit class
 * CU is the central UNIT
 * CU waits for cloud connection
 * It registers new cloud informations
 * Then it updates other cloud informations
 */
class CU{
public :
		CU();
		~CU();
		void waitForCloudConnection();
		void readGraphCommunication();
		static void* staticRun(void* arg){
			return ((CU*)arg)->run();
		}

		void* run();

		/*!
		 * \brief first function to call to register new cloud information
		 * @param soc_thread the cloud thread
		 * @param role the cloud role
		 */
		void firstUpdateCloudList(int sock_thread, int& role);

		/*!
		 * \brief If other clouds were already registered, CU communicates their information to the newly registered cloud
		 * @param soc_thread the cloud thread
		 * @param role the cloud role
		 */
		void firstConectionUpdate(int sock_thread, int role);

		/*!
		 * \brief Handles all updates of one cloud. There is one thread per cloud
		 * @param soc_thread the cloud thread
		 * @param role the cloud role
		 */
		void updateCloudList(int sock_thread);

		/*!
		 * \brief Handles updates from other clouds(if any).
		 * @param soc_thread the cloud thread
		 * @param role the cloud role
		 */

		void updateMyCloud(int sock_thread, int role);

		/*!
		 * \brief Print cloud list for debug
		 */
		void writeListCloud();


private :
		/*!
		 * \brief Main variable. This map contains all the information of all connected clouds
		 * @param int the number of the socket associated with a cloud
		 * @param Cloud the cloud information structure
		 */
		std::map<int,Cloud> listClouds;

		/*!
		 * \brief Main variable. This map contains all the information of the graph of communication
		 * @param int a role
		 * @param std::vector<int> list of id of the destinations
		 */
		std::map<int,std::vector<int> > graph;

        /*!
         * @param socketThread intermediate variable for socket creation
         * @param c intermediate variable to accept a cloud
         */
		int socketThread,c;

		/*!
		 * @param new_sock the socket number of the last connected cloud
		 * @param listening_sock The CU general socket. When a cloud is connected it will have its own socket.
		 */
		int lisening_sock, new_sock;

		/*!
		 *Intermediate variables that can be moved inside code
		 */
		struct sockaddr_in server , client;

		/*!
		 * Intermediate thread to prevent multiple threads to access variables simultaneously
		 */
		pthread_mutex_t mutex;

		/*!
		 * Used as a flag to notify other threads of new information
		 */
		pthread_cond_t  condition_var;

		/*!
		 *The socket number of the last modified cloud. Useful to retrieve its information by other clouds.
		 */
		int lastCloudModified;

		/*!
		 * The type of information to be retrieved from @lastCloudModified
		 */
		u_int32_t type;

		/*!
		 * False if no clouds registered yet. True if at least one cloud is registered. (the case of all clouds disconnecting is to be handled)
		 */
		bool firstConnexion;

};

