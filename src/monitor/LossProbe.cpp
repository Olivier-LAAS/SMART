/****************************************************************************
 * Self MAnaged Routing overlay v1 - GPL v3                                 *
 *                                                                          *
 * This file is part of SMART (Self MAnaged Routing overlay)                *
 *                                                                          *
 * SMART is a free software distributed under the GPL v3 licence            *
 *                                                                          *
 ****************************************************************************/

/**
 * @file LossProbe.cpp
 * @author 
 * @date 2 Mai 2016
 * @brief
 *
 */


#include "LossProbe.h"

// without the extern "C", the code will not compile (ok with gcc but g++ poses problem)
extern "C"
{
#include "common/iperf/iperf_api.h"
#include "common/iperf/iperf_util.h"
#include "common/iperf/iperf.h"
}


ProbeLoss::ProbeLoss(){  
	// start the iperf server in a separate thread
	std::cerr<<"ProbeLoss()"<<std::endl; 
	iperfServerThread = std::thread(&ProbeLoss::runIperfServer, this);
}

void ProbeLoss::runIperfServer()
{
    int port = IPERF_PORT;
    struct iperf_test *test;
    int consecutive_errors;

    test = iperf_new_test();
    if ( test == NULL ) {
        fprintf( stderr, "### Failed to start iperf server \n" );
        exit( EXIT_FAILURE );
    }
    iperf_defaults( test );
    iperf_set_verbose( test, 0 ); // verbose
    iperf_set_test_role( test, 's' );
    iperf_set_test_server_port( test, port );

    consecutive_errors = 0;
    for (;;) {
        if ( iperf_run_server( test ) < 0 ) {
            fprintf( stderr, "### iperf error - %s\n\n", iperf_strerror( i_errno ) );
            ++consecutive_errors;
            if (consecutive_errors >= 5) {
                fprintf(stderr, "### iperf : too many errors, exiting\n");
                break;
            }
        } else
            consecutive_errors = 0;
        iperf_reset_test( test );
    }

    iperf_free_test( test );
    exit( EXIT_SUCCESS );
}

