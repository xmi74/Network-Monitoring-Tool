#include "hashtable.h"

connection_table_t *htb_init(int table_size)
{
    connection_table_t *connection_table = malloc(table_size * sizeof(connection_stats_t *));
    if (!connection_table) {
        fprintf(stderr, "INSERT: Memory alloc fail !\n");
        exit(99);
    }
    for (int i = 0; i < table_size; i++)
    {
        (*connection_table)[i] = NULL;
    }
    return connection_table;
}

unsigned int hash_function(int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                           struct in_addr src_ip, struct in_addr dst_ip, 
                           uint16_t src_port, uint16_t dst_port, int table_size)
{
    if (is_ip6 == 1)
    {
        return ((src_ip6.s6_addr[0] ^ dst_ip6.s6_addr[0]) ^ (src_port ^ dst_port)) % table_size;
    }
    else
    {
        return ((src_ip.s_addr ^ dst_ip.s_addr) ^ (src_port ^ dst_port)) % table_size;
    }
}

connection_stats_t *htb_search(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                               struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port)
{
    unsigned int hash = hash_function(is_ip6, src_ip6, dst_ip6, src_ip, dst_ip, src_port, dst_port, HASHTABLE_SIZE);

    // entry for finding synonyms
    connection_stats_t *entry = connection_table[hash];    
    while (entry != NULL)
    {
        if (is_ip6 == entry->is_ip6)
        {
            if (is_ip6)
            {   
                // 2-way connection
                if ((memcmp(&entry->src_ip6, &src_ip6, sizeof(struct in6_addr)) == 0 &&
                    memcmp(&entry->dst_ip6, &dst_ip6, sizeof(struct in6_addr)) == 0 &&
                    entry->src_port == src_port && entry->dst_port == dst_port)
                    ||
                    (memcmp(&entry->src_ip6, &dst_ip6, sizeof(struct in6_addr)) == 0 &&
                    memcmp(&entry->dst_ip6, &src_ip6, sizeof(struct in6_addr)) == 0 &&
                    entry->src_port == src_port && entry->dst_port == dst_port))
                {
                    return entry;
                }
            }
            else
            {
                // 2-way connection
                if ((entry->src_ip.s_addr == src_ip.s_addr && 
                    entry->dst_ip.s_addr == dst_ip.s_addr && 
                    entry->src_port == src_port && 
                    entry->dst_port == dst_port) 
                    ||
                    (entry->src_ip.s_addr == dst_ip.s_addr && 
                    entry->dst_ip.s_addr == src_ip.s_addr && 
                    entry->src_port == dst_port && 
                    entry->dst_port == src_port))
                {
                    // Connection found successfuly
                    return entry;
                }
            }
        }
        entry = entry->next;        
    }
    // No connection found
    return NULL;
}

connection_stats_t *htb_insert(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                               struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port)
{
    connection_stats_t *existing_connection = htb_search(connection_table, is_ip6, src_ip6, dst_ip6, src_ip, dst_ip, src_port, dst_port);
    if (existing_connection != NULL)
    {
        return existing_connection;
    }

    unsigned int hash = hash_function(is_ip6, src_ip6, dst_ip6, src_ip, dst_ip, src_port, dst_port, HASHTABLE_SIZE);    

    connection_stats_t *new_connection = (connection_stats_t *) malloc(sizeof(struct connection_stats));
    if (!new_connection)
    {
        fprintf(stderr, "INSERT: Memory alloc fail !\n");
        exit(99);
    }

    new_connection->is_ip6 = is_ip6;
    if (is_ip6)
    {
        new_connection->src_ip6 = src_ip6;
        new_connection->dst_ip6 = dst_ip6;
    }
    else
    {
        new_connection->src_ip = src_ip;
        new_connection->dst_ip = dst_ip;
    }

    new_connection->src_port = src_port;
    new_connection->dst_port = dst_port;
    new_connection->proto = 0;
    new_connection->transmit_total_bits = 0;
    new_connection->transmit_number_of_packets = 0;
    new_connection->received_total_bits = 0;
    new_connection->received_number_of_packets = 0;
    new_connection->rx = 0;
    new_connection->rx_packets = 0;
    new_connection->tx = 0;
    new_connection->tx_packets = 0;

    new_connection->next = connection_table[hash];
    connection_table[hash] = new_connection;

    return new_connection;
}


void htb_update(connection_table_t connection_table, int is_ip6, struct in6_addr src_ip6, struct in6_addr dst_ip6,
                struct in_addr src_ip, struct in_addr dst_ip, uint16_t src_port, uint16_t dst_port, 
                uint8_t proto, unsigned long packet_size, int update_time)
{
    connection_stats_t *connection = htb_search(connection_table, is_ip6, src_ip6, dst_ip6, src_ip, dst_ip, src_port, dst_port);

    if (connection == NULL)
    {
        connection = htb_insert(connection_table, is_ip6, src_ip6, dst_ip6, src_ip, dst_ip, src_port, dst_port);
    }

    connection->proto = proto;

    int is_same_source = 0;

    if (is_ip6)
    {
        is_same_source = (memcmp(&connection->src_ip6, &src_ip6, sizeof(struct in6_addr)) == 0 && connection->src_port == src_port);
    }
    else
    {
        is_same_source = (connection->src_ip.s_addr == src_ip.s_addr && connection->src_port == src_port);
    }


    if (is_same_source)
    {
        // Adding bits (bytes * 8) and packets
        connection->transmit_total_bits += (packet_size * 8);
        connection->transmit_number_of_packets += 1;

        connection->tx = connection->transmit_total_bits / update_time;
        connection->tx_packets = connection->transmit_number_of_packets / update_time;
    }
    else
    {
        connection->received_total_bits += (packet_size * 8);
        connection->received_number_of_packets += 1;

        connection->rx = connection->received_total_bits / update_time;
        connection->rx_packets = connection->received_number_of_packets / update_time;
    }    
}

// Function used for freeing resources after quitting program
// as well as refreshing hashtable to init state after printing stats
void htb_destroy(connection_table_t *connection_table, int table_size)
{
    for (int i = 0; i < table_size; i++)
    {
        connection_stats_t *connection = (*connection_table)[i];

        while (connection != NULL)
        {
            connection_stats_t *next = connection->next;
            free(connection);
            connection = next;
        }
        (*connection_table)[i] = NULL;
    }    
}

