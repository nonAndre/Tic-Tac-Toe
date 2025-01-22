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
#include <fcntl.h>
#include <signal.h>
#include <string.h>
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

void sigHandler (int sig)
{
  printf("\nYou left the game\n");
  exit(0);
}

void termHand (int sig)
{
   printf("\nA player left the game so you win\n");
   exit(0);
}

void killHand(int sig)
{
  printf("\nGame ended by the server\n");
  exit(0);
}

void stopGame (int cod , int semCod)
{
    semOp(semCod,(unsigned short ) 0 ,1);
}

int servPid;
int appoId2;
int appoMem2;

void alarmHand(int sig)
{
   write(1,"\nTime expired\n",14);
   shmctl((appoMem2),IPC_RMID,NULL); 
   semctl(appoId2, 0 , IPC_RMID, NULL);
   kill(servPid,SIGTERM);
   exit(0);
}

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

void printBoard(char board[DIM][DIM]) {
    const char *separator = "-----------\n";
    const char *newline = "\n";
    char buffer[4]; // For " | " and "\n"

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            buffer[0] = ' ';
            buffer[1] = board[i][j];
            buffer[2] = ' ';
            buffer[3] = '\0';

            write(1, buffer, sizeof(buffer));

            if (j < 2) {
                write(1, "|", 1);
            }
        }
        write(1, newline,1);

        if (i < 2) {
            write(1, separator, 12);
        }
    }
}

int checkMoveP(int c1 , int c2 ,char simb1, char simb2 ,char board [DIM][DIM])
{
   if (c1 < 0 || c1 >= 3 || c2 < 0 || c2 >=3)
   {
    return 0;
   }

   if (board[c1][c2]==simb1 || board[c1][c2]== simb2 )
   {
    return 0;
   }
   return 1;
}



