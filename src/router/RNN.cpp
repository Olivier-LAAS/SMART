/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RNN.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */


//****************************************************************//
//*                                                              *//
//*------------------- Includes SECTION -------------------------*//
//*                                                              *//
//****************************************************************//


#include <cstring>
#include <common/Eigen/Dense>

#include "common/common.h"
#include "RNN.h"


#define MAX(x,y)  ((x)>(y)?(x):(y))
#define MIN(x,y)  ((x)<(y)?(x):(y))
#define MAX_POW   100

using Eigen::MatrixXd;

using namespace Eigen;


//****************************************************************//
//*                                                              *//
//*----------- Private Functions Definition SECTION -------------*//
//*                                                              *//
//****************************************************************//


//----------------------------------------------------------------//
// Private Function:  matrix_norm                                 //
// compute the norm-1 of a square matrix                          //
//----------------------------------------------------------------//

double matrix_norm(MatrixXd & M) {
  int S = M.rows();
  double norm = 0;

  for (int i = 0; i<S; i++)
    norm = MAX(norm, M.col(i).lpNorm<1>());

  return norm;
}


//----------------------------------------------------------------//
// Public Method:       Constructor                               //
//----------------------------------------------------------------//

RNN::RNN( size_t maxPathsToTest, MetricType mode ){
  mode_ = mode;
  nb_choices = maxPathsToTest;
  beta_=0.95;
  minimize_=true;
  decision_thr_ = 0.01;
  initialisation=false;
}

//----------------------------------------------------------------//
// Public Method:  computePathsToTest                             //
//----------------------------------------------------------------//

void RNN::computePathsToTest(std::vector< IPPath> &pathsToTest) {

    if (initialisation==false) {
        allPaths = computeAllPaths();
        initialisation = true;
        for (std::map < IPv4Address , std::vector< IPPath > >::iterator it = allPaths.begin(); it != allPaths.end(); it++) {
        	IPv4Address dest=it->first;
        	int N = it->second.size();

            for (size_t i=0; i<it->second.size(); i++) {
            	Neuron  node(it->second.at(i),i);
            	neuron[dest].push_back(node);

            	//initialization of cumulated losses and frequencies
            	neuron[dest].at(i).cumulated_loss = 0.0;
            	neuron[dest].at(i).freq = 0.0;
            	neuron[dest].at(i).reward_ = 0.0;
            }
            initWeights(N,dest);
        }
    }
    pathsToTest.clear();

    for(map<IPv4Address,vector<Neuron>>::iterator itt = neuron.begin(); itt != neuron.end(); itt++){
    	IPv4Address dest = itt ->first;
    	int N = itt->second.size();

    	IPPath internetPath;
    	internetPath.push_back(dest);

    	pathsToTest.push_back(internetPath);

    	//select the nb_coices neurons with the highest probability of being excited
        getBestNeuron(nb_choices,N,dest);
       // cout << "getBestNeuron" << endl;


        for (int i = 0; i < N; i++) {
        	//      cerr << "Reward obtained with neuron " << i << " : " << reward[i] << endl;
        	if (set_of_best_neurons[dest].find(i) != set_of_best_neurons[dest].end()){
        		IPPath path=neuron[dest].at(i).path();
        		pathsToTest.push_back(path);
        	}
        }

          //      cerr << "New decision theshold: " << decision_thr_ << endl;
    }
}

//----------------------------------------------------------------//
// Public Method:  initWeights                                    //
//----------------------------------------------------------------//

void RNN::initWeights(int N,IPv4Address dest){
  size_t     i,j;
  double  r, sum;

  //loops on all neurons i and j
  for (i=0; i<N; i++) {
    sum = 0.0;

    //get the rate of neuron i
    r = rate(i,dest);

    for (j=0; j<N; j++) {

      // update positive weight i -> j
      setPositiveWeight(i,j,r/(2.0*N),dest);

      // update negative weight i -> j
      setNegativeWeight(i,j,r/(2.0*N),dest);

      sum += positiveWeight(i,j,dest)+negativeWeight(i,j,dest);
    } // end loop on j
    //    cerr << "Init weights of neuron " << i << ": " << sum/r << endl;
  } //end loop on i
}

