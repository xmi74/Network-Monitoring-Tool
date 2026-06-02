#include "help.h"

void program_usage(pcap_if_t *acceptable_interfaces)
{
	fprintf(stderr, "PROGRAM USAGE:\n");
	fprintf(stderr, "RUN: sudo ./isa-top -i int [-s b | p]\n");
	fprintf(stderr, "\tWHERE: -i int (required)  : int represents interface that you want to sniff on\n");
	fprintf(stderr, "\tWHERE: -s (optional) 	 : sorts output based on number of bits (b) OR number of packets\n\t\tDEFAULT: sorted by bits\n");
	fprintf(stderr, "\tWHERE: -t time (optional) : time (between 1 .. 100) represents update time or number of seconds after which statistics are restarted.\n\t\tDEFAULT: time = 1\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Currently Available interfaces: \n");
	print_all_devices_info(acceptable_interfaces);
}

