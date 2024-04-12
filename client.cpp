#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void msg(const char *msg)
{
    fprintf(stdout, "%s\n", msg);
}

static void do_something(int connfd)
{
    char wbuf[] = "hello";
    write(connfd, wbuf, strlen(wbuf));

    char rbuf[64] = {};
    ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        msg("read() error");
        return;
    }
    printf("server says: %s\n", rbuf);
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
    do_something(connfd);
    close(connfd);
}