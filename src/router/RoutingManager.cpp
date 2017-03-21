/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RoutingManager.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */

#include <stdio.h>
#include <iomanip>
#include "RoutingManager.h"
#include <algorithm>

using namespace std;

template <typename Iterator>
/**
 * @brief Utility function used by function getAll_K_Permutations()
 * @param first
 * @param k
 * @param last
 * @return
 */
inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
{
    if ((first == last) || (first == k) || (last == k))
        return false;
    Iterator itr1 = first;
    Iterator itr2 = last;
    ++itr1;
    if (last == itr1)
        return false;
    itr1 = last;
    --itr1;
    itr1 = k;
    --itr2;
    while (first != itr1)
    {
        if (*--itr1 < *itr2)
        {
            Iterator j = k;
            while (!(*itr1 < *j)) ++j;
            std::iter_swap(itr1,j);
            ++itr1;
            ++j;
            itr2 = k;
            std::rotate(itr1,j,last);
            while (last != j)
            {
                ++j;
                ++itr2;
            }
            std::rotate(k,itr2,last);
            return true;
        }
    }
    std::rotate(first,k,last);
    return false;
}


/**
 * @brief Find all the permutations of size k, given a set of value
 * @param possibilities : the set of possible values
 * @param k : size of permutation
 * @return vector containing all the permutations
 */
template <typename Type>
std::vector<std::vector < Type> > getAll_K_Permutations(const std::vector<Type> & possibilities, size_t k) {
    std::vector<Type> tmp = possibilities;
    std::vector<std::vector <Type> > res;

    k = std::min(k, possibilities.size());

    std::sort(tmp.begin(), tmp.end()); // tri avant generation des permutations (indispensable)
    // generation de toutes les permutations
    do {
        std::vector<Type> b;
        for(size_t i = 0; i < k; i++)
            b.push_back(tmp[i]);
        do {
            res.push_back(b);
        } while(std::next_permutation(b.begin(), b.begin() + k));
    } while(next_combination(tmp.begin(), tmp.begin() + k, tmp.end()));

    return res;
}

std::map<IPv4Address, std::vector<IPPath> > RoutingManagerBase::computeAllPaths() {
    std::cout << "-------computeAllPaths ------" << std::endl;

    std::map<IPv4Address, std::vector<IPPath> > allPaths;
    IPVector index_hops;

    for (IPv4Address dest : index_destinations) {
    	if(dest != origin){
    		for (IPv4Address node : index_nodes) {
    			if (node != dest && node!=origin) {
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
    return allPaths;
}

//TODO delete
void RoutingManagerBase::writeAllPath(IPv4Address dest, std::vector< IPPath > paths){
	std::ofstream resultText("AllPath.txt",std::ios::app);

	if(resultText){
		resultText << "For destination " << dest << " " << std::endl;

		for(size_t i=0;i<paths.size();i++){
			for(size_t j=0;j<paths.at(i).size();j++){
				resultText << paths.at(i).at(j) << " ";
			}
			resultText << std::endl;
		}


		resultText << "  " << std::endl;
	}
}



std::vector<IPPath> RoutingManagerBase::readPathsFromUserCommandLine() {
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

void RoutingManagerBase::run() {
	// check if the set of destinations is not empty
	while (index_destinations.size() == 0) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "No destination declared in router ! Waiting for destination declaration..." << std::endl;
	}

	for (IPv4Address dest : index_destinations) {
		IPPath internetPath;
		internetPath.push_back(dest);
		currentRoutes[dest] = internetPath;
	}
    while (true)   {

    	computePathsToTest(pathsToTest); // polymorphism on funciton computing the list of paths to test

        // ask the pathManager to trigger the measurement process
        std::cout << "Probing " << pathsToTest.size() << " paths" << std::endl;
        pathManager->doMonitorPaths(mode_, pathsToTest);

        // wait for TIMEOUT
        std::this_thread::sleep_for(std::chrono::milliseconds(pathManager->probeTimeout));

        updateRoutingTable();

        std::cout << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(pathManager->probePeriod));
    }
}

void RoutingManagerBase::runSimulation(int & inst, vector<double> & avg_err){

  vector<double>  errors(4,0.0);

  for (IPv4Address dest : index_destinations) {
    IPPath internetPath;

    internetPath.push_back(dest);
    currentRoutes[dest] = internetPath;
  }

  while (simulationManager->endMatrixResult == false)   {
    if (simulationManager->readResultsFromFile() != 0) {
      for (size_t i = 0; i < index_origin.size(); i++) {
	origin = index_origin.at(i);

	computePathsToTest(pathsToTest); // polymorphism on function computing the list of paths to test

	// ask the simulator to read the result file and return result packets
	if (simulationManager->run(mode_, pathsToTest) != 0) {
	  std::map<IPv4Address, IPPath> proposedPaths = getBestPathsFromResults();
	  for (IPv4Address dest : index_destinations) {
	    if (proposedPaths.find(dest) != proposedPaths.end()) { // a path proposal exists for this destination
	      //std::cout << "Path proposal : ";
	      //printIPPath(proposedPaths[dest]);
	      currentRoutes[dest] = proposedPaths[dest];
	      simulationManager->getError(mode_, origin, dest, currentRoutes[dest], errors);
	      for (i=0; i<4; i++)
		avg_err[i] += errors[i];
	      if ( errors[1] > 0.0 )
		avg_err[4] += 1.0;
	      if ( errors[2] > 0.0 )
		avg_err[5] += 1.0;
	    } else {
	      IPPath internetPath;
	      internetPath.push_back(dest);
	      currentRoutes[dest] = internetPath;
	    }
	  }
	  //decomment the below line in order to print detailed results in simulation.txt
	  //	  simulationManager->writeResultInFile(mode_, proposedPaths, inst);
	}
	
      }
    } else {
      simulationManager->endMatrixResult = true;
    }

    if (inst%200 == 0 )
      cout << "\r" << inst;
    inst++;
  }
  cout << endl;
  //  simulationManager->writeLenght();
}


void RoutingManagerBase::updateRoutingTable() {
    using namespace std;

    //Update the best route for each destination.

    //    internetDelays.clear();
    //    internetLosses.clear();
    //    panaceaDelays.clear();
    //    panaceaLosses.clear();

    struct timeval tcurrent;
    gettimeofday(&tcurrent, NULL);
    std::cout << "==============================================" << std::endl;
    std::cout << "    Routing decisions at t=" << tcurrent.tv_sec << std::endl;
    std::cout << "==============================================" << std::endl;

    std::map<IPv4Address, IPPath> proposedPaths = getBestPathsFromResults();
    for (IPv4Address dest : index_destinations) {

        if (proposedPaths.find(dest) != proposedPaths.end()) { // a path proposal exists for this destination
            //std::cout << "Path proposal : ";
            //printIPPath(proposedPaths[dest]);
        	std::cout << std::endl;
            pathManager->setMonitorRoute(dest, proposedPaths[dest]);
            currentRoutes[dest] = proposedPaths[dest];
        } else {
            IPPath internetPath;
            internetPath.push_back(dest);
            pathManager->setMonitorRoute(dest, internetPath);
            currentRoutes[dest]= internetPath;
        }
    }


}
