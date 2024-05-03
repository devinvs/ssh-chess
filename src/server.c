#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static int fd;

// receive and handle a message
void handle_message() {
    char buf[4096];
    struct sockaddr src_addr;
    socklen_t src_len;

    size_t n = recvfrom(fd, buf, 4096, 0, &src_addr, &src_len);
    printf("got %d bytes\n", n);
    printf("addr len: %d\n", src_len);

    sendto(fd, buf, n, 0, (const struct sockaddr *) &src_addr, sizeof(struct sockaddr_un));
    printf("sent response\n");
}

int main() {
    // setup unix socket for receiving/managing games
    fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    // bind as a named pipe
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/home/devin/.chess");
    int ret = bind(fd, (const struct sockaddr *) &addr, sizeof(struct sockaddr_un));
    if (ret == -1) {
        perror("bind");
        exit(1);
    }

    ret = listen(fd, 20);
    if (ret = -1) {
        perror("listen");
        exit(1);
    }

    while (1) {
        handle_message();
    }
}
