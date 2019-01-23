#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


/**
 * Fonction de création de sémaphore
 *
 * @param <NE>      input : le numéro externe (peut être créé par la fonction
 *                          ftok())
 * @param <nbSem>   input : le nombre de sémpahores que l'on veut créer
 * @param <droits>  input : les droits d'accés au(x) sémaphore(s)
 *
 * @return  l'identifiant d'un ensemble de sémaphores (numéro interne)
 *          -1 si erreur
 */
int creerSem(key_t NE, int nbSem, int droits);

/**
 * Fonction d'ouverture d'un sémaphore déjà existant via son numéro externe
 *
 * @param <NE>      input : le numéro externe
 *
 * @return  l'identifiant d'un ensemble de sémaphores (numéro interne)
 *          -1 si erreur
 */
int ouvrirSem(key_t NE);

/**
 * Fonction initialisant un sémaphore
 *
 * @param <nEns>    input : l'identifiant d'un ensemble de sémaphores
 *                          (numéro interne)
 * @param <nSem>    input : le numéro du sémaphore à initialiser
 * @param <value>   input : la valeur d'initialisation à donner à <nSem>
 *
 * @return  0 en cas de réussite
 *          -1 si erreur
 */
int initialiserSem(int nEns, int nSem, int value);

/**
 * Fonction détruisant un sémaphore
 *
 * @param <nEns>    input : l'identifiant d'un ensemble de sémaphores
 *                          (numéro interne)
 *
 * @return  0 en cas de réussite
 *          -1 si erreur
 */
int detruireSem(int nEns);

/**
 * Si la valeur du sémaphore est positive alors on décrémente la valeur du
 * sémaphore sinon (valeur nulle) on bloque le processus dans une file d'attente
 *
 * @param <nEns>        input : l'identifiant d'un ensemble de sémaphores
 *                              (numéro interne)
 * @param <nSem>        input : le numéro du sémaphore à regarder
 * @param <nbJetons>    input : le nombre de jetons à décrémenter
 *
 * @return  0 en cas de réussite
 *          -1 si erreur
 */
int P(int nEns, int nSem, int nbJetons);

/**
 * Si des processus sont en attente, alors réveiller le premier processus sinon
 * on incrémente la valeur du sémaphore
 *
 * @param <nEns>        input : l'identifiant d'un ensemble de sémaphores
 *                              (numéro interne)
 * @param <nSem>        input : le numéro du sémaphore à regarder
 * @param <nbJetons>    input : le nombre de jetons à incrémenter
 *
 * @return  0 en cas de réussite
 *          -1 si erreur
 */
int V(int nEns, int nSem, int nbJetons);

#endif
