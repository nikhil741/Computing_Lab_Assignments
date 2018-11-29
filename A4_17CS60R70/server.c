/******* NIKHIL AGRAWAL *******/
/*******  17CS60R70 ***********/
/**** ASSIGNMENT-4(POLLING) ***/
/******* SERVER-END ***********/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>

#define PORT 8080
#define LENGTH_OF_LINE 100
#define MAXIMUM_CLIENTS 5
#define MAXIMUM_MESSAGE_WINDOW 10
#define MAXIMUM_MESSAGE_SIZE 50
#define MAXIMUM_ID_LENGTH 5

/****GLobal ProccesId and Client Fd *******************/
int ppid;
int server_fd, client_fd;

/**Shared Memory Structure***/
struct shared{
	//Shared Client Id's and Fd's
    int clientCount;
    int clientIdsArray[MAXIMUM_CLIENTS][2];
};
typedef struct shared shared;
static shared *shared1;

/****Returns Clinet Fd from client Id******/
//Input: ClientId
//Output: ClientFd
int getClientFd(int targetId){
    for(int i = 0; i < (shared1->clientCount); ++i)
    {
        if(shared1->clientIdsArray[i][0] == targetId){
            return shared1->clientIdsArray[i][1];
        }
    }
    return -1;
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
    strcpy(ptr, "OnlineClientsId's: ");
    for(int i = 0; i < (shared1->clientCount); ++i){
        sprintf(s,"%d",(shared1->clientIdsArray)[i][0]);
        strcat(ptr,s);
        strcat(ptr, " : ");
    }
    return ptr;
}

