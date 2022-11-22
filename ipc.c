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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include<pthread.h>
pthread_mutex_t lock;

void * testRand (void * arg);

void abandon(char message[]){
    perror(message);
    exit(EXIT_FAILURE);
}
struct donnees {
    int nb;
    int total;
};
int main()
{
    void * x1;
    pthread_t th1, th2;

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

    srand((int)time(NULL));
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

    sleep(5);

    pthread_create(&th1, NULL, testRand, NULL);
    pthread_create(&th2, NULL, testRand, NULL);

    pthread_join(th1, &x1);
    pthread_join(th2, &x1);



    if (shmdt((char *) commun) == -1)
        abandon("shmdt");
    /* suppression segment */
    if (shmctl(id, IPC_RMID, NULL) == -1)
        abandon("shmctl(remove)");

    pthread_mutex_destroy(&lock);

    return EXIT_SUCCESS;
}

void * testRand (void * arg) {

    key_t cle;
    int id;
    struct donnees *commun;

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

    int i=0;
    while (1) {
        printf("fin du sleep");

        printf("+ ");
        int randNum = rand()%100;

        /*
        if (scanf("%d", &reponse) != 1)
            break;
        */
        pthread_mutex_lock(&lock);
        commun->nb++;
        commun->total += randNum; //response
        printf("sRand num %d : %d\n", commun->nb, randNum);
        printf("sous-total %d= %d\n", commun->nb, commun->total);
        pthread_mutex_unlock(&lock);
        i++;
        if(i> 100)
            break;
    }
    printf("---\n");
}