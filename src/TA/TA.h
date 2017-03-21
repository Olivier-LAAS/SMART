/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file TA.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#ifndef TA_H_
#define TA_H_

#include "common/common.h"

/*!
 * External libraries used to intercept packets
 */
extern "C" {
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
}


/*!
 * Length of the netfilter queue.
 */
#define NFQLENGTH 1024

/*!
 * @brief the class that handles packet interception with NFQUEUE
 * Please refer to NFQUEUE documentation (on the web) for more details
 */
class NfQueue {
public :
    NfQueue();
    ~NfQueue();
    void on_exit(int exit_value);

    void open(nfq_callback *cb, void * callBackData);
    void close();

    void run();

    u_int16_t getNfqID() const;
    void setNfqID(const u_int16_t &value);

protected:
    bool isOpened;
    u_int16_t nfqID;
    struct nfq_handle *nfqHandle;
    struct nfq_q_handle *myQueue;
    struct nfnl_handle *netlinkHandle;

    int fd, res;
    char buf[BUFLENGTH];
};


/*!
 * @brief the transmission agent class. It handles packet interception and encapsulation into SMART dataPacket before sending to Forwarder
 * @param routerThread the thread used to communicate with router in order to receive interception orders
 * @param datapacket the SMART datapacket
 * @param packet the original packet
 * @param nfQueue the nfqueue class
 * @param socketForwarder the socket to communicate with forwarder (to send SMART packets)
 * @param socketRouter the socket to communicate with router in order to receive interception orders
 * @param nfQueueId the id of the nfqueue to be used for interception
 * @param routerIp the IP address of router
 * @param myIp my IP address
 * @param forwarderIp the IP address of the Forwarder
 * @param forwardersIP a mapping table between clients and their forwarders (used to know behind which forwarder a client is located)
 * @param doInterception a boolean mapping table to know for which destination (forwarder address not client address) we should intercept or not
 */
class TransmissionAgent {
public :
    TransmissionAgent(std::string routerip=" ", std::string localip=" ");

    void run();

    /**
     * @brief Read all configurations from files
     */
    void getIPadress();

    /*!
     * This function is used if router IP address should be passed through a file
     * Actually this parameter could be passed as an executable argument as well
     */
    void readConfigurationFromFile();

    /**
     * @brief handle abnormal error or signal (e.g. need to close the nfqueue)
     */
    void onExit();


    void sendTestPacket();

protected:
    std::thread routerThread;
    /*!
     * It creates the socket and manage communications with router (always for packet interception)
     */
    void routerCommunication();
    /*!
     * Provide IP addresses for destinations whom packets should be intercepted (destinations = final clients)
     */
    void interceptIPs(bool intercept, std::vector<std::string> dst);

    /* Not used anymore */
    void interceptAllBehindProxy(bool intercept, IPv4Address forwarderDest);

    /*!
     * Receives the packet, encapsulates it and sends it to the proxy (by now proxy = forwarder)
     */
    static int encapsulate(nfq_q_handle *myQueue, struct nfgenmsg *msg, nfq_data *pkt, void *cbData);

    DataPacket datapacket;
    Packet packet;

    NfQueue nfQueue;
    int socketForwarder, socketRouter;

    u_int16_t nfQueueId;
    IPv4Address routerIp, myIp, forwarderIP;
    std::map<IPv4Address, IPv4Address> forwardersIP; // forwardersIP[clientIP]
    std::map<IPv4Address, bool> doInterception;

};

#endif
