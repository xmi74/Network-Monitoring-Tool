#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/**
 * @brief Function parses arguments used when starting the program
 * 
 * @param argc Number of arguments
 * @param argv Array of strings of arguments
 * @param dev Pointer to selected interface
 * @param update_time Pointer to update time interval
 * @param sort_by_packets Pointer to chosen sorting method
 */
int parse_arguments(int argc, char *argv[], const char **dev, int *update_time, int *sort_by_packets);

#endif