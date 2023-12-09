#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char *arguments[10];
int valid = 1;

void get_file(int client_socket)
{
 
 int fd= open("temp.tar.gz", O_CREAT| O_WRONLY| O_TRUNC, 0777); //opening the file in create and write mode
    char buffer[1024];
    ssize_t receivedBytes = 0;
    while (recv(client_socket, buffer, sizeof(buffer), 0)) {   //receive data from the server/mirror
    write(fd, buffer, sizeof(buffer)); //copyiing it into the client pwd
    }
    close(fd);
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0); //beginning to connect with server

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // using localhost to connect on the same device
    server_addr.sin_port = htons(49151);  // using a port for server

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) { //connect between client socket and server socket
        perror("Failed");
        exit(EXIT_FAILURE);
    }

    int mircon=0; //Flag to not enter the loop
    char command[1024]; //where user command will be stored
    while (1) {
    
        send(client_socket,"Connection Request",17, 0); //sending this message to check if the client can connect to server
        char response1[1024];
        recv(client_socket, response1, sizeof(response1), 0);
        if(strcmp(response1,"Mirror")==0)         // checking the response to see if its asking to go to mirror
        {
            printf("Redirecting to Mirror pls wait!!!!\n");
            //con=0;
            close(client_socket);          //if it does then we close the connection with server
            break;                         //and break out of this while loop
        }
        printf("Enter a command: ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0'; // removing the newline character

        char *command1 = strdup(command); //making a copy

        int num_arguments = 0;
        char *token = strtok(command1, " ");
        
        while (token != NULL && num_arguments < 10) { //tokenizing for further use
            arguments[num_arguments] = token;
            num_arguments++;
            token = strtok(NULL, " ");
        }
      
        int valid = 1;
        if (strcmp(arguments[0], "fgets") == 0) { //checking the name of command
            if (num_arguments < 2 || num_arguments > 5) { //checking the arguments
                printf("Invalid command\n"); 
                valid = 0;
            }
        } else if (strcmp(arguments[0], "tarfgetz") == 0) {
            if (num_arguments < 3 || num_arguments > 4) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "filesrch") == 0) {
            if (num_arguments != 2) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "targzf") == 0) {
            if (num_arguments < 2 || num_arguments > 3) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "getdirf") == 0) {
            if (num_arguments < 3 || num_arguments > 4) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "quit") == 0) {
            if (num_arguments > 1) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else {
            printf("Invalid command. Try again\n");
            valid = 0;
        }

        if (valid) { //if the command is correct then send it to the server
            send(client_socket, command, strlen(command), 0);
            char response2[1024];
            ssize_t bytes_received = recv(client_socket, response2, sizeof(response2), 0);
            if (strstr(response2, "quit") != NULL) {
                exit(EXIT_SUCCESS);
            }
            response2[bytes_received] = '\0';     //recieve and print the response
            printf("Server Response: %s\n", response2);
        }
        if(fork());
        else
        get_file(client_socket);
    
    } 
        client_socket = socket(AF_INET, SOCK_STREAM, 0); //beginning to connect with mirror
        server_addr.sin_port = htons(49152); //different port for Mirror
        if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) { //connect client with Mirror
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
        mircon=1; //flag to be in the loop
        while(mircon){
            printf("Enter a command: ");                //Same loop as server 
            fgets(command, sizeof(command), stdin);
            command[strlen(command) - 1] = '\0'; // Remove the newline character

        char *command1 = strdup(command);
        
        // Make a copy of the command
        if (command1 == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        int num_arguments = 0;
        char *token = strtok(command1, " ");
        
        while (token != NULL && num_arguments < 10) {
            arguments[num_arguments] = token;
            num_arguments++;
            token = strtok(NULL, " ");
        }
      
        int valid = 1;
        if (strcmp(arguments[0], "fgets") == 0) {
            if (num_arguments < 2 || num_arguments > 5) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "tarfgetz") == 0) {
            if (num_arguments < 3 || num_arguments > 4) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "filesrch") == 0) {
            if (num_arguments != 2) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "targzf") == 0) {
            if (num_arguments < 2 || num_arguments > 3) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "getdirf") == 0) {
            if (num_arguments < 3 || num_arguments > 4) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else if (strcmp(arguments[0], "quit") == 0) {
            if (num_arguments > 1) {
                printf("Invalid command\n");
                valid = 0;
            }
        } else {
            printf("Invalid command. Try again\n");
            valid = 0;
        }

        if (valid){
            send(client_socket, command, strlen(command), 0); //send the command message to server

            char response[1024];
            ssize_t bytes_received = recv(client_socket, response, sizeof(response), 0); //get response from the client
            if(strstr(response,"quit") != NULL){  //To quit if user wants to quit
                exit(EXIT_SUCCESS);
            }
            response[bytes_received] = '\0';
            printf("Server Response: %s\n", response);//print response
        }
        if(fork());
        else
        get_file(client_socket);     
    }
    close(client_socket); //close client socket at the end
    return 0;
}

