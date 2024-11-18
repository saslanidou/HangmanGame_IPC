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
#include <time.h>

#define WORDS 100
#define MAX_CHAR 13
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
    //int num_letter;
    int prospathies;
    int paules;
}; 

struct memory* shm;

void handler(int signum);

int main (int argc, char *argv[])
{
	char array[WORDS][MAX_CHAR];
	int i,j, mesgid, mesgid1, shmid, len, tries, len2, len3, len4,len5, len6;
	int id,counter,pos,counter1;
	char str[20], ch1[2], ch2[2];
	FILE *fp;
	key_t key_s;
	time_t t;
	id = getpid();
	counter=0;
	counter1=0;
	
	printf("Enter name of file:\n");
	scanf("%s", str);
	
	//anoigma arxeiou
	fp=fopen(str, "r");
	if(fp==NULL)
	{
		perror("fopen failed\n");
		exit(1);
	}
	
	//diavasma leksewn apo arxeio
    for(i=0; i<WORDS; i++)
	{
		fscanf(fp, "%s[^\n]",array[i]);
	}

	//dimiourgia ouras minimatwn client-server   
    if((mesgid = msgget(KEY_Q, IPC_CREAT | 0666))<0)
    {
     	perror("msgget failed\n");
		exit(1);
    }

	//dimiourgia ouras minimatwn server-client  
    if((mesgid1 = msgget(KEY_Q1, IPC_CREAT | 0666))<0)
    {
     	perror("msgget failed\n");
		exit(1);
    }
    message.mesg_type = 1;

	//dhmiourgia kleidiou gia shared memory 
    if((key_s=ftok(".", 2))==-1)
 	{
 		perror("ftok2 failed\n");
        exit(1);
    }
    
	//dhmiourgia shared memory
    if((shmid=shmget(key_s, sizeof(struct memory), IPC_CREAT | 0666))<0)
	{
		perror("shmget failed\n");
		exit(1);
	}
	
	//prosartisi mnhmhs
    if((shm = (struct memory*)shmat(shmid, NULL, 0)) <0)
	{
        perror("shmat failed\n");
        exit(1);
    }

	 //anamoni simatos 
    shm->id1 = id; 
	shm->status = NOTREADY;
	shm->paules=1;
    
    printf("Waiting for player...\n"); 
	//paralavi minimatos  
    msgrcv(mesgid, &message, sizeof(message.mesg_text), 1, 0);  
    
	
    if((strcmp(message.mesg_text,"hi"))==0)
    {	
    	bzero(message.mesg_text, sizeof(message.mesg_text));
    	printf("Connected\n");
    	
		//epilogi tuxaias leksis
    	srand((unsigned) time(&t));
    	j=rand()%100;
    	printf("The line of the word is: %d\n", j+1);
    	
		//apostoli dedomenwn tis leksis
    	len=strlen(array[j]);
    	message.mesg_text[0]=len;
    	len2=strlen(message.mesg_text);
    	msgsnd(mesgid1, &message, len2+1, 0);

    	tries=3;
    	message.mesg_text[0]=tries;
    	len3=strlen(message.mesg_text);
    	msgsnd(mesgid1, &message, len3+1, 0);
    	
    	ch1[0]=array[j][0];
    	memcpy(message.mesg_text,ch1,sizeof(ch1[0]));
    	len4=strlen(message.mesg_text);
    	msgsnd(mesgid1, &message, len4+1, 0);
    	
    	ch2[0]=array[j][len-1];
    	memcpy(message.mesg_text,ch2,sizeof(ch2[0]));
    	len5=strlen(message.mesg_text);
    	msgsnd(mesgid1, &message, len5+1, 0);

    	message.mesg_text[0]=shmid;
    	len6=strlen(message.mesg_text);
    	msgsnd(mesgid1, &message, len6+1, 0);

    	signal(SIGUSR1, handler); //eidos simatos, kai elegxos an einai ayto pou perimenei
    	
	  	while(1)
    	{ 
			// anamoni mexri na einai READY
        	while (shm->status != READY) //not ready giati den exei kati na diavasei
        	continue; 
        	sleep(1); 
        
        	strcpy(shm->word,array[j]);
       	
     	 	for(i=0; i<len; i++)
  			{	
  				if(array[j][i]==shm->letter)
  				{
  					pos=i; //thesi grammatos
  					counter++; // gia elegxo yparksis grammatos
  					//counter1++; 
  				}
  			}

			//ean uparxei gramma
  			if(counter>0)
  			{
  				shm->thesi=pos; 
  				shm->prospathies=tries;
  				//shm->num_letter=counter1;
  			
  				counter=0;
	
				//allagi katastasis shared memory kai apostoli simatos
  				shm->status = FILLED; //einai gemato mporei na diavasei, diladi oti exw grapsei
        		kill(shm->id2, SIGUSR2);
  			
			  	//elegxos gia paules pou exoun meinei
  				shm->paules=0;
          		for(i=0; i<len; i++)
          		{	
          			if(shm->word1[i]=='_')
          			{
          				shm->paules++;
          			}
          		}
          		shm->paules=shm->paules-1;

				//an paules=0, simainei oti vrike tin leksi
        		if(shm->paules==0)
       	       {
       	       	// katastrofi ouras minimatwn
    				msgctl(mesgid, IPC_RMID, NULL);
    				msgctl(mesgid1, IPC_RMID, NULL);

					//aposindesi mnhmns (antitheto ths prosartisis)
    				shmdt(shm); 
    
    				// katastrofi shared memory 
    				shmctl(shmid,IPC_RMID,NULL);
    				exit(0);
       	       }
  		}
  		else
  		{
  			shm->thesi=0;
  			tries=tries-1; 
  			shm->prospathies=tries;
  			//shm->num_letter=0;
  			
			  //allagi katastasis shm
  			shm->status = FILLED; //oti gemise to shared memory kai perimenei kapoios na ta diavasei
       		kill(shm->id2, SIGUSR2);

			//an prospathies=0, den vrike thn leksi 
       	    if(tries==0)
       	    {	
       	       	//  katastrofi message queue 
    				msgctl(mesgid, IPC_RMID, NULL);
    				msgctl(mesgid1, IPC_RMID, NULL);
    
    				shmdt(shm); 
    
    				// katastrosi the shared memory 
    				shmctl(shmid,IPC_RMID,NULL);
    				exit(0);
       	    }
  		}
  		}
    }
	
return 0; 
}

//elegxos simatos(id) apo client kai emfanisi minimatos apo shared memory
void handler(int signum) 
{ 
    if (signum == SIGUSR1)
    { 
        printf("Received from player: "); 
        printf("%s\n", &shm->letter);
    } 
} 


