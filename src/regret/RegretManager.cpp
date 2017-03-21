/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RegretManager.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include <iostream>
#include <iomanip>

#include "RegretManager.h"
#include "algorithms.cpp"

Regret::Regret(int hop, std::string fileName, std::string fileName2) : matrixFile(fileName), simulationFile(fileName2){
	x = hop;
	rows=0;
	cols=0;
	if (!matrixFile.is_open()){
		std::cout << "file open failed";
	}
	endMatrixResult = false;
}

void Regret::readListIP(){
	std::string line;
	std::ifstream in("ip_address");
	if (in.is_open()){
		int dimension = 0;
		while (getline(in, line)) {
			tableIP.push_back(inet_addr(line.c_str()));
			dimension ++;
		}
		dim = dimension;
		in.close();
	}
}

void Regret::readConfigurationFromFile(){
	std::cout << "Reading configuration from files..." << std::endl;
	std::string line;

	// read all the nodes from the file "nodes"
	std::ifstream in("nodes");
	if (in.is_open()) {
		while (!in.eof()) {
			getline(in, line);
			if (line != "") {
				// extracts the forwarder and monitor address
				std::istringstream ss(line);
				std::string ip;

				if (!ss || !getline(ss, ip, ',')) {
					std::cout << "error reading forwarder" << std::endl;
					exit(-1);
				}
				IPv4Address forwarderAddr = inet_addr(ip.c_str());

				if (!ss || !getline(ss, ip, ',')) {
					std::cout << "error reading monitor" << std::endl;
					exit(-1);
				}
				IPv4Address monitorAddr = inet_addr(ip.c_str());


				nodes.push_back(monitorAddr);
			}
		}
		in.close();
	}

	// read all the destinations from the file "destination"
	in.open("destinations");
	if (in.is_open()) {
		while (getline(in, line)) {
			destinations.push_back(inet_addr(line.c_str()));
		}
		in.close();
	}

	// read the origin from the file "myip"
	in.open("myip");
	if (in.is_open()) {
		getline(in, line);
	    origin = inet_addr(line.c_str());
	    in.close();
	}
}

void Regret::computeAllPath() {
    std::cout << "-------computeAllPaths ------" << std::endl;

    IPVector index_hops;

    for (IPv4Address dest : destinations) {
        for (IPv4Address node : nodes) {
            if (node != dest) {
                index_hops.push_back(node);
            }
        }
        int maxPathLengthToTest = std::min(index_hops.size()+1, (size_t) DATA_MAX_PATH_LENGTH);
        for (int l = 0; l < maxPathLengthToTest; l++) {
            std::vector< IPPath > paths;
            if (l == 0){
                IPPath emptyPath;
                paths.push_back(emptyPath);
            } else {
                paths = getAll_K_Permutations(index_hops, l); // toutes les permutations de taille k
            }
            for (size_t j = 0; j< paths.size(); j++) {
                paths[j].push_back(dest);
            }
            allPaths[dest].insert(allPaths[dest].end(), paths.begin(), paths.end());
        }
        index_hops.clear();
        std::cout << "Total number of distinct path to " << IPToString(dest) << " = " << allPaths[dest].size() << std::endl;
    }
}


int Regret::readNumbers( const std::string & s ) {
	std::istringstream is( s );
    double n;
    while( is >> n ) {
        matrix.push_back( n );
    }
    return matrix.size();
}


int Regret::readResultsFromFile(){
	std::string line;

    int i=0;
    getline(matrixFile, line );

    cols =readNumbers( line );

    for ( i=1;i<dim;i++){
    	if ( getline(matrixFile, line) == 0 ) return 0;
        	readNumbers( line );
    }
    rows=i;
}


