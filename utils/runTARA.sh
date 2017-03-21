#!/bin/bash

nice --20 ./TA <routerip> <TA_ip> &
sleep .5
(nice --20 ./RA &> /dev/null) &
