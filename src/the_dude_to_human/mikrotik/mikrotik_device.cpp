// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "libssh2.h"
#include "the_dude_to_human/mikrotik/mikrotik_device.h"

#define BUFSIZE 32000

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

static int waitsocket(libssh2_socket_t socket_fd, LIBSSH2_SESSION* session) {
    fd_set fd;
    fd_set* writefd = NULL;
    fd_set* readfd = NULL;

    timeval timeout{
        .tv_sec = 10,
        .tv_usec = 0,
    };

    FD_ZERO(&fd);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
    FD_SET(socket_fd, &fd);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    /* now make sure we wait in the correct direction */
    int dir = libssh2_session_block_directions(session);

    if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    return select(static_cast<int>(socket_fd + 1), readfd, writefd, NULL, &timeout);
}

namespace Mikrotik {
MikrotikDevice::MikrotikDevice(std::string address_, u16 port_) : hostname{address_}, port{port_} {
    InitializeSSH();
}

MikrotikDevice::~MikrotikDevice() {
    if (is_connected) {
        Disconnect();
    }

    if (session) {
        libssh2_session_free(session);
    }

    if (--MikrotikDevice::lib_refcount == 0) {
        libssh2_exit();
    }
}

bool MikrotikDevice::Connect(std::string username, std::string password) {
    if (is_connected) {
        return true;
    }

    if (ConnectSSH(username, password) != 0) {
        DisconnectSSH();
        return false;
    }

    is_connected = true;
    return true;
}

bool MikrotikDevice::Disconnect() {
    if (!is_connected) {
        return true;
    }

    DisconnectSSH();

    is_connected = false;
    return true;
}

bool MikrotikDevice::Execute(std::string commandline, std::string* output) {
    if (!is_connected) {
        return false;
    }

    ExecuteSSH(commandline, output);

    return true;
}

void MikrotikDevice::DownloadDatabase() {
    if (!is_connected) {
        return;
    }
}

void MikrotikDevice::UploadDatabase() {
    if (!is_connected) {
        return;
    }
}

int MikrotikDevice::InitializeSSH() {
    if (MikrotikDevice::lib_refcount++ == 0) {
        if (auto rc = libssh2_init(0); rc) {
            fprintf(stderr, "libssh2 initialization failed (%d)\n", rc);
            return 1;
        }
    }

    /* Create a session instance */
    session = libssh2_session_init();
    if (!session) {
        fprintf(stderr, "Could not initialize SSH session.\n");
        return 2;
    }

    return 0;
}

int MikrotikDevice::ConnectSSH(std::string username, std::string password) {
    int result = 0;

#ifdef _WIN32
    WSADATA wsadata;
    result = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if (result) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", result);
        return result;
    }
#endif

    auto hostaddr = inet_addr(hostname.c_str());

    sock = static_cast<s32>(socket(AF_INET, SOCK_STREAM, 0));
    if (sock == static_cast<s32>(LIBSSH2_INVALID_SOCKET)) {
        fprintf(stderr, "failed to create socket.\n");
        return errno;
    }

    sockaddr_in sin = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {},
    };
    sin.sin_addr.s_addr = hostaddr;

    result = connect(sock, reinterpret_cast<sockaddr*>(&sin), sizeof(sockaddr_in));
    if (result) {
        return result;
    }

    auto lock = std::scoped_lock(session_mutex);

    result = libssh2_session_handshake(session, sock);
    if (result)
        return result;

    result = libssh2_userauth_password(session, username.data(), password.data());
    if (result)
        return result;

    libssh2_session_set_blocking(session, 1);

    return result;
}

int MikrotikDevice::ExecuteSSH(std::string commandline, std::string* output) {
    int result{};
    LIBSSH2_CHANNEL* channel;

    /* Exec non-blocking on the remote host */
    do {
        channel = libssh2_channel_open_session(session);
        if (channel || libssh2_session_last_error(session, NULL, NULL, 0) != LIBSSH2_ERROR_EAGAIN)
            break;
        waitsocket(sock, session);
    } while (1);
    if (!channel) {
        return 1;
    }
    while ((result = libssh2_channel_exec(channel, commandline.c_str())) == LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if (result) {
        return 1;
    }
    for (;;) {
        ssize_t nread{};
        /* loop until we block */
        do {
            std::array<char, 0x400> buffer{};
            nread = libssh2_channel_read(channel, buffer.data(), buffer.size());

            if (nread <= 0) {
                continue;
            }
            if (output == nullptr) {
                continue;
            }

            for (ssize_t i = 0; i < nread; ++i)
                *output += buffer[i];
            *output += "\n";
        } while (nread > 0);

        if (nread < 0 && nread != LIBSSH2_ERROR_EAGAIN) {
            fprintf(stderr, "libssh2_channel_read returned %ld\n", (long)nread);
        }

        /* this is due to blocking that would occur otherwise so we loop on
           this condition */
        if (nread == LIBSSH2_ERROR_EAGAIN) {
            waitsocket(sock, session);
            continue;
        }
        break;
    }

    while ((result = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
        waitsocket(sock, session);

    libssh2_channel_free(channel);
    channel = nullptr;

    return result;
}

int MikrotikDevice::DisconnectSSH() {
    int result = 0;
    auto lock = std::scoped_lock(session_mutex);

    if (session) {
        result = libssh2_session_disconnect(session, "Normal Shutdown");
    }

    if (sock != static_cast<s32>(LIBSSH2_INVALID_SOCKET)) {
        shutdown(sock, 2);
        LIBSSH2_SOCKET_CLOSE(sock);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return result;
}

} // namespace Mikrotik
