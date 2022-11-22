//
// Created by ld201024 on 22/11/22.
//
/* prod.c */
/*
   Ce programme lit une suite de nombres, et effectue le cumul dans une
   variable en mémoire partagée.
*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include<pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
void abandon(char message[]){
    perror(message);
    exit(EXIT_FAILURE);
}
struct donnees {
    int nb;
    int total;
};
int main(void)
{
    pthread_mutex_t lock;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init failed\n");
        return 1;
    }


    key_t cle;
    int id;
    struct donnees *commun;
    int reponse;
    cle = ftok(getenv("HOME"), 'A');
    if (cle == -1)
        abandon("ftok");
    id = shmget(cle, sizeof(struct donnees),
                IPC_CREAT | IPC_EXCL | 0666);
    if (id == -1) {
        switch (errno) {
            case EEXIST:
                printf("existeeeee");
                id = shmget(cle, sizeof(struct donnees), 0);
                break;
            default:
                abandon("shmget");
        }
    }
    commun = (struct donnees *) shmat(id, NULL, SHM_R | SHM_W);
    if (commun == NULL)
        abandon("shmat");
    commun->nb = 0;
    commun->total = 0;

    while (1) {
        printf("+ ");
        if (scanf("%d", &reponse) != 1)
            break;
        //ici lock
        pthread_mutex_lock(&lock);
        commun->nb++;
        commun->total += reponse;
        pthread_mutex_unlock(&lock);
        // ici unlock
        printf("sous-total %d= %d\n", commun->nb, commun->total);
    }
    printf("---\n");

    if (shmdt((char *) commun) == -1)
        abandon("shmdt");
    /* suppression segment */
    if (shmctl(id, IPC_RMID, NULL) == -1)
        abandon("shmctl(remove)");

    pthread_mutex_destroy(&lock);

    return EXIT_SUCCESS;
}