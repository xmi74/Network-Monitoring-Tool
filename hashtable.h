#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>

#define HASHTABLE_SIZE 1024

/* Hashtable structure */
typedef struct connection_stats
{
    int is_ip6;                      // 0 = IPv4; 1 = IPv6
    struct in_addr src_ip;           // Source IP
    struct in_addr dst_ip;           // Destination IP
    struct in6_addr src_ip6;         // Source IPv6
    struct in6_addr dst_ip6;         // Destination IPv6
    uint16_t src_port;               // Source Port
    uint16_t dst_port;               // Destionation Port
    uint8_t proto;                   // Protocol
    unsigned long received_total_bits; // Number of received bits for this connection
    unsigned long transmit_total_bits; // Number of transmitted bits for this connection
    unsigned long received_number_of_packets; // Number of received packets
    unsigned long transmit_number_of_packets; // Number of transmitted packets
    double rx;                       // Receive speed in bits/s
    double rx_packets;               // Receive speed in packets/s
    double tx;                       // Transmit speed in bits/s
    double tx_packets;               // Transmit speed in packets/s
    struct connection_stats *next;   // Next synonym pointer
} connection_stats_t;

// Hashtable declaration
typedef connection_stats_t *connection_table_t[HASHTABLE_SIZE];

/**
 * @brief Initialization of hashtable
 * 
 * @param table_size Size of hashtable
 */
connection_table_t *htb_init(int table_size);

/**
 * @brief Function returns hash value for given connection
 * 
 * @param is_ip6 Flag indicating whether connection uses IPv6 addresses (0=IPv4, 1=IPv6)
 * @param src_ip6 IPv6 source address
 * @param dst_ip6 IPv6 destination address
 * @param src_ip IPv4 source address
 * @param dst_ip IPv4 destination address
 * @param src_port Source port
 * @param dst_port Destination port
 * @param table_size Size of hashtable
 */
unsigned int hash_function(int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                           struct in_addr src_ip, struct in_addr dst_ip, 
                           uint16_t src_port, uint16_t dst_port, int table_size);

/**
 * @brief Function inserts new connection into the hashtable and returns pointer to inserted entry
 * 
 * @param connection_table Pointer to the hashtable
 * @param is_ip6 Flag indicating whether connection uses IPv6 addresses (0=IPv4, 1=IPv6)
 * @param src_ip6 IPv6 source address
 * @param dst_ip6 IPv6 destination address
 * @param src_ip IPv4 source address
 * @param dst_ip IPv4 destination address
 * @param src_port Source port
 * @param dst_port Destination port
 */
connection_stats_t *htb_insert(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                               struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port);

/**
 * @brief Function searches for connection in hashtable, returns NULL if connection was not found or pointer to the found connection
 * 
 * @param connection_table Pointer to the hashtable
 * @param is_ip6 Flag indicating whether connection uses IPv6 addresses (0=IPv4, 1=IPv6)
 * @param src_ip6 IPv6 source address
 * @param dst_ip6 IPv6 destination address
 * @param src_ip IPv4 source address
 * @param dst_ip IPv4 destination address
 * @param src_port Source port
 * @param dst_port Destination port
 */
connection_stats_t *htb_search(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                               struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port);

/**
 * @brief Function actualizes attributes of given connection
 * 
 * @param connection_table Pointer to the hashtable
 * @param is_ip6 Flag indicating whether connection uses IPv6 addresses (0=IPv4, 1=IPv6)
 * @param src_ip6 IPv6 source address
 * @param dst_ip6 IPv6 destination address
 * @param src_ip IPv4 source address
 * @param dst_ip IPv4 destination address
 * @param src_port Source port
 * @param dst_port Destination port
 * @param proto Protocol used in communication
 * @param packet_size Size of the packet (Bytes)
 * @param update_time Time for actualization of statistics (seconds)
 */
void htb_update(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
                uint8_t proto, unsigned long packet_size, int update_time);

/**
 * @brief Function frees allocated memory of hashtable
 * 
 * @param connection_table Pointer to the hashtable
 * @param table_size Size of hashtable
 */
void htb_destroy(connection_table_t *connection_table, int table_size);

#endif