/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file SimulationManager.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#ifndef SIMULATIONMANAGER_H
#define SIMULATIONMANAGER_H

#include "common/common.h"
#include "RoutingManagerInterface.h"


using namespace std;


/*!
 * @brief the main class for simulation
 * Simulation still needs configuration files (see README for more information)
 * Simulation uses available measurements offline (no need to have a network)
 * The measurements are available as a two dimension matrix (source -> destination)
 * @param tableIP All IP addresses of sources (the matrix uses the same entries for lines and colons)
 * @param matrix The measurement matrix
 * @param file The measurement filename
 * @param dim The matrix dimension
 * @param cols The number of colons
 * @param rows The number of lines
 * @param inst The measurement instant (used by simulation)
 *
 */
class SimulationManager {
public :
		SimulationManager(string fileName);
		~SimulationManager();

		// Configuration
		void setRoutingManager(RoutingManagerInterface *  routingManager) {this->routingManager = routingManager;}

		int run(MetricType metric, vector<IPPath> paths);

		void readListIP();

		/*
		 * @brief Read the configuration from files
		 */
		void readConfigurationFromFile();


		/*
		 * @brief Read words from a line and return it's size
		 * Read words from a line and put them in matrix
		 */
		int readNumbers( const string & s );


		/*
		 * @brief Read result from "filename"
		 * Read measurements from "filename" and put them in matrix
		 */
		int readResultsFromFile();

		/*
		 * @brief Look result for path in matrix
		 * For a path, look up all the measurements in matrix and put them in a measurement packet (result packet)
		 */
		void lookUpForResult();

		void writeResultInFile(MetricType mode, map<IPv4Address, IPPath> bestPath, int time);

		double calculateLatency(MetricType metric, IPPath path);

		void writeMatrice();
		void writeLenght();


		bool endMatrixResult;

		double matrixValue(int x, int y) { return matrix[x*cols+y]; }

		double matrixValue(IPv4Address src, IPv4Address dst) {
		  return matrixValue( id_[src], id_[dst] );
		}
		
		double compute_optimal_two_hop_distance(IPv4Address src, IPv4Address dst, MetricType mode);

		double compute_optimal_distance(IPv4Address src, IPv4Address dst, MetricType mode);

		void getError(MetricType mode, IPv4Address & src, IPv4Address & dst, IPPath & path, vector<double> & errors);


private :
		IPVector                tableIP;
		map<IPv4Address,int>    id_;
		vector <double>    matrix;
		ifstream           file;
		int                     dim;
		int                     cols;
		int                     rows;
		int                     inst;

        /*!
         * Used for simulating the Monitor Manager behavior
         */
		MonitoringInstructionPacket monitoringPacket;
		RoutingManagerInterface * routingManager;



		/*
		 * Used to decide on the length of paths (2 or 3)
		 */
		map<string, double> Length;
        // normally to be removed
		double reward;
};


#endif
