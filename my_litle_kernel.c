#define _XOPEN_SOURCE 500
#define MY_LITLE_KERNEL
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include "syscall.h"


#define MAX_PROCESS 99
#define MAX_MSG_LEN 256
#define MAX_MSGS 10

int cp=0;           // current process number running
int nb_process=0;

static int waiting_processes[100];  // Tableau pour suivre les processus en attente
static int waiting_count = 0;  // Nombre de processus en attente


static struct PCB { // Process Context Block 
        ucontext_t uc; // context of process
        char *stack;   // stack of process
        int ppid;      // Parent process
        clock_t clock; // total clock time 
        clock_t lclock; // clock value before process is scheduled
        clock_t qclock; // clock time used during scheduled time
        int quantum; // quantum for the process
        scc_t *scc; // system call context
        struct timeval alarm;
} P[100];

typedef struct {
    char message[MAX_MSG_LEN];
    int length;
    int sender_pid;
} message_t;

static message_t msg_queue[MAX_PROCESS][MAX_MSGS];  // File d'attente par processus
static int msg_count[MAX_PROCESS];                  // Nombre de messages en attente

static int get_free_process_number() {
        return ++nb_process;
}

void (*principal)();
static  int spawn(char * command) {  // spawn new process, loaded but not launched (schedule job)
        void *handle;
        int p;
        char *error;

        handle = dlopen(command, RTLD_NOW);
        if ((error = dlerror()) != NULL)  {
                fprintf(stderr, "%s\n", error);
                if (!cp) exit(EXIT_FAILURE);
                else return EXIT_FAILURE;
        }

        principal = dlsym(handle, "main");
        if ((error = dlerror()) != NULL)  {
                fprintf(stderr, "%s\n", error);
                exit(EXIT_FAILURE);
        }

        p=get_free_process_number();

        P[p].stack=(char *)malloc(1024*1024);

        if (getcontext(&P[p].uc)) {
          perror("getcontext");
          exit(EXIT_FAILURE);
        }

        P[p].uc.uc_stack.ss_sp=P[p].stack;
        P[p].uc.uc_stack.ss_size=1024*1024;
        if (sigemptyset(&P[p].uc.uc_sigmask)) {
                fprintf(stderr, "sigemptyset error\n");
                exit(EXIT_FAILURE);
        }
        P[p].uc.uc_link=&P[0].uc; // when process finish, go back to scheduler
        makecontext(&P[p].uc,principal,0);
        return p;
}

static int mlk_clock() { // return clock for process
        return P[cp].clock + clock()-P[cp].lclock;
}

static int mlk_sleep(int s) {
        gettimeofday(&P[cp].alarm,NULL);
        P[cp].alarm.tv_sec+=s;
        return 0;
}

static int mlk_getpid() {
  return cp;
}

static void mlk_wait() {
        waiting_processes[waiting_count++] = cp;
        P[cp].quantum = -1;
        P[cp].alarm.tv_sec = INT_MAX;  // Fixer une alarme très éloignée (bloque le processus)
        swapcontext(&P[cp].uc, &P[0].uc);
}

static void mlk_signal(int p) {
    for (int i = 0; i < waiting_count; i++) {
        if (waiting_processes[i] == p) {
            for (int j = i; j < waiting_count - 1; j++) {
                waiting_processes[j] = waiting_processes[j + 1];
            }
            waiting_count--;
            P[p].quantum = 1;
            gettimeofday(&P[p].alarm, NULL);  // Réinitialiser l'alarme à maintenant (prêt à exécuter)
            return;
        }
    }
}

static void mlk_send(void *msg, int l, int p) {
    int target_pid = p;
    char *mesg = msg;
    int length = l;

    // Vérification du PID
    if (target_pid <= 0 || P[target_pid].stack == NULL) {
        printf("Erreur : PID %d invalide pour mlk_send\n", target_pid);
        system_call_ctx->result = -1;
        return;
    }

    if (msg_count[target_pid] < MAX_MSGS) {
        strncpy(msg_queue[target_pid][msg_count[target_pid]].message, mesg, length);
        msg_queue[target_pid][msg_count[target_pid]].length = length;
        msg_queue[target_pid][msg_count[target_pid]].sender_pid = cp;
        msg_count[target_pid]++;
        
        // Réveille le destinataire
        mlk_signal(target_pid);
        system_call_ctx->result = length;  
    } else {
        // Bloque si la file est pleine
        mlk_wait();
    }
}

