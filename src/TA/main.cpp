/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file TAExe.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include <iostream>
#include <cstdlib>
#include "TA.h"

TransmissionAgent * tAgent;

void signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);
   tAgent->onExit();
}

int main (int argc, char *argv[]) {

	if (argc > 1) {
		TransmissionAgent transmissionAgent(argv[1], argv[2]);
		tAgent = &transmissionAgent;

		signal(SIGINT, signal_callback_handler);
		signal(SIGTERM, signal_callback_handler);

		transmissionAgent.run();

	}else{
		TransmissionAgent transmissionAgent;
		tAgent = &transmissionAgent;

		signal(SIGINT, signal_callback_handler);
		signal(SIGTERM, signal_callback_handler);

		transmissionAgent.run();
	}



    return EXIT_SUCCESS;
}
