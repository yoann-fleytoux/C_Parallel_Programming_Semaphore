#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>           // Fonction rand()

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "../include/semaphore.h"

/*
    Gestion de ressources : Modèle des producteurs/consommateurs
    Autre variante
    On crée ici un sémaphore pour les messages de type 0 et un sémaphore pour
    les messages de type 1. L'idée est d'ajouter un jeton dans le sémaphore
    correspondant au type du message. Le consommateur consomme dans l'ordre de
    la table gérée circulaire. Chaque consommateur va essayer de consommer un
    jeton dans le sémaphore correspondant au type voulu, si cela fonctionne, il
    va récupérer le message et ajouter un jeton au sémaphore correpondant au
    type de message suivant dans la table.
*/
#define VERBOSE                 1
#define ID                      2
#define N                       4   //tampon de N cases gérées circulairement
#define MUTEX_DEPOT             0
#define MUTEX_RETRAIT           1
#define MUTEX_PRODUCTEUR        2
#define MUTEX_MSG_TYPE_0        3   // Sémaphore pour les messages de type 0
#define MUTEX_MSG_TYPE_1        4   // Sémaphore pour les messages de type 1
#define NB_MESSAGE_PRODUCTEUR   4
#define NB_MESSAGE_CONSOMMATEUR 4

// Variables globales
key_t numero_externe;
int numero_interne;
pid_t pid_pere;


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

void sigintHandler(int sig_num) {
    signal(SIGINT, sigintHandler);
    if (getpid() != pid_pere) {
        // Détachement de la mémoire
        shmdt(m);
        exit(-1);
    }
}

void deposer(struct message *mes) {
    if (P(numero_interne, MUTEX_DEPOT, 1) == -1)
        perror("Erreur : fonction P");
    if (P(numero_interne, MUTEX_PRODUCTEUR, 1) == -1)
        perror("Erreur : fonction P");
    if (m->ICV == m->ICP) {
        // On fait m->type+3 car
        //      si c'est un message de type 0 -> MUTEX_MSG_TYPE_0 = 3
        //      si c'est un message de type 1 -> MUTEX_MSG_TYPE_1 = 4
        // On choisit le bon sémaphore sur lequel on veut ajouter un jeton
        if (V(numero_interne, mes->type+3, 1) == -1)
            perror("Erreur : fonction V");
    }
    if (VERBOSE)
        printf("Prod %d\t | type = %d\t | contient = %s\n",
            mes->createur, mes->type, mes->contient);
    copy_message(mes, &(m->BUF[m->ICV]));
    m->ICV = (m->ICV+1)%N;
    if (V(numero_interne, MUTEX_PRODUCTEUR, 1) == -1)
        perror("Erreur : fonction V");
    if (V(numero_interne, MUTEX_RETRAIT, 1) == -1)
        perror("Erreur : fonction V");
}


struct message* retirer(unsigned int type) {
    struct message *mes;
    if (P(numero_interne, MUTEX_RETRAIT, 1) == -1)
        perror("Erreur : fonction P");
    if (P(numero_interne, (type+3), 1) == -1)
        perror("Erreur : fonction P");
    mes = malloc(sizeof(struct message));
    if (mes == NULL) {
        fprintf(stderr, "ERREUR : allocation mémoire\n");
        exit(-1);
    }
    copy_message(&(m->BUF[m->ICP]), mes);
    if (VERBOSE)
        printf("Conso %d\t | type = %d\t | contient = %s\t | créateur = %d\n",
            getpid(), mes->type, mes->contient, mes->createur);
    m->ICP = (m->ICP+1)%N;
    // Ajout d'un jeton suivant le type de la case
    if (V(numero_interne, (m->BUF[m->ICP].type)+3, 1) == -1)
        perror("Erreur : fonction V");
    if (V(numero_interne, MUTEX_DEPOT, 1) == -1)
        perror("Erreur : fonction V");
    return mes;
}


/*
 * Création des producteurs qui vont les créer des messages et les déposer
 * dans le buffer.
 */
void producteur() {
    struct message *mes = malloc(sizeof(struct message));
    if (mes == NULL) {
        fprintf(stderr, "ERREUR : allocation mémoire\n");
        exit(-1);
    }
    // Initialisation de la fonction rand()
    srand(time(NULL) ^ (getpid()<<sizeof(int)));
    // Création du message aléatoirement
    // mes->type = (unsigned int) rand()%2;
    for (unsigned int i = 0; i < NB_MESSAGE_PRODUCTEUR; i++) {
        mes->type = i%2;
        mes->contient = tab_type[mes->type][rand()%2];
        mes->createur = getpid();
        deposer(mes);
    }
    free(mes);
}


/*
 * Création des consommateurs qui vont retirer des messages du buffer
 * et les lire.
 */
void consommateur() {
    struct message *mes;
    for (unsigned int i = 0; i < NB_MESSAGE_CONSOMMATEUR; i++) {
        mes = retirer(i%2);
        free(mes);
    }
}


void initialisation() {
    // Création d'une clé IPC System
    numero_externe = ftok("exo4.c", ID);
    // Création des sémaphores
    numero_interne = creerSem((key_t) numero_externe, 5, 0600);
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
    // Initialisation de MUTEX_MSG_TYPE_0 à 0 car 0 message de type 0 en attente
    if (initialiserSem(numero_interne, MUTEX_MSG_TYPE_0, 0) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du type 0");
        exit(-2);
    }
    // Initialisation de MUTEX_MSG_TYPE_1 à 0 car 0 message de type 1 en attente
    if (initialiserSem(numero_interne, MUTEX_MSG_TYPE_1, 0) == -1) {
        detruireSem(numero_interne);
        perror("ERREUR : initialiserSem() du type 1");
        exit(-2);
    }
    m = malloc(sizeof(struct memoire_partagee));
    m->ICV = 0;
    m->ICP = 0;
}



int main(int argc, char *argv[]) {
    int i;
    int f;

    if (argc != 3){
		fprintf(stderr, "Usage: %s nb_producteur nb_consommateur\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nb_producteur = atoi(argv[1]);
    int nb_consommateur = atoi(argv[2]);


    initialisation();

    int ni = shmget(numero_externe, sizeof(struct memoire_partagee), IPC_EXCL|IPC_CREAT|0600);

    // Attachement de la mémoire
    m = shmat(ni, 0, 0);

    // Récupération du pid du père
    pid_pere = getpid();

    // Récupération du signal ctrl+c
    signal(SIGINT, sigintHandler);

    // Création des producteurs
    for (i = 0; i < nb_producteur; i++) {
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
    for (i = 0; i < nb_consommateur; i++) {
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

    // sinon faut créer un tableau avec tous les fork() sauvegardé
    while(wait(NULL) != -1);
    // Détachement de la mémoire
    shmdt(m);
    //waitpid(-1, NULL, 0);
    detruireSem(numero_interne);
    // Detruire segment mémoire partagée
    shmctl(ni, IPC_RMID, 0);

    return 0;

}
