// Define GNU source to enable additional features in glibc.
#define _GNU_SOURCE

// Include necessary standard libraries for networking and basic functionalities.
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#define USAGE_STRING "usage: %s <server address>\n"

// Function to send requests to the server and receive responses.
void send_request(int fd) {
    char *line = NULL;
    size_t size;
    ssize_t num;

    // Read lines from standard input and send them to the server.
    while ((num = getline(&line, &size, stdin)) >= 0) {
        write(fd, line, num); // Send the line to the server.

        char buffer[1024];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // Read response from the server.
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received from server: %s\n", buffer); // Display server response.
        } else {
            break; // Break loop if no data is read.
        }
        printf("Enter input: ");
    }
    free(line); // Free allocated memory.
}

// Function to connect to the server using given host entry and port number.
int connect_to_server(struct hostent *host_entry, int port) {
    int fd;
    struct sockaddr_in their_addr; // Server address structure.

    // Create a socket for the client.
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1; // Return error if socket creation fails.
    }

    their_addr.sin_family = AF_INET; // Set address family.
    their_addr.sin_port = htons(port); // Set port number.
    their_addr.sin_addr = *((struct in_addr *)host_entry->h_addr); // Set server IP address.

    // Establish a connection to the server.
    if (connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        close(fd); // Close socket if connection fails.
        perror(0); // Print error message.
        return -1; // Return error.
    }

    return fd; // Return the file descriptor for the socket.
}

// Function to retrieve host information.
struct hostent *gethost(char *hostname) {
    struct hostent *he;

    if ((he = gethostbyname(hostname)) == NULL) {
        herror(hostname); // Print error if host lookup fails.
    }

    return he; // Return the hostent structure.
}

// Main function: entry point of the client program.
int main(int argc, char *argv[]) {
    // Check for correct usage.
    if (argc != 3) {
        fprintf(stderr, USAGE_STRING, argv[0]);
        exit(EXIT_FAILURE); // Exit if usage is incorrect.
    }

    // Resolve host name.
    struct hostent *host_entry = gethostbyname(argv[1]);

    // Convert port number from string to integer.
    int port = atoi(argv[2]);
    // Validate port number range.
    if (port < 1024 || port > 65535) {
        fprintf(stderr, "Port number must be between 1024 and 65535\n");
        exit(EXIT_FAILURE); // Exit if port number is not valid.
    }

    // Proceed if host is resolved.
    if (host_entry) {
        int fd = connect_to_server(host_entry, port); // Connect to the server.
        if (fd != -1) {
            printf("\nEnter input: ");
            send_request(fd); // Send requests and process responses.
            close(fd); // Close the socket.
        }
    }

    return 0; // End of program.
}
