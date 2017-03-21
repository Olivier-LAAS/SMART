/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RegretManager.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#ifndef REGRETMANAGER_H
#define REGRETMANAGER_H

#include "common/common.h"

/*!
 * @brief A regret result packet that contains for each destination the optimal latency,
 *  the one obtained by simulation and the regret value for this destination
 *  @param dest the IP address of the destination
 *  @param optimal the optimal latency
 *  @param simulation the latency obtained by simulation
 *  @param regret the regret value for this destination
 */
typedef struct {
	IPv4Address dest;
    double optimal;
    double simulation;
    double regret;
} ResultPacket;


/*!
 * @brief the regret class (it calculates regret for routes from one source (origin), to many destinations, through several nodes
 * @param x Number of iteration we want to attain
 * @param n The number of iteration at which we are currently
 * @param tableIP All IP addresses of sources (the matrix uses the same entries for lines and colons)
 * @param destinations All the IP addresses of destinations (destinations = forwarders)
 * @param nodes All the IP addresses of all smart nodes (nodes >=destinations)
 * @param origin The IP address of the unique source
 * @param matrix The result (measures) matrix
 * @param allPaths All paths leading to each destination
 * @param resultTable The latency of each path
 * @param optimalResultAfterN The optimal latency after N iterations for each destination
 * @param simulationResultAfterN The simulated latency after N iterations for each destination
 * @param listResultAfterN The list of ResultPacket for all destinations
 * @param result An intermediate variable to show the optimal path at each iteration
 *
 */
class Regret {
public :
		Regret(int hop, std::string fileName, std::string fileName2);

		void run();

		/*
		 * @brief Read the configuration from files
		 */
		void readListIP();
		void readConfigurationFromFile();

		void computeAllPath();

		/*
		 * @brief Read words from a line and return it's size
		 * Read words from a line and put them in matrix
		 */
		int readNumbers( const std::string & s );

		/*
		 * @brief Read result from "filename"
		 * Read result from "filename" and put them in matrix
		 */
		int readResultsFromFile();
		
		/*
		 * @brief Look up the result for a path from matrix
		 * Read the result for each segment of a path and push the sum in resultTable
		 */
		void lookUpForResult(IPPath path);

		/*
		 * @brief For all the paths, calculate the latency sum at the nth measure
		 *
		 */
		int calculateResult();

		/*
		 * @brief Find the optimal path for each destination at the nth measure
		 * 
		 */
		void findOptimalPath();

		/*
		 * @brief Read the simulation result
		 * Read the simulation result for each destination and add the result to simulationResultAfterN
		 */
		void readSimulationResult();

		/*
		 * @brief Compare the optimal and simulation result
		 * For each destination calculate the regret between simulation and optimal
		 */
		void simulationOptimalComparison();

		void writeRegretResultInFile();

private :
		int x,n;
		IPVector tableIP;
		IPVector destinations, nodes;
		IPv4Address origin;
		std::vector <double> matrix;

		/* attributes needed to read the result file*/
		std::ifstream matrixFile, simulationFile;
		int dim;
		int cols;
		int rows;

		bool endMatrixResult; /*! a boolean to indicate the end of the result matrix*/

		std::map<IPv4Address, std::vector<IPPath>> allPaths;
		std::map<IPPath, double > resultTable;
		std::map<IPv4Address, double> optimalResultAfterN, simulationResultAfterN;
		std::vector<ResultPacket> listResultAfterN;

		IPVector result;

};

#endif
