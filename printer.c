#include "printer.h"

connection_stats_t *top_connections[TOP_CONNECTIONS_LIMIT];

void get_top_connections(connection_table_t *table, connection_stats_t *top_connections[], int sort_by_packets)
{
	printf("Sorting connections by: %i\n", sort_by_packets);
	// Save all connections
	connection_stats_t *all_connections[HASHTABLE_SIZE * 10];
	int total_connections = 0;

	for (int i = 0; i < HASHTABLE_SIZE; i++)
	{
		connection_stats_t *connection = (*table)[i];
		while (connection != NULL)
		{
			all_connections[total_connections++] = connection;
			connection = connection->next;
		}
	}
	
	// Bubble sort
	for (int i = 0; i < total_connections - 1; i++)
	{
		for (int j = 0; j < total_connections - i - 1; j++)
		{
			unsigned long unit, next_unit;

			// If used switch [-s p]
			if (sort_by_packets == 1)
			{
				unit = all_connections[j]->rx_packets + all_connections[j]->tx_packets;
				next_unit = all_connections[j + 1]->rx_packets + all_connections[j + 1]->tx_packets;
			}
			// If used switch [-s p] (or no -s used)
			else
			{
				unit = all_connections[j]->received_total_bits + all_connections[j]->transmit_total_bits;
				next_unit = all_connections[j + 1]->received_total_bits + all_connections[j + 1]->transmit_total_bits;
			}

			// Swap
			if (unit < next_unit)
			{
				connection_stats_t *temp = all_connections[j];
				all_connections[j] = all_connections[j + 1];
				all_connections[j + 1] = temp;
			}
		}				
	}

	// If theres not enough connections, fill remaining with NULL
	for (int i = 0; i < TOP_CONNECTIONS_LIMIT && i < total_connections; i++)
	{
		top_connections[i] = all_connections[i];
	}
	for (int i = total_connections; i < TOP_CONNECTIONS_LIMIT; i++)
	{
		top_connections[i] = NULL;
	}		
}

/* Main function - Printing connection stats using ncurses library */
void print_connections_stats(connection_table_t *table, int sort_by_packets)
{
	connection_stats_t *top_connections[TOP_CONNECTIONS_LIMIT];

	get_top_connections(table, top_connections, sort_by_packets);

	int line = 2;
	int col_src_ip = 0;
	int col_dst_ip = 30;
	int col_proto = 60;
	int col_rx = 75;
	int col_tx = 90;

	clear();

	// First and second row - names of columns and units of measurements
	mvprintw(0, col_src_ip, "SrcIP:Port");
	mvprintw(0, col_dst_ip, "DstIP:Port");
	mvprintw(0, col_proto, "Proto");
	mvprintw(0, col_rx, "Rx");
	mvprintw(1, col_rx - 5, "(b/s)");
	mvprintw(1, col_rx + 3, "(p/s)");
	mvprintw(0, col_tx, "Tx");
	mvprintw(1, col_tx - 5, "(b/s)");
	mvprintw(1, col_tx + 3, "(p/s)");

	// Printing only top 10 connections
	for (int j = 0; j < TOP_CONNECTIONS_LIMIT && top_connections[j] != NULL; j++)
	{
		connection_stats_t *connection = top_connections[j];

		if (connection->is_ip6)
		{
			char src_ip_str[INET6_ADDRSTRLEN];
			char dst_ip_str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &connection->src_ip6, src_ip_str, INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &connection->dst_ip6, dst_ip_str, INET6_ADDRSTRLEN);
			mvprintw(line, col_src_ip, "[%s]:%u", src_ip_str, ntohs(connection->src_port));
			mvprintw(line, col_dst_ip, "[%s]:%u", dst_ip_str, ntohs(connection->dst_port));
		}
		else
		{
			mvprintw(line, col_src_ip, "%s:%u", inet_ntoa(connection->src_ip), ntohs(connection->src_port));
			mvprintw(line, col_dst_ip, "%s:%u", inet_ntoa(connection->dst_ip), ntohs(connection->dst_port));
		}


		char *print_proto;
		switch (connection->proto)
		{
			case 1:
				print_proto = "icmp";
				break;
			case 2:
				print_proto = "igmp";
				break;
			case 6:
				print_proto = "tcp";
				break;
			case 17:
				print_proto = "udp";
				break;				
			case 58:
				print_proto = "icmpv6";
				break;
			default:
				print_proto = "unknown";
				break;
		}
		mvprintw(line, col_proto, "%s", print_proto);
		double rx = connection->rx;
		if (rx > 1000 * 1000 * 1000)
		{
			rx = rx / (1000 * 1000 * 1000);
			mvprintw(line, col_rx-5, "%.1fG", rx);
		}
		else if (rx > 1000 * 1000)
		{
			rx = rx / (1000 * 1000);
			mvprintw(line, col_rx-5, "%.1fM", rx);
		}
		else if (rx > 1000)
		{
			rx = rx / 1000;
			mvprintw(line, col_rx-5, "%.1fK", rx);
		}
		else
		{
			mvprintw(line, col_rx-5, "%.1f", rx);
		}

		double rx_packets = connection->rx_packets;
		if (rx_packets > 1000 * 1000 * 1000)
		{
			rx_packets = rx_packets / (1000 * 1000 * 1000);
			mvprintw(line, col_rx+3, "%.1fG", rx_packets);
		}
		else if (rx_packets > 1000 * 1000)
		{
			rx_packets = rx_packets / (1000 * 1000);
			mvprintw(line, col_rx+3, "%.1fM", rx_packets);
		}
		else if (rx_packets > 1000)
		{
			rx_packets = rx_packets / 1000;
			mvprintw(line, col_rx+3, "%.1fK", rx_packets);
		}
		else
		{
			mvprintw(line, col_rx+3, "%.1f", rx_packets);
		}


		double tx = connection->tx;
		if (tx > 1000 * 1000 * 1000)
		{
			tx = tx / (1000 * 1000 * 1000);
			mvprintw(line, col_tx-5, "%.1fG", tx);
		}
		else if (tx > 1000 * 1000)
		{
			tx = tx / (1000 * 1000);
			mvprintw(line, col_tx-5, "%.1fM", tx);
		}
		else if (tx > 1000)
		{
			tx = tx / 1000;
			mvprintw(line, col_tx-5, "%.1fK", tx);
		}
		else
		{
			mvprintw(line, col_tx-5, "%.1f", tx);
		}

		double tx_packets = connection->tx_packets;
		if (tx_packets > 1000 * 1000 * 1000)
		{
			tx_packets = tx_packets / (1000 * 1000 * 1000);
			mvprintw(line, col_tx+3, "%.1fG", tx_packets);
		}
		else if (tx_packets > 1000 * 1000)
		{
			tx_packets = tx_packets / (1000 * 1000);
			mvprintw(line, col_tx+3, "%.1fM", tx_packets);
		}
		else if (tx_packets > 1000)
		{
			tx_packets = tx_packets / 1000;
			mvprintw(line, col_tx+3, "%.1fK", tx_packets);
		}
		else
		{
			mvprintw(line, col_tx+3, "%.1f", tx_packets);
		}

		line++;
	}
	refresh();
}

void print_all_devices_info(pcap_if_t *devices)
{
	// Printing list of devices
	int i = 1;
	while (devices != NULL)
	{
		printf("%d. Device: %s", i, devices->name);
		if (devices->description)
		{
			printf(" [%s]", devices->description);
		}		
		printf("\n");
		i++;
		devices = devices->next;
	}
}