# sockets challenge
A simple TCP server and client. 

The server will send the contents of file.txt to the client on connection.
Server will use partial send to ensure all the data is sent to the client. File will be bigger than one packet.
getaddrinfo will be used by the client to look up the domain, specified in the hosts file. 
