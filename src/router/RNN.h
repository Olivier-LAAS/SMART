/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RNN.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */



#ifndef __RNN_hxx__
#define __RNN_hxx__


#include "common/common.h"
#include "RoutingManager.h"


#define EPSILON  1.0e-5


//****************************************************************//
//*                                                              *//
//------------ Public Data Structures SECTION --------------------//
//*                                                              *//
//****************************************************************//

using namespace std;






//****************************************************************//
//*                                                              *//
//----------------- Public Classes SECTION -----------------------//
//*                                                              *//
//****************************************************************//



//----------------------------------------------------------------//
// Class :      Neuron                                            //
//                                                                //
//                                                                //
// Description : classe representant un neurone d'un RNN          //
//                                                                //
//----------------------------------------------------------------//


typedef map<int, double>::const_iterator IntDoubleIt;

class Neuron {
	// attributs prives
private:
	int decision_id_; //id of the decision associated to this neuron
	double rate_; //rate at which this neuron send spikes to other neurons (IT HAS TO BE >0)
	double proba_; //probability that this neuron be excited
	double excit_lambda_; //Exogeneous excitatory signals entering this neuron from outside the RNN
	double inhib_lambda_; //Exogeneous inhibitory signals entering this neuron from outside the RNN
	map<int, double> positive_weights_; //w^+(i,j)
	map<int, double> negative_weights_; //w^-(i,j)

	IPPath pathNeuron; //path

	// methodes de classe
public:
	double         reward_;
	double         cumulated_loss;
	double         freq;

	// constructeurs
	//constructeur par defaut
	Neuron(IPPath path,int id = 0, double rate = 1.0, double proba = 0.00001,double excit_lambda = 1.0, double inhib_lambda = 1.0) :
		pathNeuron(path), decision_id_(id), rate_(rate), proba_(proba), excit_lambda_(
					excit_lambda), inhib_lambda_(inhib_lambda)  {
		reward_=0.0;
	}

	//constructeur par recopie
	Neuron(const Neuron &other) :
		pathNeuron(other.pathNeuron), decision_id_(other.decision_id_), rate_(other.rate_), proba_(
					other.proba_), excit_lambda_(other.excit_lambda_), inhib_lambda_(
					other.inhib_lambda_), positive_weights_(
					other.positive_weights_), negative_weights_(
					other.negative_weights_) {
	}

	//accesseurs
	int const decisionId() const {return decision_id_;}

	double const rate() const {	return rate_;}

	double const proba() const {return proba_;}

	double const excitLambda() const {return excit_lambda_;}

	double const inhibLambda() const {return inhib_lambda_;	}

	double const positiveWeight(int j) {return positive_weights_[j];}

	double const negativeWeight(int j) {return negative_weights_[j];}

	IPPath const path() const {return pathNeuron;}

	int    const sizeOfPath() const { return pathNeuron.size(); }

	//double const result() const {return result_;}

	IntDoubleIt const positiveWeightBegin() const {	return positive_weights_.begin();}

	IntDoubleIt const positiveWeightEnd() const {return positive_weights_.end();}

	IntDoubleIt const negativeWeightBegin() const {return negative_weights_.begin();}

	IntDoubleIt const negativeWeightEnd() const {return negative_weights_.end();}

	void setDecisionId(int i) {decision_id_ = i;}

	void setRate(double r) {rate_ = r;}

	void setProba(double p) {proba_ = p;}

	void setExcitLambda(double l) {	excit_lambda_ = l;}

	void setInhibLambda(double l) {inhib_lambda_ = l;}

	void setPositiveWeight(int j, double v) {positive_weights_[j] = v;}

	void setNegativeWeight(int j, double v) {negative_weights_[j] = v;}

	//void setResult(double r) {result_ = r;}

};


//----------------------------------------------------------------//
// Class :   RNN                                                  //
//                                                                //
//                                                                //
// Description : classe representant un reseau de neurones        //
//                                                                //
//----------------------------------------------------------------//

class RNN: public RoutingManagerBase {
	// attributs prives
private:
  int nb; //number of neuron
  int nb_choices; //number of path to test
  double decision_thr_; //decision threshold
  double beta_; //coefficient for updating decision threshold
  bool minimize_; //true if the goal is to minimize, in which case reward=1/cost
  //vector<ExpoGen>          decision_; //random generators associated to decisions
  map<IPv4Address,vector<Neuron>> neuron; //List of Neuron for each destination

  bool initialisation;
  std::map < IPv4Address , std::vector< IPPath > > allPaths;
  std::map< IPPath, Result > resulTable;  /**< measurement results */
  std::map<IPv4Address, set<int> > set_of_best_neurons;


	// methodes de classe
public:

	// constructeurs

  RNN(size_t maxPathsToTest, MetricType mode);

  // destructeur
  ~RNN() {
  }
  
  // accesseurs
  
  // int              id() { return id_; }


  int numberOfNeurons(IPv4Address dest) {return neuron[dest].size();}
  
  int rate(int i, IPv4Address dest)  {return neuron[dest].at(i).rate();}
  
  int proba(int i, IPv4Address dest)  {return neuron[dest].at(i).proba();}
  
  double const excitLambda(int i, IPv4Address dest)  {return neuron[dest].at(i).excitLambda();}
  
  double const inhibLambda(int i, IPv4Address dest)  {return neuron[dest].at(i).inhibLambda();}
  
  double const positiveWeight(int i, int j, IPv4Address dest) {return neuron[dest].at(i).positiveWeight(j);}
  
  double const negativeWeight(int i, int j, IPv4Address dest) {return neuron[dest].at(i).negativeWeight(j);}
  
  
  double decisionThreshold() {return decision_thr_;}
  
  double beta() {return beta_;}
  
  void setRate(size_t i, double r, IPv4Address dest) {neuron[dest].at(i).setRate(r);}
  
  //void setProba(size_t i, double p, IPv4Address dest) {neuron[dest].at(i).setProba(p);}
  
  void setPositiveWeight(size_t i, size_t j, double v, IPv4Address dest) {neuron[dest].at(i).setPositiveWeight(j, v);}
  
  void setNegativeWeight(size_t i, size_t j, double v, IPv4Address dest) {neuron[dest].at(i).setNegativeWeight(j, v);}
  
  void setDecisionThreshold(double x) {decision_thr_ = x;}
  
  void setBeta(double b) {beta_ = b;}

  
  //methodes de calcul
  
  void update();
  
  void initWeights(int N, IPv4Address dest);
  
  void getBestNeuron( int nb_choices, int N, IPv4Address dest);
  
  void updateWeights(int j, double reward, int N, IPv4Address dest);
  
  void normalizeWeights(int N, IPv4Address dest);
  
  void updateProbabilities(int N, IPv4Address dest);
  
  void computePathsToTest(std::vector< IPPath> &pathsToTest) override;
  
  /**
   * @brief Method called by the path manager when he receive a new latency response
   */
  
  void putProbeResults(const PathEdgesResult& result) override;
  
  virtual std::map<IPv4Address, IPPath> getBestPathsFromResults() override;
  
  bool equal(IPPath path, IPPath vector);
};

#endif