//----------------------------------------------------------------//
// Public Method:     getBestNeuron                               //
//----------------------------------------------------------------//

void RNN::getBestNeuron( int nb_choices, int N, IPv4Address dest) {
  int                             best_neuron;
  set<int>                        set_of_worst_neurons;
  set<int>::const_iterator        it;
  double                          max_proba;
  double                          p;
  size_t                             i,j;

  //clear the set of best neurons
  set_of_best_neurons[dest].clear();

  //initialize the set of worst neurons to the full set of neurons
  for (i=0; i<N; i++){
    if ( neuron[dest][i].sizeOfPath() == 1 )
      continue;
    set_of_worst_neurons.insert(i);
  }

  //insert the nb_choices best neurons into the set of best neurons one by one
  //for that, choose the best one among the set of worst ones

  while ( set_of_best_neurons[dest].size() < nb_choices ) {
    j = 0;
    max_proba = 0.0;
    it = set_of_worst_neurons.begin();
    for ( ; it != set_of_worst_neurons.end(); ++it) {
      j = *it;
      p = neuron[dest].at(j).proba();   //proba(j, dest);
      if ( p > max_proba ) {
	best_neuron = j;
	max_proba = p;
      }
    }// end loop on worst neurons
    set_of_best_neurons[dest].insert( best_neuron );
    set_of_worst_neurons.erase( best_neuron );
  }

}

//----------------------------------------------------------------//
// Public Method:       putProbeResults                           //
//----------------------------------------------------------------//

void RNN::putProbeResults(const PathEdgesResult& result) {
	if (result.metric == LATENCY_METRIC){
		double totalRTT = result.getTotalRTT();
		if (totalRTT < pathManager->probeTimeout) {
			Result& res = resulTable[result.path];
			// This was added to be sure the metric type is passed
			res.metric = result.metric;
			// update average value
			double r =(double)(res.averageResult * res.packetsReceived + totalRTT) / (double)(res.packetsReceived +1);
			res.averageResult = r;
			res.packetsReceived++;

			for(size_t i=0; i<neuron[result.path.back()].size(); i++){
				if(equal(result.path,neuron[result.path.back()].at(i).path())){
					if (minimize_){
						neuron[result.path.back()].at(i).reward_ = 1.0 / r;
					}else{
						neuron[result.path.back()].at(i).reward_ = r;
						//vectorNeuron.at(i).cumulated_loss += r;
					}
				}
			}
		}
	}else if (result.metric == LOSS_METRIC){

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

		for(size_t i=0; i<neuron[result.path.back()].size(); i++){
			if(equal(result.path,neuron[result.path.back()].at(i).path())){
			  neuron[result.path.back()].at(i).reward_ = min/300.0;
			}
		}
	}
}


bool RNN::equal(IPPath path, IPPath vector){
	bool equal = false;
	if(path.size()==vector.size()){
		for(size_t i=0; i<path.size(); i++){
			if(path.at(i)==vector.at(i)){
				equal=true;
			}else{
				equal=false;
				break;
			}
		}
	}
	return equal;
}


//----------------------------------------------------------------//
// Public Method:       update		                              //
//----------------------------------------------------------------//

