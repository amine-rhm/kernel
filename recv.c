#include <stdio.h>
#include "syscall.h"

void main() {
    char buffer[256];
    
    while (1) {
        int n = mlk_recv(buffer, sizeof(buffer));
        if (n > 0) {
            mlk_print("Message reçu :");
            mlk_print(buffer);
        } else {
            mlk_print("[recv] Pas de messages pour le moment");
        }
        mlk_sleep(1);
    }
}

