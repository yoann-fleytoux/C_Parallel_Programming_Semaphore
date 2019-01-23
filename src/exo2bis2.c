#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>         // fonction exit()
#include <sys/types.h>      // type key_t
#include <sys/ipc.h>        // fonction ftok()
#include <sys/shm.h>
#include <sys/wait.h>       // fonction wait_pid()

#include "../include/semaphore.h"

#define ID          2
#define NB_PRINT    100
#define MUTEX_PERE  0
#define MUTEX_FILS  1

// Variables globales
key_t numero_externe;
int numero_interne;


/**
 * Fonction d'affichage du fils
 */
void fils() {
    for (int i = 0; i < NB_PRINT; i++) {
        if (P(numero_interne, MUTEX_FILS, 1) == -1)
            perror("Erreur : fonction P");
        printf("Je suis ton fils!\n");
        if (V(numero_interne, MUTEX_PERE, 1) == -1)
            perror("Erreur : fonction V");
    }
}

/**
 * Fonction d'initialisation des sémaphores
 */
void initialisation_semaphore() {
    // Création d'une clé IPC System, la même que exo2bis1
    numero_externe = ftok("main.c", ID);
    numero_interne = ouvrirSem((key_t) numero_externe);
    if (numero_interne == -1) {
        perror("ERREUR : ouvrirSem()");
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    initialisation_semaphore();

    int ni = shmget(numero_externe, 0, 0);
    key_t *pid_fils = shmat(ni, 0, 0);
    *pid_fils = getpid();
    
    fils();

    shmdt(pid_fils);

    return 0;
}
