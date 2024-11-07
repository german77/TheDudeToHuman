// SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "libssh2.h"
#include "the_dude_to_human/mikrotik/mikrotik_device.h"

#define BUFSIZE 32000
#pragma warning(disable : 4996)

static int waitsocket(libssh2_socket_t socket_fd, LIBSSH2_SESSION* session) {
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set* writefd = NULL;
    fd_set* readfd = NULL;
    int dir;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

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
    dir = libssh2_session_block_directions(session);

    if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select((int)(socket_fd + 1), readfd, writefd, NULL, &timeout);

    return rc;
}

namespace Mikrotik {
MikrotikDevice::MikrotikDevice(std::string address_, u16 port_)
    : hostname{address_}, port{port_}, sock{socket(AF_INET, SOCK_STREAM, 0)} {
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

bool MikrotikDevice::Execute(std::string commandline) {
    if (!is_connected) {
        return false;
    }

    ExecuteSSH(commandline);

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
    uint32_t hostaddr;
    struct sockaddr_in sin;
    const char* fingerprint;
    int rc;
    int exitcode = 0;
    size_t len;
    LIBSSH2_KNOWNHOSTS* nh;
    int type;

#ifdef _WIN32
    WSADATA wsadata;

    rc = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if (rc) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", rc);
        return 1;
    }
#endif

    hostaddr = inet_addr(hostname.c_str());

    /* Ultra basic "connect to port port on localhost".  Your code is
     * responsible for creating the socket establishing the connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == LIBSSH2_INVALID_SOCKET) {
        fprintf(stderr, "failed to create socket.\n");
        DisconnectSSH();
        return 1;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in))) {
        fprintf(stderr, "failed to connect.\n");
        DisconnectSSH();
        return 1;
    }

    /* tell libssh2 we want it all done non-blocking */
    libssh2_session_set_blocking(session, 0);

    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN)
        ;
    if (rc) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        DisconnectSSH();
        return 1;
    }

    nh = libssh2_knownhost_init(session);
    if (!nh) {
        /* eeek, do cleanup here */
        return 2;
    }

    /* read all hosts from here */
    libssh2_knownhost_readfile(nh, "known_hosts", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    /* store all known hosts to here */
    libssh2_knownhost_writefile(nh, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    fingerprint = libssh2_session_hostkey(session, &len, &type);
    if (fingerprint) {
        struct libssh2_knownhost* host;
        int check = libssh2_knownhost_checkp(
            nh, hostname.c_str(), port, fingerprint, len,
            LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW, &host);

        fprintf(stderr, "Host check: %d, key: %s\n", check,
                (check <= LIBSSH2_KNOWNHOST_CHECK_MISMATCH) ? host->key : "<none>");

        /*****
         * At this point, we could verify that 'check' tells us the key is
         * fine or bail out.
         *****/
    } else {
        /* eeek, do cleanup here */
        return 3;
    }
    libssh2_knownhost_free(nh);

    if (!password.empty()) {
        /* We could authenticate via password */
        while ((rc = libssh2_userauth_password(session, username.c_str(), password.c_str())) ==
               LIBSSH2_ERROR_EAGAIN)
            ;
        if (rc) {
            fprintf(stderr, "Authentication by password failed.\n");
            return rc;
        }
    }

    return exitcode;
}

int MikrotikDevice::ExecuteSSH(std::string commandline) {
    int rc;
    int exitcode = 0;
    ssize_t bytecount = 0;
    char* exitsignal = (char*)"none";
    LIBSSH2_CHANNEL* channel;

    /* Exec non-blocking on the remote host */
    do {
        channel = libssh2_channel_open_session(session);
        if (channel || libssh2_session_last_error(session, NULL, NULL, 0) != LIBSSH2_ERROR_EAGAIN)
            break;
        waitsocket(sock, session);
    } while (1);
    if (!channel) {
        fprintf(stderr, "Error\n");
        exit(1);
    }
    while ((rc = libssh2_channel_exec(channel, commandline.c_str())) == LIBSSH2_ERROR_EAGAIN) {
        waitsocket(sock, session);
    }
    if (rc) {
        fprintf(stderr, "exec error\n");
        exit(1);
    }
    for (;;) {
        ssize_t nread;
        /* loop until we block */
        do {
            char buffer[0x4000];
            nread = libssh2_channel_read(channel, buffer, sizeof(buffer));
            if (nread > 0) {
                ssize_t i;
                bytecount += nread;
                fprintf(stderr, "We read:\n");
                for (i = 0; i < nread; ++i)
                    fputc(buffer[i], stderr);
                fprintf(stderr, "\n");
            } else {
                if (nread != LIBSSH2_ERROR_EAGAIN)
                    /* no need to output this for the EAGAIN case */
                    fprintf(stderr, "libssh2_channel_read returned %ld\n", (long)nread);
            }
        } while (nread > 0);

        /* this is due to blocking that would occur otherwise so we loop on
           this condition */
        if (nread == LIBSSH2_ERROR_EAGAIN) {
            waitsocket(sock, session);
        } else
            break;
    }
    exitcode = 127;
    while ((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
        waitsocket(sock, session);

    if (rc == 0) {
        exitcode = libssh2_channel_get_exit_status(channel);
        libssh2_channel_get_exit_signal(channel, &exitsignal, NULL, NULL, NULL, NULL, NULL);
    }

    if (exitsignal)
        fprintf(stderr, "\nGot signal: %s\n", exitsignal);
    else
        fprintf(stderr, "\nEXIT: %d bytecount: %ld\n", exitcode, (long)bytecount);

    libssh2_channel_free(channel);
    channel = NULL;
    return rc;
}

int MikrotikDevice::DisconnectSSH() {
    int result = 0;

    if (session) {
        result = libssh2_session_disconnect(session, "Normal Shutdown");
    }

    if (sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(sock, 2);
        LIBSSH2_SOCKET_CLOSE(sock);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return result;
}

} // namespace Mikrotik
