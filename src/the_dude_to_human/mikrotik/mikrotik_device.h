// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <string>

#include "common/common_types.h"
#include "libssh2.h"

namespace Mikrotik {

// Connects to a mikrotik device using ssh
class MikrotikDevice {
public:
    MikrotikDevice(std::string address, u16 port_ = 22);
    ~MikrotikDevice();

    bool Connect(std::string username, std::string password);
    bool Disconnect();

    bool Execute(std::string commandline);

    void DownloadDatabase();
    void UploadDatabase();

private:
    int InitializeSSH();
    int ConnectSSH(std::string username, std::string password);

    int ExecuteSSH(std::string commandline);

    int DisconnectSSH();

    bool is_connected{};

    std::string hostname{};
    u16 port{};

private:
    static inline std::atomic_int lib_refcount = 0;

    libssh2_socket_t sock;
    LIBSSH2_SESSION* session = NULL;
};

} // namespace Mikrotik
