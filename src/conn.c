#include <stdlib.h>
#include <unistd.h>

#include "conn.h"
#include "server.h"

conn_ctx_list *list;
conn_ctx *sentinel;

void conn_ctx_list_ins(conn_ctx *node) {
    if (list->head == sentinel) {
        list->head = node;
        list->tail = node;
        node->next = sentinel;
        node->pre = sentinel;
    } else {
        list->tail->next = node;
        node->pre = list->tail;
        list->tail = node;
        node->next = sentinel;
    }
    list->size++;
}

void conn_ctx_list_rmv(conn_ctx *node) {
    if (node->pre == sentinel) {
        list->head = sentinel;
        list->tail = sentinel;
    } else {
        node->pre->next = node->next;
        if (node == list->head) {
            list->head = node->next;
        }
        if (node == list->tail) {
            list->tail = node->pre;
        }
    }
    free(node);
    list->size--;
}

void init_conn_ctx_list() {
    sentinel = malloc(sizeof(sentinel));
    sentinel->c_fd = 0;
    sentinel->eve_ctx = malloc(sizeof(event_ctx));
    sentinel->eve_ctx->type = 0;
    sentinel->next = sentinel;
    sentinel->pre = sentinel;

    list = malloc(sizeof(conn_ctx_list));
    list->head = sentinel;
    list->tail = sentinel;
    list->size = 0;
    list->insert_handler = conn_ctx_list_ins;
    list->remove_handler = conn_ctx_list_rmv;
}
