/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file PathManager.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */

#include "PathManager.h"


void PathManager::monitorCommunications() {

    do {
        int socket_desc , c;
        struct sockaddr_in server , client;

        //Create socket
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1)
        {
            std::cerr << "Could not create socket" << std::endl;
        }

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons( ROUTER_MONITOR_COMMUNICATION_PORT);

        int so_reuseaddr = 1;
        int z = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR,   &so_reuseaddr,  sizeof so_reuseaddr);

        //Bind
        if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
        {
            //print the error message
            perror("bind failed. Error");
            exit(-1);
        }

        //Listen
        listen(socket_desc , SOMAXCONN);

        //Accept an incoming connection
        std::cout << "Waiting for monitor connection..." << std::endl;

        c = sizeof(struct sockaddr_in);
        socketMonitorManager = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

        if (socketMonitorManager < 0) {
            perror("accept failed");
        }

        monitorManagerIPAddr = client.sin_addr.s_addr;
        std::cout << "Monitor connection success" << std::endl;

        isMonitorConnected = true;

        while(isMonitorConnected) {
            PathEdgesResultPacket packet;
            size_t received = recv(socketMonitorManager, (void*)&packet , sizeof(packet) , 0);

            if (!received) {
                isMonitorConnected = false;
                std::cerr << "Connection closed by monitor manager" << std::endl;
                break;
            }
            PathEdgesResult result;
            for (size_t i = 0; i< packet.pathLength; i++) {
                result.path.push_back(packet.path[i]);
                result.edgesRTT.push_back(packet.edgesRTT[i]);
            }
            result.metric = packet.metric;

            this->routingManager->putProbeResults(result);
        }

        close(socketMonitorManager);
        close(socket_desc);

    } while (true);
}


void PathManager::start() {

    //readConfigurationFromFile();

    monitorCommunicationsThread = std::thread(&PathManager::monitorCommunications, this);
    TACommunicationsThread = std::thread(&PathManager::waitForTransmissionAgentConnection, this);
    forwarderCommunicationThread = std::thread(&PathManager::waitForForwarderConnection, this);
}


void PathManager::setMonitorRoute(IPv4Address destinationMonitor, IPRoute monitorRoute) {

    // conversion from monitorRoute to forwarderRoute
    IPRoute forwarderRoute;
    for (IPv4Address monitor : monitorRoute) {
        forwarderRoute.push_back(forwarderFromMonitor[monitor]);
    }

    for (size_t j=0;j<forwarderRoute.size(); j++){
    	std::cout << IPToString(forwarderRoute.at(j)) << " " ;
    }
    std::cout << std::endl;

    IPv4Address destinationFowarder = forwarderFromMonitor[destinationMonitor];

    // send forwarderRoute to forwarder
    sendRouteToForwarder(destinationFowarder, forwarderRoute);
    std::cout << "send route to forwarder" << std::endl;

    // send Interception Notifications
    bool intercept;
    if (forwarderRoute.size() <= 1) { // if path direct or null ==> stop packet interception
        intercept = false;
    } else { // triggers the packet interception at TA
        intercept = true;
    }
    std::vector<IPv4Address> dest;
    for(std::map<IPv4Address,std::vector<IPv4Address>>::iterator it = mapClouds.begin(); it != mapClouds.end(); it++){
    	IPv4Address forwarder = it->first;
    	if(forwarder==destinationFowarder){
    		dest=it->second;
    	}
    }
    sendInterceptionNotification(destinationFowarder, intercept, dest);
}


void PathManager::sendRouteToForwarder(IPv4Address destinationFowarder, IPRoute forwarderRoute){
	DestinationAndAssignedRoute packet;
	packet.destination = destinationFowarder;
	for(size_t i=0; i<forwarderRoute.size(); i++) {
		packet.route[i]=forwarderRoute[i];
	}
	packet.Lenght = forwarderRoute.size();

	write(socketForwarder,(void*)&packet, sizeof(packet));
}

/*
static int sendVMNoficationPacket(int socket, InterceptionNotificationPacket p) {
    unsigned char buff[BUFLENGTH];
    p.proxyDest = htonl(p.proxyDest);
    p.doInterception = htons(p.doInterception);
    memcpy(&buff[0], &p.proxyDest, sizeof(p.proxyDest));
    memcpy(&buff[sizeof(p.proxyDest)], (void*)&p.doInterception, sizeof(p.doInterception));
    size_t size = write(socket , (void*)&buff[0] , sizeof(p.proxyDest)+sizeof(p.doInterception));
    return size==(sizeof(p.doInterception)+sizeof(p.proxyDest));
}
*/