int main(int argc, char *argv[])
{
  int coord1;
  int coord2;
  int cond1=0;
  
  if ( signal (SIGINT,sigHandler)==SIG_ERR)
    printf("Errore handler SIGINT\n");
  
  if (signal(SIGTERM,termHand)==SIG_ERR)
    printf("Errore sigterm handling\n");

  if (signal(SIGQUIT,killHand)==SIG_ERR)
    printf("\nErrore sigkill\n");

  if (signal(SIGALRM,alarmHand)==SIG_ERR)
    printf("\nAlarm error\n");  

  key_t chiave = ftok("src/Server.txt",'B');

    if (chiave<0)
        errExit("Chiave errata server");

  int shmid = shmget(chiave,sizeof(struct SharedMemory), 0666);

  struct SharedMemory *sharedMem = (struct SharedMemory *)shmat(shmid,NULL,0);  
  
  appoMem2 = shmid;

  key_t key = ftok("src/Semafori.txt",'A');

  if (key == -1)
        errExit("ftok failed");
    
  int semid = semget(key, 3, 0);

   
  if (semid == -1)
    errExit("semget failed");
  
  appoId2 =  semid;
   
   servPid = sharedMem->serv;

/*-----------------------------------------------------------------------------------------------------------------------------------*/
  if(argc == 2 && strcmp("bot",argv[1])!= 0 && sharedMem->bot == 0)
  {
  if(semctl(semid,1,GETVAL,0)==0)
  {
    sharedMem->proc1 = getpid();
    write(1,"You are ",8);
    write(1,&sharedMem->symbol1,sizeof(sharedMem->symbol1));
    strcpy(sharedMem->nameP1,argv[1]);
    semOp(semid,(unsigned short)1, 1);
    semOp(semid,(unsigned short) 0, 1);
    semOp(semid ,(unsigned short)2 ,-1);
  }else{
    sharedMem->proc2 = getpid();
    write(1,"You are ",8);
    write(1,&sharedMem->symbol2,sizeof(sharedMem->symbol2));
    strcpy(sharedMem->nameP2,argv[1]);
    semOp(semid,(unsigned short) 0, 1);
    semOp(semid ,(unsigned short)1 ,-2);
  }
  
  if (strcmp(sharedMem->nameP1,argv[1])==0)
  {
     

     while(sharedMem->victory != 1)
     {

      if (sharedMem->whoWin1 == 1)
       {
          write(1,"\nYou Win!!!\n",12);
       }

      if(sharedMem->whoWin2 == 2)
     {
       write(1,"\nPlayer 2 won but you can continue the game\n",44);
       do
       {
        write(1,"\nContinue? YES (1) NO (2)\n",26);
        scanf("%d",&sharedMem->choice);
       } while (sharedMem->choice != 1 && sharedMem->choice != 2);
     }
     
    if (sharedMem->choice == 2)
    {
       sharedMem->block = 1;
       stopGame(shmid,semid);
       break;
    }
       write(1,"\nIt's player's 1 turn",21);

       write(1,"\nPLAY!!!",8);
       write(1,"\n",1);
       printBoard(sharedMem->table);

       while(cond1 == 0)
       {
          if(sharedMem->timeout != 0)
          {
            alarm(sharedMem->timeout);
            write(1,"Enter the first coordinate of your move\n",40);
            
            if(scanf("%d",&coord1) != 1)
            {
              sharedMem->block = 1;
              stopGame(shmid,semid);
              break;
            }
        

            alarm(sharedMem->timeout);
            write(1,"Enter the first coordinate of your move\n",40);
           
            if(scanf("%d",&coord2) != 1 )
            {
              sharedMem->block = 1;
              stopGame(shmid,semid);
              break; //blocca se stesso
            }

          }else
          {
             write(1,"Enter the first coordinate of your move\n",40);
            scanf("%d",&coord1);
            write(1,"Enter the first coordinate of your move\n",40);
            scanf("%d",&coord2);
          }
          
          cond1 = checkMoveP(coord1-1,coord2-1,sharedMem->symbol1,sharedMem->symbol2,sharedMem->table);
       }
       sharedMem->table[coord1-1][coord2-1] = sharedMem->symbol1;
       cond1 = 0;
       write(1,"\n",1);
       //chiede mossa e scrive
       semOp(semid,(unsigned short)0 , 1); //Sblocco server
       semOp(semid,(unsigned short)2 ,-1); //blocca se stesso
     }
     semOp(semid,(unsigned short) 0 , 1 );
     write(1,&sharedMem->esito,sizeof(sharedMem->esito));
     printBoard(sharedMem->table);
  }else{
     while(sharedMem->victory != 1)
     {
       if (sharedMem->whoWin2 == 2)
       {
          write(1,"\nYou Win\n",9);
       }

       if(sharedMem->whoWin1 == 1)
       {
      write(1,"\nPlayer 1 won but you can continue the game\n",44);
       do
       {
        write(1,"\nContinue? YES (1) NO (2)\n",26);
        scanf("%d",&sharedMem->choice);
       } while (sharedMem->choice != 1 && sharedMem->choice != 2);
       }
       
      if (sharedMem->choice == 2)
      {
         sharedMem->block = 1;
         stopGame(shmid,semid);
         break;
      }

      write(1,"\nIt's player's 2 turn",21);

       write(1,"\nPLAY!!!",8);
       write(1,"\n",1);
       printBoard(sharedMem->table);

       while(cond1 == 0)
       {  
           if(sharedMem->timeout != 0)
           {
              alarm(sharedMem->timeout);
             write(1,"Enter the first coordinate of your move\n",40);
             if(scanf("%d",&coord1) != 1 )
             {
              sharedMem->block = 1;
              stopGame(shmid,semid);
              break; 
             }
             alarm(sharedMem->timeout);
             write(1,"Enter the first coordinate of your move\n",40);
             if(scanf("%d",&coord2) != 1 )
             { 
               sharedMem->block = 1;
               stopGame(shmid,semid);
               break;
             }
           }
          else
          {
             write(1,"Enter the first coordinate of your move\n",40);
          scanf("%d",&coord1);
             write(1,"Enter the first coordinate of your move\n",40);
          scanf("%d",&coord2);
          }
          
          cond1 = checkMoveP(coord1-1,coord2-1,sharedMem->symbol1,sharedMem->symbol2,sharedMem->table);
       }
       sharedMem->table[coord1-1][coord2-1] = sharedMem->symbol2;
       cond1 = 0;

       write(1,"\n",1);
       semOp(semid,(unsigned short)0 , 1); //Sblocco server
       semOp(semid,(unsigned short)1 ,-2); //blocca se stesso
     }
     semOp(semid,(unsigned short) 0 , 1 );
     write(1,&sharedMem->esito,sizeof(sharedMem->esito));
     printBoard(sharedMem->table);
     
     shmdt((void *)sharedMem);
     shmctl((shmid),IPC_RMID,NULL); 
  }
}
  /*-------------------------------------------------------------------------------------------------------------------------------------*/
  else if (strlen(argv[5]) > 0 && strcmp("bot",argv[1])!= 0 && sharedMem->bot == 0)
  {
    write(1,"\nBot\n",5);
    sharedMem->cop =1 ;
    sharedMem->proc1 = getpid();
    write(1,"You are",7);
    write(1,&sharedMem->symbol1,sizeof(sharedMem->symbol1));
    strcpy(sharedMem->nameP1,argv[1]);
    //printf("%s",sharedMem->nameP1);
    semOp(semid,(unsigned short)1, 1);
    semOp(semid,(unsigned short) 0, 2);
    semOp(semid ,(unsigned short)2 ,-1);
    
    

     while(sharedMem->victory != 1)
     {

      if (sharedMem->whoWin1 == 1)
       {
          write(1,"\nYou win\n",9);
       }

      if(sharedMem->whoWin2 == 2)
     {
       write(1,"\nPlayer 2 won but you can continue\n",35);
       do
       {
        write(1,"\nContinue? YES (1) NO (2)\n",26);
        scanf("%d",&sharedMem->choice);
       } while (sharedMem->choice != 1 && sharedMem->choice != 2);
     }
     
    if (sharedMem->choice == 2)
    {
       sharedMem->block = 1;
       stopGame(shmid,semid);
       break;
    }

       write(1,"\nIt's player's 1 turn",21);
       write(1,"\nPLAY!!!",8);
       write(1,"\n",1);
       printBoard(sharedMem->table);

       while(cond1 == 0)
       {
          if(sharedMem->timeout != 0)
          {
            alarm(sharedMem->timeout);
             write(1,"Enter the first coordinate of your move\n",40);
            
            if(scanf("%d",&coord1) != 1)
            {
              sharedMem->block = 1;
              stopGame(shmid,semid);
              break;
            }
        

            alarm(sharedMem->timeout);
             write(1,"Enter the first coordinate of your move\n",40);
           
            if(scanf("%d",&coord2) != 1 )
            {
              sharedMem->block = 1;
              stopGame(shmid,semid);
              break; //blocca se stesso
            }

          }else
          {
             write(1,"Enter the first coordinate of your move\n",40);
            scanf("%d",&coord1);
             write(1,"Enter the first coordinate of your move\n",40);
            scanf("%d",&coord2);
          }
          
          cond1 = checkMoveP(coord1-1,coord2-1,sharedMem->symbol1,sharedMem->symbol2,sharedMem->table);
       }
       sharedMem->table[coord1-1][coord2-1] = sharedMem->symbol1;
       cond1 = 0;
       write(1,"\n",1);
       //chiede mossa e scrive
       semOp(semid,(unsigned short)0 , 1); //Sblocco server
       semOp(semid,(unsigned short)2 ,-1); //blocca se stesso
     }
     semOp(semid,(unsigned short) 0 , 1 );
     write(1,&sharedMem->esito,sizeof(sharedMem->esito));
     printBoard(sharedMem->table);
  }else if (strcmp("bot",argv[1]) == 0 && sharedMem->bot == 1)
  { 
    semOp(semid ,(unsigned short)1 ,-2);
    while(sharedMem->victory != 1)
     {
       if (sharedMem->whoWin2 == 2)
       {
          write(1,"\nYou win\n",9);
       }

       if(sharedMem->whoWin1 == 1)
       {
       write(1,"\nPlayer 1 won\n",14);
       do
       {
        write(1,"\nGAME LOST\n",12);
        sharedMem->choice = 2;
       } while (sharedMem->choice != 1 && sharedMem->choice != 2);
       }
       
      if (sharedMem->choice == 2)
      {
         sharedMem->block = 1;
         stopGame(shmid,semid);
         break;
      }

       printBoard(sharedMem->table);

       while(cond1 == 0)
       { 
              coord1 = rand()%(9)+1;
              coord2 = rand()%(9)+1;
          
              cond1 = checkMoveP(coord1-1,coord2-1,sharedMem->symbol1,sharedMem->symbol2,sharedMem->table);
       }
       sharedMem->table[coord1-1][coord2-1] = sharedMem->symbol2;
       cond1 = 0;

       write(1,"\n",1);
       semOp(semid,(unsigned short)0 , 1); //Sblocco server
       semOp(semid,(unsigned short)1 ,-2); //blocca se stesso
     }
     semOp(semid,(unsigned short) 0 , 1 );
     write(1,&sharedMem->esito,sizeof(sharedMem->esito));
     printBoard(sharedMem->table);

     shmdt((void *)sharedMem);
     shmctl((shmid),IPC_RMID,NULL);
  }else{
    write(1,"\nNot like this mate\n",20);
    write(1,"1 ./TriClient nome1\n",20);
    write(1,"2 ./TriClient nome1 *\n",22);
    if (semctl(semid, 0 , IPC_RMID, NULL) == -1)
        errExit("semctl IPC_RMID failed");
  }
  shmdt((void *)sharedMem);
  shmctl((shmid),IPC_RMID,NULL); 

  return 0;
}
