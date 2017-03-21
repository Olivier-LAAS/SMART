/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file monitor.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 */

#include "monitor.h"
#include <sstream>


MonitorManager::MonitorManager(std::string routerip, std::string monitorip)
{
//	getIPadress();

	if(routerip==" "){
		readConfigurationFromFile();
		std::cout << "readConfigurationFromFile() " << std::endl;
	}else{
		routerIPAddr = inet_addr(routerip.c_str());
		std::cout << "router IP " << IPToString(routerIPAddr) << std::endl;
	}

	if(monitorip==" "){
		std::cout << "monitor IP invalid " << std::endl;
		exit(-1);
	}else{
		myIPAddr = inet_addr(monitorip.c_str());
		std::cout << "monitor IP " << IPToString(myIPAddr) << std::endl;
	}

}

void MonitorManager::getIPadress(){
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1); //! TODO careful

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	myIPAddr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	std::cout << IPToString(myIPAddr) << std::endl;
}

void MonitorManager::readConfigurationFromFile(){
	 std::string line;
	 std::ifstream in("routerip");
	 if (in.is_open()) {
		 getline(in, line);
		 routerIPAddr = inet_addr(line.c_str());
		 in.close();
     }
}

void MonitorManager::connectToRouter() {
    struct sockaddr_in addr;

    //Create socket
    socketRouter = socket(AF_INET , SOCK_STREAM , 0);
    if (socketRouter == -1)
    {
        std::cerr << "Could not create socket for incoming proxy notifications" << std::endl;
    }

    addr.sin_addr.s_addr = routerIPAddr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( ROUTER_MONITOR_COMMUNICATION_PORT);

    // Connection TCP to the router
    std::cout << ("Trying to connect to router...") << std::endl;
    while ( connect(socketRouter , (struct sockaddr *)&addr , sizeof(addr)) < 0 ) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cout << "Connected to router" << std::endl;
}

void MonitorManager::main(MetricType mode)
{
    std::cout << "MonitorManager running..." << std::endl;
    connectToRouter();

    while(true) {

        MonitoringInstructionPacket packet;
        size_t received = recv(this->socketRouter, (void*)&packet , sizeof(packet) , 0);
        if (received == 0 ) {
            std::cerr << "Router killed the connexion" << std::endl;
            close(this->socketRouter);
            connectToRouter();
        }

        std::vector<IPPath> paths;

        IPPath path;
        for (size_t i = 0; i< packet.pathLength; i++) {
            path.push_back(packet.path[i]);
        }
        paths.push_back(path);

        //std::vector<IPPath> paths = readPathsFromUserCommandLine();

        // ask for measurement on the paths
	doMonitorPaths(mode, paths); 
	//doMonitorPaths(BW_METRIC, paths); //! TODO
	//doMonitorPaths(LATENCY_METRIC, paths); //! TODO
    }
}

std::vector<IPPath> MonitorManager::readPathsFromUserCommandLine() {
    std::cout << "Please enter IP path to test latency (e.g \"10.0.0.10,10.0.0.12\"): " << std::endl;
    std::string IPs;
    std::cin >> IPs; // read the line entered by the user

    // cut the route IPs given by the user
    std::istringstream ss( IPs );
    IPPath path;
    while (ss) {
      std::string s;
      if (!getline( ss, s, ',' )) break;
      path.push_back( inet_addr(s.c_str() )); // add IP to the route
    }

    std::vector<IPPath> paths;
    paths.push_back(path);
    return paths;
}

void MonitorManager::putProbeResults(MetricType metric, const IPPath &path, const std::vector<double> &results)
{
    PathEdgesResultPacket packet;

    for (size_t i = 0; i < path.size(); i++){
        packet.path[i] = path.at(i);
        packet.edgesRTT[i] = results.at(i);
    }
    packet.pathLength = path.size();
    packet.metric = metric;

    write(this->socketRouter, (void*)&packet , sizeof(packet));
}


void MonitorManager::addProbeAgent(ProbeInterface *agent)
{
    // check if pointer is not null
    if (agent) {
        // put the agent according to its kind of metric
        probeAgents[agent->getMetric()] = agent;
    }
}

IPv4Address MonitorManager::getMyAddr()
{
    return myIPAddr;
}
IPv4Address MonitorManager::getRouterIPAddr() const
{
    return routerIPAddr;
}

void MonitorManager::doMonitorPaths(MetricType metric, std::vector<IPPath> &paths)
{
    if ( probeAgents.find(metric) != probeAgents.end() ) {
        probeAgents[metric]->doMonitorPaths(paths);
    } else {
        std::cout << "Metric " << metric << " is not implemented yet" << std::endl;
        exit(-1);
    }
}