/******************************************************************/
/*                  Client-Id Generation Module              	  */
/*Client Id will be generated as soon as client connects to server*/
/*	that is after execution of connect command from client        */
/******************************************************************/
int genrateClientId(){
    int x = rand() % 90000 + 10000;
    
    for(int i=0;i<shared1->clientCount;i++){
    	if((shared1->clientIdsArray)[i][0] == x){
    		
    		return genrateClientId();
    	}
    }
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
/*           CHECKS DESTINATION ONLINE OR NOT                */
/* 	      RETURN: 1(ONLINE)          -1(OFFLINE)             */
/*************************************************************/
int checkGivenClientIdIsUpOrNot(char s[]){
    int destinationClient = convertClientIdToInteger(s);
    int status;
    
    for(int i=0;i<shared1->clientCount;i++){
        //Success
        if(destinationClient == shared1->clientIdsArray[i][0]){
            status = 1;
            return status;
        }
    }
    //Failure
    status = -1;

    
    return status;
}


/*****ctrl+c Handle*******/
/***Message to all clients that server terminated**/
void sigHandle(int sig1){
	if(sig1 == SIGINT){
        /******Sending Close Message to Each Client***/
        int valread;
        char buffer[50];
        bzero(buffer, sizeof(buffer));
        strcpy(buffer, "-1ServerClosed");
        for(int j=0;j<shared1->clientCount;j++){
            //printf("Fd=%d\n",getClientFd(shared1->clientIdsArray[j][0]));
            valread = send(getClientFd(shared1->clientIdsArray[j][0]), buffer, strlen(buffer) , 0);
            //printf("Inside For Loop ValRead=%d\n", valread);
        } 
		close(server_fd);
		close(client_fd);
		kill(-ppid, SIGKILL);
	}

}

/*****Delete A client Entry from shared Memory****/
void deleteClientEntry(int x){
	int flag = 0;
	for(int i=0;i<shared1->clientCount;i++){
		if((shared1->clientIdsArray)[i][0] == x){
			(shared1->clientCount)--;
			flag++;
		}
		if(flag == 1){
			if(i<shared1->clientCount)
				(shared1->clientIdsArray)[i][0] = shared1->clientIdsArray[i+1][0];
		}
	}
	printf("Log: Deleted Client=%d Entry\n", x);
	return;
}


///////////////////////////////////////////////////////MAIN EXECUTION//////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char const *argv[]){
	ppid = getpid();                               //Get Process id
	if(signal(SIGINT, sigHandle) == SIG_ERR)
		printf("Cannot Catch\n");                  //Signal Handeling

	//Operation Variables
	int valread, clientId, sentFlag, destinationClientId, totalNumberOfClientsOnline, statusUpFlag;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[100] = "Server:";
    char clientIdInString[10];

    //Polling Variables
    int numfds = 0;
    int close_conn = 0;
    int compress_array =0;
    struct pollfd fds[MAXIMUM_CLIENTS];

    //Process Id Variables
    int i; 
    int n;
    int status=0;
    pid_t wpid;

    /*****TIME VARIABLES******/
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );


    //Initialize shared Memory in Function
    shared1 = (shared *)mmap(0, sizeof(*shared1), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANON, -1, 0);                            //Allocating shared memory to structure.

    

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /*****Making Non Blocking*********/
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //sets all values in a buffer to 0
    bzero((char *) &address, sizeof(address));
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

    //////// INITIALIZE TO ZEROS ////////////////
    memset(&fds,0,sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    /***Parameters of Polling*******/
    int timeout = 10000;
    int nfds = 1;
    int current_size;


    do{
    	if(poll(fds,nfds,timeout)<0)                        //Poll(System-call)
        	perror("ERROR on polling");
    	current_size = nfds;

    	for(int i = 0;i<current_size;i++)
    	{
        	if(fds[i].revents == 0)
            	continue;

        	if(fds[i].revents != POLLIN)
        	{

        	}

        	if(fds[i].fd == server_fd)
        	{
            	printf("Listening socket is readable\n\n");

            	do{
                	bzero(response, sizeof(response));
                    //*********Accept Client********//
                    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
                	{
                    	perror("accept");
                    	continue;
                        //exit(EXIT_FAILURE);
                	}
                	printf("LOG: GOT CLIENT\n");
                    
                    /************************************************************************/
                    /*                  PROCESS CLIENT                                      */
                    /*JUST AFTER CONNECT(ONLY ONE TIME EXECUTES FOR EACH CLIENT)            */
                    /*TASK: All Function Before Sending and Receiving Messages              */
                    /*CHECK: MAX_CLIENTS REACHED, WELCOME MESSAGE TO CLIENT, ONLINE_CLIENTS */
                    /************************************************************************/
                	totalNumberOfClientsOnline = (shared1->clientCount);
                	if(totalNumberOfClientsOnline<MAXIMUM_CLIENTS){                              //Check If Doens't cross maximum client Limit

                        /******CREATE FD FOR LEGITMATE CLIENT*************/
                        fds[nfds].fd = client_fd;
                    	fds[nfds].events = POLLIN;
                    	nfds++;

                        //Generate Child Id
                    	clientId = genrateClientId();
                    	printf("LOG: ClientId=%d\n", clientId);

                        //CLient Id In String
                        bzero(clientIdInString,sizeof(clientIdInString));
                        sprintf(clientIdInString,"%d",clientId);

                        //Store Client Id & CLIENT FD to shared Memory
                    	(shared1->clientIdsArray)[(shared1->clientCount)][0] = clientId;
                    	(shared1->clientIdsArray)[(shared1->clientCount)][1] = client_fd;
                        
                        //Increase CLient Counter
                    	(shared1->clientCount)++;

                        //Get Time from Server Machine
                    	time ( &rawtime );
                    	timeinfo = localtime ( &rawtime );

                    	/////WELCOME MESSAGE FOR CLIENT//////
                    	char* strptr = getIdOfClientWhichAreOnline();
                    	bzero(response, sizeof(response));
                    	strcpy(response, "Server: Your Client Id =");
                    	strcat(response, clientIdInString);
                    	strcat(response, "\nWelcome:");
                    	strcat(response, asctime (timeinfo));
                        //strcat(response, "\n");
                    	strcat(response, strptr);
                    	send(client_fd , response, strlen(response) , 0);
                	}

                	///Max Limit Reached for clients and hence quiting////
                	else{
                    	printf("LOG: Maximum Client Limit Reached DISCONNECTTING CLIENT\n");
                    	bzero(response, sizeof(response));
                    	strcpy(response, "-1");
                    	strcat(response, "Connection Limit Exceeded");
                    	send(client_fd , response, strlen(response) , 0);
                    	continue;
                	}

            	}while(client_fd != -1);
        	}

        	else {
            	close_conn = 0;
            	fflush(stdout);
            	memset(response, 0, sizeof(response));
                /**********RECEIVE MESSAGE FROM CLIENT*********************/
            	bzero(buffer, sizeof(buffer));
            	valread = recv(fds[i].fd , buffer, 1024, 0);
                
                    /****************CLIENT QUITS******************************/
            	if(valread == 0){
                	printf("Log: CLIENT:%d QUITS\n", clientId);
                    int clientValueSelf;
                    for(int j=0;j<shared1->clientCount;j++){
                        if(fds[i].fd == shared1->clientIdsArray[j][1]){
                            clientValueSelf = shared1->clientIdsArray[j][0];
                            break;
                        }
                    }
                	deleteClientEntry(clientId);
                        /**Sending Quit Client Message to Each Client***/
                    bzero(response, sizeof(response));
                    strcpy(response, "Client=");
                    bzero(clientIdInString, sizeof(clientIdInString));
                    sprintf(clientIdInString,"%d",clientValueSelf);
                    strcat(response, clientIdInString);
                    strcat(response, " Quits");
                    printf("CLientQuit:response= %s\n", response);
                    for(int j=0;j<shared1->clientCount;j++){
                        send(getClientFd(shared1->clientIdsArray[j][0]), response, strlen(response) , 0);
                    } 
                	close_conn =1;
            	}

                    /*******NON EMPTY MESSAGE SENT BY CLIENT*******/
            	else if(valread > 0){
                	printf("LOG:MessageReceived by %d message=%s\n", clientId, buffer);                    

                	   /*********BROADCAST REQUEST**************/
                	if(((strncmp(buffer, "broadcast", 9)) == 0) && strlen(buffer)>10) {
                    	printf("LOG:BroadCast REQUEST BY %d Message=%s\n", clientId, buffer);
                        //CLient Id In String
                        bzero(clientIdInString,sizeof(clientIdInString));
                        sprintf(clientIdInString,"%d",clientId);
                        //FormatMessage
                        if(buffer[strlen(buffer)-1] == '\n'){           //Remove Newline Character from buffer
                        	buffer[strlen(buffer)-1] = '\0';
                    	}                    	
                        strcat(buffer, " by ");
                    	strcat(buffer, clientIdInString);
                    	strcat(buffer, "\0");
                    	//printf("buffer=%s\n", buffer);

                    	   /******Sending Broadcast Message to Each Client***/
                    	for(int j=0;j<shared1->clientCount;j++){
                        	//printf("Fd=%d\n",getClientFd(shared1->clientIdsArray[j][0]));
                        	valread = send(getClientFd(shared1->clientIdsArray[j][0]), buffer, strlen(buffer) , 0);
                       		//printf("Inside For Loop ValRead=%d\n", valread);
                    	} 
                    	bzero(response, sizeof(response));
                    	strcpy(response, "BroadCast Request Served Succesfully!!");            
                	}

                        /**********getPeers(CLIENT ONLINE)*******************/
                    else if((strncmp(buffer, "getPeers", 8)) == 0){
                        char* strptr = getIdOfClientWhichAreOnline();
                        bzero(response, sizeof(response));
                        strcpy(response, strptr);
                        //printf("Response=%s\n",response);
                        valread = send(fds[i].fd , response, strlen(response) , 0);
                        if(valread > 0){
                        	printf("Log:MessageSentSuccesfull by %s\n", clientIdInString);
                        	bzero(response, sizeof(response));
                        	strcpy(response, "getPeersSuccessfull!!");
                        }
                        else{
                        	bzero(response, sizeof(response));
                        	strcpy(response, "getPeersFailed!!");
                        }
                    }

                        /*****MESSAGE TO SOME CLIENT*******/
                    else if(convertClientIdToInteger(buffer) && strlen(buffer)>MAXIMUM_ID_LENGTH+1){
                        statusUpFlag = checkGivenClientIdIsUpOrNot(buffer);

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

                            int targetClientId = convertClientIdToInteger(buffer);
                            printf("TargetClientFd=%d\n", getClientFd(targetClientId));
                        	
                        	bzero(response, sizeof(response));
                            strcpy(response, buffer+6);
                            //printf("Response=%s\n", response);

                            /********MESSAGE DELIVERY TO CLIENT**********/
                            valread = send(getClientFd(targetClientId) , response, strlen(response), 0);
                            if(valread>0){
                            	printf("Log:MessageSentSuccesfull by %s\n", clientIdInString);
                            	bzero(response, sizeof(response));
                            	strcpy(response, "MessageSentSuccesfully!!");
                            }
                            else{
                            	bzero(response, sizeof(response));
                            	strcpy(response, "Message Not Sent!!");
                            }
                            //printf("SendValRead=%d\n", valread);
                        }
                    }

                        /******INCORRECT MESSAGE FORMAT********/
                    else{
                        printf("LOG:Incorrect Message: TRY AGAIN\n");
                        strcpy(response, "Server: Incorrect Message Format");
                    }

                        /******SEND RESPONSE MESSAGE TO ITSELF(CLIENT)********/
                    sentFlag = send(fds[i].fd , response, strlen(response) , 0);
                    if(sentFlag>0){
                        printf("Log: MESSAGE SENT SUCCESFULLY\n");
                    }
                    else if(sentFlag==-1){
                        printf("ERROR: FAILED TO SEND MESSAGE\n");
                    }

                }
                if(close_conn){
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = 1;
                }
            }
        } 
    }while(nfds - 1);
    
    //CLOSE FILE DESCRIPTOR
    for(int i=0;i<nfds;i++){
        if(fds[i].fd >= 0)
            close(fds[i].fd);
    }
    return 0;
}