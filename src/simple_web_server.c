#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "conn.h"
#include "server.h"
#include "swap.h"

void sig_usr2_handle() {
    pid_t pid;
    pid = fork();
    if (pid <= 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // child
        char l_fd_str[32];
        char ep_fd_str[32];
        char sv0_str[32];
        char sv1_str[32];

        sprintf(l_fd_str, sizeof(l_fd_str), "LISTEN_FD=%d", l_fd);
        sprintf(ep_fd_str, sizeof(ep_fd_str), "EP_FD=%d", ep_fd);
        sprintf(sv0_str, sizeof(sv0_str), "SV0=%d", sv[0]);
        sprintf(sv1_str, sizeof(sv1_str), "SV1=%d", sv[1]);

        // construct the argv and envp, then send
        char *argv[] = {"sw-server", NULL};
        char *envp[] = {l_fd_str, ep_fd_str, sv0_str, sv1_str, NULL};

        int rc = execev("./sw-server", argv, envp);
        if (rc < 0) {
            perror("execve error");
            return;
        }
    } else {
        // parent
        close(l_fd);
        // send c_fd list
        if (fd_send() == -1) {
            return;
        } else {
            sleep(3);
            exit(0);
        }
    }
}

int sig_register() {
    if (signal(SIGUSR2, sig_usr2_handle) == SIG_ERR) {
        perror("signal");
        return -1;
    }

    // as we execev in sigusr2_handle, the sigusr2 will be blocked, clear the block
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2);
    if (sigprocmask(1, &set, NULL) != 0) {
        perror("sigprocmask");
        return -1;
    }
    return 0;
}

int main() {
    char *l_fd_str = getenv("LISTEN_FD");
    if (l_fd_str != NULL) {
        l_fd = atoi(l_fd_str);
        ep_fd = atoi(getenv("EP_FD"));
        sv[0] = atoi(getenv("SV0"));
        sv[1] = atoi(getenv("SV1"));
    } else {
        socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    }
    init_conn_ctx_list();
    sig_register();
    server_starter(8883);
    return 0;
}