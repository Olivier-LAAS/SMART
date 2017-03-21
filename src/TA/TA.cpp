/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file TA.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "TA.h"
#include "common/common.h"


    
    struct sockaddr_in forwarderAddr;
    char message[BUFLENGTH];
    struct PanaceaHeader *pnchdr = new PanaceaHeader;
    

NfQueue::NfQueue() {
    /*!
     * readConfigurationFromFile is only called if routerip is not provided as an argument to the executable
     * As a consequence if nfqID value should be read from a file, this behavior should be changed also
     */
	nfqID = 0;
    isOpened = false;
}

NfQueue::~NfQueue()
{
    close();
}

void NfQueue::on_exit(int exit_value){
    // Cleanup and close up stuff here
    std::cerr << "Removing nfqueue" << std::endl;
    nfq_destroy_queue(myQueue);
    nfq_close(nfqHandle);
    exit(exit_value);
}

void NfQueue::open(nfq_callback *cb, void* callBackData)
{
    close();

    if (!(nfqHandle = nfq_open())) {
        std::cerr << "Error in nfq_open()" << std::endl;
        exit(-1);
    }
    if (nfq_unbind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "Error in nfq_unbind_pf()" << std::endl;
        exit(1);
    }
    if (nfq_bind_pf(nfqHandle, AF_INET) < 0) {
        std::cerr << "Error in nfq_bind_pf()" << std::endl;
        exit(1);
    }

    if (!(myQueue = nfq_create_queue(nfqHandle, nfqID, cb,  callBackData))) {
        std::cerr << "Error in nfq_create_queue()" << std::endl;
        exit(1);
    }
    if (nfq_set_mode(myQueue, NFQNL_COPY_PACKET, 0xffff) < 0) {
        std::cerr << "Could not set packet copy mode" << std::endl;
        on_exit(1);
    }

    netlinkHandle = nfq_nfnlh(nfqHandle);
    nfnl_rcvbufsiz(netlinkHandle, NFQLENGTH * BUFLENGTH);
    fd = nfnl_fd(netlinkHandle);

    int opt = 1;
    setsockopt(fd, SOL_NETLINK, NETLINK_NO_ENOBUFS, &opt, sizeof(int));

    isOpened = true;

    std::cout << "Created nfqueue number " << nfqID << std::endl;
}

void NfQueue::close()
{
    if (isOpened) {
        std::cerr << "Removing nfqueue" << std::endl;
        nfq_destroy_queue(myQueue);
        nfq_close(nfqHandle);
        isOpened = false;
    }
}

void NfQueue::run()
{
    std::cout << "Waiting packet on nfQueue.." << std::endl << std::endl;
    while ((res = recv(fd, buf, sizeof(buf), 0)) && res >= 0) {
        nfq_handle_packet(nfqHandle, buf, res);
    }
}

u_int16_t NfQueue::getNfqID() const
{
    return nfqID;
}

void NfQueue::setNfqID(const u_int16_t &value)
{
    nfqID = value;
}


//------------------------------------------------------------------------------


