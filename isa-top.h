#ifndef ISA_TOP_H
#define ISA_TOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pcap.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>

#include <pthread.h>

#include "printer.h"
#include "hashtable.h"
#include "help.h"
#include "arg_parse.h"


/**
 * @brief Function analyzes packets and sends relevant informations into hashtable
 * 
 * @param args Argument used by pcap_loop (unused)
 * @param header Pointer to packet header
 * @param packet Pointer to packet data
 */
void packet_analyzer(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

/**
 * @brief Function for capturing packets in separate thread, nonstop running pcap_loop
 * 
 * @param arg Pointer to session handle
 */
void *packet_capture_thread(void *arg);

/**
 * @brief Function for periodical printing of connection stats in separate thread
 * 
 * @param arg Unused argument ?
 */
void *stats_print_thread(void *arg);

#endif