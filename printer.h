#ifndef PRINTER_H
#define PRINTER_H

#include <stdlib.h>
#include <pcap.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>

#include "hashtable.h"
#include <ncurses.h>

#define TOP_CONNECTIONS_LIMIT 10

// Array for keeping the 10 connections with highest traffic, defined in printer.c
extern connection_stats_t *top_connections[TOP_CONNECTIONS_LIMIT];

/**
 * @brief Function for sorting top 10 connections using Bubble sort algorithm
 * 
 * @param table Pointer to the hashtable
 * @param top_connections Array for storing pointers of 10 connections with highest traffic
 * @param sort_by_packets Flag determining whether sorting will be performed based on bits or packets (0 = bits, 1 = packets)
 */
void get_top_connections(connection_table_t *table, connection_stats_t *top_connections[], int sort_by_packets);

/**
 * @brief Function for printing connection stats in a formatted table using ncurses
 * 
 * @param table Pointer to the hashtable
 * @param sort_by_packets Flag determining whether sorting will be performed based on bits or packets (0 = bits, 1 = packets)
 */
void print_connections_stats(connection_table_t *table, int sort_by_packets);

/**
 * @brief Function prints names of possible network devices
 * 
 * @param devices Pointer to the list of devices
 */
void print_all_devices_info(pcap_if_t *devices);

#endif