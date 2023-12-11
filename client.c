#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#define MIN_ARGS 3
#define MAX_ARGS 3
#define SERVER_ARG_IDX 1

#define USAGE_STRING "usage: %s <server address>\n"

void send_request(int fd){
   char *line = NULL;
   size_t size;
   ssize_t num;

   while ((num = getline(&line, &size, stdin)) >= 0){
      write(fd, line, num); // send output to server
      char buffer[1024];
      ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // read output from server
      if (bytes_read > 0){
         buffer[bytes_read] = '\0';
         printf("Received from server: %s\n", buffer);
      } else {
         break;
      }
      printf("Enter input: ");
   }
   free(line);
}

int connect_to_server(struct hostent *host_entry, int port){
   int fd;
   struct sockaddr_in their_addr;

   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
      return -1;
   }

   their_addr.sin_family = AF_INET;
   their_addr.sin_port = htons(port);
   their_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);

   if (connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1){
         close(fd);
         perror(0);
         return -1;
   }

   return fd;
}

struct hostent *gethost(char *hostname){
   struct hostent *he;

   if ((he = gethostbyname(hostname)) == NULL){
      herror(hostname);
   }

   return he;
}

int main(int argc, char *argv[]){
   if (argc < MIN_ARGS || argc > MAX_ARGS){
      fprintf(stderr, USAGE_STRING, argv[0]);
      exit(EXIT_FAILURE);
   }

   struct hostent *host_entry = gethostbyname(argv[SERVER_ARG_IDX]);

   int port = atoi(argv[2]);
   if (port < 1024 || port > 65535) {
       fprintf(stderr, "Port number must be between 1024 and 65535\n");
       exit(EXIT_FAILURE);
   }

   if (host_entry){
      int fd = connect_to_server(host_entry, port);
      if (fd != -1){
         printf("\nEnter input: ");
         send_request(fd);
         close(fd);
      }
   }

   return 0;
}
