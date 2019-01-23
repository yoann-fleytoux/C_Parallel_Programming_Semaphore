#include "../include/semaphore.h"

// Définit dans la fonction semctl
union semun {
    int     val;            /* value for SETVAL */
    struct  semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;         /* array for GETALL & SETALL */
};

/*
struct sembuf {
    short sem_num;
    short sem_op;
    short sem_flg;
};
*/
int creerSem(key_t NE, int nbSem, int droits) {
    return semget(NE, nbSem, IPC_CREAT|IPC_EXCL|droits);
}

int ouvrirSem(key_t NE) {
    return semget(NE, 0, 0);
}

int initialiserSem(int nEns, int nSem, int value) {
    union semun valSemun;
    valSemun.val = value;
    return semctl(nEns, nSem, SETVAL, valSemun);
}

int detruireSem(int nEns) {
    return semctl(nEns, 0, IPC_RMID, 0);
}

int P(int nEns, int nSem, int nbJetons) {
    struct sembuf buf[1];
    buf[0].sem_num = nSem;
    buf[0].sem_op = -nbJetons;
    buf[0].sem_flg = 0; // Opération bloquante sinon IPC_NOWAIT -> non bloquant
    // nb_operations = taille du tableau, ici = 1
    return semop(nEns, buf, 1);
}

int V(int nEns, int nSem, int nbJetons) {
    return P(nEns, nSem, -nbJetons);
}

/*
int V(int nEns, int nSem, int nbJetons) {
    struct sembuf buf[1];
    buf[0].sem_num = nSem;
    buf[0].sem_op = nbJetons;
    buf[0].sem_flg = 0;
    return semop(nEns, buf, 1);
}
*/
