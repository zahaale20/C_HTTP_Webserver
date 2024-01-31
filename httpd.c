// Include necessary headers for networking and basic functionalities.
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>    
#include <sys/stat.h>

// Signal handler for cleaning up child processes (zombies).
void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// Function to send HTTP error responses.
void send_error_response(FILE *network, const char *status, const char *message) {
    fprintf(network, "%s\r\n", status); // Send HTTP status line.
    fprintf(network, "Content-Type: text/html\r\n\r\n"); // Send HTTP header.
    fprintf(network, "<html><body><h1>%s</h1></body></html>\r\n", message); // Send HTML body with error message.
}

// Function to handle HTTP requests.
void handle_request(int nfd) {
    // Open file descriptor as a FILE stream.
    FILE *network = fdopen(nfd, "r+");
    char *line = NULL;
    size_t size = 0;
    ssize_t num;

    // Error handling for file descriptor opening.
    if (network == NULL) {
        perror("Error: failed to open file descriptor.");
        close(nfd);
        return;
    }

    // Loop to handle multiple requests in a single connection.
    while (1) {
        // Read the request line from client.
        num = getline(&line, &size, network);
        if (num <= 0) {
            if (feof(network)) {
                break; // Exit loop if connection is closed by client.
            }
            // Send a 400 Bad Request error in case of a read error.
            send_error_response(network, "HTTP/1.0 400 Bad Request", "400 Bad Request");
            continue; // Continue to next request.
        }

        // Parse the request line (method and path).
        char *method = strtok(line, " ");
        char *path = strtok(NULL, " ");
        if (!method || !path) {
            send_error_response(network, "HTTP/1.0 400 Bad Request", "400 Bad Request");
            free(line);
            fclose(network);
            return;
        }

        // Handle file path and check file existence.
        path++; // Skip the leading '/' in the path.
        struct stat file_stat;
        if (stat(path, &file_stat) < 0) {
            send_error_response(network, "HTTP/1.0 404 Not Found", "404 Not Found");
        } else {
            // Process valid file requests.
            FILE *file = fopen(path, "r");
            if (!file) {
                send_error_response(network, "HTTP/1.0 403 Permission Denied", "403 Permission Denied");
            } else {
                // Send HTTP response headers for successful request.
                fprintf(network, "HTTP/1.0 200 OK\r\n");
                fprintf(network, "Content-Type: text/html\r\n");
                fprintf(network, "Content-Length: %ld\r\n\r\n", file_stat.st_size);

                // Send file content for GET requests.
                if (strcmp(method, "GET") == 0) {
                    char buffer[1024];
                    while (fgets(buffer, sizeof(buffer), file)) {
                        fprintf(network, "%s", buffer);
                    }
                }
                fclose(file); // Close the file.
            }
        }
        // Reset line buffer for next request.
        free(line);
        line = NULL;
        size = 0;
    }
    // Free memory and close network stream.
    free(line);
    fclose(network);
}
// Function to handle multiple requests using child processes.
void run_service(int fd) {
    while (1) {
        // Accept a new connection.
        int nfd = accept_connection(fd);
        if (nfd == -1) {
            perror("accept");
            continue; // Continue to next iteration if accept fails.
        }

        printf("Connection established...\n");
        pid_t pid = fork(); // Create a new process.

        if (pid == -1) {
            // Error handling for fork failure.
            perror("fork");
            close(nfd);
        } else if (pid == 0) {
            // Child process: handle request and close connection.
            close(fd);
            handle_request(nfd);
            close(nfd);
            printf("Connection closed...\n");
            exit(0); // Terminate child process.
        } else {
            // Parent process: close new file descriptor and wait for next connection.
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
