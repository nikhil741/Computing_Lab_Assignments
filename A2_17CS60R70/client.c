////////////////////   NIKHIL AGRAWAL //////////////////////////
////////////////////     17CS60R70    //////////////////////////
////////////////////   ASSIGNMENT-1   //////////////////////////
////////////////////   CLIENT-END     //////////////////////////

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
	//Input User's data
	char str[20];
	printf("Enter Your Message\n");
	gets(str);

    
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    
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

    //Sending Message to server
    send(sock , str , strlen(str) , 0 );
    
    //Reading reply from server
    valread = read( sock , buffer, 1024);
    
    //Server Response to console
    printf("%s\n",buffer );
    
    close(sock);
    return 0;
}