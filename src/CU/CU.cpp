/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file DC.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */


#include "CU.h"

CU::CU(){
	listClouds.clear();
	firstConnexion = false;
	pthread_mutex_init(&mutex, NULL);
	condition_var  = PTHREAD_COND_INITIALIZER;
	readGraphCommunication();
}

CU::~CU(){

}

void CU::waitForCloudConnection(){
	pthread_t thread_id=0;

	//Create socket
	lisening_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (lisening_sock == -1) {
		std::cerr << "Could not create socket" << std::endl;
	}

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(CLOUD_COMMUNICATION_PORT);

	int so_reuseaddr = 1;
	setsockopt(lisening_sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr,sizeof so_reuseaddr);

	//Bind
	if (bind(lisening_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		//print the error message
		std::cout << "bind failed. Error" << std::endl;
		exit(-1);
	}

	//Listen
	listen(lisening_sock, SOMAXCONN);
	c = sizeof(struct sockaddr_in);

	std::cout <<"server running" << std::endl;

	while(true){
		//Accept an incoming connection
		std::cout << "Waiting for incoming cloud connections..." << std::endl;
		new_sock = accept(lisening_sock, (struct sockaddr *) &client,(socklen_t*) &c);
		if (new_sock < 0) {
			perror("accept failed");
			exit(-1);
		}

		socketThread = pthread_create(&thread_id,0, CU::staticRun, this);
		if ( socketThread != 0 ){
			std::cerr <<"CreateThread failed  "<< std::endl;
		}
		std::cout << "Cloud registered : " << IPToString(client.sin_addr.s_addr)<< std::endl;
	}
}

void CU::readGraphCommunication(){
	std::cout << "Reading configuration from files..." << std::endl;
	std::string line;

	std::ifstream in("graphCom");
	if (in.is_open()) {
		while (getline(in, line)) {
			std::istringstream iss(line);
			int a , b;
			std::vector<int> c;
			iss >> a;
			std::cout << a << " ";
			while (iss >> b){
				c.push_back(b);
				std::cout << b << " ";
			}
			std::cout << std::endl;
			graph[a]=c;
		}

		for(std::map<int,std::vector<int>>::iterator it = graph.begin(); it!= graph.end(); it++){
			int main = it->first;
			std::vector<int> vec = it->second;
			std::cout << main << " ";
			for(int i=0; i<vec.size(); i++){
				std::cout << vec.at(i) << " ";
			}
			std::cout << std::endl;
			std::cout << "graph[main].size() " << graph[main].size() << std::endl;
		}
		in.close();
	}
}

void CU::firstUpdateCloudList(int sock_thread, int& myrole){
	InfoCloudPacket myCloud;

	int received = recv(sock_thread, (void*) &myCloud, sizeof(myCloud), 0);
	if (received == 0) {
		std::cerr << "killed the connexion" << std::endl;
		close(sock_thread);
		return;
	}
	myrole = myCloud.role;

	write(sock_thread, (void*) &firstConnexion, sizeof(firstConnexion));
	if (firstConnexion == true && graph[myrole].size()!=0) {
		firstConectionUpdate(sock_thread, myrole);
	}
	firstConnexion = true;

	std::cout << "ADD NEW CLOUD TO LISTCLOUDS " << std::endl;
	Cloud cloud;

	cloud.role = myCloud.role;
	cloud.forwarder = myCloud.addr[0];
	cloud.monitor = myCloud.addr[1];

	std::cout << "forwarder " << IPToString(cloud.forwarder) << std::endl;
	std::cout << "monitor " << IPToString(cloud.monitor) << std::endl;

	pthread_mutex_lock(&mutex);
	listClouds[sock_thread] = cloud;
	pthread_mutex_unlock(&mutex);
	writeListCloud();

	lastCloudModified = sock_thread;
	type = myCloud.type;
	pthread_cond_broadcast(&condition_var);
}

void CU::firstConectionUpdate(int sock_thread, int myrole){
	std::cout << "SEND FIRST UPDATE" << std::endl;
	FirstUpdatePacket updatePacket;

	for (std::map<int, Cloud>::iterator it = listClouds.begin();it != listClouds.end(); ++it){
		int socket=it->first;
		Cloud cloud=it->second;

		//updatePacket.role=cloud.role;

		if(std::find(graph[myrole].begin(),graph[myrole].end(),cloud.role)!=graph[myrole].end()){
			updatePacket.destination=true;
		}else{
			updatePacket.destination=false;
		}

		updatePacket.forwarder=cloud.forwarder;
		updatePacket.monitor=cloud.monitor;
		for(size_t i=0; i<cloud.clients.size(); i++){
			updatePacket.client[i]=cloud.clients.at(i);
		}
		updatePacket.length=cloud.clients.size();

		if(it==--listClouds.end()){
			updatePacket.updateDone=true;
		}else{
			updatePacket.updateDone=false;
		}
		write(sock_thread,(void*)&updatePacket, sizeof(updatePacket));
	}
	std::cout << "updateDone "<< std::endl;
}


void CU::updateCloudList(int sock_thread){
	while (true) {
		InfoCloudPacket myCloud;

		int received = recv(sock_thread, (void*) &myCloud, sizeof(myCloud), 0);
		if (received == 0) {
			std::cerr << "killed the connexion" << std::endl;
			close(sock_thread);
			return;
		}

		std::cout << "UPDATE CLOUD LIST" << std::endl;
		Cloud cloud;

		cloud = listClouds[sock_thread];
		cloud.clients.push_back(myCloud.addr[0]);
		std::cout << "clients " << IPToString(myCloud.addr[0]) << std::endl;

		pthread_mutex_lock(&mutex);
		listClouds[sock_thread] = cloud;
		pthread_mutex_unlock(&mutex);
		writeListCloud();

		lastCloudModified = sock_thread;
		type = myCloud.type;
		pthread_cond_broadcast(&condition_var);
	}
}


void CU::updateMyCloud(int sock_thread, int myrole){
	std::cout << "SEND UPDATE" << std::endl;
	InfoCloudPacket updatePacket;

	if(lastCloudModified!=sock_thread){
		Cloud cloud=listClouds[lastCloudModified];
		if(std::find(graph[myrole].begin(),graph[myrole].end(),cloud.role)!=graph[myrole].end()){
			updatePacket.destination=true;
		}else{
			updatePacket.destination=false;
		}
		//updatePacket.role=cloud.role;
		updatePacket.addr[0]=listClouds[lastCloudModified].forwarder;
		if(type==ROUTER){
			updatePacket.type=type;
			updatePacket.addr[1]=listClouds[lastCloudModified].monitor;
		}else{
			updatePacket.type=type;
			updatePacket.addr[1]=listClouds[lastCloudModified].clients.back();
		}
		write(sock_thread,(void*)&updatePacket, sizeof(updatePacket));
	}
}


void* CU::run(){

	int sock_thread = new_sock;
	int myrole;
	firstUpdateCloudList(sock_thread, myrole);

	if(graph[myrole].size()!= 0){
		std::thread t (&CU::updateCloudList,this,sock_thread);
		t.detach();

		while(true){
			pthread_cond_wait( &condition_var, &mutex );
			updateMyCloud(sock_thread, myrole);
		}
	}else {
		while(true){

		}
	}
	return 0;
}


void CU::writeListCloud(){
	std::cout << "WRITE CLOUD" << std::endl;
	for (std::map<int, Cloud>::iterator it = listClouds.begin();it != listClouds.end(); ++it){
		Cloud cloud=it->second;

		std::cout << "Forwarder : " << IPToString(cloud.forwarder) << std::endl;
		std::cout << "Monitor : " << IPToString(cloud.monitor) << std::endl;

		std::cout << "Clients : " ;
		for(size_t i=0; i<cloud.clients.size(); i++){
			std::cout << IPToString(cloud.clients.at(i)) << " ";
		}
		std::cout << std::endl<< std::endl;
	}
}

