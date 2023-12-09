#define _XOPEN_SOURCE 500 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/sendfile.h>

int serv_count = 0;  //to count the number of client connections
char response[1024]; //response to send to client
char *arguments[10]; //command
char found_file[1024] = ""; //file to be searched
int fileflag = 0;// flag to check if file is found or not
int min_size = 0; //min file size
int max_size = 0; //max file size
char *arr[1024]; //arr to store arguments of tar command
int count=3; // to keep track fo arguments for tar command
int ext_count = 1; //extension count

struct tm st = {            //struct to store day months and years of start time and end time
        .tm_mday = 0,      //day
        .tm_mon = 0,        //month
        .tm_year = 0      //years
    };
    struct tm et = {
        .tm_mday = 0,   
        .tm_mon = 0,       
        .tm_year = 0      
    };

void send_file(int clientSocket)
{
        int fd= open("temp.tar.gz",O_RDONLY| O_RDWR);
        //printf("Tar file%d",fd);
        if(fd==-1)
        return;

    char buffer[1024];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        while (bytesRead>0){
        
            
            printf("HelloW\n");

            //ssize_t sentBytes = 
            //write(clientSocket, buffer, 1024);
            send(clientSocket, buffer, sizeof(buffer), 0);
            bytesRead= read(fd, buffer, sizeof(buffer));
            //if (sentBytes <= 0) {
                //perror("Error sending file");
                //fclose(file);
                //close(clientSocket);
                //break;
            //}
        }
        close(fd);
        //close(clientSocket);
    }


void add_date(const char *date, struct tm *date_tm) { //function to tokenize the sdate provided by the user and store it in the struct above
    char *token;
    token = strtok((char *)date, "-");

    if (token) {
        date_tm->tm_year = atoi(token) - 1900; //using atoi to convert to string and subtracting -1900 because the format accepted for year
        token = strtok(NULL, "-");
        if (token) {
            date_tm->tm_mon = atoi(token) - 1;
            token = strtok(NULL, "-");
            if (token) {
                date_tm->tm_mday = atoi(token);
            }
        }
    }
}

int search_file(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) { //function used in nftw system call to find the file and store it in a array
    if (typeflag == FTW_F && strcmp(fpath + ftwbuf->base, found_file) == 0) {
        fileflag=1;
        strncpy(found_file, fpath, sizeof(found_file) - 1);
        arr[count]= (char*)malloc(strlen(found_file)+1); //dynamically allocating the individual file path size
        strcpy(arr[count],found_file);
        count++;
        return 1; 
    }
    return 0;
}

int search_details(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) { //fucntion used with nftw to extract details like filepath, size and creation time
    if (typeflag == FTW_F && strcmp(fpath + ftwbuf->base, found_file) == 0) {
        strncpy(found_file, fpath, sizeof(found_file) - 1);
        snprintf(response, 1024, "%s\n %ld\n %s\n", fpath, sb->st_size,ctime(&sb->st_mtime)); //To modify the response we send to the client
        printf("%s\n",fpath);
        printf("%ld\n",sb->st_size);
        printf("%s\n",ctime(&sb->st_mtime));
        fileflag=1;
        return 1;
    }
    return 0;
}

