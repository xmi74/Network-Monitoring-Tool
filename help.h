#ifndef HELP_H
#define HELP_H

#include <stdio.h>
#include <pcap.h>

#include "printer.h"

/**
 * @brief Function prints informations about usage of program and available interfaces on standard error output
 * 
 * @param acceptable_interfaces Pointer to the list of devices
 */
void program_usage(pcap_if_t *acceptable_interfaces);

#endif