void PathManager::sendInterceptionNotification(IPv4Address distantForwarder, bool doInterception, std::vector<IPv4Address> dest)
{
    InterceptionNotificationPacket p;
    p.doInterception = doInterception;
    p.proxy = distantForwarder;

    for(size_t i=0; i<dest.size(); i++){
    	p.destIP[i]=dest.at(i);
    }
    p.lenght=dest.size();

    if (socketsTAs.size() > 0) {
        // for each TA registered behind this router, send interception notification message
        for (std::map<IPv4Address, int>::iterator it = socketsTAs.begin(); it != socketsTAs.end(); it++) {
        	write(it->second , (void*)&p , sizeof(p));

            //if (!sendVMNoficationPacket(it->second, p))
            //    perror ("wrong sended packet size");
        }
    }
}

void PathManager::waitForForwarderConnection() {

    int  c, server_sock;
    struct sockaddr_in server , client;

    //Create socket
    server_sock = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sock == -1)
    {
        std::cerr << "Could not create socket" << std::endl;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( ROUTER_FORWARDER_COMMUNICATION_PORT );

    int so_reuseaddr = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,   &so_reuseaddr,  sizeof so_reuseaddr);

    //Bind
    if( bind(server_sock,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        std::cout << "bind failed. Error" << std::endl;
        //perror("bind failed. Error");
        exit(-1);
    }

    //Listen
    listen(server_sock , SOMAXCONN);

    //Accept an incoming connection
    std::cout << "Waiting for incoming forwarder connections..." << std::endl;

    c = sizeof(struct sockaddr_in);
    socketForwarder = accept(server_sock, (struct sockaddr *)&client, (socklen_t*)&c);

    forwarderIPAddr = client.sin_addr.s_addr;
    std::cout << "Forwarder connection success" << std::endl;

    isForwarderConnected = true;

    if (socketForwarder < 0)
    {
        perror("accept failed");
        exit(-1);
    }
    std::cout << "Forwarder registered : " << IPToString(client.sin_addr.s_addr) << std::endl;

}

void PathManager::waitForTransmissionAgentConnection() {

    int client_sock , c;
    struct sockaddr_in server , client;

    //Create socket
    socketTAServer = socket(AF_INET , SOCK_STREAM , 0);
    if (socketTAServer == -1)
    {
        std::cerr << "Could not create socket" << std::endl;
    }

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( ROUTER_TA_COMMUNICATION_PORT );

    int so_reuseaddr = 1;
    setsockopt(socketTAServer, SOL_SOCKET, SO_REUSEADDR,   &so_reuseaddr,  sizeof so_reuseaddr);

    //Bind
    if( bind(socketTAServer,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        std::cout << "bind failed. Error" << std::endl;
        //perror("bind failed. Error");
        exit(-1);
    }

    //Listen
    listen(socketTAServer , SOMAXCONN);

    c = sizeof(struct sockaddr_in);
    while(true)
    {
    	//Accept an incoming connection
    	std::cout << "Waiting for incoming vm connections..." << std::endl;
    	client_sock = accept(socketTAServer, (struct sockaddr *)&client, (socklen_t*)&c);
        socketsTAs[client.sin_addr.s_addr] = client_sock;
        write(client_sock,(void*)&forwarderIPAddr, sizeof(forwarderIPAddr));

        std::cout << "New Transmission agent registered : " << IPToString(client.sin_addr.s_addr) << std::endl;

        isTransmissionAgentConnected=true;
        transmissionAgentIP = client.sin_addr.s_addr;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        isTransmissionAgentConnected=false;

    }

    if (client_sock < 0)
    {
        perror("accept failed");
    }

    close(socketTAServer);
}


void PathManager::doMonitorPaths(MetricType metric, std::vector<IPPath> paths)  {

    for (const IPPath & path : paths) {
        MonitoringInstructionPacket packet;

        packet.metric = metric;
        for (size_t i=0; i< path.size(); i++) {
            packet.path[i] = path.at(i);
        }
        packet.pathLength = path.size();

        /*
        for (size_t j=0;j<packet.pathLength; j++){
        	std::cout << IPToString(packet.path[j]) << " " ;
        }
        std::cout << std::endl;
         */

        if (isMonitorConnected) {
            write(socketMonitorManager, (void*)&packet , sizeof(packet));
            //std::cout << "send monitoring instruction to monitor agent" << std::endl;
        } else {
            std::cerr << "Monitor disconnected, cannot send monitoring instructions" << std::endl;
        }

    }
}

