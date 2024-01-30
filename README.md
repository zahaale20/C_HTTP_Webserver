# C_HTTP_Webserver

### Description:
The "C_Simple_HTTP_Webserver" GitHub repository contains an implementation of a basic web server in C, capable of handling HTTP requests and supporting a subset of the Hypertext Transfer Protocol (HTTP). This project highlights the ability to handle multiple client requests using child processes and introduces the concept of CGI-like program invocation.

## Functional Overview:

### Web Server Implementation (httpd):

The server listens for connections on a specified port, provided as a command-line argument.
Handles HTTP HEAD and GET requests to fetch and transmit file contents.
Implements error handling for various HTTP response codes (400, 403, 404, 500, 501).

### Process Management for Concurrent Requests:
Spawns child processes to handle each incoming request.
Ensures proper resource management, including closing unneeded file descriptors and waiting for child processes.

### Request and Response Handling:
Parses HTTP requests and extracts file names.
Generates appropriate HTTP headers and content, including content type and length.
Supports text/html content type for all files.

### Error Handling:
Responds with relevant HTTP error codes for erroneous requests.
Customizes HTML messages for different error types.

### Resource Management:
Carefully manages server resources to ensure continuous operation without memory leaks.

### CGI-like Support:
Implements CGI-like functionality for executing specified server-side programs and sending their output back to the client.
Validates programs to be executed within a specific directory (cgi-like) and handles argument passing in the request URL.
Manages execution output, including temporary file creation, content size calculation, and cleanup.

### Repository Contents:
Source code for the httpd simple web server.
Example CGI-like scripts for server-side execution.
Documentation on server setup, usage, and implementation details.
Makefile for easy compilation and setup.
Additional resources on HTTP protocol handling and server-side scri
