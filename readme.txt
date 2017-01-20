# Simple Sockets Examples
A simple TCP server and client. 

The Server process will bind to port 5050 (unless a port is passed as arg). The server sends a hello world message to the client on connection.
The Server is configured to handle multiple requests, forking child processes. There is a backlog limit of 10 requests on the socket.

