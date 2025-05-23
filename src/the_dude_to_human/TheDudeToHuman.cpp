// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <regex>
#include <string>

#ifdef _WIN32
// windows.h needs to be included before shellapi.h
#include <windows.h>

#include <shellapi.h>
#endif

#undef _UNICODE
#include <getopt.h>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_validator.h"
#include "the_dude_to_human/mikrotik/mikrotik_device.h"

static void PrintHelp(const char* argv0) {
    // clang-format off
    std::cout
        << "Usage: " << argv0
        << " [options] <filename>\n"
           "--f, --file                                 Load the specified database file\n"
           "--o, --out                                  Store database location\n"
           "--m, --mikrotik=user:password@address:port  Connect to the specified mikrotik device\n"
           //"-d, --database=user:password@address:port  Connect to the specified database\n"
           "--h, --help                                 Display this help and exit\n";
    // clang-format on
}

int main(int argc, char** argv) {
    int option_index = 0;
    std::string program_args;
#ifdef _WIN32
    int argc_w;
    auto argv_w = CommandLineToArgvW(GetCommandLineW(), &argc_w);

    if (argv_w == nullptr) {
        std::cout << "Failed to get command line arguments";
        return -1;
    }
#endif

    bool has_filepath{};
    std::string filepath{};

    bool has_out_filepath{};
    std::string out_filepath{};

    bool has_mikrotik{};
    std::string mikrotik_user{};
    std::string mikrotik_password{};
    std::string mikrotik_address{};
    u16 mikrotik_port = {22};

    bool has_database{};
    std::string database_user{};
    std::string database_password{};
    std::string database_address{};
    [[maybe_unused]]u16 database_port = {3306};

    static struct option long_options[] = {
        // clang-format off
        {"file", required_argument, 0, 'f'},
        {"out", required_argument, 0, 'o'},
        {"mikrotik", required_argument, 0, 'm'},
        //{"database", optional_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0},
        // clang-format on
    };

    while (optind < argc) {
        int arg = getopt_long(argc, argv, "g:fhvp::c:u:", long_options, &option_index);
        if (arg != -1) {
            switch (static_cast<char>(arg)) {
            case 'f': {
                has_filepath = true;
                const std::string str_arg(optarg);
                filepath = str_arg;
                break;
            }
            case 'o': {
                has_out_filepath = true;
                const std::string str_arg(optarg);
                out_filepath = str_arg;
                break;
            }
            case 'h':
                PrintHelp(argv[0]);
                return 0;
            case 'm': {
                has_mikrotik = true;
                const std::string str_arg(optarg);
                // regex to check if the format is user:password@ip:port
                // with optional :password
                const std::regex re("^([^:]+)(?::(.+))?@([^:]+)(?::([0-9]+))?$");
                if (!std::regex_match(str_arg, re)) {
                    std::cout << "Wrong format for option --mikrotik\n";
                    PrintHelp(argv[0]);
                    return 0;
                }

                std::smatch match;
                std::regex_search(str_arg, match, re);
                if (match.size() != 5) {
                    std::cout << "Wrong format for option --mikrotik\n";
                    PrintHelp(argv[0]);
                    return 0;
                }

                mikrotik_user = match[1];
                mikrotik_password = match[2];
                mikrotik_address = match[3];
                if (!match[4].str().empty()) {
                    mikrotik_port =
                        static_cast<u16>(std::strtoul(match[4].str().c_str(), nullptr, 0));
                }
                if (mikrotik_address.empty()) {
                    std::cout << "Address to mikrotik device must not be empty.\n";
                    return 0;
                }
                break;
            }
            case 'd': {
                has_database = true;
                const std::string str_arg(optarg);
                // regex to check if the format is user:password@ip:port
                // with optional :password
                const std::regex re("^([^:]+)(?::(.+))?@([^:]+)(?::([0-9]+))?$");
                if (!std::regex_match(str_arg, re)) {
                    std::cout << "Wrong format for option --database\n";
                    PrintHelp(argv[0]);
                    return 0;
                }

                std::smatch match;
                std::regex_search(str_arg, match, re);
                if (match.size() != 5) {
                    std::cout << "Wrong format for option --database\n";
                    PrintHelp(argv[0]);
                    return 0;
                }

                database_user = match[1];
                database_password = match[2];
                database_address = match[3];
                if (!match[4].str().empty()) {
                    database_port =
                        static_cast<u16>(std::strtoul(match[4].str().c_str(), nullptr, 0));
                }
                if (database_address.empty()) {
                    std::cout << "Address to database device must not be empty.\n";
                    return 0;
                }
                break;
            }
            }
        } else {
            has_filepath = true;
            filepath = argv[optind];
            optind++;
        }
    }

    if (!has_filepath && !has_mikrotik && !has_database) {
        PrintHelp(argv[0]);
        return 0;
    }

    if (has_mikrotik) {
        std::cout << "Conecting to " << mikrotik_address << ":" << mikrotik_port << "\n";
        Mikrotik::MikrotikDevice device = {mikrotik_address, mikrotik_port};
        if (!device.Connect(mikrotik_user, mikrotik_password)) {
            std::cout << "Unable to connect to device\n";
            return 0;
        }
        std::string output{};
        std::cout << "Executing command 'system health print;'\n";
        device.Execute("system health print;", &output);
        std::cout << output;
        device.Disconnect();
    }

    if (has_filepath) {
        std::cout << "Reading database " << filepath << "\n";
        Database::DudeDatabase db{filepath};
        db.ListMapData();
        db.ListDeviceData();

        if (has_out_filepath) {
            std::cout << "Saving database " << out_filepath << "\n";
            db.SaveDatabase(out_filepath);
        }
    }
}
