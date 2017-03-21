#!/bin/sh
# Install all dependencies
#

check_root() {
    if [ `whoami` != "root" ]
    then
        echo "must be root"
        exit 1
    fi
}

check_lib() {

    ldconfig -p | grep libnetfilter_queue.so > /dev/null
    if [ $? -eq 0 ]
    then
    	echo "Library libnetfilter_queue : OK"
    else
	echo "Instaling libnetfilter_queue ..."
	apt-get install libnetfilter-queue-dev -y
    fi

    ldconfig -p | grep nfnetlink.so > /dev/null
    if [ $? -eq 0 ]
    then
    	echo "Library nfnetlink : OK"
    else
	echo "Instaling nfnetlink..."
	apt-get install libnfnetlink-dev -y
    fi

    g++ --version | grep 4.8 > /dev/null
    if [ $? -eq 0 ]
    then
    	echo "C++11 standard installed : OK"
    else
	apt-get install python-software-properties -y
	add-apt-repository ppa:ubuntu-toolchain-r/test -y
	apt-get update

	echo "Instaling g++"
	apt-get install g++-4.8 -y
	rm /usr/bin/g++
	ln -s /usr/bin/g++-4.8 /usr/bin/g++
       	echo "Instaling cpp"
	apt-get install cpp-4.8 -y
	rm /usr/bin/cpp
	ln -s /usr/bin/cpp-4.8 /usr/bin/cpp
	echo "Instaling gcc"
	apt-get install gcc-4.8 -y
	rm /usr/bin/gcc
	ln -s /usr/bin/gcc-4.8 /usr/bin/gcc
    fi

    # installation de make
    command -v make >/dev/null 2>&1 || apt-get install make -y

    echo "Cheking all installation..."
    ldconfig -p | grep libnetfilter_queue.so > /dev/null
    is=$?
    ldconfig -p | grep nfnetlink.so > /dev/null
    is=`expr $is + $?`
    g++ --version | grep 4.8 > /dev/null
    is=`expr $is + $?`

    if [ $is != 0 ]
    then
	echo "There was an error while installing a library!!"
	exit 1
    fi

    echo "All dependencies installed"
}


check_root
check_lib