void Regret::lookUpForResult(IPPath path){

	int x, y;

	// read from matrix the result value between the origin and the beginning of path
	for(int j=0;j<tableIP.size();j++){
		if(tableIP.at(j)==origin){
			x=j;
		}
		if(tableIP.at(j)==path.at(0)){
			y=j;
		}
	}
	resultTable[path]+=matrix[x*cols+y];


	// read from matrix the result value between each segment of the path
	for(int i=0; i<path.size()-1;i++){
		for(int j=0;j<tableIP.size();j++){
			if(tableIP.at(j)==path.at(i)){
				x=j;
			}
			if(tableIP.at(j)==path.at(i+1)){
				y=j;
			}
		}
		resultTable[path]+=matrix[x*cols+y];
	}
}

int Regret::calculateResult(){
	if (readResultsFromFile()!=0){
		for (std::map < IPv4Address, std::vector<IPPath> >::iterator it = allPaths.begin(); it!= allPaths.end(); it++){
			for (int i=0; i<it->second.size();i++){
				lookUpForResult(it->second.at(i));
			}
		}
	}else{
		endMatrixResult = true;
		return 0;
	}
	return 1;
}

void Regret::findOptimalPath(){

	std::cout << "findBestPathAfterNIteration" << std::endl;

	for (const IPv4Address dest : destinations){
		double minResult = 9999999;
		std::map< IPPath, double>::iterator minPathIter;
		bool found = false;

		for (std::map< IPPath, double>::iterator it = resultTable.begin(); it != resultTable.end(); ++it)
		{
			const IPPath& thepath = it->first;
			const double & theresults = it->second;

			if (dest == thepath.back()) {
				double latency = theresults;

				if (latency < minResult) {
					minResult = latency;
					minPathIter = it;
					found = true;
				}
			}
		}
		if (found) {
			optimalResultAfterN[dest]=minResult;
			result=minPathIter->first;
		}
	}
}


void Regret::readSimulationResult(){
	std::string line;

	for (size_t i = 0; i < destinations.size(); i++) {
		if (getline(simulationFile, line) == 0) {
			return;
		} else {
			std::istringstream is(line);

			std::string dest;
			double res;
			is >> dest >> res;

			IPv4Address destination = inet_addr(dest.c_str());
			simulationResultAfterN[destination] += res;
		}
	}
}


void Regret::simulationOptimalComparison(){
	for (std::map<IPv4Address, double>::iterator it = optimalResultAfterN.begin(); it != optimalResultAfterN.end(); ++it){

		const IPv4Address dest=it->first;
		const double optimal=it->second;

		for (std::map<IPv4Address, double>::iterator it2 = simulationResultAfterN.begin(); it2 != simulationResultAfterN.end(); ++it2){

			const IPv4Address dest2=it2->first;
			const double simulation=it2->second;

			if(dest==dest2){
				ResultPacket packet;
				packet.dest=dest;
				packet.optimal=optimal;
				packet.simulation=simulation;
				packet.regret=(simulation-optimal)/n;

				listResultAfterN.push_back(packet);
			}
		}
	}
}


void Regret::writeRegretResultInFile(){

	std::ofstream resultText("regretResult.txt", std::ios::app);
	std::cout <<  " writeResult" << std::endl;

	if(resultText){

		//resultText << "For n = " << n <<std::endl;
		resultText.precision(10);

		for (size_t i=0; i<listResultAfterN.size(); i++){

			resultText << IPToString(listResultAfterN.at(i).dest) << "	" ;
			resultText << listResultAfterN.at(i).simulation << "	" ;
			resultText << listResultAfterN.at(i).optimal << " 	" ;
			resultText << listResultAfterN.at(i).regret << " 	";

			for(size_t j=0; j<result.size(); j++){
				resultText<<IPToString(result.at(j))<<" ";
			}
			resultText << std::endl;

		}
	}
}

void Regret::run(){

	computeAllPath();
	n =1;

	// Do the program until the result file is completely read or we reach the value x decided
	while(endMatrixResult==false && n<=x){

		if(calculateResult()!=0){
			findOptimalPath();
			readSimulationResult();
			simulationOptimalComparison();
			writeRegretResultInFile();

		}

		matrix.clear();
		listResultAfterN.clear();
		n++;
	}
}
