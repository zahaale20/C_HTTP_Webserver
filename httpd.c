#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 6969

void handle_request(int nfd){
   FILE *network = fdopen(nfd, "r+");
   char *line = NULL;
   size_t size;
   ssize_t num;

   if (network == NULL){
      perror("fdopen");
      close(nfd);
      return;
   }

   while ((num = getline(&line, &size, network)) >= 0){
      // printf("Received from client: %s", line);
      write(nfd, line, num);
   }

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
