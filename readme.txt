# Simple Sockets Examples
A simple TCP server and client. 

The Server process will bind to port 5050 (unless a port is passed as arg). The server sends a hello world message to the client on connection.
The Server is configured to handle multiple requests, forking child processes. There is a backlog limit of 10 requests on the socket.

=======
Level I
Client will connect to the server the server, the server will send the contents of file.txt back to the client.
Server will use partial send to ensure all the data is sent to the client. File will be bigger than one packet.
getaddrinfo will be used by the client to look up the domain, specified in the hosts file. 
