/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RoutingManagerEXP3.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */

#include <stdio.h>
#include "RoutingManagerEXP3.h"
#include <math.h>

using namespace std;

RoutingManagerEXP3::RoutingManagerEXP3(size_t maxPathToTest, double gamma, MetricType mode) {
  mode_ = mode;
  allPathsComputed = false;
  firstRunEXP3 = true;
  this->gamma = gamma;
  this->maxPathToTest = maxPathToTest;
}


void RoutingManagerEXP3::computePathsToTest(std::vector< IPPath> &pathsToTest) {
    if (!allPathsComputed) {
        allPaths = computeAllPaths();

       allPathsComputed = true;
        for (std::map < IPv4Address , std::vector< IPPath > >::iterator it = allPaths.begin(); it != allPaths.end(); it++) {
            for (size_t i=0; i<it->second.size(); i++) {
                allPathsId[it->second.at(i)] = i;
            }
        }
    }
    pathsToTest.clear();


    if (firstRunEXP3) {
        for (const IPv4Address dest : index_destinations) {
        	if(dest != origin){
        		const std::vector< IPPath > & pathsToDest = allPaths.at(dest);
        		if(allPathsWeigths.find(dest)==allPathsWeigths.end()){
        			allPathsWeigths[dest] = std::vector<double> (pathsToDest.size(), 1.0); // initilialize all weigths to 1
        			allPathsProba[dest] = std::vector<double> (pathsToDest.size(), 1.0/(double)pathsToDest.size()); // init paths probabilities
        		}
        	}
        }
        firstRunEXP3 = false;
	cerr << "EXP3: end of initialization" << endl;
    }

    for (const IPv4Address dest : index_destinations) {
    	if(dest != origin){
	  //	  cerr << "EXP3: computing the paths to test to dest " << IPToString(dest) << endl;
    		const std::vector< IPPath > & pathsToDest = allPaths.at(dest);
    		        std::vector< double > & pathsProbas = allPathsProba[dest];
    		        std::vector< double > & pathsWeights = allPathsWeigths[dest];
    		        size_t K= pathsToDest.size();

    		        double sumWeigth= 0.0;
    		        for (size_t i= 0; i < K; i++) {
    		            sumWeigth += pathsWeights[i];
    		        }

    		        std::map<int, double> pathsProbasTmp; //pathsProbasTmp[pathId]=probaTemporaire
    		        for (size_t i= 0; i < K; i++) {
    		            pathsProbas[i] = (1.0-gamma) * pathsWeights[i] / sumWeigth + gamma/(double)K;
    		            pathsProbasTmp[i] = pathsProbas[i];
			    //			    cerr << "proba path " << i << " = " << pathsProbas[i] << endl;
    		        }

    		        // draw the set of paths to probe

    		        IPPath currentPath = currentRoutes[dest];
    		        IPPath internetPath;
    		        internetPath.push_back(dest);

    		        for (size_t n= 0; n <= maxPathToTest; n++) {

    		            double lastElementProba = 0.0;
    		            if (n==0) { // direct path
    		                size_t id = allPathsId[internetPath];
    		                lastElementProba = pathsProbasTmp[id];
    		                pathsProbasTmp.erase(id);
    		                pathsToTest.push_back(internetPath);

    		            } else  if (currentPath != internetPath && n==1) { // current path
    		                size_t id = allPathsId[currentPath];
    		                lastElementProba = pathsProbasTmp[id];
    		                pathsProbasTmp.erase(id);
    		                pathsToTest.push_back(currentPath);
    		            } else {
    		                double p = ((double) rand() / (RAND_MAX));
				//    		                cerr << "draw number=" << p << endl;
    		                double cumulativeProbability = 0.0;

    		                for (std::map<int, double>::iterator it = pathsProbasTmp.begin(); it != pathsProbasTmp.end(); it++) {
    		                    cumulativeProbability += it->second;
    		                    if (p <= cumulativeProbability) {
    		                        pathsToTest.push_back(pathsToDest[it->first]);
    		                        lastElementProba = it->second;
    		                        pathsProbasTmp.erase(it);
    		                        break;
    		                    }
    		                }

    		            }
    		            // update probabilities for next draw
    		            for (std::map<int, double>::iterator it = pathsProbasTmp.begin(); it != pathsProbasTmp.end(); it++) {
    		                it->second = it->second / (1.0-lastElementProba);
    		            }
    		        }
			//			cerr << "Number of selected paths : " << pathsToTest.size() << endl;
    		    }
    	}
}


