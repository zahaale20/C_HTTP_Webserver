#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>    
#include <sys/stat.h>

// requirement 4: error responses
void send_error_response(FILE *network, const char *status, const char *message) {
   fprintf(network, "%s\r\n", status);
   fprintf(network, "Content-Type: text/html\r\n\r\n");
   fprintf(network, "<html><body><h1>%s</h1></body></html>\r\n", message);
}

// requirement 3: support for head and get requests
void handle_request(int nfd){
   
   // open file descriptor
   FILE *network = fdopen(nfd, "r+");
   char *line = NULL;
   size_t size = 0;
   ssize_t num;

   // error handling
   if (network == NULL){
      perror("Error: failed to open file descriptor.");
      close(nfd);
      return;
   }
   
   // loop to handle multiple requests in a single connection
   while (1) {  
      // read the request line from client
      num = getline(&line, &size, network);
      //check for read errors
      if (num <= 0) {
         if (feof(network)) {  // check for end of file (connection closed by client)
            break;
         }
         // send a 400 Bad Request error in case of a read error
         send_error_response(network, "HTTP/1.0 400 Bad Request", "400 Bad Request");
         continue;  // continue to the next request instead of returning
      }

      // parse the request line
      char *method = strtok(line, " ");
      char *path = strtok(NULL, " ");
      if (!method || !path) { // check if successfully parsed
         send_error_response(network, "HTTP/1.0 400 Bad Request", "400 Bad Request");
         free(line);
         fclose(network);
         return;
      }

      // remove the leading '/' from the path
      if (path[0] == '/') path++;

      // check if requested file exists
      struct stat file_stat;
      int file_exists = stat(path, &file_stat);
      if (file_exists < 0) {
         send_error_response(network, "HTTP/1.0 404 Not Found", "404 Not Found");
      } else {
         // open requested file
         FILE *file = fopen(path, "r");
         if (file == NULL) { // handle file access permission error
               send_error_response(network, "HTTP/1.0 403 Permission Denied", "403 Permission Denied");
         } else { // else ok... send a 200 ok response header
            fprintf(network, "HTTP/1.0 200 OK\r\n");
            fprintf(network, "Content-Type: text/html\r\n");
            fprintf(network, "Content-Length: %ld\r\n\r\n", file_stat.st_size);

            // if get... send file content
            if (strcmp(method, "GET") == 0) {
               char buffer[1024];
               while (fgets(buffer, sizeof(buffer), file)) {
                  fprintf(network, "%s", buffer);
               }
            }
            fclose(file); // close file
         }
      }
      // free line buffer/reset
      free(line);
      line = NULL;
      size = 0;
   }
   // free memory and close the network stream
    free(line);
    fclose(network);
}

// requirement 2: handling multiple requests with child processes
void run_service(int fd){
  while (1){

     int nfd = accept_connection(fd);
     if (nfd == -1) {
        perror("accept");
        continue;
     }

     printf("Connection established...\n");
     pid_t pid = fork();
    
     if (pid == -1) { // error handling
        perror("fork");
        close(nfd);
     } else if (pid == 0) { // child process
        close(fd);
        handle_request(nfd);
        close(nfd);
        printf("Connection closed...\n");
        exit(0);
     } else { // parent process
        close(nfd);
     }
  }
}


// requirement 1: command-line argument for port
int main(int argc, char *argv[]){
  if (argc != 2) {
     fprintf(stderr, "Usage: %s <port>\n", argv[0]);
     exit(1);
  }

  int port = atoi(argv[1]); // parse port from command-line argument
  if (port < 1024 || port > 65535) {
     fprintf(stderr, "Port number must be between 1024 and 65535\n");
     exit(1);
  }

 int fd = create_service(port);

 if (fd == -1){
    perror(0);
    exit(1);
 }

 printf("Listening on port: %d...\n", port);
 run_service(fd);
 close(fd);

 return 0;
}
