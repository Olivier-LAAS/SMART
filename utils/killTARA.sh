#!/bin/bash

pkill RA;
ps -ef | grep TA | grep -v grep | awk '{print $2}' | xargs kill -9;

iptables -D OUTPUT -j NFQUEUE -d <dest_RA_TA_ip>

