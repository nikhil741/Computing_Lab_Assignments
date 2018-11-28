/***************************************************/
/*                  NIKHIL AGRAWAL                 */
/*                    17CS60R70                    */
/*           CLIENT SERVER CHAT APPLICATION        */
/*                   SERVER END                    */
/***************************************************/

/****Header File Block****/
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
/********************************/

#define PORT 8080
#define LENGTH_OF_LINE 100
#define MAXIMUM_CLIENTS 5
#define MAXIMUM_MESSAGE_WINDOW 10
#define MAXIMUM_MESSAGE_SIZE 50
#define MAXIMUM_ID_LENGTH 5


/****GLobal ProccesId and Client Fd *******************/
int ppid;
int server_fd, client_fd;

/*************************************************************/
/*              SHARED MEMORY STRUCTURE                      */
/*Structure of variables which need to be shared between the process*/
/*************************************************************/
struct shared{
	//CLIENT ID'S VARIABLE
    int clientCount;
    int clientIdsArray[MAXIMUM_CLIENTS];
    
    //SYNCHRONIZING ELEMENTS
    int readcount;
    //SEMAPHORE VARIABLES
    sem_t mutex;
    sem_t writer;

    //BUFFERED MESSAGE QUEUE
    int front, rear;
    char circularQueue[MAXIMUM_MESSAGE_WINDOW][MAXIMUM_MESSAGE_SIZE];
};
typedef struct shared shared;
static shared *shared1;

/******** READER ENTRY FOR CRITICAL SECTION *************/
void readerEntry(){
    sem_wait(&shared1->mutex);       // Mutual Exclusion for readcount
    (shared1->readcount)++;
    if(shared1->readcount == 1){
        sem_wait(&shared1->writer);
    }
    sem_post(&shared1-> mutex);
}

/******** READER EXIT FOR CRITICAL SECTION *************/
void readerExit(){
    sem_wait(&shared1->mutex);      //Release Mutex
    (shared1->readcount)--;           //Decrease Readcount as a reader is served
    if(shared1->readcount == 0){
        sem_post(&shared1->writer);
    }
    sem_post(&shared1->mutex);
}

/******** WRITER ENTRY FOR CRITICAL SECTION *************/
void writerEntry(){
    sem_wait(&shared1->writer);
}

/******** WRITER EXIT FOR CRITICAL SECTION *************/
void writerExit(){
    sem_post(&shared1->writer);
}

/******************************************************************/
/*                 GET ONLINE CLIENTS                             */
/*         Find all the users which are online                    */
/*     Return: String seperated by : for each client Id           */
/******************************************************************/
char* getIdOfClientWhichAreOnline(){
    static char ptr[100];
    bzero(ptr, sizeof(ptr));
    char s[50];
    //printf("Hello World\n");
    //printf("ClientCount:%d\n", (shared1->clientCount));
    strcpy(ptr, "OnlineClientsId's: ");
    
    readerEntry();
    
    for(int i = 0; i < (shared1->clientCount); ++i)
    {
        //printf("Inside For\n");
        sprintf(s,"%d",(shared1->clientIdsArray)[i]);
        strcat(ptr,s);
        strcat(ptr, " : ");
        //printf("%s\n", p);
    }
    
    readerExit();

    //strcat(ptr, "\n");
    return ptr;
}



/******************************************************************/
/*                  Client-Id Generation Module              	  */
/*Client Id will be generated as soon as client connects to server*/
/*	that is after execution of connect command from client        */
/******************************************************************/
int genrateClientId(){
    int x = rand() % 90000 + 10000;
    
    readerEntry();
    for(int i=0;i<shared1->clientCount;i++){
    	if((shared1->clientIdsArray)[i] == x){
    		readerExit();
    		return genrateClientId();
    	}
    }
    readerExit();
    return x;
}

