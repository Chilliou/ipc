//
// Created by ld201024 on 22/10/22.
//
/* ipc.c */
/*
   Ce programme vérifie l'équilibrage de la fonction rand 
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

#define NB_FORK 20
#define TABSIZE 1000000
#define NB_RELANCE 10
#define RAND_PAR_RELANCE 100000000  //On ne pouvait pas faire 1 milliard par 1 milliard le pc kill le processus 	
									// Par conséquant on relance 100 fois le 100 millions pour faire 100 milliard de random

pthread_mutex_t lock;

void * testRand ();

void abandon(char message[]){
    perror(message);
    exit(EXIT_FAILURE);
}
struct donnees {
    int tabAlea[TABSIZE];
};

int main()
{

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
                id = shmget(cle, sizeof(struct donnees), 0);
                break;
            default:
                abandon("shmget");
        }
    }

    commun = (struct donnees *) shmat(id, NULL, SHM_R | SHM_W);
    if (commun == NULL)
        abandon("shmat");

    pid_t child_pid, wpid;
    int status = 0;
    for (int id=0; id<NB_FORK; id++) {
        if ((child_pid = fork()) == 0) {
            testRand();
            exit(0);
        }
    }

    while ((wpid = wait(&status)) > 0);


    /////////////////////////////////
    // Calcul du pourcentage       //
    /////////////////////////////////

    //double  moy_nbTab = NB_RELANCE*RAND_PAR_RELANCE*NB_FORK/(sizeof(commun->tabAlea)/sizeof(commun->tabAlea[0]));

    double moy_nbTab = 20000;  // Calculer à la main puisque sinon on overflow avec le calcul au dessus 
    double maxEqui = 0.0;
    printf("la case jsp contient %d\n", commun->tabAlea[1542]);
    printf("la case jsp contient %d\n", commun->tabAlea[64545]);
    printf("la case jsp contient %d\n", commun->tabAlea[5446]);
    printf("la case jsp contient %d\n", commun->tabAlea[15443]);

    for(int i=0;i<TABSIZE;i++)
    {
    	double temp;
    	double tabI = commun->tabAlea[i];

    	if(tabI<moy_nbTab)
    	{
    		temp = (100.0*(moy_nbTab-tabI))/(moy_nbTab+100.0);
    	}
    	else
    	{
    		temp = (100.0*tabI)/moy_nbTab;
    	}
    	if(maxEqui<temp) maxEqui = temp;
    }
    maxEqui -=100;
    printf("Balancing of function rand is : %lf % \n", maxEqui);

    ///////////////////////////////////

    if (shmdt((char *) commun) == -1)
        abandon("shmdt");
    if (shmctl(id, IPC_RMID, NULL) == -1)
        abandon("shmctl(remove)");

    pthread_mutex_destroy(&lock);

    


    return EXIT_SUCCESS;
}

void * testRand () {

    key_t cle;
    int id;
    struct donnees *commun;
    pid_t tid = getpid();

    cle = ftok(getenv("HOME"), 'A');
    if (cle == -1)
        abandon("ftok");
	
    id = shmget(cle, sizeof(struct donnees), 0);
    if (id == -1) {
        switch (errno) {
            case ENOENT:
                abandon("pas de segment\n");
            default:
                abandon("shmget");
        }
    }
	
    commun = (struct donnees *) shmat(id, NULL, SHM_R | SHM_W);

    printf("%d : Start \n",tid);

    for(int z=1;z<=NB_RELANCE;z++)
    {
    	int *array_n;
    	array_n = malloc((size_t)RAND_PAR_RELANCE * sizeof(int));

    	if(!array_n)
    	{
    		fputs("SegFault\n",stderr);
    		return 1;
    	}

    	printf("%d : Start Rand Number: %d \n",tid,z);

    	int i=0;
    	while(i<RAND_PAR_RELANCE) 
    	{

    		array_n[i] = rand();
    		i++;
    	}
    	printf("%d : End Rand Number: %d \n",tid,z);

    	pthread_mutex_lock(&lock);

    	printf("%d : Start Tab Filling and Mutex Lock Number: %d \n",tid,z);

    	i=0;
    	while(i<RAND_PAR_RELANCE) 
    	{
    		
    		double produit = (double)array_n[i]/(double)RAND_MAX;
    		commun->tabAlea[(int)(produit*TABSIZE)]++; 
    		i++;
    	}

    	pthread_mutex_unlock(&lock);

    	printf("%d : End Tab Filling and Mutex UnLock Number: %d \n",tid,z);

    	free(array_n);	

    }
       
    printf("%d : End \n",tid);
}
