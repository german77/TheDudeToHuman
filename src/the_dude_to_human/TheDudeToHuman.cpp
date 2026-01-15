// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <regex>
#include <string>

#ifdef _WIN32
// windows.h needs to be included before shellapi.h
#include <windows.h>

#include <shellapi.h>
#else
#include <termios.h>
#endif

#undef _UNICODE
#include <getopt.h>

#include "the_dude_to_human/database/dude_database.h"
#include "the_dude_to_human/database/dude_validator.h"
#include "the_dude_to_human/mikrotik/mikrotik_device.h"

static void PrintVersion() {
    std::cout << "the dude to human version 1.0.0\n";
}

static void PrintHelp(const char* argv0) {
    // clang-format off
    std::cout
        << "Usage: " << argv0
        << " [options] <filename>\n"
           "-f, --file                                 Load the specified database file\n"
           "-o, --out                                  Save json database file\n"
           "-c, --credentials                          Save credentials in plain text\n"
           "-m, --mikrotik=user:password@address:port  Connect to the specified mikrotik device\n"
           //"-d, --database=user:password@address:port  Connect to the specified database\n"
           "-i, --integrity                            Validate database health\n"
           "-h, --help                                 Display this help and exit\n"
           "-v, --version                              Print tool version\n";
    // clang-format on
}

static void PrintAddressFormats() {
    // clang-format off
    std::cout
        << "Address format examples:\n"
           "    user@192.168.1.1                       IP address\n"
           "    user@domain.name                       Domain name\n"
           "    user@192.168.1.1:1234                  User defined port\n"
           "    user:password@192.168.1.1              User defined password\n"
           "    user:@192.168.1.1                      Hidden user defined password\n"
        << std::endl;
    // clang-format on
}

#ifdef _WIN32
static std::string takePassword() {
    HANDLE std_input = GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode = 0;
    GetConsoleMode(std_input, &mode);
    SetConsoleMode(std_input, mode & (~ENABLE_ECHO_INPUT));

    std::string password;
    std::getline(std::cin, password);
    SetConsoleMode(std_input, mode);
    std::cout << std::endl;

    return password;
}
#else
static std::string takePassword() {
    termios mode, new_mode;

    // Get current terminal attributes
    tcgetattr(fileno(stdin), &mode);
    new_mode = mode;
    new_mode.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &new_mode);

    std::string password;
    std::cin >> password;
    tcsetattr(fileno(stdin), TCSANOW, &mode);
    std::cout << std::endl;

    return password;
}
#endif

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
    bool has_credentials{};
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
    [[maybe_unused]] u16 database_port = {3306};

    bool check_integrity{};

    static struct option long_options[] = {
        // clang-format off
        {"file", required_argument, 0, 'f'},
        {"out", required_argument, 0, 'o'},
        {"credentials", no_argument, 0, 'c'},
        {"mikrotik", required_argument, 0, 'm'},
        {"integrity", no_argument, 0, 'i'},
        //{"database", optional_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0},
        // clang-format on
    };

    while (optind < argc) {
        int arg = getopt_long(argc, argv, "f:o:cm:ihv", long_options, &option_index);
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
            case 'c':
                has_credentials = true;
                break;
            case 'i':
                check_integrity = true;
                break;
            case 'h':
                PrintHelp(argv[0]);
                return 0;
            case 'v':
                PrintVersion();
                return 0;
            case 'm': {
                has_mikrotik = true;
                const std::string str_arg(optarg);
                // regex to check if the format is user:password@ip:port
                // with optional :password :port
                const std::regex re("^([^:]+)(:(.+)?)?@([^:]+)(?::([0-9]+))?$");

                std::smatch match;
                if (!std::regex_match(str_arg, match, re) || match.size() != 6) {
                    std::cout << "Wrong format for option --mikrotik\n";
                    PrintAddressFormats();
                    PrintHelp(argv[0]);
                    return 0;
                }

                mikrotik_user = match[1];
                mikrotik_password = match[3];
                mikrotik_address = match[4];
                if (!match[5].str().empty()) {
                    mikrotik_port =
                        static_cast<u16>(std::strtoul(match[5].str().c_str(), nullptr, 0));
                }
                if (mikrotik_address.empty()) {
                    std::cout << "Address to mikrotik device must not be empty.\n";
                    return 0;
                }
                if (match[2].length() != 0 && mikrotik_password.empty()) {
                    std::cout << "Enter Mikrotik password: ";
                    mikrotik_password = takePassword();
                }
                break;
            }
            case 'd': {
                has_database = true;
                const std::string str_arg(optarg);
                // regex to check if the format is user:password@ip:port
                // with optional :password :port
                const std::regex re("^([^:]+)(:(.+)?)?@([^:]+)(?::([0-9]+))?$");

                std::smatch match;
                if (!std::regex_match(str_arg, match, re) || match.size() != 6) {
                    std::cout << "Wrong format for option --database\n";
                    PrintAddressFormats();
                    PrintHelp(argv[0]);
                    return 0;
                }

                database_user = match[1];
                database_password = match[3];
                database_address = match[4];
                if (!match[5].str().empty()) {
                    database_port =
                        static_cast<u16>(std::strtoul(match[5].str().c_str(), nullptr, 0));
                }
                if (database_address.empty()) {
                    std::cout << "Address to database device must not be empty.\n";
                    return 0;
                }
                if (match[2].length() != 0 && mikrotik_password.empty()) {
                    std::cout << "Enter database password: ";
                    mikrotik_password = takePassword();
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

        if (check_integrity) {
            db.CheckIntegrity();
        }

        if (has_out_filepath) {
            std::cout << "Saving database " << out_filepath << "\n";
            db.SaveDatabase(out_filepath, has_credentials);
        }
    }
}
