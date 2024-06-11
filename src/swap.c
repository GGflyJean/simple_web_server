#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "conn.h"
#include "server.h"
#include "swap.h"

typedef struct msghdr msghdr;
typedef struct cmsghdr cmsghdr;
typedef struct iovec iovec;

int fd_send() {
    conn_ctx *curr = list->head;
    while (1){
        int fd = curr->c_fd;
        event_ctx *ctx = curr->eve_ctx;

        // use sendmsg send fd
        // construct the fd and related context info
        msghdr msg = {0};
        cmsghdr *cmsg;
        char buf[CMSG_SPACE(sizeof(fd))];
        memset(buf, '\0', sizeof(buf));

        iovec io = {.iov_base = ctx, .iov_len = sizeof(event_ctx)};

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        msg.msg_control = buf;
        msg.msg_controllen = sizeof(buf);

        cmsg = CMSG_FIRSTHDR(&msg);
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN(sizeof(fd));

        *((int *) CMSG_DATA(cmsg)) = fd;
        printf("pid %d, send fd %d\n", getpid(), fd);
        if (sendmsg(sv[0], &msg, 0) == -1) {
            perror("sendmsg error");
            return -1;
        }

        // rm related read event here
        epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, ctx->rev);
        if (curr == sentinel) {
            break;
        } else {
            curr = curr->next;
        }
        list->remove_handler(curr);
    }
}

int fd_recover() {
    while(1) {
        // loop until sentinel received
        event_ctx *eve_ctx = malloc(sizeof(event_ctx));
        msghdr msg = {0};
        iovec io = {.iov_base = event_ctx, .iov_len = sizeof(event_ctx)};

        msg.msg_iov = &io;
        msg.msg_iovlen = 1;

        char buf[CMSG_SPACE(sizeof(int))];
        msg.msg_control = buf;
        msg.msg_controllen = 1;

        if (recvmsg(sv[1], &msg, 0) == -1) {
            perror("recvmsg error");
            free(eve_ctx);
            return -1;
        }

        // recover the context
        cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
        int fd;
        memmove(&fd, CMSG_DATA(cmsg), sizeof(fd));
        printf("pid %d, recv and recover fd: %d\n", getpid(), fd);

        if (eve_ctx->type == 0) {
            // sentinel, return
            return 0;
        }

        // event reregister
        struct epoll_event nev;
        nev.events = EPOLLIN;
        eve_ctx->data = "pass succeed";
        nev.data.ptr = eve_ctx;

        if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, fd, &nev) < 0) {
            free(eve_ctx);
            perror("epoll_ctl accept fd");
            continue;
        }
        eve_ctx->rev = &nev;

        // connection list recover
        conn_ctx *node = malloc(sizeof(conn_ctx));
        if (node == NULL) {
            perror("recover conn fd, malloc node size error");
            free(eve_ctx);
            epoll_ctl(ep_fd, EPOLL_CTL_DEL, fd, &nev);
            continue;
        }
        eve_ctx->conn_ptr = node;
        node->c_fd = fd;
        node->eve_ctx = eve_ctx;
        list->insert_handler(node);
    }
    return 0;
}