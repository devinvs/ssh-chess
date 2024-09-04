#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>

#define MATCH_SOCK ".matchmaking"

int create_server_socket() {
    // unlink the socket if it exists
    unlink(MATCH_SOCK);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = MATCH_SOCK
    };

    int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        printf("failed to create matchmaking socket\n");
        exit(1);
    }
    listen(fd, 0x20);
    
    return fd;
}

int wait_for_client(int fd) {
    return accept(fd, NULL, NULL);
}

int send_fd(int fd, int sfd) {
    struct msghdr msg = { 0 };
    char buf[CMSG_SPACE(sizeof(sfd))];
    memset(buf, '\0', sizeof(buf));
    struct iovec io = { .iov_base = "ABC", .iov_len = 3 };

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    // put that juicy file descriptor in the message control block
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(sfd));
    *((int *)CMSG_DATA(cmsg)) = sfd; // yum

    // send the darn thing
    return sendmsg(fd, &msg, 0);
}

int main() {
    // mean SIGPIPES harshing the vibe
    signal(SIGPIPE, SIG_IGN);

    int res;
    int fd = create_server_socket();

    int a = -1;
    int b = -1;

    while (1) {
        // wait for the first client
        a = wait_for_client(fd);
        if (a == -1) {
            perror("no wait");
            exit(1);
        }

        // keep accepting and cycling clients until we have two live clients
        while (1) {
            b = wait_for_client(fd);
            if (b == -1) {
                perror("no wait b");
                exit(1);
            }

            // create a socketpair for them to communicate
            int socks[2];
            res = socketpair(AF_UNIX, SOCK_DGRAM, 0, socks);
            if (res == -1) {
                perror("pare me");
                exit(1);
            }

            // attempt to send the socket to "a". If this fails "a"
            // has bailed, "b" is now "a" and wait for more clients
            if (send_fd(a, socks[0]) != -1) {
                send_fd(b, socks[1]);
                break;
            }

            a = b;
        }
    }
}