void RoutingManagerEXP3::putProbeResults(const PathEdgesResult& result) {
	if (result.metric == LATENCY_METRIC){
		double totalRTT = result.getTotalRTT();
		//		std::cout << "received latencies results : TotalRTT=" << totalRTT << " ms" << std::endl;
		if (totalRTT < pathManager->probeTimeout) {
			Result& res = resulTable[result.path];
			// This was added to be sure the metric type is passed
			res.metric = result.metric;
			// update average value
			res.averageResult = (double)(res.averageResult * res.packetsReceived + totalRTT) / (double)(res.packetsReceived +1);
			res.packetsReceived++;
			if(result.path.size()>1){
				res.averageResult = res.averageResult;
			}
		}
	}else if (result.metric == LOSS_METRIC){
		double probability = 1;
		Result& res = resulTable[result.path];
		// This was added to be sure the metric type is passed
		res.metric = result.metric;
		// Update value
		for(size_t i=0; i<result.edgesRTT.size();i++){
			probability *= 1 - result.edgesRTT.at(i);
		}
		res.averageResult = 1 - probability;
		res.packetsReceived++;
	}else if (result.metric == BW_METRIC){
		double min = 1.0e10;
		Result& res = resulTable[result.path];
		// This was added to be sure the metric type is passed
		res.metric = result.metric;
		// update average value
		for(size_t i=0; i<result.edgesRTT.size();i++){
			if(result.edgesRTT.at(i)<min){
				min=result.edgesRTT.at(i);
			}
		}
		res.averageResult = min;
		res.packetsReceived++;
	}
}



std::map<IPv4Address, IPPath> RoutingManagerEXP3::getBestPathsFromResults() {
    map<IPv4Address, IPPath>        proposedPaths;
    double                          minCost, pathCost, reward;
    int                             NumberMinHop, NumberHop;
    map< IPPath, Result>::iterator  minPathIter;
    bool                            found;
    size_t                          pathId, K;

    for (const IPv4Address dest : index_destinations) {

      //initialization
      minCost = 99999;
      NumberMinHop = 4;
      found = false;

      //retrieve information on the paths to this destination
      const vector< IPPath > & pathsToDest = allPaths.at(dest);
      vector< double >       & pathsProbas = allPathsProba[dest];
      vector< double >       & pathsWeights = allPathsWeigths[dest];
      K = pathsToDest.size();

      //first loop to find the minimum cost to reach the destination dest
      for (map< IPPath, Result>::iterator ii = resulTable.begin(); ii != resulTable.end(); ++ii)
        {
	  const IPPath& thepath = ii->first;
	  const Result & theresults = ii->second;

	  // if this path is not headed to the right destination, ignore it
	  if (dest != thepath.back()) continue;

	    //Looking for min Latency, Loss or max BW
	    pathCost = ((theresults.metric!=BW_METRIC)?theresults.averageResult:300.0/theresults.averageResult);

	    if ( pathCost < minCost )
	      minCost = pathCost;
        }

      //second loop to find the minimum cost path (i.e., with cost minCost) among those headed to the destination dest
      for (map<IPPath, Result>::iterator ii = resulTable.begin();ii != resulTable.end(); ++ii)
        {
	  const IPPath& thepath = ii->first;
	  const Result & theresults = ii->second;
	  
	  // if this path is not headed to the right destination, ignore it
	  if (dest != thepath.back()) continue;
	  
	  //retrieve the cost and the number of hops of this path 
	  NumberHop = thepath.size();
	  pathCost = ((theresults.metric!=BW_METRIC)?theresults.averageResult:300.0/theresults.averageResult);
	  
	  //if this is the min cost path and if it has a min number of hops, then we found the path to use
	  if ( (pathCost == minCost) && (NumberHop<=NumberMinHop)) {
	    NumberMinHop = NumberHop;
	    minPathIter = ii;
	    found = true;
	  }

	  //update the weight of this path
	  pathId = allPathsId[thepath];
	  reward = 1.0/pathCost;
	  pathsWeights[pathId] = pathsWeights[pathId] * exp( gamma * reward / (pathsProbas[pathId]*K));
	}

      //record the best path among those tested in proposedPaths[dest]
      if (found) {
	IPPath path = minPathIter->first;
	proposedPaths[dest] = path;
      }
      else
	cerr << "getBestPathsFromResults: best path not found!" << endl;
    }

    //Clear the table of results
    resulTable.clear();

    return proposedPaths;
}



