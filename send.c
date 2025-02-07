#include <stdio.h>
#include "syscall.h"

void main() {
    int p = spawn("./recv.so");

    if (p <= 0) {
        mlk_print("Erreur : recv.so n'a pas pu être lancé !");
        return;
    }

    char msg[] = "Salut Processus 2";

    while (1) {
        mlk_send(msg, sizeof(msg), p);
        mlk_print("Message envoyé !");
        mlk_sleep(1);
    }
}
