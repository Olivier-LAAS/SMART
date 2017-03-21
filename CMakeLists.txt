############################################################################
# Self MAnaged Routing overlay v1 - GPL v3                                 #
#                                                                          #
# This file is part of SMART (Self MAnaged Routing overlay)                #
#                                                                          #
# SMART is a free software distributed under the GPL v3 licence            #
#                                                                          #
############################################################################

##
# @file CMakeLists.txt
# @author Anouar Aaboub
# @author Josselin Vallet
# @author Rebecca Sau
# @date 25 Mar 2015
# @brief
#
#

cmake_minimum_required(VERSION 2.6)

project(SMART)

find_package(Threads)

# using C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/exe)

# include root directory
include_directories(src )

##################### Common ######################
file(GLOB_RECURSE common_source_files	src/common/*)
add_library(common STATIC ${common_source_files})


##################### iperf ######################
file(GLOB_RECURSE iperf_source_files	src/common/iperf/*)
add_library(iperf STATIC ${iperf_source_files})


################### Forwarder #####################
file(GLOB_RECURSE forwarder_source_files src/forwarder/* )
add_executable(
	forwarder
	${forwarder_source_files})

target_link_libraries(
	forwarder
	common
	${CMAKE_THREAD_LIBS_INIT})


#################### Monitor ######################
file(GLOB_RECURSE monitor_source_files src/monitor/*)
add_executable(
	monitor
	${monitor_source_files})

target_link_libraries(
	monitor
	common
	iperf
	${CMAKE_THREAD_LIBS_INIT})


##################### Router #####################
file(GLOB_RECURSE router_source_files src/router/*)
add_executable(
	router
	${router_source_files})

target_link_libraries(
	router
	common
	${CMAKE_THREAD_LIBS_INIT})


###################### RA #######################
file(GLOB_RECURSE RA_source_files src/RA/*)
add_executable(
	RA
	${RA_source_files})

target_link_libraries(
	RA
	common
	${CMAKE_THREAD_LIBS_INIT})


###################### TA ######################
file(GLOB_RECURSE TA_source_files src/TA/*)
add_executable(
	TA
	${TA_source_files})

target_link_libraries(
	TA
	common
	nfnetlink
	netfilter_queue
	${CMAKE_THREAD_LIBS_INIT})

###################### Regret ######################
file(GLOB_RECURSE regret_source_files src/regret/*)
add_executable(
	regret
	${regret_source_files})

target_link_libraries(
	regret
	common
	nfnetlink
	netfilter_queue
	${CMAKE_THREAD_LIBS_INIT})

##################### CU #####################
file(GLOB_RECURSE CU_source_files src/CU/*)
add_executable(
	CU
	${CU_source_files})

target_link_libraries(
	CU
	common
	${CMAKE_THREAD_LIBS_INIT})

############# Doxygen Documentation #############
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(
		doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM )
endif(DOXYGEN_FOUND)

