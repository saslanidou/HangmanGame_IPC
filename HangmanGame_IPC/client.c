#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h> 
#include <signal.h>
#include <string.h>

#define MAX 10
#define KEY_Q 1234
#define KEY_Q1 4567
#define FILLED 0
#define READY 1 
#define NOTREADY -1

struct mesg_buffer
{ 
    long mesg_type; 
    char mesg_text[10]; 
}message;

struct memory 
{ 
    char word[100]; 
    char word1[100];
    int status, id1, id2; 
    char letter;
    int thesi;
    int num_letter;
    int prospathies;
    int paules;
};

struct memory* shm;

void handler(int signum);

int main (int argc, char *argv[])
{
 	key_t key_s;
 	int mesgid, mesgid1,len, j, shmid, len1, tries,i;
 	char word[20], ch1[2], ch2[2];
 	int id;
 	id= getpid();
        
  /*  if((key_s=ftok(".", 2))==-1)
 	{
 		perror("ftok2 failed\n");
        exit(1);
    }
    */
        
    if((mesgid = msgget(KEY_Q, 0666 | IPC_CREAT))<0)
    {
     	perror("msgget failed\n");
		exit(1);
    }
    message.mesg_type = 1; 
        
    if((mesgid1 = msgget(KEY_Q1, IPC_CREAT | 0666))<0)
    {
     	perror("msgget failed\n");
		exit(1);
    }
  
    // hi gia connect
    printf("Type hi if you want to connect with the server : \n"); 
    fgets(message.mesg_text, sizeof(message.mesg_text),stdin);
        
    len1=strlen(message.mesg_text);

    //antikatastasi \n me \0  
    if(message.mesg_text[len1-1]=='\n')
    {
        message.mesg_text[len1-1]='\0';
    }
    msgsnd(mesgid, &message, len1+1, 0);
        
    if((strcmp(message.mesg_text,"hi"))==0)
    {
    	printf("Connected with server\n");

        //paralavi dedomenwn tis leksis
        msgrcv(mesgid1, &message, sizeof(message.mesg_text),1, 0);
        len=message.mesg_text[0];
        printf("The number of letters is: %d\n", len);
        
        msgrcv(mesgid1, &message, sizeof(message.mesg_text),1, 0);
        tries=message.mesg_text[0];
        printf("The number of tries you have is: %d\n", tries);
        
        msgrcv(mesgid1, &message, sizeof(message.mesg_text),1, 0);
        strcpy(ch1, message.mesg_text);
        printf("The first letter is: %s\n", ch1);
        
        msgrcv(mesgid1, &message, sizeof(message.mesg_text),1, 0);
        strcpy(ch2, message.mesg_text);
        printf("The last letter is: %s\n", ch2);
        
        msgrcv(mesgid1, &message, sizeof(message.mesg_text),1, 0);
        shmid=message.mesg_text[0];

        // prosartisi(desmeusi) client sto shared memory
        if((shm = (struct memory*)shmat(shmid, NULL, 0))<0)
	    {
        	perror("shmat failed\n");
        	exit(1);
        }
        
        //anamoni simatos
	    shm->id2 = id; 
	    shm->status = NOTREADY; //perimenei mexri na yparksei kati sto shm
        
        //emfanisis arxikou promt
        printf("Welcome!!!\n");

        printf("%s",ch1);
        
        for(i=0; i<len-2; i++)
        {
        	printf("_");
        }
        
        printf("%s\n", ch2);
        
        bzero(shm->word1,sizeof(shm->word1));
        
        //word1:leksi me prwto kai teleutaio xaraktira kai paules
        shm->word1[0]=ch1[0];
        
        for(i=1; i<len-1; i++)
        {
        	shm->word1[i]='_';
        }
        
        shm->word1[len-1]=ch2[0];
        
        signal(SIGUSR2, handler);
       
        while (1)
    	{ 
            sleep(1);
        

         //eisodos grammatos kai allagi katastasis shm
        printf("Enter a letter: "); 
        scanf("%s", &shm->letter);
        shm->status = READY; //grapsame kati sto shm 

        //apostoli simatos
        kill(shm->id1, SIGUSR1); 
  
        while (shm->status == READY) 
        {
            continue; 
        }
        
        //ean prospathies=0, tote exase
        if(shm->prospathies==0)
        {
          	printf("\nYou lost :(\n");
          	printf("The word was : %s\n", shm->word);
          	
          	//katastrofi message queue 
    		msgctl(mesgid, IPC_RMID, NULL);
    		msgctl(mesgid1, IPC_RMID, NULL);
    
    		shmdt(shm); 
    
    		//katastrofi shared memory 
    		shmctl(shmid,IPC_RMID,NULL);
    		exit(0);
        }

         //elegxos deksia kai aristera , gia yparksi idou grammatos 
        if(shm->thesi>0)
        {
          	shm->word1[shm->thesi]=shm->letter;
          	
          	for(i=0; i<shm->thesi; i++)
          	{
          		if(shm->word[i]==shm->letter)
          		{
          			shm->thesi=i;
          			shm->word1[shm->thesi]=shm->letter;	
          		}
		    }

		    for(i=shm->thesi+1; i<len; i++)
          	{
          		if(shm->word[i]==shm->letter)
          		{
          			shm->thesi=i;
          			shm->word1[shm->thesi]=shm->letter;	
          		}
          	}
        }
         //emfanisi leksis me grammata pou vrike h oxi 
        puts(shm->word1);
        printf("\n");

         //elegxos gia paules pou exoun meinei 
        shm->paules=0;
        for(i=0; i<len; i++)
        {	
          	if(shm->word1[i]=='_')
          	{
          		shm->paules++;
          	}
          }
          
          //ean paules=0, tote kerdise
          if(shm->paules==0)
          {	
          	printf("\nYou found it :)\n");
          	printf("Congratulations!!!\n");
          	
          	//to destroy the message queue 
    		msgctl(mesgid, IPC_RMID, NULL);
    		msgctl(mesgid1, IPC_RMID, NULL);
    
    		shmdt(shm); 
    
    		//destroy the shared memory 
    		shmctl(shmid,IPC_RMID,NULL);
    		exit(0);
          }
    } 
  }
return 0;
}

//elegxos simatos(id) apo server kai emfanisi minimatos apo shared memory
void handler(int signum) 
{ 
    if (signum == SIGUSR2)
    {  
        printf("The number of tries you have is: %d\n",shm->prospathies);
    } 
} 



