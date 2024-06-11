struct conn_ctx;
typedef struct event_ctx {
    int e_fd;                   // event fd, l_fd or c_fd
    int type;                   // mark if belong to sentinel
    char *data;                 // event data, unused this project
    struct epoll_event *rev;    // read event only this project
    struct conn_ctx *conn_ptr;  // ptr to connection context
} event_ctx;

extern int max_events;          // global variable for max_events
extern int ep_fd;               // global variable for epoll_fd
extern int sv[2];               // global varibale for socket pair, parent recv/send: sv[0], child recv/send: sv[1]
extern int l_fd;                // global variable for listen_fd