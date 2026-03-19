// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <mutex>
#include <string>

#include "common/common_types.h"

struct _LIBSSH2_SESSION;
typedef struct _LIBSSH2_SESSION LIBSSH2_SESSION;

struct _LIBSSH2_SFTP;
typedef struct _LIBSSH2_SFTP LIBSSH2_SFTP;

struct _LIBSSH2_SFTP_HANDLE;
typedef struct _LIBSSH2_SFTP_HANDLE LIBSSH2_SFTP_HANDLE;

namespace Mikrotik {

// Connects to a mikrotik device using ssh
class MikrotikDevice {
public:
    MikrotikDevice(std::string address, u16 port_ = 22);
    ~MikrotikDevice();

    bool Connect(std::string username, std::string password);
    bool Disconnect();

    bool Execute(std::string commandline, std::string* output = nullptr);

    bool DownloadDatabase(std::string filename);
    void UploadDatabase();

private:
    int InitializeSSH();
    int InitializeSFTP();
    int ConnectSSH(std::string username, std::string password);

    int ExecuteSSH(std::string commandline, std::string* output = nullptr);
    int DownloadFile(std::string filename);

    int DisconnectSSH();
    int DisconnectSFTP();

    bool is_connected{};

    std::string hostname{};
    u16 port{};

private:
    static inline std::atomic_int lib_refcount = 0;

    s32 sock{};
    LIBSSH2_SESSION* session = nullptr;
    LIBSSH2_SFTP* sftp_session = nullptr;
    LIBSSH2_SFTP_HANDLE* sftp_handle = nullptr;
    std::mutex session_mutex;
};

} // namespace Mikrotik
