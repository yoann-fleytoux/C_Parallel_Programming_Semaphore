#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>           // Fonction srand()

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "../include/semaphore.h"

/*
    Gestion de ressources : Modèle des producteurs/consommateurs
    Variante 0
*/
#define VERBOSE             1   // Mode de débug
#define ID                  2
#define N                   4   //tampon de N cases gérées circulairement
#define MUTEX_DEPOT         0
#define MUTEX_RETRAIT       1
#define MUTEX_PRODUCTEUR    2
#define MUTEX_CONSOMMATEUR  3
#define NB_PRODUCTEUR       10
#define NB_CONSOMMATEUR     10

// Variables globales
key_t numero_externe;
int numero_interne;


struct message {
    unsigned int type;
    char *contient;
    int createur;
};

struct memoire_partagee {
    int ICP;    // incide case pleine
    int ICV;    // indice case vide
    struct message BUF[N];
};

struct memoire_partagee *m;

char *tab_type[2][2] = {{"blanc", "noir"}, {"recto", "verso"}};

void copy_message(struct message *m1, struct message *m2) {
    m2->type = m1->type;
    m2->contient = m1->contient;
    m2->createur = m1->createur;
}


void deposer(struct message *mes) {
    if (P(numero_interne, MUTEX_DEPOT, 1) == -1)
        perror("Erreur : fonction P");
    if (P(numero_interne, MUTEX_PRODUCTEUR, 1) == -1)
        perror("Erreur : fonction P");
    copy_message(mes, &(m->BUF[m->ICV]));
    m->ICV = (m->ICV+1)%N;
    if (V(numero_interne, MUTEX_PRODUCTEUR, 1) == -1)
        perror("Erreur : fonction V");
    if (V(numero_interne, MUTEX_RETRAIT, 1) == -1)
        perror("Erreur : fonction V");
}


struct message* retirer() {
    struct message *mes = malloc(sizeof(struct message));
    if (mes == NULL) {
        fprintf(stderr, "ERREUR : allocation mémoire\n");
        exit(-1);
    }
    if (P(numero_interne, MUTEX_RETRAIT, 1) == -1)
        perror("Erreur : fonction P");
    if (P(numero_interne, MUTEX_CONSOMMATEUR, 1) == -1)
        perror("Erreur : fonction P");
    copy_message(&(m->BUF[m->ICP]), mes);
    m->ICP = (m->ICP+1)%N;
    if (V(numero_interne, MUTEX_CONSOMMATEUR, 1) == -1)
        perror("Erreur : fonction V");
    if (V(numero_interne, MUTEX_DEPOT, 1) == -1)
        perror("Erreur : fonction V");
    return mes;
}

void producteur() {
    struct message *mes = malloc(sizeof(struct message));
    if (mes == NULL) {
        fprintf(stderr, "ERREUR : allocation mémoire\n");
        exit(-1);
    }
    // Initialisation de la fonction rand()
    srand(time(NULL) ^ (getpid()<<(sizeof(pid_t)*4)));
    // Création du message aléatoirement
    mes->type = (unsigned int) rand()%2;
    mes->contient = tab_type[mes->type][(int) rand()%2];
    mes->createur = getpid();
    if (VERBOSE)
        printf("Prod %d\t | type = %d\t | contient = %s\n",
            mes->createur, mes->type, mes->contient);
    deposer(mes);
    free(mes);
}

void consommateur() {
    struct message *mes = retirer();
    if (VERBOSE)
        printf("Conso %d\t | type = %d\t | contient = %s\t | créateur = %d\n",
            getpid(), mes->type, mes->contient, mes->createur);
    free(mes);
}



void initialisation() {
    // Création d'une clé IPC System
    numero_externe = ftok("exo3.c", ID);
    // Création des sémaphores
    numero_interne = creerSem((key_t) numero_externe, 4, 0600);
    if (numero_interne == -1) {
        perror("ERREUR : creerSem()");
        exit(-1);
    }
    if (initialiserSem(numero_interne, MUTEX_DEPOT, N) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du MUTEX_DEPOT");
        exit(-2);
    }
    if (initialiserSem(numero_interne, MUTEX_RETRAIT, 0) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du MUTEX_RETRAIT");
        exit(-2);
    }
    if (initialiserSem(numero_interne, MUTEX_PRODUCTEUR, 1) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du MUTEX_PRODUCTEUR");
        exit(-2);
    }
    if (initialiserSem(numero_interne, MUTEX_CONSOMMATEUR, 1) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du MUTEX_CONSOMMATEUR");
        exit(-2);
    }
    m = malloc(sizeof(struct memoire_partagee));
    m->ICV = 0;
    m->ICP = 0;
}



int main(int argc, char *argv[]) {
    int i;
    int f;
    initialisation();

    int ni = shmget(numero_externe, sizeof(struct memoire_partagee), IPC_EXCL|IPC_CREAT|0600);

    // Attachement de la mémoire
    m = shmat(ni, 0, 0);
    // Création des producteurs
    for (i = 0; i < NB_PRODUCTEUR; i++) {
        f = fork();
        switch (f) {
            case -1:
                fprintf(stderr, "ERREUR: fork()\n");
                break;
            case 0:
                producteur();
                shmdt(m);
                exit(0);
            default:
                break;
        }
    }

    // Création des consommateurs
    for (i = 0; i < NB_CONSOMMATEUR; i++) {
        f = fork();
        switch (f) {
            case -1:
                fprintf(stderr, "ERREUR: fork()\n");
                break;
            case 0:
                consommateur();
                shmdt(m);
                exit(0);
            default:
                break;
        }
    }

    // sinon faut créer un tableau avec tous les fork() sauvegardés
    while(wait(NULL) != -1);
    //waitpid(f, NULL, 0);
    detruireSem(numero_interne);
    // Detruire segment mémoire partagée
    shmctl(ni, IPC_RMID, 0);

    return 0;

}
