/*******************************************
* VR486033
* ANDREA RIGON
* 18 / 05 / 2024
********************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include "semaphore.h"
#include "errExit.h"
#define DIM 3

struct SharedMemory
{
    char table[DIM][DIM];
    char nameP1[50];
    char nameP2[50];
    char symbol1;
    char symbol2;
    int victory;
    int choice;
    int whoWin1;
    int whoWin2;
    char esito[50];
    char esito2[50];
    int block;
    int timeout;
    pid_t serv;
    pid_t proc1;
    pid_t proc2;
    int cop;
    int bot;
};

// function to print the semaphore set's state
void printSemaphoresValue (int semid) {
    unsigned short semVal[3];
    union semun arg;
    arg.array = semVal;

    // get the current state of the set
    if (semctl(semid, 0 /*ignored*/, GETALL, arg) == -1)
        errExit("semctl GETALL failed");

    // print the semaphore's value
    printf("semaphore set state:\n");
    for (int i = 0; i < 3; i++)
        printf("id: %d --> %d\n", i, semVal[i]);
}

int win (char board[DIM][DIM],char symbol)
{
    for (int i = 0; i < DIM; i++) {
        if (board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) {
            return 1;
        }
    }

    // Check columns
    for (int i = 0; i < DIM; i++) {
        if (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol) {
            return 1;
        }
    }

    // Check diagonals
    if (board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) {
        return 1;
    }
    if (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol) {
        return 1;
    }

    return 0;
}

int draw (char board[DIM][DIM])
{
    int cont=0;

    for(int i=0;i<DIM;i++)
    {
        for(int j=0;j<DIM;j++)
        {
          if (board[i][j]!=' ')
          {
            cont++;
          }
        }
    }

    if (cont == (DIM*DIM))
    {
       return 1;
    }
    return 0;
}

int sig_count = 0;
int proc1Appoggio;
int proc2Appoggio;
int appoId;
int appoMem;

void clean ()
{
    kill(proc1Appoggio,SIGQUIT);
    kill(proc2Appoggio,SIGQUIT);

    if (semctl(appoId, 0 , IPC_RMID, NULL) == -1)
        errExit("semctl IPC_RMID failed");
    shmctl((appoMem),IPC_RMID,NULL);  
    exit(0);
}

void intHandling (int sig)
{
     if (sig_count == 0)
     {
        write(1,"Pressing ctrl-c twice within 5 seconds to end the game",54);
        sig_count++;
        sleep(5);
     }
     else
     {
       write(1,"\nGame ended\n",12);
       clean();
     }
}

