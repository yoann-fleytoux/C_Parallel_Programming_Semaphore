#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>         // fonction exit()
#include <sys/types.h>      // type key_t
#include <sys/ipc.h>        // fonction ftok()
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
 * Fonction d'affichage du père
 */
void pere() {
    for (int i = 0; i < NB_PRINT; i++) {
        if (P(numero_interne, MUTEX_PERE, 1) == -1)
            perror("Erreur : fonction P");
        printf("Je suis ton père!\n");
        if (V(numero_interne, MUTEX_FILS, 1) == -1)
            perror("Erreur : fonction V");
    }
}

/**
 * Fonction d'initialisation des sémaphores
 */
void initialisation_semaphore() {
    // Création d'une clé IPC System
    numero_externe = ftok("main.c", ID);
    // Création d
    numero_interne = creerSem((key_t) numero_externe, 2, 0600);
    if (numero_interne == -1) {
        perror("ERREUR : creerSem()");
        exit(-1);
    }
    if (initialiserSem(numero_interne, MUTEX_PERE, 1) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du pere");
        exit(-2);
    }
    if (initialiserSem(numero_interne, MUTEX_FILS, 0) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du fils");
        exit(-2);
    }
}

int main(int argc, char *argv[]) {
    initialisation_semaphore();

    int f = fork();

    switch (f) {
        case -1:
            fprintf(stderr, "ERREUR: fork()\n");
            break;
        case 0:
            fils();
            exit(0);
        default:
            pere();
            break;
    }
    waitpid(f, NULL, 0);
    detruireSem(numero_interne);

    return 0;
}
