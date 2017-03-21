/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file DC/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */
#include "common/common.h"
#include "CU.h"


int main (int argc, char *argv[]) {
    std::cout << "SMART CU running" << std::endl;

    CU cu;
    std::cout << "CU created" << std::endl;
    cu.waitForCloudConnection();


    return EXIT_SUCCESS;
}
