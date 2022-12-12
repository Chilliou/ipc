#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <time.h>



#define NB_FORK 2
#define TABSIZE 1000000
#define NB_RELANCE 5
#define RAND_PAR_RELANCE 100000000  //On ne pouvait pas faire 1 milliard par 1 milliard le pc kill le processus     
                                    // Par conséquant on relance 100 fois le 100 millions pour faire 100 milliard de random


struct donnees {
    int tabAlea[TABSIZE];
};


int main (int argc, char **argv){
    key_t shmkey;                 
    int shmid;                   
    sem_t *sem;                   
    pid_t pid;                    
    struct donnees *commun;                      
    srand((int)time(NULL));
    int reponse;

    shmkey = ftok(getenv("HOME"), 'A');     

    shmid = shmget (shmkey,sizeof(struct donnees), 0644 | IPC_CREAT);
    if (shmid < 0){                           
        perror ("shmget\n");
        exit (1);
    }

    commun = (struct donnees *) shmat (shmid, NULL, 0);   
    sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 25); //valeur du semmaphore je sais pas a quoi ça sert

    for (int i = 0; i < NB_FORK; i++)
    {
        pid = fork ();
        if (pid < 0)  // Si il y a une erreur
        { 
            sem_unlink ("pSem");   
            sem_close(sem);   
            printf ("Fork error.\n");
        }
        else if (pid == 0)
            break;             
    }


    /******************************************************/
    /******************   PARENT           ****************/
    /******************************************************/
    if (pid != 0){
        /* wait for all children to exit */
        while (pid = waitpid (-1, NULL, 0)){
            if (errno == ECHILD)
                break;
        }


    double moy_nbTab = 1000;  // Calculer à la main puisque sinon on overflow avec le calcul au dessus 
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

    shmdt (commun);
    shmctl (shmid, IPC_RMID, 0);
    printf("detachement du semaphores fait\n");


    /* cleanup semaphores */
    sem_unlink ("pSem");   
    sem_close(sem); 
    printf("nettoyage du semaphore fait\n"); 

    exit (EXIT_SUCCESS);
}

    /******************************************************/
    /******************   CHILD           *****************/
    /******************************************************/
    else{
        pid_t fid = getpid();
        printf("%d : Start \n",fid);

        for(int z=1;z<=NB_RELANCE;z++)
        {
            int *array_n;
            array_n = malloc((size_t)RAND_PAR_RELANCE * sizeof(int));

            if(!array_n)
            {
                fputs("SegFault\n",stderr);
                return 1;
            }

            printf("%d : Start Rand Number: %d \n",fid,z);

            int i=0;
            while(i<RAND_PAR_RELANCE) 
            {

                array_n[i] = rand();
                i++;
            }
            printf("%d : End Rand Number: %d \n",fid,z);

            sem_wait (sem);

            printf("%d : Start Tab Filling and Mutex Lock Number: %d \n",fid,z);

            i=0;
            while(i<RAND_PAR_RELANCE) 
            {
                
                double produit = (double)array_n[i]/(double)RAND_MAX;
                commun->tabAlea[(int)(produit*TABSIZE)]++; 
                i++;
            }

            sem_post (sem);

            printf("%d : End Tab Filling and Mutex UnLock Number: %d \n",fid,z);

            free(array_n);  

        }
           
        printf("%d : End \n",fid);

        exit (EXIT_SUCCESS);
    }
}