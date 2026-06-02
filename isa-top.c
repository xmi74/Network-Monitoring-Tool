#include "isa-top.h"

pcap_t *handle;				// Session handle
struct timeval starting_time;
connection_table_t *connection_table;
pthread_t capture_thread, stats_thread;
int update_time, sort_by_packets;

void cleanup(int signum)
{
	htb_destroy(connection_table, HASHTABLE_SIZE);
	endwin();
	pcap_close(handle);

	printf("Closing Program. Freeing resources\n");
	exit(0);
}


int main(int argc, char *argv[])
{
	signal(SIGINT, cleanup);

	const char *dev;	
	pcap_if_t *all_devices;		// List of devices
	char errbuf[PCAP_ERRBUF_SIZE];	// Error buffer

	bpf_u_int32 mask;		
	bpf_u_int32 ip;			

	char filter_exp[] = "ip or ip6";		// Filter expression
	struct bpf_program fp;		// Compiled filter expression
	

	// Finding all devices
	if (pcap_findalldevs(&all_devices, errbuf) == -1)
	{
		fprintf(stderr, "ERROR: Finding devices\n%s\n", errbuf);
		return 1;
	}

	/************ Handling Arguments ************/
	if (parse_arguments(argc, argv, &dev, &update_time, &sort_by_packets) == 0)
	{
		program_usage(all_devices);
		if (all_devices != NULL) 
		{
			pcap_freealldevs(all_devices);
		}
		return 1;
	}

	if (pcap_lookupnet(dev, &ip, &mask, errbuf) == -1)
	{
		fprintf(stderr, "ERROR: Finding IP and network mask\n%s\n", errbuf);
		ip = 0;
		mask = 0;

		if (all_devices != NULL) 
		{
			pcap_freealldevs(all_devices);
		}

		return 1;
	}

	// Opening session - promiscuous mode, 1000ms read timeout
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (!handle)
	{
		fprintf(stderr, "ERROR: Opening device %s\n%s", dev, errbuf);

		if (all_devices != NULL) 
		{
			pcap_freealldevs(all_devices);
		}

		return 1;
	}
	printf("Device %s: Opened successfully !\n", dev);

	if (pcap_compile(handle, &fp, filter_exp, 0, ip) == -1)
	{
		fprintf(stderr, "ERROR: Compiling filter: %s\n%s\n", filter_exp, pcap_geterr(handle));

		if (all_devices != NULL) 
		{
			pcap_freealldevs(all_devices);
		}

		return 1;
	}

	if (pcap_setfilter(handle, &fp) == -1)
	{
		fprintf(stderr, "ERROR: Installing filter: %s\n%s\n", filter_exp, pcap_geterr(handle));

		if (all_devices != NULL) 
		{
			pcap_freealldevs(all_devices);
		}

		return 1;
	}

	if (all_devices != NULL) 
	{
		pcap_freealldevs(all_devices);
	}

	// Initializing hashtable
	connection_table = htb_init(HASHTABLE_SIZE);
	// Initializing ncurses
	initscr();	
	curs_set(0);
	noecho();

	/************ Handling Threads ************/
	pthread_create(&stats_thread, NULL, stats_print_thread, NULL);
	pthread_create(&capture_thread, NULL, packet_capture_thread, (void *)handle); 

	pthread_join(stats_thread, NULL);
	pthread_join(capture_thread, NULL);  

	// Session end
	cleanup(0);

	return 0;
}


void packet_analyzer(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	// Catch size of the packet (in Bytes), minus Ethernet header size (14B)
	unsigned long packet_size = header->len - 14; 		

	struct in_addr src_ip = {0};
	struct in_addr dst_ip = {0};
	struct in6_addr src_ip6 = {0};
	struct in6_addr dst_ip6 = {0};
	uint16_t src_port = 0;
	uint16_t dst_port = 0;
	uint8_t proto = 0;
	int is_ip6 = 0;

	struct ether_header *ether_h;
	ether_h = (struct ether_header *) packet;	

	switch (ntohs(ether_h->ether_type))
	{
		case ETHERTYPE_IP: 		// IPv4
			struct ip *ipv4_h = (struct ip *) (packet + sizeof(struct ether_header));

			src_ip = ipv4_h->ip_src;
			dst_ip = ipv4_h->ip_dst;
			proto = ipv4_h->ip_p;
			is_ip6 = 0;

			switch (proto)
			{
				case IPPROTO_TCP:
					struct tcphdr *tcp_h;
					tcp_h = (struct tcphdr *) ((u_char *) ipv4_h + (ipv4_h->ip_hl * 4));

					src_port = tcp_h->source;
					dst_port = tcp_h->dest;			
					break;
				case IPPROTO_UDP:
					struct udphdr *udp_h;
					udp_h = (struct udphdr *) ((u_char *) ipv4_h + (ipv4_h->ip_hl * 4));					

					src_port = udp_h->source;
					dst_port = udp_h->dest;
					break;
				default:
					break;
			}

			break;

		case ETHERTYPE_IPV6:	// IPv6
			struct ip6_hdr *ipv6_h = (struct ip6_hdr *) (packet + sizeof(struct ether_header));
			src_ip6 = ipv6_h->ip6_src;
			dst_ip6 = ipv6_h->ip6_dst;
			proto = ipv6_h->ip6_ctlun.ip6_un1.ip6_un1_nxt;
			is_ip6 = 1;
			
			switch (proto)
			{
				case IPPROTO_TCP:
					struct tcphdr *tcp_h = (struct tcphdr *) (ipv6_h + 1);
					src_port = ntohs(tcp_h->source);
					dst_port = ntohs(tcp_h->dest);
					break;
				case IPPROTO_UDP:
					struct udphdr *udp_h = (struct udphdr *) (ipv6_h + 1);
					src_port = ntohs(udp_h->source);
					dst_port = ntohs(udp_h->dest);
					break;
				default:
					break;
			}

			break;

		default:
			fprintf(stderr, "ERROR: Ethernet type\nParsing of this ethernet type not implemented\n");
			exit(1);
	}

	htb_update(*connection_table, is_ip6, src_ip6, dst_ip6,
				src_ip, dst_ip, src_port, dst_port, proto, packet_size, update_time);
}


void *packet_capture_thread(void *arg)
{
	pcap_t *handle = (pcap_t *)arg;
	pcap_loop(handle, 0, packet_analyzer, NULL);
	return NULL;
}

void *stats_print_thread(void *arg)
{
	while (1)
	{
		sleep(update_time);

		print_connections_stats(connection_table, sort_by_packets);

		// Destroy all existing connections after printing stats
		htb_destroy(connection_table, HASHTABLE_SIZE);
	}
}
