/*
 * Copyright (c) 2016, Martin Lund
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <termios.h>
#include "config.h"
#include "options.h"

struct option_t option =
{
    SCPI,    // Default command
    1,       // Default timeout
    "",      // Default IP address
    "*IDN?", // Default SCPI command
    false,   // Default no hex dump
    false,   // Default no file dump
    "",      // Default dump filename
    false,   // Default no interactive mode
    false,   // Default no run script
};

void print_help(char *argv[])
{
    printf("Usage: %s [--version] [--help] <command> [<options>] [<scpi command>]\n", argv[0]);
    printf("\n");
    printf("  -v, --version                Display version\n");
    printf("  -h, --help                   Display help\n");
    printf("\n");
    printf("Commands:\n");
    printf("  discover                     Search for LXI devices\n");
    printf("  scpi                         Send SCPI command\n");
    printf("\n");
    printf("Discover options:\n");
    printf("  -t, --timeout <seconds>      Timeout (default: %d)\n", option.timeout);
    printf("\n");
    printf("Scpi options:\n");
    printf("  -i, --ip <ip>                IP address\n");
    printf("  -t, --timeout <seconds>      Timeout (default: %d)\n", option.timeout);
    printf("  -x, --dump-hex               Print response in hexidecimal\n");
    printf("  -f, --dump-file <filename>   Save response to file\n");
    printf("  -a, --interactive            Enter interactive mode\n");
    printf("  -r, --run-script <filename>  Run script\n");
    printf("\n");
}

void print_version(void)
{
    printf("lxi v%s\n", VERSION);
}

void parse_options(int argc, char *argv[])
{
    int c;

    // Print help if no arguments provided
    if (argc == 1)
    {
        print_help(argv);
        exit(EXIT_SUCCESS);
    }

    // getopt_long stores the option index here
    int option_index = 0;

    // Skip ahead past command
    optind = 2;

    if (strcmp(argv[1], "discover") == 0)
    {
        option.command = DISCOVER;

        static struct option long_options[] =
        {
            {"timeout",	       required_argument, 0, 't'},
            {0,                0,                 0,  0 }
        };

        /* Parse discover options */
        c = getopt_long(argc, argv, "t:", long_options, &option_index);

        while (c != -1)
        {
            switch (c)
            {
                case 't':
                    option.timeout = atoi(optarg);
                    break;
                case '?':
                    exit(EXIT_FAILURE);
            }
            c = getopt_long(argc, argv, "t:", long_options, &option_index);
        }
    }
    else if (strcmp(argv[1], "scpi") == 0)
    {
        option.command = SCPI;

        static struct option long_options[] =
        {
            {"timeout",	       required_argument, 0, 't'},
            {"ip",             required_argument, 0, 'i'},
            {"dump-hex",       no_argument,       0, 'x'},
            {"dump-file",      required_argument, 0, 'f'},
            {"interactive",    no_argument,       0, 'a'},
            {"run-script",     required_argument, 0, 'r'},
            {0,                0,                 0,  0 }
        };

        /* Parse scpi options */
        c = getopt_long(argc, argv, "i:t:xf:ar:", long_options, &option_index);

        while (c != -1)
        {
            switch (c)
            {
                case 't':
                    option.timeout = atoi(optarg);
                    break;

                case 'i':
                    strncpy(option.ip, optarg, 500);
                    break;

                case 'x':
                    option.dump_hex = true;
                    break;

                case 'f':
                    option.dump_file = true;
                    option.filename = optarg;
                    break;

                case 'a':
                    option.interactive = true;
                    break;

                case 'r':
                    option.run_script = true;
                    option.filename = optarg;
                    break;

                case '?':
                    exit(EXIT_FAILURE);
            }
            c = getopt_long(argc, argv, "t:", long_options, &option_index);
        }
    } else
    {
        // No command provided so we restore index
        optind = 1;

        static struct option long_options[] =
        {
            {"version",	       no_argument,       0, 'v'},
            {"help",           no_argument,       0, 'h'},
            {0,                0,                 0,  0 }
        };

        /* Parse options */
        c = getopt_long(argc, argv, "vh", long_options, &option_index);

        while (c != -1)
        {
            switch (c)
            {
                case 'v':
                    print_version();
                    exit(EXIT_SUCCESS);

                case 'h':
                    print_help(argv);
                    exit(EXIT_SUCCESS);

                case '?':
                    exit(EXIT_FAILURE);
            }
            c = getopt_long(argc, argv, "vh", long_options, &option_index);
        }
    }

    if ((option.command == SCPI) && (optind != argc))
    {
        strncpy(option.scpi_command, argv[optind++], 500);
        if (strlen(option.ip) == 0)
        {
            printf("Error: No IP address specified\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Print any unknown arguments */
    if (optind < argc)
    {
        printf("Unknown arguments: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
        exit(EXIT_FAILURE);
    }
}