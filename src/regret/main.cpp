/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RegretManager/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "RegretManager.h"


int main (int argc, char *argv[]) {
	std::cout << "SMART Regret running" << std::endl;
	int nbMax = 99999;

	if (argc > 3) // the user want to use a specific kind of routing manager
	        nbMax = atoi(argv[3]);

	Regret regret(nbMax, std::string(argv[1]), std::string(argv[2]));

	regret.readListIP();
	regret.readConfigurationFromFile();
	regret.run();

    return EXIT_SUCCESS;
}
