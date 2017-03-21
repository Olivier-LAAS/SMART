/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RoutingManagerEXP3.h
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 5 Apr 2015
 * @brief
 *
 */


#ifndef ROUTINGMANAGEREXP3_H
#define ROUTINGMANAGEREXP3_H

#include "common/common.h"
#include "RoutingManager.h"

class RoutingManagerEXP3 : public RoutingManagerBase {

public:
  RoutingManagerEXP3(size_t maxPathToTest=5, double gamma=0.9, MetricType mode=LATENCY_METRIC);

  void computePathsToTest(std::vector< IPPath> &pathsToTest) override;

  /**
   * @brief Method called by the path manager when he receive a new latency response
   */
  void putProbeResults(const PathEdgesResult& result) override;
  
  
  virtual std::map<IPv4Address, IPPath> getBestPathsFromResults() override;

protected:
  double gamma;
  size_t maxPathToTest;
  bool   firstRunEXP3;
  bool   allPathsComputed;
  std::map < IPv4Address , std::vector< IPPath > > allPaths;
  std::map< IPPath, Result > resulTable;  /**< measurement results */
  std::map < IPPath , size_t > allPathsId;
  std::map < IPv4Address , std::vector< double > > allPathsWeigths;
  std::map < IPv4Address , std::vector< double > > allPathsProba;
};

#endif // ROUTINGMANAGEREXP3_H