int main (int argc, char *argv[]) {
    int cont = 0;

    if (argc != 4 || (strcmp(argv[2],argv[3]) == 0))
    {
        printf("It is necessary to insert timeout , X , O in this form\n");
        return 0;
    }
    
    if (signal(SIGINT,intHandling)==SIG_ERR)
        printf("Error handling server\n");
 

    key_t chiave = ftok("src/Server.txt",'B');
    if (chiave<0)
        errExit("Chiave errata server");

    int shmid = shmget(chiave,sizeof(struct SharedMemory),IPC_CREAT | 0666);
    struct SharedMemory *sharedMem = (struct SharedMemory *)shmat(shmid,NULL,0);   
    
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
        {
            sharedMem->table[i][j]='b';
        }
    }
    
    key_t key = ftok("src/Semafori.txt",'A');

    if (key == -1)
        errExit("ftok failed");
    
    int semid = semget(key, 3, IPC_CREAT | S_IRUSR | S_IWUSR);

    appoId = semid;
    appoMem = shmid;
    if (semid == -1)
        errExit("semget failed");
    
    // Initialize the semaphore set
    unsigned short semInitVal[] = {0, 0, 0};
    union semun arg;
    arg.array = semInitVal;

    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("semctl SETALL failed");

    sharedMem->symbol1 = *argv[2];
    sharedMem->symbol2 = *argv[3];
    sharedMem->victory = 0;
    sharedMem->timeout = atoi(argv[1]);
    sharedMem->serv = getpid();
    sharedMem->bot=0;
    for (int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
        {
            sharedMem->table[i][j] = ' ';
        }
    }

    printf("\nWaiting for the clients\n");
    semOp(semid,(unsigned short) 0 ,-2);
    printf("\nLet's start\n");
    
    if (sharedMem->cop == 1)
    {
        write(1,"\nBot on\n",8);
    }
    proc1Appoggio = sharedMem->proc1;
    proc2Appoggio = sharedMem->proc2;
    

   if (sharedMem->cop == 0)
   {
    while(sharedMem->victory!=1 && sharedMem->block!=1)
    {        
           semOp(semid ,(unsigned short) 2 , 1);    //sblocco player 1
           semOp(semid, (unsigned short) 0 , -1);   //blocco server 
                                                  //gioca player 1 che sblocca il server 

           //controllo vittoria
           if(win(sharedMem->table,sharedMem->symbol2) == 1 && cont == 0)
           {
             sharedMem->whoWin2 = 2;
             strcpy(sharedMem->esito,"\nPlayer 2 won PLAYER1 lost\n");
             cont++;
           }
           if(win(sharedMem->table,sharedMem->symbol1) == 1 && cont == 0)
           { 
             sharedMem->whoWin1 = 1;
             strcpy(sharedMem->esito,"\nPlayer 1 won PLAYER2 lost\n");
             cont++;
           }
        if(draw(sharedMem->table)==1 && win(sharedMem->table,sharedMem->symbol1) == 0 && win(sharedMem->table,sharedMem->symbol2) == 0 && cont==0)
         {
            strcpy(sharedMem->esito,"\nDrae\n");
            cont++;
         }
          
          sharedMem->victory=draw(sharedMem->table);
        
    
           semOp(semid , (unsigned short) 1 , 2);   //sblocco player 2
           semOp(semid ,(unsigned short) 0 , -1 ); //blocco server
                                                  //gioca player 2
           //controllo vittoria 

         if(win(sharedMem->table,sharedMem->symbol1) == 1 && cont == 0)
           { 
             sharedMem->whoWin1 = 1;
             strcpy(sharedMem->esito,"\nPlayer 1 won PLAYER2 lost\n");
             cont++;
           }
          if(win(sharedMem->table,sharedMem->symbol2) == 1 && cont == 0)
           { 
             sharedMem->whoWin2 = 2;
             strcpy(sharedMem->esito,"\nPlayer 2 won PLAYER1 lost\n");
             cont++;
           }  
         if(draw(sharedMem->table)==1 && win(sharedMem->table,sharedMem->symbol1) == 0 && win(sharedMem->table,sharedMem->symbol2) == 0 && cont==0)
         {
            strcpy(sharedMem->esito,"\nDraw\n");
            cont++;
         }
        sharedMem->victory=draw(sharedMem->table);   

    }
   }else
   {
      pid_t pid;

      pid = fork();

      if(pid < 0)
      {
        errExit("\nErrore nella fork\n");
      }else if (pid == 0)
      {  
         sharedMem->bot = 1;
         execl("TriClient","TriClient","bot",(char *) NULL);
      }else
      {
        while(sharedMem->victory!=1 && sharedMem->block!=1)
        {        
           semOp(semid ,(unsigned short) 2 , 1);    //sblocco player 1
           semOp(semid, (unsigned short) 0 , -1);   //blocco server 
                                                  //gioca player 1 che sblocca il server 

           //controllo vittoria
           if(win(sharedMem->table,sharedMem->symbol2) == 1 && cont == 0)
           {
             sharedMem->whoWin2 = 2;
             strcpy(sharedMem->esito,"\nPlayer 2 won PLAYER1 lost\n");
             cont++;
           }
           if(win(sharedMem->table,sharedMem->symbol1) == 1 && cont == 0)
           { 
             sharedMem->whoWin1 = 1;
             strcpy(sharedMem->esito,"\nPlayer 1 won PLAYER2 lost\n");
             cont++;
           }
        if(draw(sharedMem->table)==1 && win(sharedMem->table,sharedMem->symbol1) == 0 && win(sharedMem->table,sharedMem->symbol2) == 0 && cont==0)
         {
            strcpy(sharedMem->esito,"\nDraw\n");
            cont++;
         }
          
          sharedMem->victory=draw(sharedMem->table);
        
    
           semOp(semid , (unsigned short) 1 , 2);   //sblocco player 2
           semOp(semid ,(unsigned short) 0 , -1 ); //blocco server
                                                  //gioca player 2
           //controllo vittoria 

         if(win(sharedMem->table,sharedMem->symbol1) == 1 && cont == 0)
           { 
             sharedMem->whoWin1 = 1;
             strcpy(sharedMem->esito,"\nPlayer 1 won  PLAYER2 lost\n");
             cont++;
           }
          if(win(sharedMem->table,sharedMem->symbol2) == 1 && cont == 0)
           { 
             sharedMem->whoWin2 = 2;
             strcpy(sharedMem->esito,"\nPlayer 2 won PLAYER1 lost\n");
             cont++;
           }  
         if(draw(sharedMem->table)==1 && win(sharedMem->table,sharedMem->symbol1) == 0 && win(sharedMem->table,sharedMem->symbol2) == 0 && cont==0)
         {
            strcpy(sharedMem->esito,"\nDraw\n");
            cont++;
         }
        sharedMem->victory=draw(sharedMem->table);   

        }
      }
   }
    
    if  (sharedMem->block == 1)
    {
        write(1,"\nOne player left so the game ends\n",34);
    }

    semOp(semid ,(unsigned short) 2 , 1); 
    semOp(semid, (unsigned short) 0 , -1);
       // remove the created semaphore set
    
    kill(sharedMem->proc1,SIGTERM);

    if (semctl(semid, 0 , IPC_RMID, NULL) == -1)
        errExit("semctl IPC_RMID failed");
    
    
    shmdt((void *)sharedMem);
    shmctl((shmid),IPC_RMID,NULL);    

    return 0;
}