TransmissionAgent::TransmissionAgent(std::string routerip, std::string localip){
    if ((socketForwarder = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		printf("csocket");

    //getIPadress();
    if(routerip==" "){
    	readConfigurationFromFile();
    	std::cout << "readConfigurationFromFile() " << std::endl;
    }else{
    	routerIp = inet_addr(routerip.c_str());
    	std::cout << "router IP " << IPToString(routerIp) << std::endl;
    }
    if(localip==" "){
    	std::cout << "Invalid local (TA) IP " << std::endl;
	exit(-1);
    }else{
    	myIp = inet_addr(localip.c_str());
    	std::cout << "local (TA) IP " << IPToString(myIp) << std::endl;
    }
    
    int bufferSize = 1024*4096;
	if (setsockopt(socketForwarder, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize)) == -1) {
	  std::cout << "Error changing UDP socket buffer size\n";
		exit(1);
	}

}


/**
 * \brief send a data packet for testing
 *
 */
void TransmissionAgent::sendTestPacket()
{
    datapacket.getPanaceaHeader()->destinationForwarderAddress = inet_addr("10.0.0.10");
    datapacket.getPanaceaHeader()->sourceAddress = inet_addr("10.0.0.11");
    //datapacket.panacea_header()->path[0] = inet_addr("10.0.0.10");
    datapacket.getPanaceaHeader()->pathLength = 0;
    datapacket.getPanaceaHeader()->nextHop = 0;

    std::cout << "panaceaHeader" << std::endl;

    // Packet reinjection
    struct sockaddr_in forwarderAddr;
    bzero(&forwarderAddr, sizeof(forwarderAddr));
    forwarderAddr.sin_family = AF_INET;
    forwarderAddr.sin_port = htons(DATA_PORT);
    forwarderAddr.sin_addr.s_addr = forwarderIP;

    if (sendto(socketForwarder, &datapacket.msg, sizeof(PanaceaHeader), 0, (struct sockaddr*) &forwarderAddr, sizeof(forwarderAddr)) == -1){
            perror("error sendto()");
    }
    std::cout << "Data packet send with size =" << sizeof(PanaceaHeader) << std::endl;
}


void TransmissionAgent::run()
{
  pnchdr = (PanaceaHeader*) message;


    pnchdr->sourceAddress = myIp;
    pnchdr->pathLength = 0;
    pnchdr->nextHop = 0;
    
    // Packet reinjection
    bzero(&forwarderAddr, sizeof(forwarderAddr));
    forwarderAddr.sin_family = AF_INET;
    forwarderAddr.sin_port = htons(DATA_PORT);
    

    routerThread = std::thread(&TransmissionAgent::routerCommunication, this);

    std::cout << "router Thread launched" << std::endl;
   // sendTestPacket(); // TODO Remove

    nfQueue.open(&TransmissionAgent::encapsulate, (void*)this);
    nfQueue.run();
    std::cout << "exit main loop ==> kill the queue" << std::endl;
    nfQueue.close();
}

void TransmissionAgent::getIPadress(){
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	//strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
	strncpy(ifr.ifr_name, "gt0", IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	myIp = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
	std::cout << "my IP " << IPToString(myIp) << std::endl;
}

void TransmissionAgent::readConfigurationFromFile()
{
    std::string line;
    std::ifstream in("routerip");

    if (in.is_open()) {
        getline(in, line);
        routerIp = inet_addr(line.c_str());
        in.close();
    }

    /*this code should be replaced by the commented code below on case of multiple nfqueues*/

    std::string tmp = "0";
    nfQueue.setNfqID(atoi(tmp.c_str()));

    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
     *this code is important if nfqueue value should be different for multiple TAs on the same VM
     *!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /*
    in.open("nfqueueid");
    if (in.is_open()) {
        std::string tmp;
        getline(in, tmp);
        nfQueue.setNfqID(atoi(tmp.c_str()));
        in.close();
    }
    */
}


void TransmissionAgent::onExit() {
    // Cleanup and close up stuff here
    std::cerr << "Removing nfqueue" << std::endl;

    // disables interception for all intercepted IP
    std::cerr << "Disables SMART interception" << std::endl;
    for (std::map<IPv4Address, bool>::iterator it = doInterception.begin(); it != doInterception.end(); it++) {
        if (it->second) {
        	std::vector<std::string> ipsToIntercept;

        	for (std::map<IPv4Address, IPv4Address>::iterator itt = forwardersIP.begin(); itt != forwardersIP.end(); it++) {
        		if (itt->second == it->first) {
        			ipsToIntercept.push_back(IPToString(itt->first));
        		}
        	}
        	interceptIPs(false,ipsToIntercept);
        }
    }

    // close nfqueue
    nfQueue.close();

    close(socketForwarder);
    close(socketRouter);
    exit(-1);
}

void TransmissionAgent::routerCommunication(){

    //! Create socket
    socketRouter = socket(AF_INET , SOCK_STREAM , 0);
    if (socketRouter == -1)
    {
        std::cerr << "Could not create socket for incoming router notifications" << std::endl;
    }

    struct sockaddr_in routerAddress;
    routerAddress.sin_addr.s_addr = routerIp;
    routerAddress.sin_family = AF_INET;
    routerAddress.sin_port = htons( ROUTER_TA_COMMUNICATION_PORT );

    //! Connection TCP to the router
    std::cout << "Trying to connect to router..." << std::endl;
    while ( connect(socketRouter , (struct sockaddr *)&routerAddress , sizeof(routerAddress)) < 0 ) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    std::cout << "Connected to router" << std::endl;

    int rcv = recv(socketRouter, (void*)&forwarderIP, sizeof(forwarderIP), 0);

    //! Wait of message on behalf of the router
    InterceptionNotificationPacket notification;
    while (true) {
        //! Receive a reply from the server
        //int received = waitRouterNotificationPacket(&notification);
        int received = recv(socketRouter, (void*)&notification, sizeof(notification), 0);

        if (received == 0) {
            perror ("Closed the connection ! ==> Terminating program");
            onExit();
        } else if(received < 0) {
            perror ("Wrong size of packet received from router ==> Terminating program");
            onExit();
        }

        bool intercept = notification.doInterception;
        IPv4Address proxyDest = notification.proxy;

        if (doInterception[proxyDest] != intercept) {

            struct timeval tcurrent;
            gettimeofday(&tcurrent, NULL);
            std::cout << "T=" << tcurrent.tv_sec  << " : ";

            doInterception[proxyDest] = intercept;

            if (intercept) {
            	std::cout << "Start";
            } else {
            	std::cout << "Stop";
            }
            std::cout << " iptables interception for dst " << IPToString(proxyDest) << std::endl;

            std::vector<std::string> ipsToIntercept;

            for(size_t i=0; i<notification.lenght; i++){
            	forwardersIP[notification.destIP[i]]=proxyDest;
            	ipsToIntercept.push_back(IPToString(notification.destIP[i]));
            }
            interceptIPs(intercept,ipsToIntercept);
        }
    }
    close(socketRouter);
}

/*

int TransmissionAgent::waitRouterNotificationPacket(InterceptionNotificationPacket *p) {
    unsigned char buff[BUFLENGTH];
    size_t size=recv(socketRouter , (void*)&buff , sizeof(p->proxyDest)+sizeof(p->doInterception) , 0);
    if( size != sizeof(p->proxyDest)+sizeof(p->doInterception))
        return size;
    memcpy((void*)&p->proxyDest, &buff[0],  sizeof(p->proxyDest));
    memcpy((void*)&p->doInterception, &buff[sizeof(p->proxyDest)], sizeof(p->doInterception));
    p->proxyDest = ntohl(p->proxyDest);
    p->doInterception = ntohs(p->doInterception);
    return 1;
}
*/

//! function intercept IP from applications
void TransmissionAgent::interceptIPs(bool intercept, std::vector<std::string> dst) {
    // creation of the command line IPTable
    std::string cmdIpTables;
    std::string addOrRemove;
    if (intercept) {
        addOrRemove = "-A";
    } else {
        addOrRemove = "-D";
    }

    cmdIpTables = "iptables " + addOrRemove + " OUTPUT -j NFQUEUE -d ";
    bool first = true;
    for (std::vector<std::string>::iterator it = dst.begin(); it != dst.end(); it++) {
        if (first) {
            first = false;
        } else {
            cmdIpTables += ",";
        }
        cmdIpTables += *it;
    }
    //! We specify the number of the queue
    cmdIpTables += " --queue-num ";
    cmdIpTables += std::to_string(nfQueue.getNfqID());
    cmdIpTables += " > /dev/null 2>&1";
    //std::cout << "Cmd=" << cmdIpTables << std::endl;
    system(cmdIpTables.c_str()); // execute command iptable
}

void TransmissionAgent::interceptAllBehindProxy(bool intercept, IPv4Address forwarderDest) {
    if (intercept) {
        std::cout << "Start";
    } else {
        std::cout << "Stop";
    }
    std::cout << " iptables interception for dst " << IPToString(forwarderDest) << std::endl;

    std::vector<std::string> ipsToIntercept;

    for (std::map<IPv4Address, IPv4Address>::iterator it = forwardersIP.begin(); it != forwardersIP.end(); it++) {
        if (it->second == forwarderDest) {
            ipsToIntercept.push_back(IPToString(it->first));
        }
    }
    interceptIPs(intercept,ipsToIntercept);
}

int TransmissionAgent::encapsulate(nfq_q_handle *myQueue, nfgenmsg *msg, nfq_data *pkt, void *cbData) {

    TransmissionAgent * ta = (TransmissionAgent*) cbData;
    uint32_t id = 0;
    nfqnl_msg_packet_hdr *header;
    if ((header = nfq_get_msg_packet_hdr(pkt))) {
        id = ntohl(header->packet_id);
    }

    unsigned char *load;
    unsigned int loadLenght;
    loadLenght = nfq_get_payload(pkt, (unsigned char**)&load);

    //char message[BUFLENGTH];
    unsigned int msgLenght;
    //struct PanaceaHeader *pnchdr;
    struct iphdr *originaliphdr;

    //pnchdr = (PanaceaHeader*) message;
    originaliphdr = (iphdr *) (message + sizeof(PanaceaHeader));
    memcpy(message + sizeof(PanaceaHeader), load, loadLenght);
    msgLenght = loadLenght + sizeof(PanaceaHeader);

    pnchdr->destinationForwarderAddress = ta->forwardersIP[originaliphdr->daddr];
    //pnchdr->sourceAddress = ta->myIp;
    //pnchdr->pathLength = 0;
    //pnchdr->nextHop = 0;

    // Packet reinjection
    //struct sockaddr_in forwarderAddr;
    //bzero(&forwarderAddr, sizeof(forwarderAddr));
    //forwarderAddr.sin_family = AF_INET;
    //forwarderAddr.sin_port = htons(DATA_PORT);
    forwarderAddr.sin_addr.s_addr = ta->forwarderIP;
    //forwarderAddr.sin_addr.s_addr = originaliphdr->daddr;
    //forwarderAddr.sin_addr.s_addr = pnchdr->destinationForwarderAddress;

    if (sendto(ta->socketForwarder, message, msgLenght, 0 /*MSG_DONTWAIT*/, (struct sockaddr*) &forwarderAddr, sizeof(forwarderAddr)) == -1)
        perror("error sendto()");

    return nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
}