void RNN::update(){
	int             t = 0;
	double          best_reward, x;
	double          cumulated_loss_rnn = 0.0;
	int             i, j, k;




	for(map<IPv4Address,vector<Neuron>>::iterator it = neuron.begin(); it != neuron.end(); it++){
		IPv4Address dest = it->first;


		int N = neuron[dest].size();
		//compute the loss of each decision and the reward R(t) of neuron j
		best_reward = 0.0;
		for (i = 0; i < N; i++) {
			//      cerr << "Reward obtained with neuron " << i << " : " << reward[i] << endl;
			if (set_of_best_neurons[dest].find(i) != set_of_best_neurons[dest].end()) {
				if (neuron[dest].at(i).reward_ > best_reward) {
					best_reward = neuron[dest].at(i).reward_;
					j = i;
				}
			}
		}
		//updated cumulated loss of the algorithm and freq of best neuron
		cumulated_loss_rnn += 1.0 / best_reward;
		neuron[dest].at(j).freq += 1.0;
		//    cerr << "Selected neuron: " << j << endl;

		//update weights and probabilities
		//set<int>::iterator itt = set_of_best_neurons[dest].begin();

		for (set<int>::iterator itt = set_of_best_neurons[dest].begin(); itt != set_of_best_neurons[dest].end(); ++itt) {
		   // std::cout << "blablabla " << std::endl;

			//j is the next neuron in the set
			j = *itt;

			//update weights
			updateWeights(j, neuron[dest].at(j).reward_, N, dest);

			//normalize weights
			normalizeWeights(N, dest);
			//cout << "normalizeWeights" << endl;

			//recompute neuron's probabilities
			updateProbabilities(N, dest);;
			//cout << "updateProbabilities" << endl;

			//update decision threshold
			decision_thr_ = beta_ * decision_thr_+(1 - beta_)*neuron[dest].at(j).reward_;
		}
	}
}


//----------------------------------------------------------------//
// Public Method:   updateWeights                                 //
//----------------------------------------------------------------//

void RNN::updateWeights(int j, double reward, int N, IPv4Address dest) {
  size_t     i,k;
  double  w;
  double  norm_reward = reward/decision_thr_;
  double  change;


  //first case: reward is greater than decision threshold
  if ( decision_thr_ <= reward ) {
    //    cerr << "Reward is higher than decision threshold!" << endl;
    for (i=0; i<N; i++) {
      w = positiveWeight(i,j,dest);
      change = (norm_reward-1.0)*w;
      setPositiveWeight(i,j,w+change, dest);
      //	  cerr << "w^+(" << i << "," << j << ") = " << positiveWeight(i,j) << endl;
      for (k=0; k<N; k++) {
	if ( k == j ) continue;
	w = negativeWeight(i,k,dest);
	setNegativeWeight(i,k,w+change/(N-2), dest);
	//	  cerr << "w^-(" << i << "," << k << ") = " << negativeWeight(i,k) << endl;
      } // loop on k
    } // loop on i
  } // end if
  //second case: reward is lower than decision threshold
  else {
    //    cerr << "Reward is lower than decision threshold!" << endl;
    for (i=0; i<N; i++) {
      w = negativeWeight(i,j,dest);
      change = (1.0-norm_reward)*w;
      setNegativeWeight(i,j,w+change,dest);
      //	  cerr << "w^-(" << i << "," << j << ") = " << negativeWeight(i,j) << endl;
      for (k=0; k<N; k++) {
	if ( k == j ) continue;
	w = positiveWeight(i,k,dest);
	setPositiveWeight(i,k,w+change/(N-2),dest);
	//	  cerr << "w^+(" << i << "," << k << ") = " << positiveWeight(i,k) << endl;
      } // loop on k
    } // loop on i
  } // end else
}


//----------------------------------------------------------------//
// Public Method:  normalizeWeights                               //
//----------------------------------------------------------------//

