/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                      *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                       *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence              *
 *                                                                          *
 ****************************************************************************/

/**
 * @file forwarder/main.cpp
 * @author Anouar Aaboub
 * @author Josselin Vallet
 * @author Rebecca Sau
 * @date 25 Mar 2015
 * @brief
 *
 */

#include "common/common.h"
#include "forwarder.h"



int main (int argc, char *argv[]) {
    std::cout << "SMART Forwarder running" << std::endl;

    if(argc > 1){
    	DataForwarder forwarder(argv[1]);
    	std::cout << "Data forwarder created" << std::endl;
    	forwarder.run();
    }else{
    	DataForwarder forwarder;
    	std::cout << "Data forwarder created" << std::endl;
    	forwarder.run();
    }

    return EXIT_SUCCESS;
}
