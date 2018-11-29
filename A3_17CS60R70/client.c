/***************************************************/
/*                  NIKHIL AGRAWAL                 */
/*                    17CS60R70                    */
/*           CLIENT SERVER CHAT APPLICATION        */
/*                   CLIENT END                    */
/***************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PORT 8080


int main(int argc, char const *argv[])
{
	//Input User's data
    struct sockaddr_in address;
    int sock = 0, valread, clientId, childProcessId, statusFlag;
    struct sockaddr_in serv_addr;
    char *buffer1;
    size_t buffer1_size = 500;
    buffer1 =  (char*)malloc(buffer1_size);
    char buffer2[1024] = {0};

    //SOCKET CREATION
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    //Initialize with zeros
    //all bits are set to zero ni serv_addr
    memset(&serv_addr, '0', sizeof(serv_addr));

    
    serv_addr.sin_family = AF_INET;
    
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address\n");
        return -1;
    }

    //Connecting to sever
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected Succesfully!!\n");
    printf("getPeers: To get all online Clients\n");
    printf("broadcast <Message>: Broadcast Message to all clients\n");

   
    valread = read(sock , buffer2, 1024);

    if(valread>0){
    	printf("%s\n", buffer2);
        if((strncmp(buffer2, "-1", 2)) == 0){
            printf("EXIT\n");
            exit(0);
        }

   		childProcessId = fork();                                      //fork

    }else if(valread == -1){
    	printf("Error: Server Not Responding\n");
    }

    


    //Parent Process
    //Message for client from Stdin
    if(childProcessId==0){
    	while(1){
    		fflush(stdin);
            //printf("Inside Non_Block Loop\n");
            //CLient Entered Messag
            bzero(buffer1,sizeof(buffer1));
            printf("Enter Message to send\n");
            valread = getline(&buffer1, &buffer1_size,stdin);
            //printf("buffer1 : %s",buffer1);
            
            if((valread!=-1) && strncmp(buffer1, "exit", 4) == 0){
                printf("Exit Message\n");
            }

            else if(valread!=-1){
                //Send Message to server
                statusFlag = send(sock , buffer1, strlen(buffer1) , 0);           //Message Sent
                if(statusFlag > 0){
                    printf("Message Sent TO server\n");
                }
                else if(statusFlag == -1){
                    printf("Message Sending Failed\n");
                }
            }

    	}// End While Loop
    }

    //Client Message from Server
    //child process
    if(childProcessId!=0){
    	while(1){
    		fflush(stdin);
            bzero(buffer2,sizeof(buffer2));
            
            //memset(buffer2, '0', sizeof(buffer2[0]*1024));
            
            valread = read(sock , buffer2, 1024);
            if(valread == 0)
            {
            	printf("Server Terminated\n");
            	printf("Client is being Terminated\n");
                kill(-getpid(),SIGKILL);
            }
            if(valread!=-1){
            	if((strncmp(buffer2, "-1", 2)) == 0){
            		printf("ERROR:%s\n", buffer2);
            		//exit(0);
            	}
            	printf("Received Message: %s\n", buffer2);
            }
    
    	}//(While Loop Ends)
    }

    //printf("Enter Message or Wait for SomeOne to Send Message:\nMessage Format <CLIENT_ID> <MESSAGE>\n"
    close(sock);
    return 0;
}