void RNN::normalizeWeights(int N, IPv4Address dest) {
	size_t     i,j;
  double  w;
  double  r, r_star;

  //loop on all neurons i
  //  cerr << "Weight normalization:" << endl;
  for (i=0; i<N; i++) {

    //init r_star to 0 and r to neuron's rate
    r_star = 0.0;
    r = rate(i, dest);

    //first loop on all neurons to compute r_star=sum of positive and negative weights
    for (j=0; j<N; j++)
      r_star += positiveWeight(i,j,dest)+negativeWeight(i,j,dest);
    //    cerr << "r(" << i << ")=" << r << " and r_star(" << i << ")=" << r_star << endl;

    //second loop on all neurons to normalize weights
    for (j=0; j<N; j++) {
      //normalize positive weight
      w = positiveWeight(i,j,dest);
      w *= r/r_star;
      setPositiveWeight(i,j,w, dest);
      //      cerr << "w^+(" << i << "," << j << ") = " << positiveWeight(i,j) << endl;

      //normalize negative weight
      w = negativeWeight(i,j,dest);
      w *= r/r_star;
      setNegativeWeight(i,j,w, dest);
      //      cerr << "w^-(" << i << "," << j << ") = " << negativeWeight(i,j) << endl;
    } // end loop on j
  } //end loop on i
}


//----------------------------------------------------------------//
// Public Method:  updateProbabilities                            //
//----------------------------------------------------------------//

void RNN::updateProbabilities(int N, IPv4Address dest) {
  MatrixXd     pos_P(N,N);
  MatrixXd     neg_P(N,N);
  MatrixXd     F(N,N);
  MatrixXd     A(N,N);
  MatrixXd     M(N,N);
  RowVectorXd  pos_lambda(N);
  RowVectorXd  neg_lambda(N);
  RowVectorXd  excit_lambda(N);
  RowVectorXd  inhib_lambda(N);
  RowVectorXd  f(N);
  RowVectorXd  neg_lambda_old(N);
  RowVectorXd  r(N);
  RowVectorXd  g(N);
 // RowVectorXd  q(N);
  RowVectorXd  err_vect(N);
  int          i,j,k,nb_it;
  double       err;
  double q;

  //initialization
  for (i=0; i<N; i++) {

    //initiaization of vectors
    r(i) = rate(i,dest);
    excit_lambda(i) = excitLambda(i,dest);
    inhib_lambda(i) = inhibLambda(i,dest);
    neg_lambda(i) = 1.2*inhibLambda(i,dest);

    /*
    cout << r(i) << endl;
    cout << excit_lambda(i) << endl;
    cout << inhib_lambda(i) << endl;
    cout << neg_lambda(i) << endl;
    */

    //initialization of matrices
    for (j=0; j<N; j++) {
      pos_P(i,j) = positiveWeight(i,j,dest)/r(i);
      neg_P(i,j) = negativeWeight(i,j,dest)/r(i);

    }
  }
  neg_lambda_old = neg_lambda;
  //std::cout << "initialization" << std::endl;

  //fixed-point loop
  for (nb_it=0; nb_it <100; nb_it++) {

//     if ( nb_it % 10 == 0 )
//       cerr << "ITERATION " << nb_it << endl;

    //compute the matrices F and A=F*pos_P
    for (i=0; i<N; i++) {
      f(i) = r(i)/(r(i)+neg_lambda(i));
//       if ( nb_it % 10 == 0 )
// 	cerr << "f(" << i << ") = " << f(i) << endl;
    }
    F = f.asDiagonal();
    A = F*pos_P;

   // std::cout << "compute F and A" << std::endl;

    //    cerr << "Matrix F:" << endl << F << endl;
    //    cerr << "Matrix A:" << endl << A << endl;

    //compute M=\sum_{n=0}^\infty{(F*pos_P)^n}
    M = MatrixXd::Identity(N,N);
    for (k=0; k <MAX_POW; k++) {
      M = M+A;
      A = A*A;
      if ( k % 10 == 0 ) {
	if ( matrix_norm(A) < EPSILON )
	  break;
      }
    }
    //    cerr << "Matrix M:" << endl << M << endl;
   // std::cout << "compute M" << std::endl;


    //compute the vector g
    g = excit_lambda.transpose();
    g = g*M*F*neg_P;

   // std::cout << "compute the vector g" << std::endl;
    //update the vector neg_lambda et pos_lambda
    neg_lambda = inhib_lambda + g;
    pos_lambda = excit_lambda.transpose();
    pos_lambda = pos_lambda*M;

   // std::cout << "update the vector" << std::endl;
    //convergence test
    err_vect = neg_lambda - neg_lambda_old;
    err = err_vect.lpNorm<1>();
    if ( err < EPSILON*neg_lambda.lpNorm<1>() ) {
//       cerr << "FINAL ITERATION : " << nb_it << endl;
//       for (i=0; i<N; i++)
// 	cerr << "f(" << i << ") = " << f(i) << endl;
      break;
    }

    //copy of neg_lambda for next iteration
    neg_lambda_old = neg_lambda;
  }

  //check that the solution is nonnegative and update neuron's probabilities
  for (i=0; i<N; i++) {
    if ( neg_lambda(i) < 0.0 )
      cerr << "Erreur: lambda^-(" << i << ") = " << neg_lambda(i) << endl;
    if ( pos_lambda(i) < 0.0 )
      cerr << "Erreur: lambda^+(" << i << ") = " << pos_lambda(i) << endl;
    q = MIN(1.0,pos_lambda(i)/(r(i)+neg_lambda(i)));
    //    cerr << "q(" << i << ") = " << q(i) << endl;
    //setProba(i,q(i),dest);
    neuron[dest].at(i).setProba(q);
  }

}