/*************************************************************/
/*            Convert String->Integer Module                 */
/* 	Will convert first few digits of message to integer      */
/*	As in the message the destination Id is Mentioned so it  */
/*	       will get appended as prefix in message            */
/*************************************************************/
int convertClientIdToInteger(char s[]){
    char number[5];
    int i = 0, k=0;
    while(isdigit(s[i])){            //Loop till digit in input character
        number[k++] = s[i];
        i++;
    }
    if(k!=0) 
    	return atoi(number);             //Convert to integer from character array

    return 0;          //No digit in message
}

/*************************************************************/
/*                  Queue  Module                            */
/* 	Functions: Enque, Deque, empty, g                        */
/*************************************************************/
int isEmpty(){
    int status;

    readerEntry();
	if((shared1->front == -1 && shared1->rear == -1)){
		status = 1;
	}
	else{
	status = -1;
	}
    readerExit();
    return status;
}

/*********ENQUEUE************/
int enQueue(char message[]){
    //printf("INSIDE ENQUE\n");
    //printf("Message=%s\n", message);
    //find circularQueue Full or not
    int status;
    
    writerEntry();
    if(((shared1->front == 0) && (shared1->rear == MAXIMUM_MESSAGE_WINDOW-1)) || (shared1->front == shared1->rear + 1)) {
        printf("ERROR_ENQUE:Message Window Is full\n");
        status = -1;
    }

    //circularQueue Empty And Entry is possible
    else{
        if(shared1->rear == -1){
            (shared1->rear)++;
            (shared1->front)++;
        }
        else if(shared1->rear == MAXIMUM_MESSAGE_WINDOW - 1){
            shared1->rear = 0;
        }
        else{
            (shared1->rear)++;
        }
        shared1->circularQueue[shared1->rear][0] = '\0';
        strcpy(shared1->circularQueue[shared1->rear],message);
        //printf("Message Written in circularQueue:%s\n", shared1->circularQueue[shared1->rear])
        status = 1;

    }
    writerExit();
    //printf("Outside Enqueue\n");
    return status;
    
}

/******************************************************/
/*         DEQUES FRONT MESSAGE FROM QUEUE            */
/******************************************************/
void deque(){
	//printf("INSIDE DEQUEUE\n");

    writerEntry();
    //Check for empty case of circularQueue
    if((shared1->front == -1 && shared1->rear == -1)){
        printf("Message circularQueue is empty\n");
    }
    else{
        //printf("Message:%s\n", messagePtr);
        if(shared1->front == shared1->rear){
            shared1->front = shared1->rear = -1;
            //printf("Setting Front & Rear to -1:Front=%d Rear=%d\n", shared1->front, shared1->rear);
        }
        else if(shared1->front == MAXIMUM_MESSAGE_WINDOW-1){
            (shared1->front) = 0;
        }
        else{
        	shared1->front++;
        }
    }
    writerExit();
    //printf("Exit Deque\n");
}

/******************************************************/
/*         PRINTS QUEUES BUFFERED MESSAGES            */
/******************************************************/
void printAllBufferedMessages(){
	readerEntry();
	int i = shared1->front;
	printf("INSIDE PRINT ALL MESSAGE \nAll Messages in Queue:\n");
	
    while(i!=shared1->rear){
		printf("%s\n", shared1->circularQueue[i]);
		if(i == MAXIMUM_MESSAGE_WINDOW-1){
			i=0;
		}
		else{
			i++;
		}
		
	} //While End
	printf("%s\n", shared1->circularQueue[i]);
	readerExit();
	printf("PrintExit:printAllBufferedMessages\n");
    return;
}



/*************************************************************/
/*           CHECKS DESTINATION ONLINE OR NOT                */
/* 	      RETURN: 1(ONLINE)          -1(OFFLINE)             */
/*************************************************************/
int checkGivenClientIdIsUpOrNot(char s[]){
    int destinationClient = convertClientIdToInteger(s);
    int status;
    readerEntry();
    for(int i=0;i<shared1->clientCount;i++){
        //Success
        if(destinationClient == shared1->clientIdsArray[i]){
            status = 1;
            readerExit();
            return status;
        }
    }
    //Failure
    status = -1;

    readerExit();
    return status;
}

