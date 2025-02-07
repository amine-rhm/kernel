#include <signal.h>
#include <unistd.h>

struct SCC { // System Call Context
        int number;
        union {
                char * s;
                int i;
        } u;
        int pid;
        int longueur;
        int result;
}; // process fill a SCC struct and set pointer before syscall

typedef struct SCC scc_t;

#ifdef MY_LITLE_KERNEL
extern scc_t * system_call_ctx; // process fill a SCC struct and set pointer before syscall
#else
extern scc_t * system_call_ctx; // process fill a SCC struct and set pointer before syscall

scc_t scc;

int spawn(char *c) {
        scc.number=1;
        scc.u.s=c;
        system_call_ctx=&scc;
        kill(getpid(),SIGUSR1);
        return scc.result;
}

int mlk_clock() {
        scc.number=2;
        system_call_ctx=&scc;
        kill(getpid(),SIGUSR1);
        return scc.result;
}

int mlk_sleep(int s) {
        scc.number=3;
        scc.u.i=s;
        system_call_ctx=&scc;
        kill(getpid(),SIGUSR1);
        return scc.result;
}

int mlk_getpid() {
        scc.number=4;
        system_call_ctx=&scc;
        kill(getpid(),SIGUSR1);
        return scc.result;
}


int mlk_print(char *msg) {
    scc.number = 5;
    scc.u.s = msg;
    system_call_ctx = &scc;
    kill(getpid(), SIGUSR1);
    return scc.result;
}

int mlk_signal(int p) {
    scc.number = 6; 
    scc.u.i = p;
    system_call_ctx = &scc;
    kill(getpid(), SIGUSR1);
    return scc.result;
}

int mlk_wait() {
    scc.number = 7;
    system_call_ctx = &scc;
    kill(getpid(), SIGUSR1);
    return scc.result;
}

int mlk_send(void *mesg, int l, int p)  {
    scc.number = 8;
    scc.u.s = mesg;
    scc.pid = p;
    scc.longueur = l;
    system_call_ctx = &scc;
    kill(getpid(), SIGUSR1);
    return scc.result;
}

int mlk_recv(void *buff, int l)  {
    scc.number = 9;
    scc.u.s = buff;
    scc.longueur = l;
    system_call_ctx = &scc;
    kill(getpid(), SIGUSR1);
    return scc.result;
} 


#endif