int search_size(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {// function used with nftw to search files based on the client specified the size range
    if (typeflag == FTW_F &&  ftwbuf->level <=1) {
        if (sb->st_size >= min_size && sb->st_size <= max_size) { //comparing with the sizes provided by the user and selecting the ones we need
            strncpy(found_file, fpath, sizeof(found_file) - 1);
            printf("%s\n",found_file);
            arr[count]= (char*)malloc(strlen(found_file)+1);
            strcpy(arr[count],found_file);
            printf("%s\n",arr[count]);
            ++count;
        }
    }
    return 0;
}

int search_ext(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) { //function used with nftw to search files with specific extensions
    if (typeflag == FTW_F &&  ftwbuf->level <=1) {
        const char *file_extension_with_dot = strrchr(fpath, '.');
         const char *file_extension = file_extension_with_dot + 1;
        if (file_extension != NULL && strcmp(file_extension, arguments[ext_count]) == 0) { //each extension is searched using string compare on each argument passed by the client
            strncpy(found_file, fpath, sizeof(found_file) - 1);
            arr[count]= (char*)malloc(strlen(found_file)+1);
            strcpy(arr[count],found_file);
            printf("%s\n",arr[count]);
            fileflag=1;
            ++count;
        }
    }
    return 0;
}

int search_date(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    time_t start_time = mktime(&st);            //converting it to time_t type so that its easy to compare
    time_t end_time = mktime(&et); 
    if (typeflag == FTW_F) {
        if (sb->st_mtime >= start_time && sb->st_mtime <= end_time && ftwbuf->level <=2) { //keep only the files which were created between specified time period
            strncpy(found_file, fpath, sizeof(found_file) - 1);
            arr[count]= (char*)malloc(strlen(found_file)+1);
            strcpy(arr[count],found_file);
            fileflag=1;
            ++count;
        }
    }
    return 0;
}

void process_arguments(int num_arguments, char *arguments[], char *response) { //fucntion to get through each arguments of the command
    char *directory = ".";  //pwd directory for every nftw system call

    if(strcmp(arguments[0],"fgets")==0)         //check if the command is fgets
        {
            arr[0]="tar";                          //set the first three to tar -cf temp.tar.gz so that command can be framed
            arr[1]="-cf";
            arr[2]="temp.tar.gz";
            for(int i=1;i<num_arguments ;++i)     //run the loop over each argument to find those files
            {
                char *filename = arguments[i];
                strncpy(found_file, filename, sizeof(found_file) - 1);
                nftw(directory, search_file, 10, FTW_PHYS);
            }

            if(count> num_arguments + 1) //once the count is greater than number arguments +1 it should run the tar command using fork system call
                 {
                     int p = fork();
                     if(p==0)
                     {
                        execvp("tar",arr);
                     }
                 }
            strcpy(response, "Command was implemented");
                  
        }
    else if(strcmp(arguments[0],"tarfgetz")==0){ //check if it is tarfgetz command
            min_size = atoi(arguments[1]);  //using atoi to convert to integer
            max_size = atoi(arguments[2]);
            arr[0]="tar";
            arr[1]="-cf";
            arr[2]="temp.tar.gz";
            
            nftw(directory, search_size, 10, FTW_PHYS); //find all the files between the specified size range

            if(count>num_arguments + 1)
                 {
                     int p = fork();
                     if(p==0)
                     {
                        execvp("tar",arr);                 //apply the tar command on found files
                     }
                 }
            strcpy(response, "Command was implemented");  //send response to client
    }
        else if(strcmp(arguments[0],"filesrch")==0){  
            char *filename = arguments[1];
            strncpy(found_file, filename, sizeof(found_file) - 1);
            nftw(directory, search_details, 10, FTW_PHYS); //simply finding the files and sending details as response for filesrch
            if(fileflag==0){
                printf("File not Found\n");
                strcpy(response, "File not Found\n");  //print file not found in case the fileflag is 0
            }
    }
        else if (strcmp(arguments[0],"targzf")==0)
        {
            char *filename = arguments[1];
            strncpy(found_file, filename, sizeof(found_file) - 1);
            arr[0]="tar";
            arr[1]="-cf";
            arr[2]="temp.tar.gz";
            for(int i = 0; i<num_arguments-1; i++){ //Running the for loop till num_arguments-1 to go through every file extention
                nftw(directory, search_ext, 10, FTW_PHYS);
                ext_count++;
            }
            ext_count = 0;
            if(count>num_arguments + 1)
                 {
                     int p = fork();
                     if(p==0)
                     {
                        execvp("tar",arr);        //applying tar if files were found
                     }
                 }
            if(fileflag==0){  //otherwise print appropriate messages
                printf("File not Found\n"); 
                strcpy(response, "File not Found\n");  
            }else{
                strcpy(response, "Command was implemented");
            }
        }
        else if (strcmp(arguments[0],"getdirf")==0)
        {
            add_date(arguments[1], &st);       //extrating dates from the arguments of the client command and storing them to strcut variable
            add_date(arguments[2], &et);
            time_t start_time = mktime(&st);  //convert them to time_t type
            time_t end_time = mktime(&et);
            arr[0]="tar";
            arr[1]="-cf";
            arr[2]="temp.tar.gz";
            nftw(directory, search_date, 10, FTW_PHYS); //look for files which were created in that time period and then store them in arr
            if(count>3)
                 {
                     int p = fork();
                     if(p==0)
                     {
                        execvp("tar",arr);   //tar the found files
                     }
                }
            if(fileflag==0){               //return appropriate messages as response
                printf("File not Found\n");
                strcpy(response, "File not Found\n");
            }else{
                strcpy(response, "Command was implemented");
            }

        }
        else
        {
            strcpy(response, "Invalid command");  //incase no command matches the command is considered wrong
        }

    }
    
    void process_client(int client_socket) { //function to recieve messages from cliend and send back responses

    while (1) {
        char command[1024];
        ssize_t bytes_received = recv(client_socket, command, sizeof(command), 0);         //recieve the command
        command[bytes_received] = '\0';                //adding a null at the end so that its easy to tokenize

        for (int i = 0; i < bytes_received; i++) {
            if (command[i] == '\n') {               //remove the newline character so that it wont cause error in nftw system call
                command[i] = '\0';                //repalce it with null
                break;  
            }
        }
        int num_arguments = 0;  //to store the number of arguments

        char *token = strtok(command, " ");         //to token using the " "
        while (token != NULL && num_arguments < 10) {
            for (int i = 0; i < strlen(token); i++) {
                if (token[i] == '\n') {
                    token[i] = '\0';
                    break; 
                }
            }
            arguments[num_arguments] = token; //store the individual arguments in arguments array
            num_arguments++;
            token = strtok(NULL, " ");
        }
        if(strstr(command,"quit") != NULL){  //Send quit message to client
            strcpy(response, "quit");
        }
        else{
            process_arguments(num_arguments, arguments, response); //call the process_argument function
        }
        send(client_socket, response, strlen(response), 0); //give client a response to their command
       
    }

    close(client_socket); //close the client socket if it exits the loop
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(49151);  //Use the port for server 
    server_addr.sin_addr.s_addr = INADDR_ANY;  //use any address to accept the request from

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)); //bind to listen to the incoming connections
    listen(server_socket, 5);
    printf("Server listening...\n");

    while (1) {
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len); //accept connection from a client
        serv_count++;                           //increment the variable with each client
        if(serv_count>12 && serv_count%2==0) //if the count is greater then 12 then all even numbered connection will go to mirror
        {  
            char* response = "Mirror";   
                    char command[1024];
                    send(client_socket, response, strlen(response), 0); //send mirror as response to client so that it proceeds to connect with mirror
                    printf("Too many clients exiting!!!\n");
                    close(client_socket);
        }

        else if(serv_count>6 && serv_count<13){ //if the count is between 6 and 13 then again all connections must go to mirror
                    char* response = "Mirror";
                    char command[1024];
                    send(client_socket, response, strlen(response), 0);
                    printf("Too many clients exiting!!!\n");
        }
        else //or else they should all go to server
        {
        pid_t pid = fork();
            if (pid == 0) {
                
            printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); 
            printf("%d\n",serv_count);
                // Child process handles client request
                process_client(client_socket); //call the process_client function

                int fd = open("temp.tar.gz",O_RDONLY);
                if(fd==-1)
                {
                    perror("File not found");
                }

                struct stat fileStat;
                fstat(fd, &fileStat);
                if (send(client_socket, &fileStat.st_size, sizeof(off_t), 0) == -1) {
                        perror("Error sending file size");
                        close(fd);
                        break;
                    }

                char buffer[1024];
                ssize_t bytesRead;
                    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
                        if (send(client_socket, buffer, bytesRead, 0) == -1) {
                            perror("Error sending file");
                            close(fd);
                            break;
                        }
                    }
                    close(fd);

                exit(EXIT_SUCCESS);
            } else if (pid > 0) { //parent process will continue in the loop 
                send_file(client_socket);//call the send_file to send the created tar file to the client
                close(client_socket);
            } else {
                close(client_socket);
            }
       }
    }
    close(server_socket);
    return 0;
}