// std::map<IPv4Address, IPPath> RoutingManagerEXP3::getBestPathsFromResults() {
//     std::map<IPv4Address, IPPath> proposedPaths;
//     for (const IPv4Address dest : index_destinations) {

//         double minResult = 99999;
//         double maxResult = 0;
//         int NumberMinHop = 4;
//         std::map< IPPath, Result>::iterator minPathIter;
//         bool found = false;

//         for (std::map< IPPath, Result>::iterator ii = resulTable.begin(); ii != resulTable.end(); ++ii)
//         {
//             const IPPath& thepath = ii->first;
//             const Result & theresults = ii->second;

//             if (dest == thepath.back()) { // if this path is headed to the right destination

//             	if(theresults.metric==LATENCY_METRIC){
//             		double latency = theresults.averageResult;

//             		if (latency < minResult) {
//             			minResult = latency;
//             		}
//             	}else if(theresults.metric==LOSS_METRIC){
//             		double losses = 100 * (1.0 - (double)theresults.packetsReceived / (double)PROBE_PACKETS_TO_SEND);

//             	}else if (theresults.metric == BW_METRIC){
//             		double BW = theresults.averageResult;

//             		if (BW > maxResult) {
//             			maxResult = BW;
//             		}
//             	}
//             }
//         }

//         for (std::map<IPPath, Result>::iterator ii = resulTable.begin();ii != resulTable.end(); ++ii)
//         {
//         	const IPPath& thepath = ii->first;
//         	const Result & theresults = ii->second;

//         	if (dest == thepath.back())
//         	{
//         		int NumberHop = thepath.size();
//         		double result = 0;
//         		double BW = 1;

//         		if(theresults.metric==LATENCY_METRIC){
//         			result = theresults.averageResult;
//         		}else if(theresults.metric==LOSS_METRIC){
//         			result = 100* (1.0 - (double) theresults.packetsReceived/ (double) PROBE_PACKETS_TO_SEND);
//         		}else if (theresults.metric == BW_METRIC){
//         			BW = theresults.averageResult;
//             	}

//         		if (result == minResult && NumberHop<=NumberMinHop) {
//         			NumberMinHop = NumberHop;
//         			minPathIter = ii;
//         			found = true;
//         		}

//         		if (BW == maxResult && NumberHop<=NumberMinHop) {
//         			NumberMinHop = NumberHop;
//         			minPathIter = ii;
//         			found = true;
//         		}
//         	}
//          }
//         if (found) {
//             IPPath path = minPathIter->first;
//             proposedPaths[dest] = path;
//             size_t pathId = allPathsId[path];

//             const std::vector< IPPath > & pathsToDest = allPaths.at(dest);
//             size_t K = pathsToDest.size();

//             std::vector< double > & pathsProbas = allPathsProba[dest];
//             std::vector< double > & pathsWeights = allPathsWeigths[dest];

//             double reward;
//             if(minPathIter->second.metric==LATENCY_METRIC){
//             	reward = 1.0/ minResult;
//             }else if(minPathIter->second.metric==LOSS_METRIC){
//             	reward = (double)(PROBE_TIMEOUT_IN_MS - minResult) / (double) PROBE_TIMEOUT_IN_MS;
//             }else if (minPathIter->second.metric == BW_METRIC){
//             	reward = maxResult/300.0;
//         	}
// 	    cerr << "Reward is " << reward << endl;
// 	    cerr << "Path weight is updated from " << pathsWeights[pathId];
//             pathsWeights[pathId] = pathsWeights[pathId] * exp( gamma * reward / (pathsProbas[pathId]*K));
// 	    cerr << " to " << pathsWeights[pathId] << endl;
//         }
//     }
//     resulTable.clear();
//     return proposedPaths;
// }

