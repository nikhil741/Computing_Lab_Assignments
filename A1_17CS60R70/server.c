////////////////////   NIKHIL AGRAWAL //////////////////////////
////////////////////     17CS60R70    //////////////////////////
////////////////////   ASSIGNMENT-1   //////////////////////////
////////////////////   SERVER-END     //////////////////////////


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define LENGTH_OF_LINE 100

//Parse LineNumber To read from file
//Convert character array number to integer
int lineNumberToRead(char s[], int n){
    char number[5];
    int i = 6, k=0;
    while(isdigit(s[i])){            //Loop till digit in input character
        number[k++] = s[i];
        i++;
    }
     
    return atoi(number);             //Convert to integer from character array
}


//Read line from the file requested by client
//Returns the line from file
char* readLineFromFile(FILE* fp, int lineNumber){
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    int i=0, readFlag=0;
    char *p = malloc(sizeof(char) * (LENGTH_OF_LINE + 1));

    //Read Line by line incrementing the line Number
    //As soon as the line matches return string to client
    while((read = getline(&line, &len, fp)) != -1){
        if(i==lineNumber){
            p = line;
            readFlag++;
            break;
        }
        i++;
    }

    //User requested to read line greater than document length
    if(readFlag==0){
    	p="Requested Line Not Present In File";
    }
    return p;
}

//Write message to file
//Input: File POinter, Character array(Message)
char* writeMessageFromClient(FILE* fp, char p[]){
    char tempBuffer[102];
    char *reply = malloc(sizeof(char) * (LENGTH_OF_LINE + 1));
    tempBuffer[0] = '\0';   
    
    //Setting file pointer to the end
    fseek(fp, 0, SEEK_END);

    //Appending newline character before the user's message
    strcpy(tempBuffer, "\n");
    strcat(tempBuffer, p+7);

    //write operation to file
    int statusFlagOfWriteOperation = fputs(tempBuffer, fp);
    
    //operation failed
    if(statusFlagOfWriteOperation == EOF){
        printf("FAILED TO WRITE\n");
        reply =  "WRITE OPERATION FAILED";
    }
    
    //Operation success
    else{
        //Server End
        printf("WRITTEN SUCCESFULLY");
        
        //Client End Response
        reply = "MESSAGE WRITTEN SUCCESFULLY";
    }
    return reply;
}

int main(int argc, char const *argv[])
{
    
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[100] = "Server:";
      
    int lineNumber;
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
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    valread = read( new_socket , buffer, 1024);
    printf("%s\n",buffer);

    //Open file
    FILE *fptr;
    char filename[15];
    fptr = fopen("./data.txt", "a+");
    //File handeling
    if(fptr == NULL){
        printf("Unable to open FILE\n");
        exit(EXIT_FAILURE);
    }

    //READ COMMAND FROM CLIENT
    //Read from the file
    if(strncmp(buffer, "READX", 5) == 0){
        //GET Line Number
        lineNumber = lineNumberToRead(buffer, strlen(buffer));
        
        //Prepare Response message for client
        strcat(response, " Read\n");
        strcat(response, readLineFromFile(fptr, lineNumber));
    }

    //WRITE COMMAND FROM CLIENT
    else if(strncmp(buffer, "WRITEX", 6) == 0){
        //Prepare Response message for client
        strcat(response, writeMessageFromClient(fptr, buffer));
    }

    //ERROR
    else{

        //Prepare Response message for client
        strcat(response, " Invalid Command");
        printf("Invalid command\n");
    }


    //Sending response Message to client
    send(new_socket , response, strlen(response) , 0);
    
    fclose(fptr);
    close(server_fd);
    return 0;
}   