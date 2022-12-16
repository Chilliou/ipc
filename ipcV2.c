//
// Created by ld201024 on 22/10/22.
//
/* ipc.c */
/*
   Ce programme vérifie l'équilibrage de la fonction rand 
   https://github.com/Chilliou/ipc
*/

#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <time.h>
#include <string.h>



#define NB_FORK 2
#define TABSIZE 1000000
#define RAND_PAR_RELANCE 1000000000  //Ne pas prendre une trop grosse valeur sinon ça bug

key_t shmkey;                 
int shmid;                   
sem_t *sem;                   
pid_t pid;                    
struct donnees *commun;     


struct donnees {
    int tabAlea[TABSIZE];
};

void generateChilds()
{
    // Génération des processus fils
    for(int i = 0; i < NB_FORK; ++i ) {
        // Si on est dans un processus fils
        if( !fork() ) 
        {
            pid_t fid = getpid();         //Surement inutile mais sans le redéfinir j'avais 0

            srand((int)time(NULL)); // Au cas où

            int *array_n;
            array_n = malloc((size_t)TABSIZE * sizeof(int));

            if(!array_n)
            {
                fputs("SegFault\n",stderr);
                exit(EXIT_FAILURE);
            }


            for( int i = 0; i < RAND_PAR_RELANCE; ++i )
            {
                array_n[rand() / (RAND_MAX / TABSIZE)]++;
            }

            sem_wait (sem);  //Verrouiller un sémaphore 

            for( int i = 0; i < RAND_PAR_RELANCE; ++i )
            {
                commun->tabAlea[i] += array_n[i];
            }


            sem_post (sem); //Déverrouiller un sémaphore  

            free(array_n);      
            
            exit (EXIT_SUCCESS);
            
        }
    }

}

void caculateStat()
{

    int verif = 0;
    int endroidDepassement = 0;

    for(int i=0;i<TABSIZE;i++)
    {

        double total = (double) NB_FORK * (double)RAND_PAR_RELANCE;   //Changement de la méthode de calcul
        double val = commun->tabAlea[i] / total;

        if( val > 0.05 ) {
            verif = 1;
            printf("%d pourcentage depasser %2f% ! : %i ~= %.2f%\n", i, 0.05*(double)100, commun->tabAlea[i], val*(double)100);
            endroidDepassement = i;
        }

        
    }
    if(!verif)
        printf("Le pourcentage est rester normal\n");
    else
        printf("Le pourcentage est dépaser a la case %d avec la valeur \n",endroidDepassement,commun->tabAlea[i]);
    }


int main (){
                     
    srand((int)time(NULL));

    shmkey = ftok(getenv("HOME"), 'A');     //Convertir un nom de fichier et un identificateur de projet en clé IPC System V  

    shmid = shmget (shmkey,sizeof(struct donnees), 0644 | IPC_CREAT);  // Allouer un segment de mémoire partagée  
    if (shmid < 0)
    {                           
        perror ("shmget\n");
        exit (1);
    }

    commun = (struct donnees *) shmat (shmid, NULL, 0);  //attache le segment de mémoire partagée
    sem = sem_open ("pSem", O_CREAT | O_EXCL, 0644, 25); //Initialiser et ouvrir un sémaphore nommé  
                                                         //O_CREAT est utlisé donc mode et value sont ignorés
        


    generateChilds();

    while (pid = waitpid (-1, NULL, 0)){
            if (errno == ECHILD)
                break;
        }


    caculateStat();

    /* shared memory detach */
    shmdt (commun);
    shmctl (shmid, IPC_RMID, 0);

    /* cleanup semaphores */
    sem_unlink ("pSem");   
    sem_close(sem);  
    
    
    exit(EXIT_SUCCESS);
        
}   
