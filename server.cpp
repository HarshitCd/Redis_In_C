// #include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include "utils.h"

const size_t k_max_msg = 4096;
const size_t header_len = 4;

static void do_something(int connfd)
{
    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        msg("read() error");
        return;
    }
    printf("client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int32_t one_request(int connfd)
{
    char rbuf[header_len + k_max_msg + 1];
    errno = 0;

    // Getting the header data
    int32_t err = read_full(connfd, rbuf, header_len);
    if (err)
    {
        if (errno == 0)
        {
            msg("EOF");
        }
        else
        {
            msg("read() err");
        }

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, header_len);
    if (len > k_max_msg)
    {
        msg("message too long");
        return -1;
    }

    // Getting the request body
    err = read_full(connfd, &rbuf[header_len], len);
    if (err)
    {
        msg("read() error");
        return err;
    }
    rbuf[header_len + len] = '\0';
    printf("server says: %s\n", &rbuf[header_len]);

    // Do something
    const char reply[] = "world";
    len = (uint32_t)strlen(reply);

    char wbuf[header_len + len];
    memcpy(wbuf, &len, header_len);
    memcpy(&wbuf[header_len], reply, len);

    return write_all(connfd, wbuf, header_len + len);
}

int32_t main()
{
    // Create a socket and get a file descriptor back
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("socket()");
    }

    // Setting socket option to reuse address and not wait for TIME_WAIT
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // Creating the listen address structure
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    // Binding the port to "0.0.0.0:1234"
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("bind()");
    }

    // Listening to the address for connections
    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("listen()");
    }

    // Accepting connections and doing something with the data sent throw the connection
    while (true)
    {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (sockaddr *)&client_addr, &addrlen);
        if (connfd < 0)
        {
            continue; // error
        }

        while (true)
        {
            int32_t err = one_request(connfd);
            if (err)
            {
                break;
            }
        }

        close(connfd);
    }

    return 0;
}