/*************************************************************/
/*                  BroadCast Module                         */
/* 			Will Enque Message for all Clients               */
/*************************************************************/
void BroadCastMessage(char message[]){
	//printf("INSIDE BROADCAST MESSAGE:%s\n", message);
	//printf("%s\n", message);
	int i;
	char messageToBeEnque[100];
    bzero(messageToBeEnque, sizeof(messageToBeEnque));
    readerEntry();
    int totalClients = (shared1->clientCount);
	readerExit();
    
    //printf("TotalClients=%d\n", totalClients);

    for(i=0;i<totalClients;i++){
    	//printf("InsideFor\n");
		//messageToBeEnque[0] = '\0';
		readerEntry();
		sprintf(messageToBeEnque,"%d",(shared1->clientIdsArray)[i]);
		readerExit();
		//printf("InsideFor\n");
		strcat(messageToBeEnque, message);
		printf("MessageTOBeEnque:%s\n", messageToBeEnque);
		enQueue(messageToBeEnque);
		//printf("BroadCastMessageEnqued:%s\n", messageToBeEnque);
	}
	//printf("BroadCast Enque Exit\n");
	return;
}

void sigHandle(int sig1){
	if(sig1 == SIGINT){
		close(server_fd);
		close(client_fd);
		kill(-ppid, SIGKILL);
	}

}

void deleteClientEntry(int x){
	writerEntry();
	int flag = 0;
	for(int i=0;i<shared1->clientCount;i++){
		if((shared1->clientIdsArray)[i] == x){
			(shared1->clientCount)--;
			flag++;
		}
		if(flag == 1){
			if(i<shared1->clientCount)
				(shared1->clientIdsArray)[i] = shared1->clientIdsArray[i+1];
		}
	}
	writerExit();
	printf("Log: Deleted Client=%d Entry\n", x);
	return;
}