double ProbeLoss::getLoss(struct sockaddr_in *add)
{

    bool newTest = false;

    IPv4Address ipAdd = add->sin_addr.s_addr;

    struct timeval now;
    struct timeval start;
    
    gettimeofday(&now, NULL);

    if (iperfTestFreshness.find(ipAdd) != iperfTestFreshness.end()) {
        struct timeval freshness;
        freshness = iperfTestFreshness[ipAdd];

        long mtime = time_diff(freshness,now); // diff in ms

        if (mtime > MAX_FRESHNESS_IN_MS) { // if violated then erase result and proceed to a new one
//            std::cerr<<"### result no longer fresh, retesting"<<std::endl;
            iperfTestFreshness[ipAdd] = now;
            std::map<IPv4Address, double>::iterator it = iperfResults.find(ipAdd);
            if (it != iperfResults.end()) {
                iperfResults.erase(it);
                newTest = true;
            }
        }

    }
    else {
        newTest= true;
        iperfTestFreshness[ipAdd] = now;
    }

    if (!newTest) { // then get existing result or wait for old test to finish
        std::map<IPv4Address, double>::iterator it = iperfResults.find(ipAdd);
        if (it != iperfResults.end()) {
            //std::cerr<<"### result already fresh "<<it->second<<std::endl;
            return it->second;
        }
        else { // wait TODO threading of getLoss, without threading this condition should never occur
            std::cerr<<"### test already started -- waiting"<<std::endl;
            gettimeofday(&start, NULL);
            do {
                //usleep(500000); // sleep
                std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_RESULT_SLEEP_IN_MS));
                it = iperfResults.find(ipAdd);

                gettimeofday(&now, NULL);
                long mtime = time_diff(start,now); // diff in ms
                if (mtime > WAIT_RESULT_TIMEOUT_IN_MS) { // then timeout
                    std::cerr<<"###!! Test timeout stop waiting"<<std::endl;
                    return 0.0; // failed test return 0.0
                }

            } while (it == iperfResults.end());
            if (it != iperfResults.end()) {
                std::cerr<<"### test finsihed, getting result"<<std::endl;
                return it->second;
            }
            else {
                std::cerr<<"### problem no result"<<std::endl;
                return 0.0;
            }
        }
    }
    
    char * host = strdup(inet_ntoa(add->sin_addr)); // return the IP
    
    int port = IPERF_PORT;
    struct iperf_test *test;

    double theLoss = 0;

    int result = -1;
    
    gettimeofday(&start, NULL);

    do {

        test = iperf_new_test();
        if ( test == NULL ) {
            fprintf( stderr, "### failed to create test\n");
            return 0.0; // something went wrong, return 0 loss 
        }
        iperf_defaults( test );
        iperf_set_verbose( test, 0 ); // verbose

        iperf_set_test_role( test, 'c' );
        iperf_set_test_server_hostname( test, host );
        iperf_set_test_server_port( test, port );
        /* iperf_set_test_reverse( test, 1 ); */
        //! TODO verify impact of test duration, usually the higher the better. Disadvantage : various waiting tests and delays on bw restult fetching, consider threading
        //! attention weird behaviour detected when omit is activated, sometimes iperf keeps going on with weird time intervals!
        iperf_set_test_omit( test, 2 ); // in case we do not want to consider initial results -- this is important
        iperf_set_test_duration( test, 10 ); // test duration in seconds, must be int, cannot add fractions of seconds
        iperf_set_test_reporter_interval( test, 0 ); // to divide total duration into stat intervals
        iperf_set_test_stats_interval( test, 1 ); // to divide total duration into stat intervals


        result = iperf_run_client( test );
        if ( result < 0 ) { // client might be busy, try again

            gettimeofday(&now, NULL);
            long mtime = time_diff(start,now); // diff in ms

            if (mtime > IPERF_BUSY_SERVER_TIMEOUT_IN_MS) { // then timeout
                std::cerr<<"###!! Test timeout"<<std::endl;
                iperf_free_test( test );
                return 0.0; // failed test return 0.0
            }
            std::cerr<<"### failed test sleeping"<<std::endl;
            iperf_free_test( test );
            //usleep(400000);
            std::this_thread::sleep_for(std::chrono::milliseconds(IPERF_FAIL_SLEEP_IN_MS));

        }

    } while (result < 0);

    struct iperf_stream *sp = NULL;
    sp = SLIST_FIRST(&test->streams);
    if (sp) {
// TODO: extract number of loss
	iperf_size_t bytes_sent = sp->result->bytes_sent;
	int retransmit = sp->result->stream_retrans;
std::cerr << std::endl << std::endl;
std::cerr << "Bytes received : " << sp->result->bytes_received << std::endl;
std::cerr << "Bytes send : " << sp->result->bytes_sent << std::endl;
std::cerr << "Blocks size : " << test->bytes_sent/test->blocks_sent << ", " << test->bytes_sent/(test->blocks_sent*1024) << "kB" << std::endl;
std::cerr << "Blocks sent : " << test->blocks_sent << std::endl;
std::cerr << "Loss rate 2 : " << 100 * (double)sp->result->stream_retrans / (double)test->blocks_sent << std::endl;
std::cerr << "Total loss rate 2 : " << 100 * (double)sp->result->stream_prev_total_retrans / (double)test->blocks_sent << std::endl;
std::cerr << "Max send cwnd : " << sp->result->stream_max_snd_cwnd << std::endl;
std::cerr << "Previous total retransmit : " << sp->result->stream_prev_total_retrans << std::endl;
std::cerr << "Retransmit : " << sp->result->stream_retrans << std::endl << std::endl;

//	std::cerr << "Retransmit : " << retransmit << ", sent : " << bytes_sent << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;
	theLoss = 100 * (double)sp->result->stream_retrans / (double)test->blocks_sent;
    }

    iperf_free_test( test );
    
    gettimeofday(&now, NULL);
    iperfTestFreshness[ipAdd] = now; // update date of result
    iperfResults[ipAdd]=theLoss;

    return theLoss;
}

