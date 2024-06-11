#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>  // for linux
#include <sys/fcntl.h>
#include <unistd.h>

#include "conn.h"
#include "server.h"
#include "swap.h"

int max_events = 10;
int ep_fd = 0;
int sv[2] = {0};
int l_fd = 0;

void init_server();
void init_epoll_fd();
void epoll_handler();

void server_starter(int port) {
    if (fd == 0) {
        init_server(port);
        init_epoll_fd();
    } else {
        // hot update
        fd_recover();
    }

    epoll_handler();
}

void init_server(int port) {
    int s_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_fd <= 0) {
        perror("socket");
        close(s_fd);
        return;
    }

    // make s_fd non block
    int flags = fcntl(s_fd, F_GETFL, 0);
    fcntl(s_fd, F_SETFL, flags | O_NONBLOCK);

    // bind and listen
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(s_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s_fd);
        return;
    }

    // listen s_fd and set accept queue size = 128
    if (listen(s_fd, 128) < 0) {
        perror("listen");
        close(s_fd);
        return;
    }

    l_fd = s_fd;
}

void init_epoll_fd() {
    // create and epoll and set event queue size = 128
    ep_fd = epoll_create(128);
    if (ep_fd < 0) {
        perror("epoll_create");
        close(ep_fd);
        close(l_fd);
        return;
    }

    // register accept event
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = l_fd;
    if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, l_fd, &ev) < 0) {
        perror("epoll_ctl l_fd");
        close(ep_fd);
        close(l_fd);
        return;
    }
}
void epoll_handler() {
    // a epoll event list
    struct epoll_event events[max_events];
    // wait and hanlder net event
    whild(1) {
        // wait net events 10s
        // if any events arrived, will write into events
        int nfds = epoll_wait(ep_fd, events, max_events, 10000);
        printf("read nfds: %d\n", nfds);
        if (nfds < 0) {
            perror("epoll_wait");
            continue;
        }

        int i;
        for (i = 0; i < nfds; i++) {
            struct epoll_event curr_ev = events[i];
            if (curr_ev.data.fd == l_fd) {  // means a new connection
                int c_fd = accept(l_fd, NULL, NULL);
                if (c_fd <= 0) {
                    perror("accept");
                    continue;
                }

                struct epoll_event nev;
                nev.events = EPOLLIN;
                struct event_ctx *e_ctx = malloc(sizeof(event_ctx));
                e_ctx->e_fd = c_fd;
                e_ctx->data = "pass succeed"; // you can put anything here
                e_ctx->type = 1;    // 1 for not sentinel
                nev.data.ptr = e_ctx;
                
                // read event register
                // when EPOLLIN event arrive, this ptr will be used in program
                e_ctx->rev = &nev;
                if (epoll_ctl(ep_fd, EPOLL_CTL_ADD, c_fd, &nev) < 0) {
                    perror("epoll_ctl accept fd");
                    continue;
                }

                // add conn_ctx to list
                conn_ctx *new_conn = malloc(sizeof(conn_ctx));
                if (new_conn == NULL) {
                    perror("malloc conn_ctx");
                    // actually, continue is a wrong action
                    continue;
                }
                e_ctx->conn_ptr = new_conn;
                new_conn->c_fd = c_fd;
                new_conn->eve_ctx = e_ctx;
                list->insert_handler(new_conn);
            } else {
                // when a read event received
                char buf[1024]; // a simple read buffer
                struct event_ctx *e_ctx = curr_ev.data.ptr; // the context you set when accept a connection
                ssize_t size = read(e_ctx->e_fd, &buf, 1023);
                if (size <= 0) {
                    // maybe means close segment or error in tcp
                    printf("read size 0\n");
                    if (epoll_ctl(ep_fd, EPOLL_CTL_DEL, e_ctx->e_fd, &curr_ev) < 0) {
                        perror("epoll_ctl rm ev");
                    }
                    close(e_ctx->e_fd); // close connection
                    list->remove_handler(e_ctx->conn_ptr);
                    continue;
                } else {
                    // print the read msg
                    printf("pid %d read from socket: %s, event data: %s\n", getpid(), buf, e_ctx->data);
                }
            }
        }
    }
}
