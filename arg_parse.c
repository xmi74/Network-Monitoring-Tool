#include "arg_parse.h"

int parse_arguments(int argc, char *argv[], const char **dev, int *update_time, int *sort_by_packets)
{
    *update_time = 1;    // Default - update time = 1
    *sort_by_packets = 0; // Default - sorting by number of bits

    struct option long_options[] = 
    {
        {"interface", required_argument, 0, 'i'},
        {"time", required_argument, 0, 't'},
        {"sort", required_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "i:t:s:", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'i':
                *dev = optarg;
                break;
            case 't':
                *update_time = atoi(optarg);
                if (*update_time < 1 || *update_time > 100)
                {
                    fprintf(stderr, "Choose update time between 1 .. 100\n");
                    return 0;
                }
                break;
            case 's':
                if (optarg == NULL)
                {
                    fprintf(stderr, "-s: optarg=null\n");
                    return 0;
                }

                if (strcmp(optarg, "p") == 0)
                {
                    *sort_by_packets = 1;
                }
                else if (strcmp(optarg, "b") == 0)
                {
                    *sort_by_packets = 0;
                }
                else
                {
                    fprintf(stderr, "Invalid option after -s switch. Usage: [-s b/p] b for sorting by bits; p for sorting by packets\n");
                    return 0;
                }                
                break;
            default:
                fprintf(stderr, "Wrong Usage\n");
                return 0;
        }
    }

    if (optind < argc)
    {
        fprintf(stderr, "Invalid argument: %s\n", argv[optind]);
        return 0;
    }

    if (*dev == NULL)
    {
        fprintf(stderr, "Please specify interfac\n");
        return 0;
    }

    return 1;
}

