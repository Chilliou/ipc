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
    int i=0;
    while (1) {

        printf("+ ");
        int randNum = rand()%100;

        /*
        if (scanf("%d", &reponse) != 1)
            break;
        */
        commun->nb++;
        commun->total += randNum; //response
        printf("sRand num %d : %d\n", commun->nb, randNum);
        printf("sous-total %d= %d\n", commun->nb, commun->total);
        i++;
        if(i> 100)
            break;
    }
    printf("---\n");

    if (shmdt((char *) commun) == -1)
        abandon("shmdt");
    /* suppression segment */
    if (shmctl(id, IPC_RMID, NULL) == -1)
        abandon("shmctl(remove)");
    return EXIT_SUCCESS;
}