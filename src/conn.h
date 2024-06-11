struct event_ctx;
typedef struct conn_ctx {   // connection context
    int c_fd;
    struct event_ctx *eve_ctx;
    struct conn_ctx *next;
    struct conn_ctx *pre;
} conn_ctx;

typedef void (*conn_list_ins_func)(conn_ctx *node);
typedef void (*conn_list_rmv_func)(conn_ctx *node);
typedef struct conn_ctx_list {  // connection context list
    conn_ctx *head;
    conn_ctx *tail;
    int size;
    conn_list_ins_func insert_handler;
    conn_list_rmv_func remove_handler;
} conn_ctx_list;

void init_conn_ctx_list();

extern conn_ctx_list *list;
extern conn_ctx *sentinel;