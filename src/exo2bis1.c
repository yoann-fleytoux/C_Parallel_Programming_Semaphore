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


/* Les programmes exo2bis1 et exo2bis2 permettent de tester la fonction
 * ouvrirSem().
 * Il faut d'abord exécuter la fonction exo2bis1 qui va créer les sémaphores,
 * puis exécuter exo2bis2 qui va ouvrir la mémoire des sémaphores créée
 * précédemment
 */



// Variables globales
key_t numero_externe;
int numero_interne;


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
    // Création des sémaphores
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

    int ni = shmget(numero_externe, sizeof(key_t), IPC_EXCL|IPC_CREAT|0600);
    // Attachement de la mémoire
    key_t *pid_fils = shmat(ni, 0, 0);

    pere();

    waitpid(*pid_fils, NULL, 0);
    shmdt(pid_fils);
    shmctl(ni, IPC_RMID, 0);
    detruireSem(numero_interne);

    return 0;
}