//----------------------------------------------------------------//
// Public Method:  getBestPathsFromResults                        //
//----------------------------------------------------------------//

std::map<IPv4Address, IPPath> RNN::getBestPathsFromResults() {

	update();

	std::map<IPv4Address, IPPath> proposedPaths;
	for (const IPv4Address dest : index_destinations) {
		double minResult = 1.0e10;
		int NumberMinHop = 4;
		double maxResult = 0;
		std::map<IPPath, Result>::iterator minPathIter;
		bool found = false;

		for (std::map<IPPath, Result>::iterator ii = resulTable.begin();ii != resulTable.end(); ++ii) {
			const IPPath& thepath = ii->first;
			const Result & theresults = ii->second;

			if (dest == thepath.back()) {

				if (theresults.metric == LATENCY_METRIC) {
					double latency = theresults.averageResult;

					if (latency < minResult) {
						minResult = latency;
					}
				} else if (theresults.metric == LOSS_METRIC) {
					double losses = 100*(1.0-(double) theresults.packetsReceived/(double) PROBE_PACKETS_TO_SEND);

					if (losses < minResult) {
						minResult = losses;
					}
				} else if (theresults.metric == BW_METRIC) {
					double BW = theresults.averageResult;

					if (BW > maxResult) {
						maxResult = BW;
					}
				}
			}
		}

		for (std::map<IPPath, Result>::iterator ii = resulTable.begin();
				ii != resulTable.end(); ++ii) {
			const IPPath& thepath = ii->first;
			const Result & theresults = ii->second;

			if (dest == thepath.back()) {
				int NumberHop = thepath.size();
				double result = 0;
				double BW = 1;

				if (theresults.metric == LATENCY_METRIC) {
					result = theresults.averageResult;

				} else if (theresults.metric == LOSS_METRIC) {
					result = 100*(1.0-(double) theresults.packetsReceived/(double) PROBE_PACKETS_TO_SEND);
				}else if (theresults.metric == BW_METRIC){
        			BW = theresults.averageResult;
				}

				if (result == minResult && NumberHop <= NumberMinHop) {
					NumberMinHop = NumberHop;
					minPathIter = ii;
					found = true;

				}

				if (BW == maxResult && NumberHop<=NumberMinHop) {
					NumberMinHop = NumberHop;
					minPathIter = ii;
					found = true;
				}
			}
		}
		if (found) {
			proposedPaths[dest] = minPathIter->first;
		}
	}
	resulTable.clear();
	return proposedPaths;
}

