/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file RA/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#include <iostream>
#include <cstdlib>
#include "RA.h"

int main (int argc, char *argv[]) {
    std::cout << "SMART RA running" << std::endl;
    ReceptionAgent receptionAgent;
    receptionAgent.run();

    return EXIT_SUCCESS;
}

