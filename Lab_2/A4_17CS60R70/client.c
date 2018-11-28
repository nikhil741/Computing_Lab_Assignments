/******* NIKHIL AGRAWAL *******/
/*******  17CS60R70 ***********/
/***  ASSIGNMENT-4(POLLING) ***/
/******* CLIENT-END ***********/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>


void INThandler(int);        //Signal handeler Function

int my_sock;
int main(int argc, char *argv[])
{
    struct sockaddr_in their_addr;
    int portno;
    int len;

    portno = 8080;

    /***********SOCKET VARIABLES AND ADDRESS********/
    my_sock = socket(AF_INET,SOCK_STREAM,0);                            //Socket Creation
    memset(their_addr.sin_zero,'\0',sizeof(their_addr.sin_zero));
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(portno);
    their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    setvbuf (stdout, NULL, _IONBF, 0);

    /*****SIGNAL HANDELING************/
    signal(SIGINT, INThandler);
    signal(SIGTERM, INThandler);
    signal(SIGSTOP, INThandler);
    signal(SIGTSTP, INThandler);

    /****CONNECT***********/
    if(connect(my_sock,(struct sockaddr *)&their_addr,sizeof(their_addr)) < 0) {
        perror("connection not esatablished");
        exit(1);
    }

    fcntl(my_sock, F_SETFL,O_NONBLOCK);                      //MAKING EXECUTION NON-BLOCKING

    /*********POLLING FILE DESCIBTOR***********/
    struct pollfd fds[2];
    fds[0].fd=0;
    fds[0].events = POLLIN;
    fds[1].fd=my_sock;
    fds[1].events = POLLIN;
    while(1) {
        int rv = poll(fds, 2, -1);                            //SYSTEM CALL(POLL)
     
        //******MESSAGE FROM STDIN**********//
        if(fds[0].revents & POLLIN) {
            char inp[1024] = {0};
            fgets(inp, sizeof(inp), stdin) > 0;
            inp[strcspn(inp, "\n")] = '\0';
            len = send(my_sock, inp, strlen(inp), 0);
            if (strcmp("EXIT", inp) == 0) {
                printf("Client is disconnecting...");
                INThandler(SIGINT);
            }
            if (len < 0) {
                printf("Server not responding..");
                close(my_sock);
                exit(1);
            }
        }

        //******MESSAGE FROM SERVER(FROM SOME CLIENT)**********//
        if(fds[1].revents & POLLIN){
            char msg[1024]={0};
            ssize_t len;
            memset(&msg[0], 0, strlen(msg));
            len=recv(my_sock,msg,sizeof(msg),0);
            for(int i=0;i<len;i++){
            	printf("%c",msg[i]);
            }
            
            printf("\n");

            if(strncmp(msg,"-1", 2)==0){
                printf("closing in 3 seconds...");
                close(my_sock);
                sleep(1);
                exit(0);
            }
        }

    }
    return 0;
}

//Signal Handeler
void  INThandler(int sig)
{
    close(my_sock);
    printf("\n Shutting down the client.... \n");
    exit(0);
}