int main(int argc, char const *argv[])
{
	
	ppid = getpid();
	if(signal(SIGINT, sigHandle) == SIG_ERR)
		printf("Cannot Catch\n");
    
    int valread, clientId, sentFlag, destinationClientId, totalNumberOfClientsOnline;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[100] = "Server:";
    char clientIdInString[10];
    int childId;
    int lineNumber;

    int i; 
    int n;
    int status=0;
    pid_t wpid;

    /*****TIME VARIABLES******/
    time_t rawtime;
  	struct tm * timeinfo;
  	time ( &rawtime );

    //printf("Server is trying for up\n");

    //Initialize shared Memory in Function
    shared1 = (shared *)mmap(0, sizeof(*shared1), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANON, -1, 0);                            //Allocating shared memory to structure.


	shared1->front = shared1->rear = -1;    

    sem_init(&shared1->writer,1,1);
    sem_init(&shared1->mutex,1,1);              //All Semaphores are created.
    //sem_init(&shared1->finalWrite,1,1);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    //Listen
        if (listen(server_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
    
    printf("LOG: Server's Up\n");


    /********CONTINUOS CHECK FOR NEW CLIENT**************/
    while(1){
    	//all bits are set to zero ni serv_addr
    	bzero(response, sizeof(response));
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("LOG: GOT CLIENT\n");
        
        /************************************************************************/
        /*                  PROCESS CLIENT                                      */
        /*                JUST AFTER CONNECT                                    */
        /*TASK: All Function Before Sending and Receiving Messages              */
        /*CHECK: MAX_CLIENTS REACHED, WELCOME MESSAGE TO CLIENT, ONLINE_CLIENTS */
        /************************************************************************/
        readerEntry();
        totalNumberOfClientsOnline = (shared1->clientCount);
        readerExit();
        if(totalNumberOfClientsOnline<MAXIMUM_CLIENTS){
            
            //Generate Child Id
            clientId = genrateClientId();
            printf("LOG: ClientId=%d\n", clientId);
            
            readerEntry();
            //Store Client Id to shared Memory
            (shared1->clientIdsArray)[(shared1->clientCount)] = clientId;
            //Increase CLient Counter
            (shared1->clientCount)++;
            readerExit();

            bzero(clientIdInString,sizeof(clientIdInString));
            sprintf(clientIdInString,"%d",clientId);
            
            //Get Time from Server Machine
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );

            char* strptr = getIdOfClientWhichAreOnline();
            strcpy(response, "Server: Your Client Id =");
            strcat(response, clientIdInString);
            strcat(response, "\nWelcome:");
            strcat(response, asctime (timeinfo));
            //strcat(response, "\n");
            strcat(response, strptr);
            send(client_fd , response, strlen(response) , 0);
            //free(strptr);
            childId = fork();                                     //fork()

        }

        else{
            printf("LOG: Maximum Client Limit Reached DISCONNECTTING CLIENT\n");
            strcpy(response, "-1");
            strcat(response, "Connection Limit Exceeded");
            
            send(client_fd , response, strlen(response) , 0);
        }


        /************************************************************************/
        /*                  SERVER CHILD FOR EACH CLIENT                        */
        /*TASK: CHECK MESSAGE FROM AND TO CLIENT & DELIVER IT TO DESTINATION    */
        /*CHECK: EMPTY_MESSAGE, CLIENT_QUIT, MESSAGE_FOR_CLIENT                 */
        /************************************************************************/
        if(childId == 0){
            /*****MAKE read NON-BLOCKING*********/
            int flags = fcntl(client_fd, F_GETFL, 0);
            fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
            
            /*********Infinite loop to serve it's client*******************/
            while(1){
                fflush(stdout);
                //all bits are set to zero ni serv_addr
    			memset(response, 0, sizeof(response));
    			//bzero(response, sizeof(response));
                //printf("Matters:::%s\n", response);
                /**********RECEIVE MESSAGE FROM CLIENT*********************/
                bzero(buffer, sizeof(buffer));
                valread = read(client_fd , buffer, 1024);
                //printf("Non_Blocking:%d\n", valread);
                /****************CLIENT QUITS******************************/
                if(valread == 0){
                	printf("Log: CLIENT:%d QUITS\n", clientId);
                	deleteClientEntry(clientId);
                	
                	//Broadcast Client Termination Message
                	char str_temp[50];
                	bzero(str_temp, sizeof(str_temp));
        			strcpy(str_temp, " Client=");
        			strcat(str_temp, clientIdInString);
        			strcat(str_temp, " Terminated");
        			printf("Temp_Aaray%s\n", str_temp);
                	BroadCastMessage(str_temp);
                	exit(0);
                }
                /*******NON EMPTY MESSAGE SENT BY CLIENT*******/
                else if(valread > 0){
                    printf("LOG:MessageReceived by %d message=%s", clientId, buffer);                    

                    /*********BROADCAST REQUEST**************/
                    if(((strncmp(buffer, "broadcast", 9)) == 0) && strlen(buffer)>10) {
                    	printf("LOG:BroadCast REQUEST BY %d Message=%s\n", clientId, buffer);
                    	
                    	/**Remove NewLine Char from String Received***/
                    	if(buffer[strlen(buffer)-1] == '\n'){
                    		buffer[strlen(buffer)-1] = '\0';
                    	}
                    	strcat(buffer, " by ");
                    	strcat(buffer, clientIdInString);
                    	strcat(buffer, "\0");
                        
                        BroadCastMessage(buffer+9);              
                
                    	//printAllBufferedMessages(); 
                    	strcpy(response, "Server: BroadCast Request Serving");            
                    }

                    /**********GET ALL CLIENTS REQUEST*******************/
                    else if((strncmp(buffer, "getPeers", 8)) == 0){
                    	char* strptr = getIdOfClientWhichAreOnline();
                    	strcpy(response, strptr);
                    	//free(strptr);
                    	//sentFlag = send(client_fd , response, strlen(response) , 0);

                    }
                    else if((strncmp(buffer, "printAll", 8)) == 0){
						printAllBufferedMessages();
                    }
                    /*****MESSAGE TO SOME CLIENT*******/
                    else if(convertClientIdToInteger(buffer) && strlen(buffer)>MAXIMUM_ID_LENGTH+1){

                    	int statusUpFlag = checkGivenClientIdIsUpOrNot(buffer);
                    	/*****CLient Not Online*****/
                    	if(statusUpFlag == -1){
                    		printf("Error: Invalid Destination Address By CLient\n");
                    		strcpy(response, "SERVER:Destination Address Invalid");
                    	}
                    	/*****CLIENT ONLINE**********/
                    	else if(statusUpFlag == 1){
                    		//printf("CLientIdInString=%s\n", clientIdInString);
                    		
                    		/***FORMAT MESSAGE*****/
                    		/**Remove NewLine Char from String Received***/
                    		if(buffer[strlen(buffer)-1] == '\n'){
                    			buffer[strlen(buffer)-1] = '\0';
                    		}
                    		strcat(buffer, " by ");
                    		strcat(buffer, clientIdInString);
                    		enQueue(buffer);                                                //enqueue()
                    	}
                	}

                	/******INCORRECT MESSAGE FORMAT********/
                	else{
                		printf("LOG:Incorrect Message: TRY AGAIN\n");
                		strcpy(response, "Server: Incorrect Message Format");
                	}

                	/****SEND RESPONSE MESSAGE TO CLIENT********/
                    sentFlag = send(client_fd , response, strlen(response) , 0);
                    if(sentFlag>0){
                    	printf("Log: MESSAGE SENT SUCCESFULLY\n");
                    }
                    else if(sentFlag==-1){
                    	printf("ERROR: FAILED TO SEND MESSAGE\n");
                    }

                }


           		/*********MESSAGE DELIVERY: TO CLIENT****************/
    			bzero(response,sizeof(response));
    			//memset(response, 0, sizeof(response));
    			//bzero(response, sizeof(response));
                
    			/***Queue is Not Empty***/
                if(isEmpty() == -1) {
                	//printf("Log:InsideDeliveryBufferIsNotEmpty\n");
                	
                	readerEntry();
                	int destination = convertClientIdToInteger(shared1->circularQueue[shared1->front]);
                	readerExit();

                	/****MESSAGE FOR CLIENT DELIVER IT*****/
                	if(destination == clientId){
                		readerEntry();
						printf("Log:Mathced. ClientId=%d  DeliveringMessage=%s\n", clientId, shared1->circularQueue[shared1->front]);
						readerExit();
						//response[0] = '\0';
						//strcpy(response, "SERVER:");

						readerEntry();
						strcat(response, (shared1->circularQueue[shared1->front])+6);
						readerExit();
					
						//printf("Matters:::%s\n", response);
						sentFlag = send(client_fd, response, strlen(response), 0);
					
						if(sentFlag == -1){
							printf("ERROR: UNABLE TO DELIVER MESSAGE=%s\n", response);
						}
						else if(sentFlag > 0){
							printf("LOG: MESSAGE SENT SUCCESSFULLY\n");
						}

						deque();
					}

					/*** MESSAGE FOR CLIENT WHICH IS OFFLINE *****/
					char messageTODeliver[100];
					bzero(messageTODeliver, sizeof(messageTODeliver));
					readerEntry();
					strcpy(messageTODeliver,shared1->circularQueue[shared1->front]);
					readerExit();
					if((checkGivenClientIdIsUpOrNot(messageTODeliver)) == -1 && isEmpty() == -1) {
						printf("%s\n", messageTODeliver);
						printf("LOG: MESSAGE FOR DESTINATION=%d AND IS OFFLINE\n", destination);
						deque();
					}
				}//(Message Delivery)

            }//Non-Blocking Execution(While Loop)

        } //Child End
    }//While Loop End
    return 0;
}   