static int mlk_recv(void *buff, int l) {
    char *buffer = buff;
    int max_len = l;

    if (msg_count[cp] > 0) {
        // Lire le message le plus ancien (FIFO)
        message_t mesg = msg_queue[cp][0];
        int copy_len = (mesg.length < max_len) ? mesg.length : max_len;

        strncpy(buffer, mesg.message, copy_len);
        buffer[copy_len] = '\0';

        // Décaler les messages (supprimer le premier)
        for (int i = 1; i < msg_count[cp]; i++) {
            msg_queue[cp][i - 1] = msg_queue[cp][i];
        }
        msg_count[cp]--;

        system_call_ctx->result = copy_len;
        return system_call_ctx->result;
    } else {
        // Bloquer jusqu'à ce qu'un message arrive
        mlk_wait();
        system_call_ctx->result = 0;
        return system_call_ctx->result;
    }
}



static void system_call() {
        int r;
        switch (system_call_ctx->number) {
                case 1: r=spawn(system_call_ctx->u.s);
                        system_call_ctx->result=r;
                        break;
                case 2: r=mlk_clock();
                        system_call_ctx->result=r;
                        break;
                case 3: r=mlk_sleep(system_call_ctx->u.i);
                        system_call_ctx->result=r;
                        break;
                case 4: r=mlk_getpid();
                        system_call_ctx->result=r;
                        break;
                case 5:
                        printf("[%d] %s\n", cp, system_call_ctx->u.s);
                        system_call_ctx->result = 0;
                        break;
                case 6:
                        mlk_signal(system_call_ctx->u.i);
                        system_call_ctx->result = 0;
                        break;
                case 7: 
                        mlk_wait();
                        system_call_ctx->result = 0;
                        break;
                case 8:
                        mlk_send(system_call_ctx->u.s, system_call_ctx->longueur, system_call_ctx->pid);
                        break;
                case 9:
                        mlk_recv(system_call_ctx->u.s, system_call_ctx->longueur);
                        break;
			
               default: system_call_ctx->result=-1;
        }
}


static int choose_next_process() { // RR(1)
        struct timeval tv;
        if (!nb_process) return -1;
        int p=cp%nb_process;
        while (gettimeofday(&tv,NULL),P[p+1].alarm.tv_sec>tv.tv_sec) p=(p+1)%nb_process;
        P[p+1].quantum=1;
        return p+1;
}

static void scheduler() {
        struct itimerval itv;
        while (1) {
                cp=choose_next_process();

                if (cp==-1) {
                        printf("Plus de process\n");
                        exit(EXIT_SUCCESS);
                }

                P[cp].lclock=clock(); // 

                if (P[cp].quantum!=-1) { // set alarm for next preemtion
                        itv.it_value.tv_sec=P[cp].quantum; itv.it_value.tv_usec=0;
                        itv.it_interval.tv_sec=0; itv.it_interval.tv_usec=0;
        
                        if (setitimer(ITIMER_REAL,&itv,NULL)) {
                                printf("Error setitimer\n");
                                exit(2);
                        }
                }       
                swapcontext(&P[0].uc,&P[cp].uc);
        }
}


void sig_handler(int s, siginfo_t *si, void* uctx) {
  if (s!=SIGALRM && s!=SIGUSR1) {
    write(0,"SIGNAL PAS BON\n",15);
    exit(EXIT_FAILURE);
  }
  if (!cp) {
    write(0,"SIGNAL IN SCHEDULER !!!!\n",15);
    exit(EXIT_FAILURE);
  }
  P[cp].clock+=(P[cp].qclock=clock()-P[cp].lclock);  // update process time
  if (s==SIGUSR1) {
        P[cp].scc=system_call_ctx;
        system_call();
   }
  swapcontext(&P[cp].uc,&P[0].uc); // back to scheduler
}

int main()
{       sigset_t signal_set;
        struct sigaction sa;

        // Kernel may never be interrupted : set mask
        if (sigemptyset(&signal_set)) {
                fprintf(stderr, "sigemptyset error\n");
                exit(EXIT_FAILURE);
        }

        if (sigaddset(&signal_set,SIGALRM)) {
                fprintf(stderr, "sigaddset error\n");
                exit(EXIT_FAILURE);
        }

        if (sigaddset(&signal_set,SIGUSR1)) {
                fprintf(stderr, "sigaddset error\n");
                exit(EXIT_FAILURE);
        }

        if (sigprocmask(SIG_BLOCK,&signal_set,NULL)) {
                fprintf(stderr, "sigprocmask error\n");
                exit(EXIT_FAILURE);
        }

 
        sa.sa_sigaction=sig_handler;
        sa.sa_flags=SA_SIGINFO|SA_ONSTACK;
        if (sigaction(SIGALRM,&sa,NULL)) {
                fprintf(stderr, "sigaction error\n");
                exit(EXIT_FAILURE);
        }

        if (sigaction(SIGUSR1,&sa,NULL)) {
                fprintf(stderr, "sigaction error\n");
                exit(EXIT_FAILURE);
        }
        printf("Kernel Information : there is %lu clock per second\n",CLOCKS_PER_SEC);
        spawn("./init.so");
        scheduler();
	
        exit(EXIT_SUCCESS);
}
