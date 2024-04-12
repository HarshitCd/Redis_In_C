#include <stdint.h>
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

static int query(int connfd, const char *content)
{
    uint32_t len = (uint32_t)strlen(content);
    if (len > k_max_msg)
    {
        return -1;
    }

    char wbuf[header_len + k_max_msg];
    memcpy(wbuf, &len, 4); // assume little endian
    memcpy(&wbuf[4], content, len);

    int32_t err = write_all(connfd, wbuf, header_len + len);
    if (err)
    {
        return err;
    }

    char rbuf[header_len + k_max_msg + 1];
    errno = 0;

    // Getting the header data
    err = read_full(connfd, rbuf, header_len);
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

    len = 0;
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

    return 0;
}

int main()
{
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd < 0)
    {
        die("socket()");
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = ntohs(1234);
    server_addr.sin_addr.s_addr = ntohl(0);

    int rv = connect(connfd, (const sockaddr *)&server_addr, sizeof(server_addr));

    int err = query(connfd, "hello1");
    if (err)
    {
        goto L_DONE;
    }

    err = query(connfd, "hello2");
    if (err)
    {
        goto L_DONE;
    }

    err = query(connfd, "hello3");
    if (err)
    {
        goto L_DONE;
    }

L_DONE:
    close(connfd);
    return 0;
}